/*  vdiimpl.c 1.126 2017/10/23
    Copyright 2003-2017 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	VDI Implementation for TestDB

    Description:
	This file implements Nlview's VDI (virtual data-base interface)
	for the static C-code compiled test data-base (tdb) located in
	data*.c.  This file is almost identical to
	vdiimpl.cpp - but implemented in C.
    ===========================================================================
*/

#include "vdiimpl.h"
#include "vdi.h"
#include "data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* --------------------------------------------------------------------
 * make shorter Vdi types
 * --------------------------------------------------------------------
 */
typedef struct VdiObject  VdiObj;
typedef struct VdiDB      VdiDB;
typedef struct VdiIter    VdiIter;
typedef struct VdiCiter   VdiCiter;
typedef const struct VdiHObject VdiHObj;
typedef struct VdiHAttr   VdiHAttr;
typedef struct VdiHHi     VdiHHi;

#define PINCOUNT(inst) ((inst)->pinList ? zListLength((inst)->pinList) : 0)
#define PIN(inst,idx)  (inst)->pinList[idx]

#define MAX_BUF 64			/* max.number of additional attrs */
 
#ifndef NDEBUG
#define PARANOID
#endif

#ifdef PARANOID
#define CHECK_NET_NBUN_INST(vdiMod, vdiObj, Type, List) \
{ \
    int j; \
    Type** list = ((Module*)vdiMod)->List; \
    if (list) {	\
	for(j=0; j<zListLength(list); j++) \
	    if ((Type*)vdiObj == list[j]) break; /* pointer compare is OK */ \
	assert(j<zListLength(list)); \
    } \
}(void)NULL	/* allow to append ';' character in code */
#define CHECK_PORT_PBUS(vdiMod, vdiObj, Type, List) \
{ \
    int j; \
    Type* list = ((Module*)vdiMod)->c.List; \
    if (list) {	\
	for(j=0; j<zListLength(list); j++) \
	    if ((Type*)vdiObj == list+j) break; /* pointer compare is OK */ \
	assert(j<zListLength(list)); \
    } \
}(void)NULL	/* allow to append ';' character in code */
#else
#define CHECK_NET_NBUN_INST(vdiMod, vdiObj, Type, List) /* do nothing */ \
(void)vdiMod
#define CHECK_PORT_PBUS(vdiMod, vdiObj, Type, List)     /* do nothing */ \
(void)vdiMod
#endif /* PARANOID */

#ifdef _MSC_VER
    /* 'sprintf': This function or variable may be unsafe (_MSC_VER >= 1400) */
#   pragma warning(disable:4996)
    /* conversion from 'double ' to 'float ', possible loss of data */
#   pragma warning(disable:4244)
#endif

/* ============================================================================
 * assert() macro - system independent, prevent gcc from referring libgcc.a
 * ============================================================================
 */
/*LCOV_EXCL_START*/
static void fatalError(const char* file, int line, const char* expr) {
    fprintf(stderr, "%s:%d - %s\n", file, line, expr);
    fflush(stderr);
    abort();
}
/*LCOV_EXCL_STOP*/
#ifdef __STDC__
#define assert(e) ((e) ? (void)0 : fatalError(__FILE__, __LINE__, #e))
#else   /* PCC */
#define assert(e) ((e) ? (void)0 : fatalError(__FILE__, __LINE__, "e"))
#endif



/* --------------------------------------------------------------------
 * zSearchAttributeValue - search for attribute by name in given list
 * --------------------------------------------------------------------
 *
 * Arguments:
 *    attrList : DB object's attrList
 *	  name : name of attribute (case sensitive)
 *      return : pointer to value string or NULL
 * --------------------------------------------------------------------
 */
static const char* zSearchAttributeValue(DB* db, const char** attrList,
	const char* name)
{
    int i;
    size_t len = strlen(name);
    int cnt = attrList ? zListLength(attrList) : 0;
    (void)db;	 /* avoid compiler warning */
    for (i=0; i<cnt; i++) {
        const char* attr = attrList[i];
        if( strncmp(attr,name,len) == 0 && attr[len] == '=' ) {
            return attr+len+1;
        }
    }
    return 0;
}

/* --------------------------------------------------------------------
 * IsDigit - same as isdigit, but independent from "locale" nonsense...
 * IsSpace - same as isspace, but independent from "locale" nonsense...
 * Strtod  - same as strtod,  but independent from "locale" nonsense...
 * --------------------------------------------------------------------
 */
#define	_N	0x04	/* isdigit */
#define	_S	0x08	/* isspace */
#define LOW(ch) ((ch) | 0x20)

static const unsigned char ctype_tab [128] = {
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	_S,	_S,	_S,	_S,	_S,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	_S,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	_N,	_N,	_N,	_N,	_N,	_N,	_N,	_N,
	_N,	_N,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0
};

static int IsDigit(const unsigned char c) {
    return !(c & ~0x7F) && (ctype_tab[c] & (_N));
}
static int IsSpace(const unsigned char c) {
    return !(c & ~0x7F) && (ctype_tab[c] & (_S));
}
static double Strtod(const char* nptr, char** endptr)
{
    register const char* s = nptr;
    char		 sign = 0;
    register double	 val = 0;
    register int	 has_digits = 0;

    while (IsSpace(*s)) s++;	/* eat initial spaces */

    if (*s == '-') sign = *s++;		/* eat sign character */
    else if (*s == '+')    s++;

    while (IsDigit(*s)) {
	val *= 10.0;
	val += *s++ - '0';
	has_digits = 1;
    }
    if (*s == '.' && (has_digits || IsDigit(s[1]))) {
	double fract = 0;
	double denom = 1.0;
	s++;
	while (IsDigit(*s)) {
	    fract *= 10;
	    fract += *s++ - '0';
	    denom *= 10;
	    has_digits = 1;
	}
	val += fract / denom;
    }
    if (!has_digits) {			/* no digits so far.. this is nothing */
	s = nptr;
	goto noexp;
    }

    if (LOW(*s) == 'e') {
	double exp;
	const char* chk = s+1;		/* first check for valid exponent */
	if (*chk == '-' || *chk == '+') chk++;
	if (!IsDigit(*chk)) goto noexp;

	exp = strtol(++s, endptr, 10);
	if (val != 0.0) val *= pow(10.0, exp);
    } else {
      noexp:
	if (endptr) *endptr = (char*)s;
    }
    return sign ? -val : val;
}

/* --------------------------------------------------------------------
 * getValue - convert spice-like value string to floating value
 * --------------------------------------------------------------------
 *
 * Arguments:
 *         str : a string representation of a value, e.g. "1k"
 *      return : the 'double'-value of str's meaning
 * --------------------------------------------------------------------
 */
static float getValue(const char* str) {
    char* unit;
    double u;
    double val = Strtod(str, &unit);

    /* eat spaces between number and unit */
    while(IsSpace(*unit)) unit++;

    switch(unit[0]) {
	    case 'T': case 't': u=1e12;  break;	/* tera */
	    case 'G': case 'g': u=1e9;   break; /* giga */
	    case 'K': case 'k': u=1e3;   break;	/* kilo */
	    case 'U': case 'u': u=1e-6;  break;	/* micro */
	    case 'N': case 'n': u=1e-9;  break; /* nano */
	    case 'P': case 'p': u=1e-12; break; /* pico */
	    case 'F': case 'f': u=1e-15; break; /* femto */
	    case 'A': case 'a': u=1e-18; break; /* ato */
	    case 'M': case 'm':			/* mega or m (milli) */
		if (LOW(unit[1]) == 'e' && LOW(unit[2]) == 'g') { u=1e6;     }
		else						{ u=1e-3;    }
		break;
	    case '\0':
		return val;
	    default:
		return 0.0;
    }
    return val * u;
}

/* --------------------------------------------------------------------
 * vdiVersion
 * implVersion
 * --------------------------------------------------------------------
 */
static int vdiVersion(VdiDB* vdidb) {
    (void)vdidb;
    return VdiVersion;
}

static const char* implVersion(VdiDB* vdidb) {
    (void)vdidb; /* avoid compiler warning */
    return "1.126 TDB [vdiimpl.c]";
}

/* --------------------------------------------------------------------
 * modFind, instFind, portFind, pbusFind, netFind, nbunFind
 *
 * Search for an object by name within a given Module (except
 * "modFind" searches within the DataBase root).  Each object
 * has a uniq name (for the Module itself we ignore the 2nd
 * name "vname").
 * --------------------------------------------------------------------
 */
#define FIND(TYPE,LIST,NAME,MEMBER) \
  { \
    int    i; \
    TYPE** lst = LIST; \
    int    count = lst ? zListLength(lst) : 0; \
    for(i=0; i < count; i++) { \
	if (strcmp(NAME, lst[i]->MEMBER) == 0) return (VdiObj*)lst[i]; \
    } \
    return NULL; \
  }

static VdiObj* modFind(VdiDB* vdiDB, const char* name, const char* vname) {
    DB*     db = (DB*)vdiDB;
    (void)vname; /* avoid compiler warning */
    FIND(Module,db->moduleList,name,c.name)
}

static VdiObj* instFind(VdiDB* vdidb, const char* name, VdiObj* vdiMod) {
    Module* mod = (Module*)vdiMod;
    (void)vdidb; /* avoid compiler warning */
    FIND(Inst,mod->instList,name,name)
}

static VdiObj* portFind(VdiDB* vdidb, const char* name, VdiObj* vdiMod) {
    Module* mod = (Module*)vdiMod;
    int     portNo;
    (void)vdidb; /* avoid compiler warning */
    if (!mod->c.portList) return NULL;

    for(portNo=0; portNo < zListLength(mod->c.portList); portNo++) {
	Port* port = mod->c.portList + portNo;
	if(port->busMember == -1 && (strcmp(name, port->name) == 0)) {
            /* only Ports that are not a PortBus member are 'visible' */
	    return (VdiObj*)port;
	}
    }
    return NULL;
}

static VdiObj* pbusFind(VdiDB* vdidb, const char* name, VdiObj* vdiMod) {
    Module* mod = (Module*)vdiMod;
    int     i, count;
    PortBus* pbusList = mod->c.portBusList;
    (void)vdidb; /* avoid compiler warning */

    count = pbusList ? zListLength(pbusList) : 0;

    for (i=0; i<count; i++) {
        if(strcmp(pbusList[i].name, name)==0) return (VdiObj*)(pbusList+i);
    }
    return NULL;
}

static VdiObj* netFind(VdiDB* vdidb, const char* name, VdiObj* vdiMod) {
    Module* mod = (Module*)vdiMod;
    (void)vdidb; /* avoid compiler warning */
    FIND(Net,mod->netList,name,name)
}

