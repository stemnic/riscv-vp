/*  gdump.c 1.53 2018/10/15

    Copyright 2003-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	Dump Data from Graphics Export Interface

    Author:
	Lothar Linhard

    Description:
	Gdump dumps the GEI 1:1 into the given dump file.
	It maps all the gei.h enums into readable labels
	and prints indented lines; the indentation represents
	the nesting level of the iterators (the depth of the
	containment tree).

	More docs in gdump.h
    ===========================================================================
*/

#ifdef __cplusplus
#error "__FILE__ is pure C, do NOT compile with C++ !"
#endif

#include "gdump.h"
#include "gei.h"
#include "gassert.h"
#include "ghash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _MSC_VER
    /* 'sprintf': This function or variable may be unsafe (_MSC_VER >= 1400) */
#   pragma warning(disable:4996)
#endif

/* ===========================================================================
 * Define bool/true/false
 * ===========================================================================
 */
typedef int bool;
#define false 0
#define true  1



/* ===========================================================================
 * global variables for GEI Dump
 * ===========================================================================
 */
static FILE* Dump  = NULL;		/* dump-file or stdout, set in Start */
static int   Dump2Stdout = 0;		/* >0, if Dump==stdout, set in Start */
static int   ModuleCount = 0;		/* incremented by each GexDumpModule */
static bool  DumpJoin = 0;		/* not GexDumpFNoJoin */
static bool  DumpConn = 0;		/* not GexDumpFNoConn */
static bool  DumpShape = 0;		/* not GexDumpFNoShape */
static bool  DumpPlace = 0;		/* not GexDumpFNoPlace */
static bool  DumpWire = 0;		/* not GexDumpFNoWire */
static bool  DumpVerify = 0;		/* not GexDumpFNoVerify */
static bool  DumpInstPrefix = 0;	/* GexDumpFInstPrefix */
static bool  DumpCompass = 0;		/* GexDumpFCompass */
static char  LastErrorMsg[256];		/* buffer for error messages */

#define Obj   struct GeiObject
#define Iter  struct GeiIter
typedef const struct GeiAccessFunc Gei;


/* ---------------------------------------------------------------------------
 * AttrNameMap - hash table for AttrNames
 * ---------------------------------------------------------------------------
 */
struct AttrName {
    const char*	name;		/* name of attribute */
    int		nameLen;	/* because name may not be not null-terminated*/
};
static int cmpAttrName(const struct AttrName a1, const struct AttrName a2)
{
    register const char* n1 = a1.name;
    register const char* n2 = a2.name; 
    int l, d = a1.nameLen - a2.nameLen;

    if (d!=0) for(l=a1.nameLen; l; n1++, n2++, l--) {
	if (*n1 != *n2) {
	    return *(const unsigned char *)n1 - *(const unsigned char *)n2;
	}
    }
    return d;
}
static unsigned hashAttrName(const struct AttrName a)
{
    register const char* key = a.name;
    register unsigned hash = 0;
    if (key) for (;;) {
        if (hash&0xf0000000U) hash %= 0x00fffffdU; /* 16777213: prime near 16M*/
        hash <<= 4;
        hash += *key++;
	if (key - a.name == a.nameLen) break;
    }
    return hash;
}
declareHash(  static, AttrNameMap, struct AttrName, int)
implementHash(static, AttrNameMap, struct AttrName, int)


/* ---------------------------------------------------------------------------
 * sType      - return string for given GeiSymType
 * pType      - return string for given GeiPointType
 * oType      - return string for given GeiSymOrient
 * ripType    - return string for given GeiRipType
 * ---------------------------------------------------------------------------
 */
static const char* sType(enum GeiSymType type)
{
    static const struct {
	const char*      label;
	enum GeiSymType type;
    } t[] = {
	{ "",         GeiSymTInst      },
	{ "HIER",     GeiSymTHier      },
	{ "IN",       GeiSymTINPORT    },
	{ "OUT",      GeiSymTOUTPORT   },
	{ "INOUT",    GeiSymTINOUTPORT },
	{ "PAGEIN",   GeiSymTPAGEIN    },
	{ "PAGEOUT",  GeiSymTPAGEOUT   },
	{ "PAGEINOUT",GeiSymTPAGEINOUT },
	{ "POWER",    GeiSymTPOWER     },
	{ "GROUND",   GeiSymTGROUND    },
	{ "NEGPOWER", GeiSymTNEGPOWER  }
    };
    assert(type < sizeof(t)/sizeof(t[0]) && type == t[type].type);
    return t[type].label;
}