static VdiObj* nbunFind(VdiDB* vdidb, const char* name, VdiObj* vdiMod) {
    Module* mod = (Module*)vdiMod;
    (void)vdidb; /* avoid compiler warning */
    FIND(NetBus,mod->netBusList,name,name)
}

/* ====================================================================
 * Iter, IterFuncs - data structs for object iterators
 *
 * A union of 7 different lists is used to implement 8 different iterator
 * types.  (the comments in the "union" below describe who-uses-what).
 *
 * The "IterFuncs" implements the "virtual" function vector for each
 * iterator type.  The term "virtual function" is borrowed from C++...
 *
 * We use a "union" instead of inheritance to implement this kind of
 * polymorphism (set of 8 iterator types with a common interface).
 * ====================================================================
 */
typedef struct IterFuncs IterFuncs;
typedef struct Iter	 Iter;
struct Iter {
    IterFuncs* funcs;			/* pointer to "virtual" functions */
    union {
	Module** modList;		/* ptr-list of Module  - modIter */
	Inst**   instList;		/* ptr-list of Inst    - instIter */
	Port*    portList;		/* list     of Port    - portIter */
	PortBus* pbusList;		/* list     of PortBus - pbusIter */
	Net**    netList;		/* ptr-list of Net     - netIter  */
	NetBus** nbusList;		/* ptr-list of NetBus  - nbunIter */
	Net**    subnetList;		/* ptr-list of Net     - subIter  */
    } u;
    int   idx;
};

struct IterFuncs {
    void    (*next)(      Iter* iter);
    VdiObj* (*obj) (const Iter* iter);
};

/* --------------------------------------------------------------------
 * imore, inext, iobj - the common iterator interface
 * --------------------------------------------------------------------
 */
static int imore(const VdiIter* vdiIter) {
    /*
     * All dynamic growing lists in the union "iter->u" have same list
     * head, so we can just use any of them to get the list length.
     * This enables us to implement "imore" only once for all iterator
     * types.
     */
    const Iter* it = (const Iter*)vdiIter;
    int cnt = it->u.modList ? zListLength(it->u.modList) : 0;
    return it->idx < cnt;
}

static void inext(VdiIter* vdiIter) {
    Iter* it = (Iter*)vdiIter;
    it->funcs->next(it);		/* call iterator's "virtual" function */
}

static VdiObj* iobj(const VdiIter* vdiIter) {
    const Iter* it = (const Iter*)vdiIter;
    return it->funcs->obj(it);		/* call iterator's "virtual" function */
}

/* --------------------------------------------------------------------
 *    anyIterNext - IterFuncs.next for most Iters (except for portIter)
 *    ptrIterObj -  IterFuncs.obj for all pointer-lists (except port/pbus)
 * --------------------------------------------------------------------
 */
static void anyIterNext(Iter* it) {
    assert(imore((VdiIter*)it));
    it->idx++;
}

static VdiObj* ptrIterObj(const Iter* it) {
    assert(imore((VdiIter*)it));
    /*
     * All pointer lists in the union "iter->u" (that are modList, instList,
     * netList, nbusList and subnetList) have same element size - sizeof(void*),
     * so we can just use any of them to get the pointer value.  We use this
     * function for modIter, instIter, netIter, nbunIter and subIter.
     */
    return (VdiObj*)(it->u.modList[it->idx]);
}


/* --------------------------------------------------------------------
 * modIter - create iterator
 * --------------------------------------------------------------------
 */
static IterFuncs modIterFuncs = { anyIterNext, ptrIterObj };

static VdiIter* modIter(VdiDB* vdiDB) {
    DB* db = (DB*)vdiDB;
    Iter* it = malloc(sizeof(Iter));
    it->funcs     = &modIterFuncs;
    it->u.modList = db->moduleList;
    it->idx       = 0;
    return (VdiIter*)it;
}

/* --------------------------------------------------------------------
 * instIter - create iterator
 * --------------------------------------------------------------------
 */
static VdiIter* instIter(VdiDB* vdidb, VdiObj* vdiMod) {
    Module* mod = (Module*)vdiMod;
    static IterFuncs instIterFuncs = { anyIterNext, ptrIterObj };
    Iter* it = malloc(sizeof(Iter));
    assert(mod->c.flags & CellFModule);
    it->funcs      = &instIterFuncs;
    it->u.instList = mod->instList;
    it->idx        = 0;
    (void)vdidb; /* avoid compiler warning */
    return (VdiIter*)it;
}

/* --------------------------------------------------------------------
 * portIter - create iterator
 *
 * Define my own "next" function, because the available mod->c.portList
 * (see portIter below) includes portBus-members, but the VDI portIter
 * must skip them (skipping is done in function portSkipBusmembers).
 *
 * Define my own "obj" function, because the available mod->c.portList
 * is not a pointer-list (so we cannot use the default obj function above).
 * --------------------------------------------------------------------
 */
static void portSkipBusmembers(Iter* it) {
    int cnt = it->u.portList ? zListLength(it->u.portList) : 0;
    while(it->idx < cnt) {
	if (it->u.portList[it->idx].busMember == -1) break;
	it->idx++;
    }
}

static void portIterNext(Iter* it) {
    assert(imore((VdiIter*)it));
    it->idx++;
    portSkipBusmembers(it);
}

static VdiObj* portIterObj(const Iter* it) {
    assert(imore((VdiIter*)it));
    return (VdiObj*)(it->u.portList+it->idx);
}

static VdiIter* portIter(VdiDB* vdidb, VdiObj* vdiMod) {
    Module* mod = (Module*)vdiMod;
    static IterFuncs portIterFuncs = { portIterNext, portIterObj };
    Iter* it = malloc(sizeof(Iter));
    assert(mod->c.flags & CellFModule);
    it->funcs      = &portIterFuncs;
    it->u.portList = mod->c.portList;
    it->idx        = 0;
    portSkipBusmembers(it);
    (void)vdidb; /* avoid compiler warning */
    return (VdiIter*)it;
}

/* --------------------------------------------------------------------
 * pbusIter - create iterator
 *
 * Define my own "obj" function, because the available mod->c.portBusList
 * is not a pointer-list (so we cannot use the default obj function above).
 * --------------------------------------------------------------------
 */
static VdiObj* pbusIterObj(const Iter* it) {
    assert(imore((VdiIter*)it));
    return (VdiObj*)(it->u.pbusList+it->idx);
}

static VdiIter* pbusIter(VdiDB* vdidb, VdiObj* vdiMod) {
    Module* mod = (Module*)vdiMod;
    static IterFuncs pbusIterFuncs = { anyIterNext, pbusIterObj };
    Iter* it = malloc(sizeof(Iter));
    assert(mod->c.flags & CellFModule);
    it->funcs      = &pbusIterFuncs;
    it->u.pbusList = mod->c.portBusList;
    it->idx        = 0;
    (void)vdidb; /* avoid compiler warning */
    return (VdiIter*)it;
}

/* --------------------------------------------------------------------
 * netIter - create iterator
 * --------------------------------------------------------------------
 */
static VdiIter* netIter(VdiDB* vdidb, VdiObj* vdiMod) {
    Module* mod = (Module*)vdiMod;
    static IterFuncs netIterFuncs = { anyIterNext, ptrIterObj };
    Iter* it = malloc(sizeof(Iter));
    assert(mod->c.flags & CellFModule);
    it->funcs     = &netIterFuncs;
    it->u.netList = mod->netList;
    it->idx       = 0;
    (void)vdidb; /* avoid compiler warning */
    return (VdiIter*)it;
}

/* --------------------------------------------------------------------
 * nbunIter - create iterator
 * --------------------------------------------------------------------
 */
static VdiIter* nbunIter(VdiDB* vdidb, VdiObj* vdiMod) {
    Module* mod = (Module*)vdiMod;
    static IterFuncs nbunIterFuncs = { anyIterNext, ptrIterObj };
    Iter* it = malloc(sizeof(Iter));
    assert(mod->c.flags & CellFModule);
    it->funcs      = &nbunIterFuncs;
    it->u.nbusList = mod->netBusList;
    it->idx        = 0;
    (void)vdidb; /* avoid compiler warning */
    return (VdiIter*)it;
}

/* --------------------------------------------------------------------
 * subIter - create iterator
 * --------------------------------------------------------------------
 */
static VdiIter* subIter(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiNetBundle) {
    NetBus* netBus = (NetBus*)vdiNetBundle;
    static IterFuncs subIterFuncs = { anyIterNext, ptrIterObj };
    Iter* it = malloc(sizeof(Iter));
    CHECK_NET_NBUN_INST(vdiMod,vdiNetBundle,NetBus,netBusList);
    it->funcs        = &subIterFuncs;
    it->u.subnetList = netBus->subnetList;
    it->idx          = 0;
    (void)vdidb;  /* avoid compiler warning */
    return (VdiIter*)it;
}

/* --------------------------------------------------------------------
 * freeIter - destroy iterator
 * --------------------------------------------------------------------
 */
static void freeIter(VdiDB* vdidb, VdiIter* vdiIter) {
    Iter* iter = (Iter*)vdiIter;
    (void)vdidb; /* avoid compiler warning */

    free(iter);
}


/* ====================================================================
 * moduleObjCount - return the object counts for the given module
 * ====================================================================
 */
static void moduleObjCount(VdiDB* vdidb, VdiObj* vdiMod, long cnt[5]) {
    Module* mod = (Module*)vdiMod;
    (void)vdidb; /* avoid compiler warning */
    assert(mod->c.flags & CellFModule);

    /* this will include all the subports! subtract them later */
    cnt[0] = mod->c.portList ?    zListLength(mod->c.portList)    : 0;
    cnt[1] = mod->c.portBusList ? zListLength(mod->c.portBusList) : 0;
    cnt[2] = mod->instList ?      zListLength(mod->instList)      : 0;
    cnt[3] = mod->netList ?       zListLength(mod->netList)       : 0;
    cnt[4] = mod->netBusList ?    zListLength(mod->netBusList)    : 0;

    if (mod->c.portBusList && (cnt[1] > 0)) {
	int subPortCnt = 0;
	int i;
	/* loop over all portBuses and add up all subPorts */
	PortBus* pb = mod->c.portBusList;  /* pointer to first portBusList */
	for( i = 0; i < cnt[1]; i++) {
	    assert(pb);
	    subPortCnt += abs(pb->first - pb->last) + 1;
	    pb++;                       /* proceed to next portBus in list */
	}
	cnt[0] -= subPortCnt;
	assert( cnt[0] >= 0 );
    }
}


/* --------------------------------------------------------------------
 * modName, instName, portName, pbusName, netName, nbunName
 *
 * Return object's name (same as used by the *Find functions above).
 * We don't need the Module's "vname", so we return a null-string for it.
 * The DBGCOPY macro is enabled for CONCEPT's debugging purposes.
 * --------------------------------------------------------------------
 */
#ifdef CONCEPT_DEBUGGING
/* copy all names into a static buffer and return the same static buffer in
 * each call - this verifies that Nlview's VDI actually picks up the string
 * data correctly (directly after each call to a *Name() function).
 */
static char namebuf1[1025];
static char namebuf2[1025];
static char namebuf3[1025];
static char namebuf4[1025];
#define DBGCOPY(p)  (sprintf(namebuf1,"%.1024s",(p)), namebuf1)
#define DBGCOPY2(p) (sprintf(namebuf2,"%.1024s",(p)), namebuf2)
#define DBGCOPY3(p) (sprintf(namebuf3,"%.1024s",(p)), namebuf3)
#define DBGCOPY4(p) (sprintf(namebuf4,"%.1024s",(p)), namebuf4)
#else
#define DBGCOPY(p)  p
#define DBGCOPY2(p) p
#define DBGCOPY3(p) p
#define DBGCOPY4(p) p
#endif

static const char* modName(VdiDB* vdidb, VdiObj* vdiMod, const char** vname) {
    Module* mod = (Module*)vdiMod;
    Cell* cell = &mod->c;
    (void)vdidb; /* avoid compiler warning */

    assert(cell->flags & CellFModule);
    assert(vname!=NULL);
    *vname=DBGCOPY2("");
    return DBGCOPY(cell->name);
}

static const char* instName(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiInst) {
    Inst* inst = (Inst*)vdiInst;
    (void)vdidb; /* avoid compiler warning */
    CHECK_NET_NBUN_INST(vdiMod,vdiInst,Inst,instList);
    return DBGCOPY(inst->name);
}

static const char* portName(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiPort) {
    Port* port = (Port*)vdiPort;
    (void)vdidb; /* avoid compiler warning */
    CHECK_PORT_PBUS(vdiMod,vdiPort,Port,portList);
    return DBGCOPY(port->name);
}

static const char* pbusName(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiPortBus) {
    PortBus* pbus = (PortBus*)vdiPortBus;
    (void)vdidb; /* avoid compiler warning */
    CHECK_PORT_PBUS(vdiMod,vdiPortBus,PortBus,portBusList);
    return DBGCOPY(pbus->name);
}

static const char* netName(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiNet) {
    Net* net = (Net*)vdiNet;
    (void)vdidb; /* avoid compiler warning */
    CHECK_NET_NBUN_INST(vdiMod,vdiNet,Net,netList);
    return DBGCOPY(net->name);
}

static const char* nbunName(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiNetBus) {
    NetBus* netBus = (NetBus*)vdiNetBus;
    (void)vdidb; /* avoid compiler warning */
    CHECK_NET_NBUN_INST(vdiMod,vdiNetBus,NetBus,netBusList);
    return DBGCOPY(netBus->name);
}


/* ====================================================================
 * Pass Attributes to Nlview
 * ====================================================================
 */
/* --------------------------------------------------------------------
 * addAttr - add all attributes from attrList to attr[]
 *
 * Arguments:
 *    attrList : DB object's attrList
 *       offset: skip first *offset entries in array attr,
 *               increased by number of attributes processed (result)
 *		 may be NULL.
 *       symdef: if not NULL then it returns a pointer to the value sub-string
 *               of @symbol attribute (if attribute is present) instead of
 *               adding it to the resulting list of VdiHAttr entries.
 */
static VdiHAttr* addAttr(const char** attrList, int* offset,const char** symdef)
{
    /**
     * Static buffers:
     * attr : NULL-terminated list of attributes to return.
     * names: temporarily holding attribute names
     *        each with a maximum length of MAX_LEN characters including
     *        terminating '\0'.
     */
    enum { MAX_LEN = 128 };		/* max.attr name string length */
    static VdiHAttr attr[MAX_BUF];	/* static buffer for return value */
    static char     names[MAX_BUF][MAX_LEN];
    const char*     value;
    int cnt = attrList ? zListLength(attrList) : 0;
    int i;
    int offsetBuf = 0;
    if (!offset) offset = &offsetBuf;

    for (i=0; i<cnt && *offset < MAX_BUF-1; i++) {
	const char* a = strchr(attrList[i],'=');
	if (a == NULL) {
	    sprintf(names[*offset], "%.*s", MAX_LEN, attrList[i]);
	    value = "";
	} else {
	    size_t len = a - attrList[i];
	    if (symdef && strncmp(attrList[i],"@symbol",len) == 0) {
		*symdef = a+1;
		continue;
	    }
	    if (len > MAX_LEN-1) len = MAX_LEN-1;
	    strncpy(names[*offset], attrList[i], len);
	    names[*offset][len] = '\0';
	    value = a+1;
	}
	attr[*offset].name   = names[*offset];
	attr[*offset].value  = value;
	attr[*offset].format = NULL;
	(*offset)++;
    }
    attr[*offset].name = NULL;          /* NULL-terminate the attr list */
    return attr;
}


#if 0
#define DBGCATTR(type, h, iname, pname) \
    printf("Cpinattr(topinfo=%s, path=%s pathSep=%c inst=%s %s=%s)\n", \
	h->topinfo, h->path, h->pathSep, iname, type, pname);
#define DBGATTR(type, h, name) \
    printf("%sAttr(topinfo=%s, path=%s, pathSep=%c %s=%s)\n", type, \
	h->topinfo, h->path, h->pathSep, type, name);
#else
#define DBGCATTR(type,h,iname,pname) /* */
#define DBGATTR(type, h, name)       /* */
#endif



/* ====================================================================
 * instAttr, portAttr, pbusAttr, netAttr, nbunAttr
 * ====================================================================
 *
 * instAttr - copy each DataBase Instance attribute and each Primitive/
 *	      Module attribute.
 * --------------------------------------------------------------------
 */
static const VdiHAttr*
instAttr(VdiDB* vdiDB, VdiHObj* vdiHobj, VdiHHi* hiInfo, int* flags)
{
    DB*   db   = (DB*)vdiDB;
    Inst* inst = (Inst*)vdiHobj->obj;
    Cell* cell = inst->cellRef;
    int   count;
    int	  i;
    int   attrcnt=0;
    VdiHAttr* attr;
    (void)hiInfo; /* avoid compiler warning */

    DBGATTR("inst", vdiHobj, inst->name);
#ifdef CONCEPT_DEBUGGING
    {
	Module* mod = (Module*)vdiHobj->mod;
	if (strcmp(mod->c.name,"at")==0) {
	    if (strcmp(inst->name,"ai05_bulktest")==0) {
		/* (hier)pinBus I[2:0] of inst ai05_bulktest has highlight.
		 * the (hier)sub-pin I[1], too.
		 */
		*flags |= (0x2/*pin/pinBus*/        |0x20/*sub-Pin*/)|
			  (0x8/*hierPin/hierPinBus*/|0x80/*sub-hierPin*/);
	    }
	}
    }
#endif

    assert(((Module*)vdiHobj->mod)->c.flags & CellFModule);
    CHECK_NET_NBUN_INST(vdiHobj->mod,vdiHobj->obj,Inst,instList);

    /* Check if at least one port/portBus or pin/pinBus defines an attribute.
     * if so, then flags |= 0x11 (to make Nlview scanning the pins and sub-pins)
     * or set flags |= 0x44 (to make Nlview scanning the down-ports and
     * down-sub-ports)
     */
    if (cell->flags & CellFModule) {
	count = cell->portList ? zListLength(cell->portList) : 0;
	for (i=0; i<count; i++) {
	    if (cell->portList[i].attrList) { *flags |= 0x44; goto next; }
	}
	count = cell->portBusList ? zListLength(cell->portBusList) : 0;
	for (i=0; i<count; i++) {
	    if (cell->portBusList[i].attrList) { *flags |= 0x4; goto next; }
	}
    }
  next:
    count = PINCOUNT(inst);
    for (i=0; i<count; i++) {
	if (PIN(inst,i).attrList) { *flags |= 0x11; break; }
    }


    /* add all instance attributes */
    attr = addAttr(inst->attrList, &attrcnt, NULL);

    /* For some transistor devices add @value */
    if (!(cell->flags & CellFModule)) {
	Primitive* prim = (Primitive*)cell;
	static char buf[64];

	switch (prim->c.func) {
	    const char *w, *l, *c;
	    case PrimFNMOS:
	    case PrimFPMOS:
		w = zSearchAttributeValue(db, inst->attrList, "w");
		l = zSearchAttributeValue(db, inst->attrList, "l");
		sprintf(buf, "W=%.20s\nL=%.20s", w?w:"", l?l:"");
		break;

	    case PrimFCAP:
		c = zSearchAttributeValue(db, inst->attrList, "C");
		sprintf(buf, "%.20s", c?c:"");
		break;

	    default:
		buf[0] = '\0';
		break;
	}
	if(buf[0] && attrcnt < MAX_BUF-1) {
	    /* add @value to attribute list */
	    attr[attrcnt].name   = "@value";
	    attr[attrcnt].value  = buf;
#ifdef CONCEPT_DEBUGGING
	    attr[attrcnt].format = "(+#3a3a3a,#eecc99,0.75)";
#else
	    attr[attrcnt].format = NULL;
#endif /* CONCEPT_DEBUGGING */
	    attrcnt++;
	    attr[attrcnt].name = NULL;		/* terminate list */
	}
    }
    return attr;
}

/* --------------------------------------------------------------------
 * portAttr - copy each DataBase Port attribute 
 * pbusAttr - copy each DataBase PortBus attribute 
 * netAttr  - copy each DataBase Net attribute 
 * nbunAttr - copy each DataBase NetBundle attribute 
 * --------------------------------------------------------------------
 */
static const VdiHAttr*
portAttr(VdiDB* vdiDB, VdiHObj* vdiHobj, VdiHHi* hiInfo) {
    Port* port = (Port*)vdiHobj->obj;
    (void)vdiDB;  /* avoid compiler warning */
    (void)hiInfo; /* avoid compiler warning */

    assert(((Module*)vdiHobj->mod)->c.flags & CellFModule);
    DBGATTR("port", vdiHobj, port->name);
    CHECK_PORT_PBUS(vdiHobj->mod,vdiHobj->obj,Port,portList);
    return addAttr(port->attrList, NULL, NULL);
}