static const char* pType(enum GeiPointType type)
{
    static const struct {
	const char*       label;
	enum GeiPointType type;
    } t[17] = {
	{ NULL,		0               },
	{ "BegPin",	GeiPointTBegPin }, /* Begin wire at a pin */
    	{ "EndPin",	GeiPointTEndPin }, /* End a wire at a pin */
	{ "MidPin",	GeiPointTMidPin }, /* Pin on wire mid-point */
    	{ "C",		GeiPointTCorner }, /* Corner at 2 intersecting segms */
    	{ "MidT",	GeiPointTMidT   }, /* T junction in middle of a segm */
    	{ "BegT",	GeiPointTBegT   }, /* Begin wire at a T junction (I) */
    	{ "EndT",	GeiPointTEndT   }, /* End wire at a T junction (I) */
    	{ "NoC",	GeiPointTNoCorner},/* Additional vertex, new flags? */
    	{ "",		0   },
    	{ "",		0   },
    	{ "BegW",	GeiPointTBegWire}, /* Begin wire anywhere, for Symbol */
    	{ "MidA",	GeiPointTMidArc }, /* Middle of an Arc, for Symbol */
    	{ "EndW",	GeiPointTEndWire}, /* End wire anywhere, for Symbol */
    	{ "Dot",	GeiPointTDot    }, /* Dot */
    	{ "RipL",	GeiPointTRipL   }, /* Ripper to Left Side */
    	{ "RipR",	GeiPointTRipR   }  /* Ripper to Right Side */
    };
    assert(type < 17 && type == t[type].type);
    return t[type].label;
}
static const char* oType(enum GeiSymOrient type)
{
    static const struct {
	const char*      label;
	enum GeiSymOrient type;
    } t[8] = {
	{ "R0",		GeiSymOrientR0    },
	{ "MY",		GeiSymOrientMY    },
	{ "MX",		GeiSymOrientMX    },
	{ "R180",	GeiSymOrientR180  },
	{ "MYR90",	GeiSymOrientMYR90 },
	{ "R90",	GeiSymOrientR90   },
	{ "R270",	GeiSymOrientR270  },
	{ "MXR90",	GeiSymOrientMXR90 }
    };
    assert(type == t[type].type);
    return t[type].label;
}
static const char* ripType(enum GeiRipType type)
{
    static const struct {
	const char*     label;
	enum GeiRipType type;
    } t[4] = {
	{ "Triangle",		GeiRipTTriangle    },
	{ "TriangleBus",	GeiRipTTriangleBus },
	{ "Slash",		GeiRipTSlash       },
	{ "SlashBus",		GeiRipTSlashBus    }
    };
    assert(type == t[type].type);
    return t[type].label;
}
static const char* lineStyle(enum GeiLineStyle style)
{
    static const struct {
	const char*     label;
	enum GeiLineStyle style;
    } t[4] = {
	{ "default",	GeiLineSDefault},
	{ "solid",	GeiLineSSolid},
	{ "dashed",	GeiLineSDashed},
	{ "dashed2",	GeiLineSDashed2}
    };
    assert(style == t[style].style);
    return t[style].label;
}

/* ---------------------------------------------------------------------------
 * pSymAttr   - print GeiSymAttrInfo to Dump file
 * pAttr      - print GeiAttrInfo to Dump file, skip already exported ones
 * pInst      - print GeiInstInfo to Dump file (for inst def and ref)
 * pJoin      - print GeiJoinInfo to Dump file
 * ---------------------------------------------------------------------------
 */
static void pSymAttr(int indent, const struct GeiSymAttrInfo* ai)
{
    while(indent >= 8) {putc('\t', Dump); indent-= 8;}
    while(indent >= 1) {putc(' ',  Dump); indent--; }
    if (DumpShape) {
	fprintf(Dump, 
	    "sattr %s (%d,%d) just=%d vert=%d text=%d marks=%d additional=%d\n",
	    ai->name, ai->x, ai->y, (int)ai->just,
	    ai->vertical, ai->text, ai->marks, ai->additional);
    } else {
	fprintf(Dump, "sattr %s text=%d marks=%d additional=%d\n", 
			ai->name, ai->text, ai->marks, ai->additional);
    }
}

static void pAttr(int indent, const char* type, const struct GeiAttrInfo* ai,
	struct AttrNameMap* alreadyExported)
{
    int   len = ai->nameLen;
    char* nl;
    char  name[128];

    if (alreadyExported) {
	/* Ignore already exported attributes using a hash table. */
	struct AttrName aname;
	int notUsed = 0;
	aname.name = ai->name;
	aname.nameLen = len;
	if (AttrNameMapFind(alreadyExported, aname, &notUsed)) return;

	AttrNameMapInsert(alreadyExported, aname, &notUsed);
    }

    if (len > 128-1) len = 128-1;
    strncpy(name, ai->name, len);
    name[len] = '\0';

    while(indent >= 8) {putc('\t', Dump); indent-= 8;}
    while(indent >= 1) {putc(' ',  Dump); indent--; }
    nl = strchr(ai->value, '\n');
    if (nl) {
	char  val[128];
	len = (int)(nl-ai->value);
	if (len > 128-1) len = 128-1;
	strncpy(val, ai->value, len);
	val[len] = '\0';
	fprintf(Dump,"%s %s = %s\\n...", type, name, val);
    } else {
	fprintf(Dump,"%s %s = %s", type, name, ai->value);
    }
    if (ai->fmtForeground[0]) fprintf(Dump, ", fg=%s", ai->fmtForeground);
    if (ai->fmtBackground[0]) fprintf(Dump, ", bg=%s", ai->fmtBackground);
    if (ai->fmtFontscale!=1.0F) fprintf(Dump,", sc=%f", ai->fmtFontscale);
    if (ai->fmtUserData[0])   fprintf(Dump, ", user=%s", ai->fmtUserData);
    putc('\n', Dump);
}

static void pInst(Gei* g, struct GeiInstInfo* ii, bool def)
{
    struct GeiSymInfo symi;
    if (def) {
	fprintf(Dump,"   ");	/* indent */
	if (DumpInstPrefix) fprintf(Dump," inst");
    }
    assert(ii->sym);
    g->symInfo(ii->sym, &symi);
    fprintf(Dump," %s[%s%s]", sType(symi.type), symi.name, symi.vname);

    if (def) {
	char scalebuf[64];
	scalebuf[0] = '\0';
	if (symi.scale != 1.0F) {
	    sprintf(scalebuf, " scale=%g", symi.scale);
	}
	fprintf(Dump," %s%s%s %s", oType(symi.orient),
		    scalebuf,
		    symi.privatized ? " privatized" : "",
		    ii->name);

	if (DumpPlace) {
	    if (symi.master) {
		fprintf(Dump," (%d%+d,%d%+d)",
		    ii->x, symi.xmove, ii->y, symi.ymove);
	    } else {
		fprintf(Dump," (%d,%d)", ii->x, ii->y);
	    }
	}
    } else {
	fprintf(Dump," %s", ii->name);
    }
    fprintf(Dump, "\n");
}

static void pJoin(Gei* g, struct GeiJoinInfo* ji)
{
    struct GeiNetInfo ni;

    assert(ji->net_or_nbun);
    g->netInfo(ji->net_or_nbun, &ni);

    if (ji->subnetIdx == -1) {
	assert(ni.busWidth == 0);
	fprintf(Dump,"join net %s", ni.name);
    } else {
	assert(ji->subnetIdx < (int)ni.busWidth);
	fprintf(Dump,"join #%d of netBundle %s (%s)",
	    ji->subnetIdx, ni.name, ji->subname);
    }
    fprintf(Dump,"\n");
}

/* ---------------------------------------------------------------------------
 * isConnector - return true for connector instances and false for an ordinary
 *		 instance (GeiSymTInst or GeiSymTHier).  It also returns:
 *		 "pin/hierPin/pinBus/hierPinBus" (for ordinary instance pins)
 *		 "port/portBus" (for IO connectors)
 *		 "/Bus" (for other connectors, like PAGEIN, POWER, etc).
 *
 *		 This function is used to dump instance pins and
 *		 connector's dummy-pin (no name).
 * ---------------------------------------------------------------------------
 */
static bool isConnector(enum GeiSymType type, bool ishier, bool isbus,
    char* label, bool portMode)
{
    const char* t = "";
    switch (type) {
	case GeiSymTInst:
	    assert(!ishier);
	    /*FALLTHRU*/
	case GeiSymTHier:
	    /* ordinary instance pin/pinBus */
	    sprintf(label, isbus ? "%sBus" : "%s", ishier ? "hierPin" : "pin");
	    return false;

	case GeiSymTINPORT:
	case GeiSymTOUTPORT:
	case GeiSymTINOUTPORT:
	    t = "port";
	default:
	    /* connector/bus-connector */
	    assert(!ishier);
	    sprintf(label, isbus ? "%sBus" : "%s", portMode ? t : "conn");
	    return true;
    }
}

/* ---------------------------------------------------------------------------
 * verifyJoin	verify IjoinInfo against IconnInfo (slow O(n*n) check)
 *		
 *		Current pin at (inst, pi, subpinIdx) - IjoinInfo --> "ji"
 *		Search ji->net_or_nbun using connIter/bconnIter and check
 *		if there is exactly one ConnInfo that points back to the
 *		current pin.
 * ---------------------------------------------------------------------------
 */
static bool verifyJoin(Gei* g, struct GeiJoinInfo* ji,
    Obj* inst, struct GeiPinInfo* pi, int subpinIdx)
{
    int   found = 0;
    struct GeiNetInfo ni;

    assert(ji->net_or_nbun);
    g->netInfo(ji->net_or_nbun, &ni);

    if (ji->subnetIdx == -1) {		/* single-bit net */
	Iter* it;
	assert(ni.busWidth == 0);
	for (it=g->connIter(ji->net_or_nbun); g->Imore(it); g->Inext(it)) {
	    struct GeiConnInfo ci;
	    g->IconnInfo(it, &ci);
	    if (ci.inst == inst &&
		ci.name == pi->name &&
		ci.hier == pi->hier &&
		ci.hidden == pi->hidden &&
		ci.subpinIdx == subpinIdx &&
		ci.subname == ji->subpinname) found++;
	}
	g->freeIter(it);
    } else {				/* sub-net of a netBundle */
	unsigned bit;
	assert(ji->subnetIdx < (int)ni.busWidth);
	for (bit=0; bit<ni.busWidth; bit++) {
	    Iter* it;
	    Obj*  nbun = ji->net_or_nbun;
	    const char* sn;
	    for (it=g->bconnIter(nbun,bit,&sn); g->Imore(it); g->Inext(it)) {
		struct GeiConnInfo ci;
		g->IconnInfo(it, &ci);
		if (ci.inst == inst &&
		    ci.name == pi->name &&
		    ci.hier == pi->hier &&
		    ci.hidden == pi->hidden &&
		    ci.subpinIdx == subpinIdx &&
		    ci.subname == ji->subpinname) found++;
	    }
	    g->freeIter(it);
	}
    }

    if (found != 1) {					/* LCOV_EXCL_START */
	struct GeiInstInfo ii;
	g->instInfo(inst, &ii);
	if (subpinIdx == -1) sprintf(LastErrorMsg,
		"inst %.50s pin %.20s", ii.name, pi->name);
	else sprintf(LastErrorMsg,
		"inst %.50s %.20s (#%d of pinBus %.20s)",
		ii.name, ji->subpinname, subpinIdx, pi->name);

	if (ji->subnetIdx == -1) sprintf(LastErrorMsg+strlen(LastErrorMsg),
		" join net \"%.50s\" but %d reverse conn", ni.name, found);
	else sprintf(LastErrorMsg+strlen(LastErrorMsg),
		" join %.50s (#%d of netBundle %.50s) but %d reverse conn",
		ji->subname, ji->subnetIdx, ni.name, found);

	return false;
    }							/* LCOV_EXCL_STOP */
    return true;
}

/* ---------------------------------------------------------------------------
 * verifyConn	verify IconnInfo against IjoinInfo
 *		
 *		Current net (net_or_nbun, subnetIdx, subnetname) at conn "ci"
 *		Search ci->inst with pinIter for the pin/pinBus that maches
 *		ci->name and then use IjoinInfo to get the connected net
 *		(JoinInfo's net_or_nbun, subnetIdx, subname) and check if this
 *		is identical to the given procedure arguments.
 * ---------------------------------------------------------------------------
 */