static const VdiHAttr*
pbusAttr(VdiDB* vdiDB, VdiHObj* vdiHobj, VdiHHi* hiInfo) {
    PortBus* pbus = (PortBus*)vdiHobj->obj;
    (void)vdiDB;  /* avoid compiler warning */

    assert(((Module*)vdiHobj->mod)->c.flags & CellFModule);
    DBGATTR("pbus", vdiHobj, pbus->name);
    CHECK_PORT_PBUS(vdiHobj->mod,vdiHobj->obj,PortBus,portBusList);

#ifdef CONCEPT_DEBUGGING
    { /* produce dummy "filled" highlight for custom portBus */
	Module* mod = (Module*)vdiHobj->mod;
	if (strcmp(mod->c.name,"ai05")==0) {
	    if (strcmp(pbus->name,"I[2:0]")==0) {
		/* highlight complete portBus in ai05 */
		hiInfo->hi    = 1;
		hiInfo->fill  = 1;
		hiInfo->color = 4;
		hiInfo->width = 3;
		hiInfo->style = 1;	/* 1:solid style */
		hiInfo->prio  = 0;
		hiInfo->segm  = NULL;
	    }
	}
    }
#else
    (void)hiInfo; /* avoid compiler warning */
#endif /* CONCEPT_DEBUGGING */
    return addAttr(pbus->attrList, NULL, NULL);
}

static const VdiHAttr*
netAttr(VdiDB* vdiDB, VdiHObj* vdiHobj, VdiHHi* hiInfo) {
    Net* net = (Net*)vdiHobj->obj;
    (void)vdiDB;  /* avoid compiler warning */

    assert(((Module*)vdiHobj->mod)->c.flags & CellFModule);
    DBGATTR("net", vdiHobj, net->name);
    CHECK_NET_NBUN_INST(vdiHobj->mod,vdiHobj->obj,Net,netList);

#ifdef CONCEPT_DEBUGGING
    { /* produce dummy highlight for two nets */
	Module* mod = (Module*)vdiHobj->mod;
	if (strcmp(mod->c.name,"ai05")==0) {
	  if (strcmp(net->name, "Vdd")==0) {
	    /* complete and one segments of Vdd */
	    static struct VdiHSegmHi segm;
	    hiInfo->hi      = 1;
	    hiInfo->color   = 1;
	    hiInfo->width   = 2;
	    hiInfo->style   = 1;	/* 1:solid style */
	    hiInfo->prio    = 15;
	    hiInfo->sublist = 3;
	    hiInfo->segm    = &segm;

	    segm.h.hi      = 1;
	    segm.h.color   = 4;
	    segm.h.width   = 5;
	    segm.h.style   = 2;		/* 2:dashed style */
	    segm.h.prio    = 0;
	    segm.h.sublist = 6;
	    segm.h.segm    = NULL;
	    segm.con[0].type    = 0; /* pin Q1.C */
	    segm.con[0].comp    = instFind(vdiDB, "Q1", vdiHobj->mod);
	    segm.con[0].pinname = DBGCOPY("C");
	    segm.con[1].type    = 0; /* pin Q1.C (a pseudo net segment) */
	    segm.con[1].comp    = instFind(vdiDB, "Q1", vdiHobj->mod);
	    segm.con[1].pinname = DBGCOPY2("C");
	  } else if (strcmp(net->name, "n_O0")==0) {
	    /* two segments of n_O0 */
	    static struct VdiHSegmHi segm[2];
	    hiInfo->hi      = 0;
	    hiInfo->segm    = &segm[0];

	    segm[0].h.hi      = 1;
	    segm[0].h.color   = 0;
	    segm[0].h.style   = 3;		/* 3:dashed2 style */
	    segm[0].h.prio    = 0;
	    segm[0].h.sublist = -1;	/* use the global highlight list */
	    segm[0].h.segm    = &segm[1];
	    segm[0].con[0].type    = 0; /* pin Q1.S */
	    segm[0].con[0].comp    = instFind(vdiDB, "Q1", vdiHobj->mod);
	    segm[0].con[0].pinname = DBGCOPY("S");
	    segm[0].con[1].type    = 0; /* pin Q2.S */
	    segm[0].con[1].comp    = instFind(vdiDB, "Q2", vdiHobj->mod);
	    segm[0].con[1].pinname = DBGCOPY2("S");
	    segm[1].h.hi      = 1;
	    segm[1].h.color   = 2;
	    segm[1].h.style   = 3;		/* 3:dashed2 style */
	    segm[1].h.prio    = 0;
	    segm[1].h.sublist = -1;	/* use the global highlight list */
	    segm[1].h.segm    = NULL;
	    segm[1].con[0].type    = 2; /* (sub)Port O[0] of portBus O[2:0] */
	    segm[1].con[0].comp    = pbusFind(vdiDB, "O[2:0]", vdiHobj->mod);
	    segm[1].con[0].pinname = DBGCOPY3("O[0]");
	    segm[1].con[1].type    = 2; /* (sub)Port O[0] of portBus O[2:0] */
	    segm[1].con[1].comp    = pbusFind(vdiDB, "O[2:0]", vdiHobj->mod);
	    segm[1].con[1].pinname = DBGCOPY4("O[0]");
	  } else if (strcmp(net->name, "Gnd")==0) {
	    /* complete Gnd in ai05_bulktest */
	    hiInfo->hi      = 1;
	    hiInfo->color   = 10;
	    hiInfo->width   = 11;
	    hiInfo->style   = 5;	/* 5:dotted style */
	    hiInfo->sublist = -1;	/* use the global highlight list */
	    hiInfo->segm    = NULL;
	  }
        } else if (strcmp(mod->c.name, "ai15")==0) {
	  if (strcmp(net->name, "net[0]")==0) {
	    /* complete and two segments of net[0] */
	    static struct VdiHSegmHi segm[2];
	    hiInfo->hi      = 1;
	    hiInfo->color   = 1;
	    hiInfo->width   = 2;
	    hiInfo->sublist = 3;
	    hiInfo->segm    = &segm[0];

	    segm[0].h.hi      = 1;
	    segm[0].h.color   = 4;
	    segm[0].h.width   = 5;
	    segm[0].h.style   = 5;	/* 5:dotted style */
	    segm[0].h.prio    = 0;
	    segm[0].h.sublist = 6;
	    segm[0].h.segm    = &segm[1];
	    segm[0].con[0].type    = 0; /* pin I2.A */
	    segm[0].con[0].comp    = instFind(vdiDB, "I2", vdiHobj->mod);
	    segm[0].con[0].pinname = DBGCOPY("A");
	    segm[0].con[1].type    = 1; /* port IN */
	    segm[0].con[1].comp    = portFind(vdiDB, "IN", vdiHobj->mod);
	    segm[0].con[1].pinname = DBGCOPY2("IN");
	    segm[1].h.hi      = 1;
	    segm[1].h.color   = 7;
	    segm[1].h.width   = 8;
	    segm[1].h.style   = 0;
	    segm[1].h.prio    = 0;
	    segm[1].h.sublist = 9;
	    segm[1].h.segm    = NULL;
	    segm[1].con[0].type    = 0; /* pin I2.A */
	    segm[1].con[0].comp    = instFind(vdiDB, "I2", vdiHobj->mod);
	    segm[1].con[0].pinname = DBGCOPY3("A");
	    segm[1].con[1].type    = 0; /* pin I1.D */
	    segm[1].con[1].comp    = instFind(vdiDB, "I1", vdiHobj->mod);
	    segm[1].con[1].pinname = DBGCOPY4("D");
	  } else if (strcmp(net->name, "net[1]")==0) {
	    /* complete net[1] in ai15_attrtest */
	    hiInfo->hi      = 1;
	    hiInfo->color   = 10;
	    hiInfo->width   = 11;
	    hiInfo->style   = 4;	/* 4:dashed50 style */
	    hiInfo->prio    = 7;
	    hiInfo->sublist = 12;
	    hiInfo->segm    = NULL;
	  }
	}
    }
#else
    (void)hiInfo; /* avoid compiler warning */
#endif /* CONCEPT_DEBUGGING */
    return addAttr(net->attrList, NULL, NULL);
}

static const VdiHAttr*
nbunAttr(VdiDB* vdiDB, VdiHObj* vdiHobj, VdiHHi* hiInfo) {
    NetBus* nbun = (NetBus*)vdiHobj->obj;
    (void)vdiDB;  /* avoid compiler warning */
    (void)hiInfo; /* avoid compiler warning */

    assert(((Module*)vdiHobj->mod)->c.flags & CellFModule);
    DBGATTR("nbun", vdiHobj, nbun->name);
    CHECK_NET_NBUN_INST(vdiHobj->mod,vdiHobj->obj,NetBus,netBusList);
    return addAttr(nbun->attrList, NULL, NULL);
}
 


/* ====================================================================
 * downModule, primName, modType, primType, primFlags, instFlags, instValue
 * ====================================================================
 */
static VdiObj* downModule(VdiDB* vdidb, VdiObj* vdimod, VdiObj* vdiInst) {
    Inst* inst = (Inst*)vdiInst;
    Cell* cell = inst->cellRef;
    (void)vdidb; /* avoid compiler warning */

    CHECK_NET_NBUN_INST(vdimod,vdiInst,Inst,instList);
    if(cell->flags & CellFModule) return (VdiObj*)cell;
    return NULL;
}

static const char* primName(VdiDB* vdidb, VdiObj* vdimod, VdiObj* vdiInst,
    const char** vname) {
    Inst* inst = (Inst*)vdiInst;
    Cell* cell = inst->cellRef;
    (void)vdidb;  /* avoid compiler warning */

    CHECK_NET_NBUN_INST(vdimod,vdiInst,Inst,instList);
    *vname = DBGCOPY2("");
    if(cell->flags & CellFModule) return NULL;
    return DBGCOPY(cell->name);
}