static bool verifyConn(Gei* g, struct GeiConnInfo* ci,
    Obj* net_or_nbun, int subnetIdx, const char* subnetname)
{
    int   found1 = 0;
    int   found2 = 0;
    int   match  = 0;
    Iter* it;

    assert(ci->inst);
    for (it=g->pinIter(ci->inst); g->Imore(it); g->Inext(it)) {
	struct GeiPinInfo pi;
	g->IpinInfo(it, &pi);
	if (pi.name == ci->name &&
	    pi.hier == ci->hier &&
	    pi.hidden == ci->hidden &&
	    (ci->subpinIdx==-1) == (pi.busWidth==0)) {
	    struct GeiJoinInfo ji;

	    found1++;
	    assert(ci->subpinIdx < (int)pi.busWidth);

	    g->IjoinInfo(it, ci->subpinIdx, &ji);
	    if (ji.subpinname  == ci->subname) found2++;

	    if (ji.net_or_nbun == net_or_nbun &&
		ji.subnetIdx   == subnetIdx   &&
		ji.subname     == subnetname) match++;
	}
    }
    g->freeIter(it);

    if (found1 != 1 || found2 != 1 || match != 1) {	/* LCOV_EXCL_START */
	struct GeiNetInfo ni;
	struct GeiInstInfo ii;

	g->netInfo(net_or_nbun, &ni);
	g->instInfo(ci->inst,   &ii);

	if (subnetIdx == -1) sprintf(LastErrorMsg, "net %.50s", ni.name);
	else sprintf(LastErrorMsg, "%.50s (#%d of netBundle %.50s)",
		subnetname, subnetIdx, ni.name);

	if (ci->subpinIdx == -1) sprintf(LastErrorMsg+strlen(LastErrorMsg),
		" conn pin \"%.50s\" \"%.50s\"", ci->name, ii.name);
	else sprintf(LastErrorMsg+strlen(LastErrorMsg),
		" conn %.50s (#%d of pinBus %.50s) \"%.50s\"",
		ci->subname, ci->subpinIdx, ci->name, ii.name);

	sprintf(LastErrorMsg+strlen(LastErrorMsg),
	    " but (%d,%d,%d) reverse join", found1, found2, match);
	return false;
    }							/* LCOV_EXCL_STOP */
    return true;
}

static bool expWire(  Gei*, const struct GeiPointInfo*, bool);
static bool expRipper(Gei*, const struct GeiRipInfo*, int);


/* ===========================================================================
 * expSymbol - export the given symbol to Dump
 * ===========================================================================
 */
static bool expSymbol(Gei* g, Obj* sym) {
    Iter*             it;
    struct GeiSymInfo symi;
    bool	      filled = false;
    g->symInfo(sym, &symi);
    assert(!symi.privatized && !symi.master);
    if (symi.familyRefCount == 0) return true;

    fprintf(Dump,"  symbol %s[%s%s]\n",
	    sType(symi.type), symi.name, symi.vname);

    /* export each symbol pin */
    for (it=g->spinIter(sym); g->Imore(it); g->Inext(it)) {
	Iter* ait;
        struct GeiSymPinInfo spin;
        g->IspinInfo(it,  &spin);
	if (spin.busWidth) {
	    fprintf(Dump,"    spinBus[%d] %s", spin.busWidth, spin.name);
	} else {
	    fprintf(Dump,"    spin %s", spin.name);
	}
	if (DumpPlace) fprintf(Dump," (%d,%d)", spin.x, spin.y);
	if (DumpShape) {
	    /* add non-default symbol pin linewidth, linestyle and linecolor */
	    if (spin.linewidth != -1)
		fprintf(Dump," linewidth=%d", spin.linewidth);
	    if (spin.linestyle != GeiLineSDefault)
		fprintf(Dump," linestyle=%s", lineStyle(spin.linestyle));
	    if (spin.linecolor)
		fprintf(Dump," linecolor=boxcolor%d", spin.linecolor);
	}
	fprintf(Dump,"\n");

	for (ait=g->spattrIter(g->Iobj(it));g->Imore(ait);g->Inext(ait)){
	    struct GeiSymAttrInfo sai;
	    g->IsattrInfo(ait, &sai);
	    pSymAttr(6, &sai);
	}
	g->freeIter(ait);

	if (spin.polygon_len>0) {
	    unsigned i;
	    for (i=0; i<spin.polygon_len; i++)
		expWire(g, spin.polygon+i, false);
	}
    }
    g->freeIter(it);

    /* export symbol attributes
     */
    for (it=g->sattrIter(sym); g->Imore(it); g->Inext(it)) {
        struct GeiSymAttrInfo sattr;
        g->IsattrInfo(it,  &sattr);
	pSymAttr(4, &sattr);
    }
    g->freeIter(it);

    /* export symbol properties
     */
    for (it=g->spropIter(sym); g->Imore(it); g->Inext(it)) {
        struct GeiAttrInfo sprop;
        g->IspropInfo(it,  &sprop);
	pAttr(4, "sprop", &sprop, NULL);
    }
    g->freeIter(it);

    /* export symbol shape polygons (point by point)
     */
    if (DumpShape) {
      /* add non-default symbol linewidth, linestyle and linecolor */
      if (symi.linewidth!=-1)
	fprintf(Dump,"    linewidth=%d\n", symi.linewidth);
      if (symi.linestyle!=GeiLineSDefault)
	fprintf(Dump,"    linestyle=%s\n", lineStyle(symi.linestyle));
      if (symi.linecolor)
	fprintf(Dump,"    linecolor=boxcolor%d\n", symi.linecolor);
      for (it=g->shapeIter(sym); g->Imore(it); g->Inext(it)) {
        struct GeiPointInfo point;
        g->IshapeInfo(it,  &point);
	switch (point.type) {
	    case GeiPointTBegPin:
	    case GeiPointTBegWire:
	    case GeiPointTBegT:
		filled = point.filled;
		fprintf(Dump,"    polygon filled=%d, %s(%d,%d)",
			filled, pType(point.type), point.x, point.y);
		break;

	    case GeiPointTEndPin:
	    case GeiPointTEndWire:
	    case GeiPointTEndT:
		if (filled != (bool)point.filled) goto bad;
		fprintf(Dump,", %s(%d,%d)\n",pType(point.type),point.x,point.y);
		break;


	    case GeiPointTCorner:
	    case GeiPointTNoCorner:
	    case GeiPointTMidPin:
	    case GeiPointTMidT:
	    case GeiPointTMidArc:
		if (filled != (bool)point.filled) goto bad;
		fprintf(Dump,", %s(%d,%d)",pType(point.type), point.x,point.y);
		break;

	    case GeiPointTDot:
	    case GeiPointTRipL:
	    case GeiPointTRipR:
		goto bad;				/* LCOV_EXCL_LINE */
	}
	continue;

      bad:						/* LCOV_EXCL_START */
	g->freeIter(it);
	sprintf(LastErrorMsg,
	    "symbol %.50s: unexpected point, type=%d, (%d,%d), filled=%d",
	    symi.name, point.type, point.x, point.y, point.filled);
	return false;
      }							/* LCOV_EXCL_STOP */
      g->freeIter(it);
    }
    return true;
}

/* ===========================================================================
 * expPage - export the given schematic page to Dump
 * ===========================================================================
 */
static bool expInst(     Gei* g, Obj* inst);
static bool expNet(      Gei* g, Obj* net);
static bool expNetBundle(Gei* g, Obj* netBundle);
static bool expOverlay(  Gei* g, Iter* cur_overlay);

static bool expPage(Gei* g, Obj* page) {
    Iter*              it;
    struct GeiPageInfo pi;

    g->pageInfo(page, &pi);

    fprintf(Dump,"  page %d l=%d t=%d r=%d, b=%d\n",
	    pi.pageNumber,
	    pi.pageLeft, pi.pageTop, pi.pageRight, pi.pageBottom);

    /* export each instance */
    for (it=g->instIter(page); g->Imore(it); g->Inext(it)) {
	Obj* inst = g->Iobj(it);
	bool ok = expInst(g, inst);
	if (!ok) goto bad;
    }
    g->freeIter(it);

    /* export each net */
    for (it=g->netIter(page); g->Imore(it); g->Inext(it)) {
	Obj* net = g->Iobj(it);
	bool ok = expNet(g, net);
	if (!ok) goto bad;
    }
    g->freeIter(it);

    /* export each netBundle */
    for (it=g->nbunIter(page); g->Imore(it); g->Inext(it)) {
	Obj* nbun = g->Iobj(it);
	bool ok = expNetBundle(g, nbun);
	if (!ok) goto bad;
    }
    g->freeIter(it);

    /* export overlay bundle */
    for (it=g->overIter(page); g->Imore(it); g->Inext(it)) {
	bool ok = expOverlay(g, it);
	if (!ok) goto bad;
    }
    g->freeIter(it);
    return true;

  bad:
    g->freeIter(it);					/* LCOV_EXCL_LINE */
    return false;					/* LCOV_EXCL_LINE */
}

/* ===========================================================================
 * expInst - export the given instance to Dump
 * ===========================================================================
 */
static bool expInst(Gei* g, Obj* inst)
{
    Iter*              it;
    struct GeiInstInfo ii;
    struct AttrNameMap alreadyExported;
    AttrNameMapInit(&alreadyExported, hashAttrName, cmpAttrName);

    g->instInfo(inst, &ii);
    pInst(g, &ii, true);

    /* export each pin */
    for (it=g->pinIter(inst); g->Imore(it); g->Inext(it)) {
	bool isconn;
	char label[12];		/* max "hierPinBus" */
	Iter* ait;
	struct GeiPinInfo pi;
	g->IpinInfo(it, &pi);

	isconn = isConnector(ii.type, pi.hier, pi.busWidth > 0, label, false);

	fprintf(Dump, "      %s", label);
	if (pi.busWidth) fprintf(Dump, "[%d]", pi.busWidth);

	if (isconn) {
	    assert(!pi.name && !pi.swap_name);
	} else {
	    fprintf(Dump," %s", pi.name);
	    if (pi.swap_name) fprintf(Dump," swapped=%s", pi.swap_name);
	}

	/* print joined nets */
	if (pi.busWidth) {
	    struct GeiJoinInfo ji;
	    unsigned           bit;
	    fprintf(Dump,"\n");
	    for (bit=0; bit<pi.busWidth; bit++) {
		g->IjoinInfo(it, bit, &ji);
		if (ji.net_or_nbun && DumpJoin) {
		    fprintf(Dump,"        #%d(%s):", bit, ji.subpinname);
		    pJoin(g, &ji);
		}
		if (ji.net_or_nbun && DumpVerify)
		    if (!verifyJoin(g, &ji, inst, &pi, bit)) return false;
	    }
	} else {
	    struct GeiJoinInfo ji;
	    g->IjoinInfo(it, -1, &ji);
	    assert(!ji.subpinname);
	    if (ji.net_or_nbun && DumpJoin) {
		fprintf(Dump," - ");
		pJoin(g, &ji);
	    } else {
		fprintf(Dump,"\n");
	    }
	    if (ji.net_or_nbun && DumpVerify)
		if (!verifyJoin(g, &ji, inst, &pi, -1)) return false;
	}

	/* export all visible pin attributes and their displays */
	for (ait=g->pattrDspIter(g->Iobj(it),inst);g->Imore(ait);g->Inext(ait)){
	    struct GeiSymAttrInfo sai;
	    struct GeiAttrInfo ai;
	    g->IattrDspInfo(ait, &sai, &ai);
	    /* Note: The SymAttr is exported as is without the ii.x/y offset */
	    if (sai.name && sai.additional) pSymAttr(8, &sai);
	    if (ai.name) pAttr(8, "attr", &ai, &alreadyExported);
	}
	g->freeIter(ait);

	/* export all remaining pin attributes */
	for (ait=g->pattrIter(g->Iobj(it)); g->Imore(ait); g->Inext(ait)) {
	    struct GeiAttrInfo ai;
	    g->IattrInfo(ait, &ai);
	    pAttr(8, "attr", &ai, &alreadyExported);
	}
	g->freeIter(ait);

	AttrNameMapFree(&alreadyExported);	/* Re-init for the next pin */
    }
    g->freeIter(it);

    /* export all visible instance attributes and their displays */
    for (it=g->attrDspIter(inst); g->Imore(it); g->Inext(it)) {
	struct GeiSymAttrInfo sai;
	struct GeiAttrInfo ai;
	g->IattrDspInfo(it, &sai, &ai);
	/* Note: The SymAttr is exported as is without the ii.x/y offset */
	if (sai.name && sai.additional) pSymAttr(6, &sai);
	if (ai.name) pAttr(6, "attr", &ai, &alreadyExported);
    }
    g->freeIter(it);

    /* export all remaining instance attributes */
    for (it=g->attrIter(inst); g->Imore(it); g->Inext(it)) {
	struct GeiAttrInfo ai;
	g->IattrInfo(it, &ai);
	pAttr(6, "attr", &ai, &alreadyExported);
    }
    g->freeIter(it);
    AttrNameMapFree(&alreadyExported);

    return true;
}