static struct {
    PrimitiveFunc     func;
    enum VdiInstType  type;     
    int		      flags;     
} primTab[] = {
    {(PrimitiveFunc)0,	VdiInstTUNKNOWN,  0 },

    { PrimFUNKNOWN,	VdiInstTUNKNOWN,  0 },
    { PrimFAND,		VdiInstTAND,      0 },
    { PrimFNAND,	VdiInstTNAND,     0 },
    { PrimFOR,		VdiInstTOR,       0 },
    { PrimFNOR,		VdiInstTNOR,      0 },
    { PrimFXOR,		VdiInstTXOR,      0 },
    { PrimFXNOR,	VdiInstTXNOR,     0 },

    { PrimFBUF,		VdiInstTBUF,      0 },
    { PrimFINV,		VdiInstTINV,      0 },

    { PrimFMUX,		VdiInstTMUX,      0 },
    { PrimFADD,		VdiInstTALU,      0 },
    { PrimFDFF,		VdiInstTLATCH,    0 },
    { PrimFLATCH,	VdiInstTLATCH,    0 },

    { PrimFBUFIF0 ,   	VdiInstTBUFIF0,   0 },
    { PrimFBUFIF1,    	VdiInstTBUFIF1,   0 },
    { PrimFINVIF0 ,   	VdiInstTINVIF0,   0 },
    { PrimFINVIF1,    	VdiInstTINVIF1,   0 },
    { PrimFTRAN,	VdiInstTTRAN,     0 },
    { PrimFTRANIF0 ,  	VdiInstTTRANIF0,  0 },
    { PrimFTRANIF1,   	VdiInstTTRANIF1,  0 },

    { PrimFNMOS,	VdiInstTNMOS,     VdiInstFDevice },
    { PrimFPMOS,	VdiInstTPMOS,     VdiInstFDevice },
    { PrimFCMOS,	VdiInstTUNKNOWN,  VdiInstFDevice },
    { PrimFNPN,		VdiInstTNPN,      VdiInstFDevice|VdiInstFPolar },
    { PrimFPNP,		VdiInstTPNP,      VdiInstFDevice|VdiInstFPolar },
    { PrimFRES,		VdiInstTRES,      VdiInstFDevice },
    { PrimFCAP,		VdiInstTCAP,      VdiInstFDevice|VdiInstFNoFlow },
    { PrimFINDUCTOR,  	VdiInstTINDUCTOR, VdiInstFDevice },
    { PrimFDIODE,	VdiInstTDIODE,	  VdiInstFDevice|VdiInstFWeakFlow },
    { PrimFZDIODE,    	VdiInstTZDIODE,   VdiInstFDevice|VdiInstFWeakFlow },
    { PrimFSWITCH,    	VdiInstTSWITCH,   VdiInstFDevice },
    { PrimFVSOURCE,   	VdiInstTVSOURCE,  VdiInstFDevice },
    { PrimFISOURCE,   	VdiInstTISOURCE,  VdiInstFDevice },
    { PrimFTRANSLINE, 	VdiInstTTRANSLINE,0 },
    { PrimFUDRCLINE,  	VdiInstTUDRCLINE, 0 },
    { PrimFAMP,		VdiInstTAMP,      0 },

    { PrimFREDUCE_AND,	VdiInstTAND,      0 },
    { PrimFREDUCE_NAND,	VdiInstTNAND,     0 },
    { PrimFREDUCE_OR,	VdiInstTOR,       0 },
    { PrimFREDUCE_NOR,	VdiInstTNOR,      0 },
    { PrimFREDUCE_XOR,	VdiInstTXOR,      0 },
    { PrimFREDUCE_XNOR,	VdiInstTXNOR,     0 },

    { PrimFWIDE_AND,	VdiInstTAND,      0 },
    { PrimFWIDE_NAND,	VdiInstTNAND,     0 },
    { PrimFWIDE_OR,	VdiInstTOR,       0 },
    { PrimFWIDE_NOR,	VdiInstTNOR,      0 },
    { PrimFWIDE_XOR,	VdiInstTXOR,      0 },
    { PrimFWIDE_XNOR,	VdiInstTXNOR,     0 },
    { PrimFWIDE_BUF,	VdiInstTBUF,      0 },
    { PrimFWIDE_INV,	VdiInstTINV,      0 },
    { PrimFWIDE_TRI,	VdiInstTBUFIF1,   0 },

    { PrimFWIDE_MUX,    VdiInstTMUX,      0 },
    { PrimFDECODER,	VdiInstTDECODER,  0 }
};

static enum VdiInstType addFunc(Cell* cell,
	char param[16], const char** symdef, const VdiHAttr** symprops)
{
    if (symprops) *symprops = addAttr(cell->attrList, NULL, symdef);

    if (param && (cell->func == PrimFADD)) {
	param[0] = '+';
	param[1] = 0;
    }
    assert(primTab[cell->func].func == cell->func);
    return primTab[cell->func].type;
}

static enum VdiInstType modType(VdiDB* vdidb, VdiObj* vdimod,
	char param[16], const char** symdef, const VdiHAttr** symprops)
{
    Module* mod = (Module*)vdimod;
    Cell*   cell = &mod->c;
    (void)vdidb;  /* avoid compiler warning */
    if (symprops) *symprops = addAttr(mod->c.attrList, NULL, symdef);

    assert(cell->flags & CellFModule);
    return addFunc(cell, param, symdef, symprops);
}

static enum VdiInstType primType(VdiDB* vdidb, VdiObj* vdimod, VdiObj* vdiInst,
    char param[16], const char** symdef, const VdiHAttr** symprops)
{
    Inst*	inst = (Inst*)vdiInst;
    Cell*	cell = inst->cellRef;
    Primitive*	prim = (Primitive*)cell;
    (void)vdidb;  /* avoid compiler warning */

    assert(!(cell->flags & CellFModule));
    CHECK_NET_NBUN_INST(vdimod,vdiInst,Inst,instList);

    assert(cell == &prim->c);
    return addFunc(&prim->c, param, symdef, symprops);
}

static enum VdiInstFlags primFlags(VdiDB* vdidb,
    VdiObj* vdimod, VdiObj* vdiInst) {
    Inst* inst = (Inst*)vdiInst;
    Cell* cell = inst->cellRef;
    Primitive* prim = (Primitive*)cell;
    int flags;
    (void)vdidb; /* avoid compiler warning */

    assert(!(cell->flags & CellFModule));
    CHECK_NET_NBUN_INST(vdimod,vdiInst,Inst,instList);

    assert((sizeof(primTab)/sizeof(primTab[0]))==PrimFLAST);
    assert(primTab[prim->c.func].func == prim->c.func);

    flags = primTab[prim->c.func].flags;
    if (prim->c.flags & CellFWeakFlow) {
	flags |= VdiInstFWeakFlow;
    }
    return (enum VdiInstFlags)flags;
}

/* --------------------------------------------------------------------
 * instFlags, instValue, portFlags, netFlags.
 * --------------------------------------------------------------------
 */
static enum VdiInstFlags instFlags(VdiDB* vdidb,
    VdiObj* vdimod, VdiObj* vdiInst) {
    Inst* inst = (Inst*)vdiInst;
    Cell* cell = inst->cellRef;
    int flags = 0;
    (void)vdidb; /* avoid compiler warning */

    CHECK_NET_NBUN_INST(vdimod,vdiInst,Inst,instList);
    if(!(cell->flags & CellFModule)) {
	Primitive* prim = (Primitive*)cell;
	assert(primTab[prim->c.func].func == prim->c.func);
	flags = primTab[prim->c.func].flags;

	/* make all primitive insts targets for "addcone -targetFlaggedInst" */
	flags |= VdiInstFTarget;

	/* never set -autohide flag at primitives */
	flags |= VdiInstFNAutohide;

	/* propagate feedthru flag for primitives */
	if (cell->flags & CellFFeedthru) flags |= VdiInstFFeedthru;
    }
    if (inst->flags & InstFWeakFlow)	flags |= VdiInstFWeakFlow;

    if (inst->flags & InstFOrientR90)	flags |= VdiInstR90;
    if (inst->flags & InstFOrientR180)	flags |= VdiInstR180;
    if (inst->flags & InstFOrientR270)	flags |= VdiInstR270;
    if (inst->flags & InstFOrientMY)	flags |= VdiInstMY;
    if (inst->flags & InstFOrientMYR90)	flags |= VdiInstMYR90;
    if (inst->flags & InstFOrientMX)	flags |= VdiInstMX;
    if (inst->flags & InstFOrientMXR90)	flags |= VdiInstMXR90;

    /* check if any of cell's port/portBus has a hide bit set */
    if (cell->portList) {	/* loop over all ports/subPorts */
	const Port* port = cell->portList;
	int cnt = zListLength(cell->portList);
	for (; --cnt >= 0; port++) {
	    if (port->flags & PortFHideMask) {
		flags |= VdiInstFPinHide;
		break;
	    }
	}
    }
    if (cell->portBusList) {	/* loop over all portBuses */
	const PortBus* pbus = cell->portBusList;
	int cnt = zListLength(cell->portBusList);
	for (; --cnt >= 0; pbus++) {
	    if (pbus->flags & PortFHideMask) {
		flags |= VdiInstFPinHide;
		break;
	    }
	}
    }

    return (enum VdiInstFlags)flags;
}

static float instValue(VdiDB* vdiDB, VdiObj* vdimod, VdiObj* vdiInst) {
    DB*  db    = (DB*)vdiDB;
    Inst* inst = (Inst*)vdiInst;
    Cell* cell = inst->cellRef;
    const char* name;
    const char* value;
    Primitive* prim;
    CHECK_NET_NBUN_INST(vdimod,vdiInst,Inst,instList);

    if(cell->flags & CellFModule) return 0.0F;
    prim = (Primitive*)cell;
    switch (prim->c.func) {
	case PrimFCAP:		name = "C"; break;
	case PrimFRES:		name = "R"; break;
	case PrimFINDUCTOR:	name = "L"; break;
	default:		return 0.0F;
    }
    value = zSearchAttributeValue(db, inst->attrList, name);
    return value ? getValue(value) : 0.0F;
}

static enum VdiPortFlags mapPortFlags(PortFlags flags, Inst* inst, int pinIdx) {
    int f = flags & PortFInOut;
    /* Make sure, we're using the same bits and semantic */
    assert(VdiPortFUnknown == PortFUnknown);
    assert(VdiPortFOut     == PortFOut);
    assert(VdiPortFIn      == PortFIn);
    assert(VdiPortFInOut   == PortFInOut);

    if (flags & PortFNeg)   f |= VdiPortFNeg;
    if (flags & PortFClock) f |= VdiPortFClk;

    if (inst && (flags & PortFHideMask) != 0) {
	if (flags & PortFHide)     f |= VdiPortFHide;
	if (flags & PortFHideStub) f |= VdiPortFHideStub;
	if (flags & PortFDontHide) f |= VdiPortFDontHide;
    }