/* ===========================================================================
 * expConn - export the given instance pin connection to Dump
 * ===========================================================================
 */
static bool expConn(Gei* g, const struct GeiConnInfo* ci,
    int bitno, const char* bitname)
{
    char label[12];		/* max "hierPinBus" */
    struct GeiInstInfo ii;
    g->instInfo(ci->inst, &ii);

    fprintf(Dump, "      ");
    if (bitno != -1) fprintf(Dump, "#%d(%s):", bitno, bitname);

    if (isConnector(ii.type, ci->hier, ci->subpinIdx != -1, label, true)) {
	if (ci->subpinIdx != -1) {
	    assert(!ci->name);
	    fprintf(Dump, "conn #%d of %s", ci->subpinIdx, label);
	    if (ci->subname) fprintf(Dump," (%s)", ci->subname);
	} else {
	    assert(!ci->name && !ci->subname);
	    fprintf(Dump,"conn %s", label);
	}
    } else {
	if (ci->subpinIdx != -1) {
	    fprintf(Dump,"conn #%d of %s \"%s\" (%s) at",
		    ci->subpinIdx, label, ci->name, ci->subname);
	} else {
	    fprintf(Dump,"conn %s \"%s\" at", label, ci->name);
	}
    }
    pInst(g, &ii, false);
    return true;
}

/* ===========================================================================
 * expNet - export the given net to Dump
 * ===========================================================================
 */
static bool expNet(Gei* g, Obj* net)
{
    Iter*             it;
    struct GeiNetInfo ni;
    struct AttrNameMap alreadyExported;
    AttrNameMapInit(&alreadyExported, hashAttrName, cmpAttrName);

    g->netInfo(net, &ni);
    assert(ni.busWidth == 0);

    fprintf(Dump,"    net %s\n", ni.name);

    /* export all visible net attributes and their displays */
    for (it=g->nattrDspIter(net); g->Imore(it); g->Inext(it)) {
	struct GeiSymAttrInfo sai;
	struct GeiAttrInfo ai;
	g->IattrDspInfo(it, &sai, &ai);
	if (sai.name) pSymAttr(6, &sai);
	if (ai.name) pAttr(6, "attr", &ai, &alreadyExported);
    }
    g->freeIter(it);

    /* export remaining all net attributes */
    for(it=g->nattrIter(net); g->Imore(it); g->Inext(it)) {
	struct GeiAttrInfo ai;
	g->IattrInfo(it, &ai);
	pAttr(6, "attr", &ai, &alreadyExported);
    }
    g->freeIter(it);
    AttrNameMapFree(&alreadyExported);

    /* export each connected instance pin */
    for (it=g->connIter(net); g->Imore(it); g->Inext(it)) {
	struct GeiConnInfo ci;
	g->IconnInfo(it, &ci);
	if (DumpConn)   if (!expConn(g, &ci, -1, NULL)) goto bad;
	if (DumpVerify) if (!verifyConn(g, &ci, net, -1, NULL)) goto bad;
    }
    g->freeIter(it);

    /* export net wire polygons (point by point)
     */
    for (it=g->wireIter(net); g->Imore(it); g->Inext(it)) {
	struct GeiPointInfo pi;
	g->IwireInfo(it, &pi);
	if (DumpWire) if (!expWire(g, &pi, false)) goto bad;
    }
    g->freeIter(it);
    return true;

  bad:
    g->freeIter(it);					/* LCOV_EXCL_LINE */
    return false;					/* LCOV_EXCL_LINE */
}

/* ===========================================================================
 * expNetBundle - export the given netBundle to Dump
 * ===========================================================================
 */