    if (inst && !(inst->cellRef->flags & CellFModule)) {
	Primitive* prim = (Primitive*)inst->cellRef;
	assert(pinIdx < PINCOUNT(inst));

	/* We got inst and pinIdx identifying an instance pin - we need that
	 * only for MUX, DFF and LATCH Primitive to set additional
	 * Top/Bot/Clk Vdi flags.  Those Primitives define the meaning of
	 * their ports by order - e.g. MUX's pin#3 is the selection pin
	 * (see DataBase doc tapiprim.html for details).  The port-order-number
	 * is defined by "pinIdx" - we distinguish two cases: (1) pinIdx
	 * points to an ordinary pin or (2) to a sub-pin (busMember != -1).
	 * For case (2) we replace pinIdx by the index of the portBus
	 * to get the port-order-number for bused primitives.
	 */
	if (prim->c.portList[pinIdx].busMember != -1) {
	    pinIdx = prim->c.portList[pinIdx].busMember;
	}
	switch (prim->c.func) {
	    case PrimFMUX:
	    case PrimFWIDE_MUX:
		if (pinIdx == 3) f |= VdiPortFTop;	/* Select pin */
		break;

	    case PrimFADD:
		if (pinIdx == 1) f |= VdiPortFBot;	/* Carry-Out pin */
		if (pinIdx == 4) f |= VdiPortFTop;	/* Carry-In  pin */
		break;

	    case PrimFDFF:
		if (pinIdx == 2) f |= VdiPortFClk;	/* Clock pin */
	    case PrimFLATCH:
		if (pinIdx == 3) f |= VdiPortFBot;	/* Reset pin */
		if (pinIdx == 4) f |= VdiPortFTop;	/* Set pin */
		break;

	    default:
		break;
	}
    }
    if (flags & PortFDIR) {
	f &= ~(VdiPortFTop|VdiPortFBot|VdiPortFLeft|VdiPortFRight);
	if (flags & PortFTop)   f |= VdiPortFTop;
	if (flags & PortFBot)   f |= VdiPortFBot;
	if (flags & PortFLeft)  f |= VdiPortFLeft;
	if (flags & PortFRight) f |= VdiPortFRight;
    }

    return (enum VdiPortFlags)f;
}

static enum VdiPortFlags portFlags(VdiDB* vdidb,
    VdiObj* vdimod, VdiObj* vdiPort) {
    Port* port = (Port*)vdiPort;
    (void)vdidb; /* avoid compiler warning */

    CHECK_PORT_PBUS(vdimod,vdiPort,Port,portList);
    return mapPortFlags(port->flags, NULL, 0);
}

static enum VdiNetFlags netFlags(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiNet) {
    Net* net = (Net*)vdiNet;
    Module* mod = (Module*)vdiMod;
    int flags;
    (void)vdidb; /* avoid compiler warning */
    (void)mod;   /* avoid compiler warning */

    CHECK_NET_NBUN_INST(vdiMod,vdiNet,Net,netList);

    /* handle power/ground flags */
    switch (net->flags & (NetFPower|NetFGround|NetFNegPower)) {
	case NetFPower:    flags = VdiNetFPower;    break;
	case NetFGround:   flags = VdiNetFGround;   break;
	case NetFNegPower: flags = VdiNetFNegPower; break;
	default:           flags = 0;
    }
    if (net->flags & NetFTarget)   flags |= VdiNetFTarget;  /* search target */
    if (net->flags & NetFConst)    flags |= VdiNetFConst;   /* const net val */
    if (net->flags & NetFFly)      flags |= VdiNetFFly;     /* ratnest route */
    if (net->flags & NetFGlobal)   flags |= VdiNetFHide;    /* don't route   */
    if (net->flags & NetFAutogen)  flags |= VdiNetFHide;    /* don't route   */
    if (net->flags & NetFHide)     flags |= VdiNetFHide;    /* don't route   */
    if (net->flags & NetFDontHide) {
	flags &= ~VdiNetFHide;
	flags |= VdiNetFDontHide;			    /* never hide    */
    }
    if (net->flags & NetFVSource) {
	flags &= ~(VdiNetFPower|VdiNetFGround|VdiNetFNegPower);
	flags |= VdiNetFVSource; /* is a routed net treated as a voltage src */
    }

    return (enum VdiNetFlags)flags;
}

/* ====================================================================
 * Citer, CiterFuncs - data structs for connectivity iterators
 *
 * A set of 4 classes is used to implement 4 connectivity-iterators
 * sharing a common interface of 10 "virtual" functions - we implement
 * this kind of polymorphism by emulating C++ inheritance - each of
 * the 4 classes are derived from the base class "Citer"
 * that's why each struct must have the CiterFuncs* as first element).
 *
 * Here is a table describing what con-iterator uses what struct:
 *	class		used by
 *	===========	=============
 *	NetCiter	conIterNet
 *	CmembersCiter	conIter_Cmembers
 *	InstCiter	conIterInst
 *	PBusCiter	conIterPBus
 *
 * The CiterFuncs implements the "virtual" function vector (10 virtual
 * functions) for each con-iterator type.  There is no need for all
 * con-iterators to implement all of the 10 functions, the un-needed
 * ones get a NULL pointer in the vector.
 * ====================================================================
 */
typedef struct CiterFuncs CiterFuncs;
typedef struct Citer      Citer;
struct Citer {
    CiterFuncs* funcs;			/* pointer to 10 "virtual" functions */
};

struct CiterFuncs {
    int              (*more)     (const Citer* citer);
    void             (*next)     (      Citer* citer);
    VdiObj*          (*net)      (const Citer* citer);
    VdiObj*          (*comp)     (const Citer* citer, int* type);
    int	     	     (*pinnumber)(const Citer* citer);
    const char*      (*pinname)  (const Citer* citer);
    const char*      (*pbusname) (const Citer* citer);
    enum VdiPortFlags(*portflags)(const Citer* citer);
    VdiCiter*        (*members)  (const Citer* citer);
    const VdiHAttr*  (*pinattr)  (const Citer* citer, VdiHObj*, VdiHHi* hiInfo);
};

/* --------------------------------------------------------------------
 * The common con-iterator interface (each interface function calls the
 * corresponding implementation function thru CiterFuncs' virtual function
 * vector):
 *
 * cmore, cnext, cnet, ccomp, cpinnumber, cpinname, cpbusname,
 * cportflags, cmembers and cpinattr.
 * --------------------------------------------------------------------
 */
static void cnext(VdiCiter* vdiCiter) {
    Citer* citer = (Citer*)vdiCiter;
    citer->funcs->next(citer);
}

static VdiObj* ccomp(const VdiCiter* vdiCiter, int* type) {
    const Citer* citer = (const Citer*)vdiCiter;
    return citer->funcs->comp ? citer->funcs->comp(citer, type) : NULL;
}

#define CITERCALL(TYPE,FUNC,VFUNC,DEF) \
static TYPE FUNC(const VdiCiter* vdiCiter) { \
    const Citer* citer = (const Citer*)vdiCiter; \
    return citer->funcs->VFUNC ? citer->funcs->VFUNC(citer) :  DEF; \
}
CITERCALL(int,cmore,more,0)
CITERCALL(VdiObj*,cnet,net,NULL)
CITERCALL(int,cpinnumber,pinnumber,-1)
CITERCALL(const char*,cpinname,pinname,NULL)
CITERCALL(const char*,cpbusname,pbusname,NULL)
CITERCALL(enum VdiPortFlags,cportflags,portflags,(enum VdiPortFlags)0)
CITERCALL(VdiCiter*,cmembers,members,NULL)
#undef CITERCALL

static const VdiHAttr*
cpinattr(VdiCiter* vdiCiter, VdiHObj* vdiHobj,VdiHHi* hiInfo) {
    const Citer* citer = (const Citer*)vdiCiter;
    return citer->funcs->pinattr ?
	citer->funcs->pinattr(citer, vdiHobj, hiInfo) : NULL;
}

/* --------------------------------------------------------------------
 * conIterNet
 *	Iterate over each net connection (single-bit) - stop at either
 *	(a) instance pin (netCiter comp's type = 0)
 *	(b) port         (netCiter comp's type = 1)
 *	(c) sub-port     (netCiter comp's type = 2)
 * --------------------------------------------------------------------
 */
typedef struct NetCiter NetCiter;
struct NetCiter {
    CiterFuncs* funcs;			/* pointer to 10 "virtual" functions */
    Module* mod;
    PinRef* pinRefList;
    int     conIdx;
};

static int netCiterMore(const Citer* citer) {
    const NetCiter* cit = (const NetCiter*)citer;
    int cnt = cit->pinRefList ? zListLength(cit->pinRefList) : 0;
    return cit->conIdx < cnt;
}

static void netCiterNext(Citer* citer) {
    NetCiter* cit = (NetCiter*)citer;
    assert(cit->conIdx < (cit->pinRefList ? zListLength(cit->pinRefList) : 0));
    cit->conIdx++;
}

static VdiObj* netCiterComp(const Citer* citer, int* type) {
    const NetCiter* cit = (const NetCiter*)citer;
    PinRef* pinRef = cit->pinRefList + cit->conIdx;
    assert(cit->conIdx < (cit->pinRefList ? zListLength(cit->pinRefList) : 0));
    if(pinRef->inst) {
	*type = 0;
	return (VdiObj*)pinRef->inst;
    } else {
	Port* port = cit->mod->c.portList+pinRef->pinNo;
	if(port->busMember==-1) {
	    *type = 1;
	    return (VdiObj*)port;
	} else {
	    PortBus* portBus = cit->mod->c.portBusList+port->busMember;
	    *type = 2;
	    return (VdiObj*)portBus;
	}
    }
}

static int netCiterPinnumber(const Citer* citer) {
    const NetCiter* cit = (const NetCiter*)citer;
    PinRef* pinRef = cit->pinRefList + cit->conIdx; 
    assert(cit->conIdx < (cit->pinRefList ? zListLength(cit->pinRefList) : 0));
    return pinRef->pinNo;
}

#define INIT_PORT \
    const NetCiter* cit = (const NetCiter*)citer; \
    PinRef* pinRef = cit->pinRefList + cit->conIdx;  \
    Port* port; \
    assert(cit->conIdx < (cit->pinRefList ? zListLength(cit->pinRefList):0)); \
    if(pinRef->inst) { \
	port = pinRef->inst->cellRef->portList+pinRef->pinNo; \
    } else { \
	port = cit->mod->c.portList+pinRef->pinNo; \
    } \
    (void)NULL		/* allow to append ';' character in code */

static const char* netCiterPinname(const Citer* citer) {
    INIT_PORT;
    return DBGCOPY(port->name);
}

static enum VdiPortFlags netCiterPortflags(const Citer* citer) {
    INIT_PORT;
    return mapPortFlags(port->flags, pinRef->inst, pinRef->pinNo);
}
#undef INIT_PORT

static CiterFuncs netCiterFuncs = {
    netCiterMore,      netCiterNext,    
    0,                 netCiterComp,      
    netCiterPinnumber,
    netCiterPinname,   0, 
    netCiterPortflags, 0,
    0
};

static VdiCiter* conIterNet(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiNet) {
    Module* mod = (Module*)vdiMod;
    Net*    net = (Net*)vdiNet;
    NetCiter* cit = malloc(sizeof(NetCiter));
    CHECK_NET_NBUN_INST(vdiMod,vdiNet,Net,netList);
    (void)vdidb; /* avoid compiler warning */

    cit->funcs      = &netCiterFuncs;
    cit->mod        = mod;
    cit->pinRefList = net->pinRefList;
    cit->conIdx     = 0;
    return (VdiCiter*)cit;
}

/* --------------------------------------------------------------------
 * conIter_Cmembers
 * --------------------------------------------------------------------
 */
typedef struct CmembersCiter CmembersCiter;
struct CmembersCiter {
    CiterFuncs* funcs;			/* pointer to 10 "virtual" functions */
    Inst*    inst;
    PortBus* portBus;
    int      pinIdx;
    int      incDir;
};

static int cmembersCiterMore(const Citer* citer) {
    const CmembersCiter* cit = (const CmembersCiter*)citer;
    return cit->pinIdx != (cit->portBus->last + cit->incDir);
}

static void cmembersCiterNext(Citer* citer) {
    CmembersCiter* cit = (CmembersCiter*)citer;
    assert(cit->pinIdx < PINCOUNT(cit->inst));
    cit->pinIdx+= cit->incDir;
}

static VdiObj* cmembersCiterNet(const Citer* citer) {
    const CmembersCiter* cit = (const CmembersCiter*)citer;
    assert(cit->pinIdx < PINCOUNT(cit->inst));
    return (VdiObj*)PIN(cit->inst,cit->pinIdx).net;
}

static const char* cmembersCiterPinname(const Citer* citer) {
    const CmembersCiter* cit = (const CmembersCiter*)citer;
    Port* port;
    assert(cit->pinIdx < PINCOUNT(cit->inst));
    port = cit->inst->cellRef->portList+cit->pinIdx;
    return DBGCOPY(port->name);
}

static enum VdiPortFlags cmembersCiterPortflags(const Citer* citer) {
    const CmembersCiter* cit = (const CmembersCiter*)citer;
    assert(cit->pinIdx < PINCOUNT(cit->inst));

    /* subpin dir == portbus dir */
    return mapPortFlags(cit->portBus->flags, cit->inst, cit->pinIdx);
}

static const VdiHAttr* cmembersCiterPinattr(const Citer* citer,
				  VdiHObj* vdiHobj, VdiHHi* hiInfo) {
    const CmembersCiter* cit = (const CmembersCiter*)citer;
    Port* port = cit->inst->cellRef->portList+cit->pinIdx;
    assert(cit->pinIdx < PINCOUNT(cit->inst));
    assert(vdiHobj->obj == NULL);
    assert(vdiHobj->mod == NULL);

    DBGCATTR("subPin", vdiHobj, cit->inst->name, port->name);

#ifdef CONCEPT_DEBUGGING
    if (strcmp(cit->inst->name,"ai05_bulktest")==0) {
	if (strcmp(cit->portBus->name,"I[2:0]")==0) {
	    if (strcmp(port->name,"I[1]")==0) {
		/* highlight sub-pin I[1] @ pinBus I[2:0] @ inst ai05_bulktest*/
		hiInfo->hi    = 1;
		hiInfo->color = 1;
		hiInfo->width = 1;
		hiInfo->style = 2;	/* 2:dashed style */
		hiInfo->segm  = NULL;
	    }
	}
    }
#else
    (void)hiInfo; /* avoid compiler warning */
    (void)port;   /* avoid compiler warning */
#endif
    /* add all sub-pin attributes */
    return addAttr(PIN(cit->inst,cit->pinIdx).attrList, NULL, NULL);
}

static CiterFuncs cmembersCiterFuncs = {
    cmembersCiterMore,      cmembersCiterNext,    
    cmembersCiterNet,       0,      
    0,
    cmembersCiterPinname,   0, 
    cmembersCiterPortflags, 0,
    cmembersCiterPinattr
};

static VdiCiter* conIter_Cmembers(Inst* inst, PortBus* portBus) {
    CmembersCiter* cit = malloc(sizeof(CmembersCiter));
    cit->funcs  = &cmembersCiterFuncs;
    cit->inst    = inst;
    cit->portBus = portBus;
    cit->pinIdx  = portBus->first;
    cit->incDir  = (cit->portBus->last >= cit->portBus->first) ? 1: -1;
    return (VdiCiter*)cit;
}

/* --------------------------------------------------------------------
 * conIterInst
 *	Iterate over each single-bit pin (but not bus members) and then
 *	over each pinBus.  If idx=[0...pinCount) then the iterator is at
 *	a single-bit pin; if idx >= pinCount, then the iterator is at
 * 	pinBus cit->inst->cellRef->portBusList[bIdx] (bIdx = idx-pinCount).
 *
 *	The private pinSkipBusmembers() is called to skip
 *	sub-pins while the iterator is in range [0...pinCount)
 *	because the cellRef's portList include both, ports and sub-ports.
 * --------------------------------------------------------------------
 */
typedef struct InstCiter InstCiter;
struct InstCiter {
    CiterFuncs* funcs;			/* pointer to 10 "virtual" functions */
    DB*   db;
    Inst* inst;
    int   idx;
};

static void pinSkipBusmembers(InstCiter* cit) {
    int   pinCount = PINCOUNT(cit->inst);
    Port* portList = cit->inst->cellRef->portList;
    while(cit->idx < pinCount) {
	if(portList[cit->idx].busMember!=-1) cit->idx++;
	else break;
    }
}

static int instCiterMore(const Citer* citer) {
    const InstCiter* cit = (const InstCiter*)citer;
    int pinCount = PINCOUNT(cit->inst);
    if(cit->idx < pinCount) {
	return 1;
    } else if(cit->inst->cellRef->portBusList) {
	int pbusCount = zListLength(cit->inst->cellRef->portBusList);
	return cit->idx < pinCount+pbusCount;
    } 
    return 0;
}

static void instCiterNext(Citer* citer) {
    InstCiter* cit = (InstCiter*)citer;
    cit->idx++;
    pinSkipBusmembers(cit);
}

static VdiObj* instCiterNet(const Citer* citer) {
    const InstCiter* cit = (const InstCiter*)citer;
    assert(cit->idx < PINCOUNT(cit->inst));	/* not called for PinBus */
    return (VdiObj*)PIN(cit->inst,cit->idx).net;
}

#define INIT_PORT_PORTBUS \
    const InstCiter* cit = (const InstCiter*)citer; \
    Port*    port; \
    PortBus* portBus; \
    int pinCount = PINCOUNT(cit->inst); \
    if(cit->idx < pinCount) { \
	/* iterator stops at single-bit port */ \
	port = cit->inst->cellRef->portList+cit->idx; \
	portBus = NULL; \
    } else { \
	/* iterator stops at portBus */ \
	int pIdx = cit->idx - pinCount; \
	assert(cit->inst->cellRef->portBusList); \
	assert(pIdx < zListLength(cit->inst->cellRef->portBusList)); \
	portBus = cit->inst->cellRef->portBusList+pIdx; \
	port = NULL; \
    } \
    (void)port; (void)portBus

static const char* instCiterPinname(const Citer* citer) {
    INIT_PORT_PORTBUS;
    return port ? DBGCOPY(port->name) : NULL;
}

static const char* instCiterPBusname(const Citer* citer) {
    INIT_PORT_PORTBUS;
    return portBus ? DBGCOPY(portBus->name) : NULL;
}

static enum VdiPortFlags instCiterPortflags(const Citer* citer) {
    INIT_PORT_PORTBUS;
    if (port) {
	return mapPortFlags(port->flags, cit->inst, cit->idx);
    } else {
	return mapPortFlags(portBus->flags, cit->inst, portBus->first);
    }
}

static VdiCiter* instCiterMembers(const Citer* citer) {
    INIT_PORT_PORTBUS;
    if (port) {
	return NULL;
    } else {
	return conIter_Cmembers(cit->inst, portBus);
    }
}

static const VdiHAttr* instCiterPinattr(const Citer* citer,
				  VdiHObj* vdiHobj, VdiHHi* hiInfo) {

    /* copy each DataBase Pin/PinBus and Port/PortBus attribute */
    const VdiHAttr* attr;

    INIT_PORT_PORTBUS;

    assert(vdiHobj->obj == NULL);
    assert(vdiHobj->mod == NULL);

    if(port) {
	DBGCATTR("pin", vdiHobj, cit->inst->name, port->name);

	/* add all pin attributes */
	attr = addAttr(PIN(cit->inst,cit->idx).attrList, NULL, NULL);

    } else {
	DBGCATTR("pinBus", vdiHobj, cit->inst->name, portBus->name);

	/* the tdb has no pinBus - no PinBus attributes.
	 * Here, we return the attributes of the first sub-pin of the given
	 * pinBus.
	 */
	attr = addAttr(PIN(cit->inst,portBus->first).attrList, NULL, NULL);
#ifdef CONCEPT_DEBUGGING
	if (strcmp(cit->inst->name,"ai05_bulktest")==0) {
	    if (strcmp(portBus->name,"I[2:0]")==0) {
		/* highlight pinBus I[2:0] of inst ai05_bulktest */
		hiInfo->hi    = 1;
		hiInfo->color = 3;
		hiInfo->width = 3;
		hiInfo->style = 1;	/* 1:solid style */
		hiInfo->segm  = NULL;
	    }
	}
#else
	(void)hiInfo; /* avoid compiler warning */
#endif /* CONCEPT_DEBUGGING */
    }
    return attr;
}
#undef INIT_PORT_PORTBUS

static CiterFuncs instCiterFuncs = {
    instCiterMore,      instCiterNext,    
    instCiterNet,       0,      
    0,
    instCiterPinname,   instCiterPBusname, 
    instCiterPortflags, instCiterMembers,
    instCiterPinattr
};

static VdiCiter* conIterInst(VdiDB* vdiDB, VdiObj* vdimod, VdiObj* vdiInst) {
    Inst* inst = (Inst*)vdiInst;
    DB*   db   = (DB*)vdiDB;
    InstCiter* cit = malloc(sizeof(InstCiter));
    CHECK_NET_NBUN_INST(vdimod,vdiInst,Inst,instList);

    cit->db   = db;
    cit->inst = inst;
    cit->idx  = 0;
    cit->funcs  = &instCiterFuncs;
    pinSkipBusmembers(cit);
    return (VdiCiter*)cit;
}

/* --------------------------------------------------------------------
 * conIterPBus
 * --------------------------------------------------------------------
 */
typedef struct PBusCiter PBusCiter;
struct PBusCiter {
    CiterFuncs* funcs;			/* pointer to 10 "virtual" functions */
    PortBus* portBus;
    Module*  mod;
    int      portIdx;
    int      incDir;
};