static bool expNetBundle(Gei* g, Obj* netBundle)
{
    Iter*	it;
    unsigned	i;
    struct GeiNetInfo ni;
    struct AttrNameMap alreadyExported;
    AttrNameMapInit(&alreadyExported, hashAttrName, cmpAttrName);

    g->netInfo(netBundle, &ni);
    assert(ni.busWidth > 0);

    fprintf(Dump,"    netBundle[%d] %s\n", ni.busWidth, ni.name);

    /* export all visible netbundle attributes and their displays */
    for (it=g->nattrDspIter(netBundle); g->Imore(it); g->Inext(it)) {
	struct GeiSymAttrInfo sai;
	struct GeiAttrInfo ai;
	g->IattrDspInfo(it, &sai, &ai);
	if (sai.name) pSymAttr(6, &sai);
	if (ai.name) pAttr(6, "attr", &ai, &alreadyExported);
    }
    g->freeIter(it);

    /* export all remaining netbundle attributes */
    for(it=g->nattrIter(netBundle); g->Imore(it); g->Inext(it)) {
	struct GeiAttrInfo ai;
	g->IattrInfo(it, &ai);
	pAttr(6, "attr", &ai, &alreadyExported);
    }
    g->freeIter(it);
    AttrNameMapFree(&alreadyExported);

    for (i=0; i<ni.busWidth; i++) {
	const char* sn;
	/* export each connected instance pin */
	for (it=g->bconnIter(netBundle,i,&sn); g->Imore(it); g->Inext(it)) {
	    struct GeiConnInfo ci;
	    g->IconnInfo(it, &ci);
	    if (DumpConn)   if (!expConn(g, &ci, i, sn)) goto bad;
	    if (DumpVerify) if (!verifyConn(g, &ci, netBundle, i, sn)) goto bad;
	}
	g->freeIter(it);
    }

    /* export netBundle wire polygons (point by point)
     */
    for (it=g->wireIter(netBundle); g->Imore(it); g->Inext(it)) {
	struct GeiPointInfo pi;
	g->IwireInfo(it, &pi);
	if (DumpWire) if (!expWire(g, &pi, false)) goto bad;
    }
    g->freeIter(it);

    /* export each sub-net connecting at a ripper */
    for (it=g->ripIter(netBundle); g->Imore(it); g->Inext(it)) {
	Iter*             wit;
	struct GeiRipInfo ri;
	g->IripInfo(it, &ri);
	if (!expRipper(g, &ri, 8)) goto bad;

	if (ri.countDown > 0) continue;
	/* Only once at each sub-bus ripper (if countDown = 0)
	 * export netBundle's subnet wire polygons (point by point)
	 */
	for (wit=g->subWIter(it); g->Imore(wit); g->Inext(wit)) {
	    struct GeiPointInfo pi;
	    g->IsubWInfo(wit, &pi);
	    if (DumpWire) if (!expWire(g, &pi, true)) goto bad;
	}
	g->freeIter(wit);
    }
    g->freeIter(it);
    return true;

  bad:
    g->freeIter(it);					/* LCOV_EXCL_LINE */
    return false;					/* LCOV_EXCL_LINE */
}

/* ===========================================================================
 * expOverlay - export given overlay bus to Dump
 * ===========================================================================
 */
static bool expOverlay(Gei* g, Iter* cur_overlay)
{
    Iter* ovWit;
    Iter* ovRit;
    struct GeiOverInfo oi;

    fprintf(Dump,"    overlay at:");
    g->IoverInfo(cur_overlay, &oi);

    assert(oi.inst);
    {
	char label[12];
	struct GeiInstInfo ii;
	g->instInfo(oi.inst, &ii);

	if (isConnector(ii.type, oi.hier, oi.busWidth > 0, label, false)) {
	    assert(!oi.name && !oi.hier);
	    fprintf(Dump," %s[%d] at", label, oi.busWidth);
	} else {
	    fprintf(Dump," %s[%d] \"%s\" at", label, oi.busWidth, oi.name);
	}
	pInst(g, &ii, false);

	assert(oi.busWidth > 0);
    }

    /* nested ovWIter - print wire info at a certain "overlay" bus */
    for (ovWit=g->ovWIter(cur_overlay); g->Imore(ovWit); g->Inext(ovWit)) {
	struct GeiPointInfo pti;
	g->IovWInfo(ovWit, &pti);
	if (DumpWire) if (!expWire(g, &pti, false)) return false;
    }
    g->freeIter(ovWit);

    /* nested ovRIter - loops over the GeiRipTSlash rippers of bus */
    for (ovRit=g->ovRIter(cur_overlay); g->Imore(ovRit); g->Inext(ovRit)) {
	struct GeiRipInfo ri;
	g->IovRInfo(ovRit, &ri);
	if (!expRipper(g, &ri, 8)) return false;
    }
    g->freeIter(ovRit);
    return true;
}

/* ===========================================================================
 * expWire - export one net-wire point (part of Wire info)
 * ===========================================================================
 */
static bool expWire(Gei* g, const struct GeiPointInfo* pi, bool sub)
{
    (void)g; /* please compiler */
    fprintf(Dump,"%s %s(%d,%d)\n",
	sub ? "\t  pt":"      pt", pType(pi->type), pi->x, pi->y);
    return true;
}

/* ===========================================================================
 * expRipper - export one ripper (GeiRipInfo struct)
 * ===========================================================================
 */