static int pbusCiterMore(const Citer* citer) {
    const PBusCiter* cit = (const PBusCiter*)citer;
    return cit->portIdx != (cit->portBus->last + cit->incDir);
}

static void pbusCiterNext(Citer* citer) {
    PBusCiter* cit = (PBusCiter*)citer;
    assert(cit->portIdx < zListLength(cit->mod->c.portList));
    cit->portIdx += cit->incDir;
}

#define INIT_PORT \
    const PBusCiter* cit = (const PBusCiter*)citer; \
    Port* port = cit->mod->c.portList + cit->portIdx; \
    assert(cit->portIdx < zListLength(cit->mod->c.portList)); \
    (void)NULL		/* allow to append ';' character in code */

static VdiObj* pbusCiterNet(const Citer* citer) {
    INIT_PORT;
    return (VdiObj*)port->net;
}

static const char* pbusCiterPinname(const Citer* citer) {
    INIT_PORT;
    return DBGCOPY(port->name);
}

static enum VdiPortFlags pbusCiterPortflags(const Citer* citer) {
    INIT_PORT;
    return mapPortFlags(port->flags, NULL, 0);
}

static const VdiHAttr* pbusCiterPinattr(const Citer* citer,
				  VdiHObj* vdiHobj, VdiHHi* hiInfo) {
    INIT_PORT;
    DBGCATTR("subPort", vdiHobj, cit->mod->c.name, port->name);
    assert(vdiHobj->obj == NULL);
    assert(vdiHobj->mod == NULL);
#ifdef CONCEPT_DEBUGGING
    if (strcmp(cit->mod->c.name,"ai05")==0) {
	if (strcmp(cit->portBus->name,"I[2:0]")==0) {
	    if (strcmp(port->name,"I[1]")==0) {
		/* highlight sub-port I[1] of portBus I[2:0] in ai05 */
		hiInfo->hi    = 1;
		hiInfo->color = 2;
		hiInfo->width = 1;
		hiInfo->style = 2;	/* 2:dashed style */
		hiInfo->segm  = NULL;
	    }
	}
    }
#else
    (void)hiInfo; /* avoid compiler warning */
#endif /* CONCEPT_DEBUGGING */

    /* add all sub-port attributes */
    return addAttr(port->attrList, NULL, NULL);
}
#undef INIT_PORT

static CiterFuncs pbusCiterFuncs = {
    pbusCiterMore,      pbusCiterNext,    
    pbusCiterNet,       0,      
    0,
    pbusCiterPinname,   0, 
    pbusCiterPortflags, 0,
    pbusCiterPinattr
};

static VdiCiter* conIterPBus(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiPBus) {
    Module*  mod     = (Module*)vdiMod;
    PortBus* portBus = (PortBus*)vdiPBus;
    PBusCiter* cit = malloc(sizeof(PBusCiter));
    CHECK_PORT_PBUS(vdiMod,vdiPBus,PortBus,portBusList);
    (void)vdidb; /* avoid compiler warning */

    cit->funcs   = &pbusCiterFuncs;
    cit->mod     = mod;
    cit->portBus = portBus;
    cit->portIdx = portBus->first;
    cit->incDir  = (cit->portBus->last >= cit->portBus->first) ? 1: -1;
    return (VdiCiter*)cit;
}

/* --------------------------------------------------------------------
 * freeCiter - free memory for con-iterator
 * --------------------------------------------------------------------
 */
static void freeCiter(VdiDB* vdidb, VdiCiter* vdiCiter) {
    Citer* citer = (Citer*)vdiCiter;
    (void)vdidb; /* avoid compiler warning */

    free(citer);
}

/* --------------------------------------------------------------------
 * conPort - return connected net or NULL
 * --------------------------------------------------------------------
 */
static VdiObj* conPort(VdiDB* vdidb, VdiObj* vdimod, VdiObj* vdiPort) {
    Port* port = (Port*)vdiPort;
    (void)vdidb; /* avoid compiler warning */

    CHECK_PORT_PBUS(vdimod,vdiPort,Port,portList);
    return (VdiObj*)port->net;
}

/* --------------------------------------------------------------------
 * conInst - return connected net or NULL
 * --------------------------------------------------------------------
 */
static VdiObj* conInst(VdiDB* vdidb, VdiObj* vdimod, VdiObj* vdiInst, int pno) {
    Inst* inst = (Inst*)vdiInst;
    (void)vdidb; /* avoid compiler warning */

    CHECK_NET_NBUN_INST(vdimod,vdiInst,Inst,instList);
    if(pno < 0 || pno >= PINCOUNT(inst)) return NULL;	/* 3-pin transistor...*/
    return (VdiObj*)PIN(inst,pno).net;
}

/* --------------------------------------------------------------------
 * portType - return user-defined port symbol shape (inline)
 * --------------------------------------------------------------------
 */
static void portType(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiPort,
		     const char** snP, const char** svnP, const char** symdefP)
{
    Port* port = (Port*)vdiPort;
    Module* mod = (Module*)vdiMod;
    (void)vdidb; /* avoid compiler warning */
    (void)port;  /* avoid compiler warning */
    (void)mod;   /* avoid compiler warning */

    CHECK_PORT_PBUS(vdiMod,vdiPort,Port,portList);

#ifdef CONCEPT_DEBUGGING
    if (strcmp(mod->c.name,"ai12")==0) {
      if (strcmp(port->name,"B")==0) {
	*snP     = DBGCOPY( "symdef customInPort for B");
	*svnP    = DBGCOPY2("");
	*symdefP = DBGCOPY3("DEF path 10 -10 10 10 \
		       port ignored right -loc 30 0 10 0 \
		       attrdsp @cell -lr 6 5 10 \
		       attrdsp @name -lc 18 -9 10");
      } else if (strcmp(port->name,"C")==0) {
	*snP     = DBGCOPY( "in");
	*svnP    = DBGCOPY2("");
      } else if (strcmp(port->name,"X")==0) {
	*snP     = DBGCOPY( "out");
	*svnP    = DBGCOPY2("");
      } else if (strcmp(port->name,"Z")==0) {
	*snP     = DBGCOPY( "inout");
	*svnP    = DBGCOPY2("");
      }
    }
#else
    (void)snP;     /* avoid compiler warning */
    (void)svnP;    /* avoid compiler warning */
    (void)symdefP; /* avoid compiler warning */
#endif /* CONCEPT_DEBUGGING */
}

/* --------------------------------------------------------------------
 * pbusType - return user-defined pbus symbol shape (inline)
 * --------------------------------------------------------------------
 */
static void pbusType(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiPortBus,
		     const char** snP, const char** svnP, const char** symdefP)
{
    PortBus* pbus = (PortBus*)vdiPortBus;
    Module* mod = (Module*)vdiMod;
    (void)vdidb; /* avoid compiler warning */
    (void)pbus;  /* avoid compiler warning */
    (void)mod;   /* avoid compiler warning */

    CHECK_PORT_PBUS(vdiMod,vdiPortBus,PortBus,portBusList);

#ifdef CONCEPT_DEBUGGING
    if (strcmp(mod->c.name,"ai05")==0 && strcmp(pbus->name,"I[2:0]")==0) {
	*snP     = DBGCOPY("symdef customInPortBus for I[2:0]");
	*svnP    = DBGCOPY2("");
	*symdefP = DBGCOPY3("Def fpath 10 -10 20 0 10 10 0a 0a 10 -10 \
		       port ignored right -loc 20 0 20 0 \
		       attrdsp @name -lc 18 -9 10 \
		       attrdsp @cell -uc 18 9 10 \
		       fillcolor 3");
    }
#else
    (void)snP;     /* avoid compiler warning */
    (void)svnP;    /* avoid compiler warning */
    (void)symdefP; /* avoid compiler warning */
#endif /* CONCEPT_DEBUGGING */
}

/* --------------------------------------------------------------------
 * pgnetType - return user-defined power/ground symbol shape (inline)
 * --------------------------------------------------------------------
 */
static void pgnetType(VdiDB* vdidb, VdiObj* vdiMod, VdiObj* vdiNet,
		     const char** snP, const char** svnP, const char** symdefP)
{
    Net* net = (Net*)vdiNet;
    Module* mod = (Module*)vdiMod;
    (void)vdidb; /* avoid compiler warning */
    (void)mod;   /* avoid compiler warning */
    (void)net;   /* avoid compiler warning */

#ifdef CONCEPT_DEBUGGING
    if (strcmp(mod->c.name,"ai12")==0) {
      if (strcmp(net->name,"vcc")==0) {
	*snP     = DBGCOPY( "symdef for vcc");
	*svnP    = DBGCOPY2("");
	*symdefP = DBGCOPY3("-DEF fpath 0 0 -5 -25 5 -25 0 0 \
		       port ignored bottom -loc 0 0 \
		       attrdsp @pgtype -lc 0 -27 10");
      } else if (strcmp(net->name,"0")==0) {
	*snP     = DBGCOPY( "gnd");
	*svnP    = DBGCOPY2("");
      }
    }
#else
    (void)snP;     /* avoid compiler warning */
    (void)svnP;    /* avoid compiler warning */
    (void)symdefP; /* avoid compiler warning */
#endif /* CONCEPT_DEBUGGING */
}

/* ====================================================================
 * tdbAccessFunc - the main function pointer list.
 * ====================================================================
 */
static struct VdiAccessFunc tdbAccessFunc = {
    vdiVersion,
    implVersion,

    modFind,
    instFind,
    portFind,
    pbusFind,
    netFind,
    nbunFind,

    imore,
    inext,
    iobj,

    modIter,
    instIter,
    portIter,
    pbusIter,
    netIter,
    nbunIter,
    subIter,

    freeIter,
    moduleObjCount,

    modName,
    instName,
    portName,
    pbusName,
    netName,
    nbunName,

    instAttr,
    portAttr,
    pbusAttr,
    netAttr,
    nbunAttr,

    downModule,
    primName,
    modType,
    primType,
    primFlags,
    instFlags,
    instValue,
    portFlags,
    netFlags,

    cmore,
    cnext,
    cnet,
    NULL,	/* CnetOOM is not implemented */
    ccomp,
    NULL,	/* CcompOOM is not implemented */
    cpinnumber,
    cpinname,
    cpbusname,
    cportflags,
    cmembers,
    cpinattr,

    conIterNet,
    NULL,	/* conIterNetOOM is not implemented */
    conIterInst,
    NULL,	/* conIterInstArcs is not implemented */
    conIterPBus,
    freeCiter,
    conPort,
    conInst,

    portType,
    pbusType,
    pgnetType
};

struct VdiAccessFunc* VdiImpl = &tdbAccessFunc;