static bool expRipper(Gei* g, const struct GeiRipInfo* ri, int indent)
{
    (void)g; /* please compiler */
    while(indent >= 8) {putc('\t', Dump); indent-= 8;}
    while(indent >= 1) {putc(' ',  Dump); indent--; }
    if (DumpWire) fprintf(Dump,
	"rip %s (%d,%d) --> (%d,%d)", 
	ripType(ri->type), ri->xBus, ri->yBus, ri->xSubnet, ri->ySubnet);

    if (ri->net_or_nbun) {
	/* from ovRIter */
	struct GeiNetInfo ni;
	g->netInfo(ri->net_or_nbun, &ni);
	if (ri->subnetIdx == -1) {
	    /* Slash single-bit net */
	    fprintf(Dump, " \"%s\" countDown=%d\n", ni.name, ri->countDown);
	    assert(ni.busWidth == 0);
	} else {
	    /* Slash other bus's sub-net */
	    fprintf(Dump, " #%d of \"%s\" (%s) countDown=%d\n",
		ri->subnetIdx, ni.name, ri->subname, ri->countDown);
	    assert(ri->subnetIdx < (int)ni.busWidth);
	}
    } else {
	/* from ripIter */
	/* Triangle or Slash my subnet */
	char compass[6];
	int  len = 0;

	if (DumpCompass) {
	    compass[len++] = ' ';
	    if (ri->compass & 8/*NORTH*/) compass[len++] = 'N';
	    if (ri->compass & 4/*SOUTH*/) compass[len++] = 'S';
	    if (ri->compass & 2/*EAST*/)  compass[len++] = 'E';
	    if (ri->compass & 1/*WEST*/)  compass[len++] = 'W';
	}
	compass[len++] = '\0';

	fprintf(Dump, " subnet#%d (%s) countDown=%d%s\n",
	    ri->subnetIdx, ri->subname, ri->countDown, compass);
    }
    return true;
}


/* ===========================================================================
 * GEI Dump Main Functions:
 * ========================
 *
 * GexDumpStart -      Start Dump export, open dump file and
 *                     initialize all variables.
 * 
 * GexDumpModule -     Dump the given module thru Nlview's GEI; this
 *                     function is called once for each hierarchical module.
 * 
 * GexDumpEnd -        Finish Dump export, cleanup local variables and
 *                     close the file.
 * ===========================================================================
 */
int/*bool*/ GexDumpStart(const char* dumpfile, unsigned flags) {

    DumpJoin       = (flags & GexDumpFNoJoin)     ? false : true;
    DumpConn       = (flags & GexDumpFNoConn)     ? false : true;
    DumpShape      = (flags & GexDumpFNoShape)    ? false : true;
    DumpPlace      = (flags & GexDumpFNoPlace)    ? false : true;
    DumpWire       = (flags & GexDumpFNoWire)     ? false : true;
    DumpVerify     = (flags & GexDumpFNoVerify)   ? false : true;
    DumpInstPrefix = (flags & GexDumpFInstPrefix) ? true : false;
    DumpCompass    = (flags & GexDumpFCompass)    ? true : false;

    if (Dump) GexDumpEnd(); /* end missing, we do it here automatically */

    Dump2Stdout = (strcmp(dumpfile, "-") == 0 ? 1 : 0);
    if (Dump2Stdout) {	/* use stdout as output */
	Dump = stdout;
    } else {
	Dump = fopen(dumpfile,  "w");
	if (!Dump) {
	    sprintf(LastErrorMsg, "%.100s: %.100s",dumpfile, strerror(errno));
	    return false;
	}
    }
    ModuleCount = 0;
    return true;
}

/* ===========================================================================
 * GEI Dump module
 * ===========================================================================
 */
int/*bool*/ GexDumpModule(const char* module, Gei* g,const char** params) {
    Iter* it;

    (void)params;
    if(!Dump) {
	sprintf(LastErrorMsg, "\"start\" missing");
	return false;
    }
    if (g->magic != GeiMagic) {				/* LCOV_EXCL_START */
	sprintf(LastErrorMsg, "bad magic value in Nlview GEI pointer %p",
		(void*)g);
	return false;
    }
    if (g->geiVersion() != GeiVersion) {
	sprintf(LastErrorMsg, "bad Nlview GEI version %d (%d expected)",
	    g->geiVersion(), GeiVersion);
	return false;
    }							/* LCOV_EXCL_STOP */

    ModuleCount++;
    fprintf(Dump,"Exporting Module %d %.50s\n", ModuleCount, module);

    /* Loop over all symbols in current module. For the very first module
     * (ModuleCount == 1) call symIter(1) else call symIter(0); this
     * makes symIter return all stub symbols (IO, OffPage, Power, etc)
     * only once.
     * If symIter returns NULL, then "begGraphicExp" was not called before.
     */
    it = g->symIter(ModuleCount==1);
    if (!it) {
	sprintf(LastErrorMsg, "Nlview command \"begGraphicExp\" not called");
	return false;
    }
    for (; g->Imore(it); g->Inext(it)) {
	Obj*  sym = g->Iobj(it);
	bool  ok = expSymbol(g, sym);
	if (!ok) goto bad;
    }
    g->freeIter(it);


    /* Loop over all schematic pages in current module.
     */
    for (it=g->pageIter(); g->Imore(it); g->Inext(it)) {
	Obj*  page = g->Iobj(it);
	bool  ok = expPage(g, page);
	if (!ok) goto bad;
    }
    g->freeIter(it);
    return true;

  bad:
    g->freeIter(it);					/* LCOV_EXCL_LINE */
    return false;					/* LCOV_EXCL_LINE */
}

/* ===========================================================================
 * GEI Dump end
 * ===========================================================================
 */
int/*bool*/ GexDumpEnd() {

    if(!Dump) {
	sprintf(LastErrorMsg, "\"start\" missing");
	return false;
    }
    fprintf(Dump, "Finished.\n");
    if (!Dump2Stdout) fclose(Dump);
    Dump  = NULL;

    return true;
}

/* ===========================================================================
 * Return last error message
 * ===========================================================================
 */
const char* GexDumpLastErr() {
    return LastErrorMsg;
}
