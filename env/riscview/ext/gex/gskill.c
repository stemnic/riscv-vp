/*  gskill.c 1.77 2018/11/07

    Copyright 2007-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	skill export

    Author:
	Heinz Bruederlin

    Description:
	export nlview schematics into skill ready for import into CDS.
	This is an example for the usage of the GEI (graphical export
	interface).
	The current implementation is in *EXPERIMENTAL* state and may
	not always produce correct connectivity or graphics.
	For details see in gskill.h
    ===========================================================================
*/

#ifdef __cplusplus
#error "__FILE__ is pure C, do NOT compile with C++ !"
#endif

#include "gei.h"
#include "ghash.h"
#include "gskill.h"
#include "gassert.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <math.h>


#ifdef _MSC_VER
    /* conversion from 'size_t'  to 'int', possible loss of data */
#   pragma warning(disable:4267)

    /* conversion from '__int64' to 'int', possible loss of data
     * conversion from 'long' to 'short', possible loss of data
     */
#   pragma warning(disable:4244)

    /* 'sprintf': This function or variable may be unsafe (_MSC_VER >= 1400) */
#   pragma warning(disable:4996)
#endif

#ifdef _WIN64
    /* for P64 data model, Microsoft calls it LLP64 */
#ifdef _MSC_VER
    typedef unsigned __int64 uintPointer;
#else
#include <inttypes.h>
    typedef uint64_t uintPointer;
#endif
#else
    /* for IPL32 and LP64 data models */
    typedef unsigned long uintPointer;
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/* GE_INTERPOLATE_ARCS
 *   If defined, use Path/Polygon to interpolate circular arcs and
 *   support filled shapes and non-cosmetic lines (backward compatibility);
 *   else use real, exact Arc (never Path/Polygon), at the cost
 *   of sacrificing filled shapes and non-cosmetic lines (a cosmetic line
 *   has no width and does not scale with the zoom factor).
 */
/* #define GE_INTERPOLATE_ARCS */


/* ---------------------------------------------------------------------------
 * implement Strcasecmp - not defined in ANSI standard
 * ---------------------------------------------------------------------------
 */
#define TO_LOWER(c) (((c) >= 'A' && (c) <= 'Z') ? (c) + ('a' - 'A') : (c))
#define TO_UPPER(c) (((c) >= 'a' && (c) <= 'z') ? (c) - ('a' - 'A') : (c))

static int Strcasecmp(const char* a, const char* b) {
    for (; TO_LOWER(*a) == TO_LOWER(*b); a++, b++) {
	if (*a == '\0') return 0;
    }
    return TO_LOWER(*a) - TO_LOWER(*b);
}

/* ===========================================================================
 * Define bool/true/false
 * ===========================================================================
 */
typedef int bool;
#define false 0
#define true  1

/* ===========================================================================
 * Gei & Ge Typedefs
 * ===========================================================================
 */
typedef enum   GeiSymType	GeiSymType;
typedef enum   GeiSymOrient	GeiSymOrient;
typedef enum   GeiPinDir	GeiPinDir;
typedef enum   GeiAttrJust	GeiAttrJust;
typedef const struct GeiAccessFunc Gei;
typedef struct GeiObject	Obj;
typedef struct GeiIter		Iter;
typedef struct GeiPageInfo	PageInfo;
typedef struct GeiInstInfo	InstInfo;
typedef struct GeiAttrInfo	AttrInfo;
typedef struct GeiPinInfo	PinInfo;
typedef struct GeiNetInfo	NetInfo;
typedef struct GeiConnInfo	ConnInfo;
typedef struct GeiSymInfo	SymInfo;
typedef struct GeiSymAttrInfo	SymAttrInfo;
typedef struct GeiSymPinInfo	SymPinInfo;
typedef struct GeiPointInfo	PointInfo;
typedef struct GeiRipInfo	RipInfo;
typedef struct GeiOverInfo	OverInfo;
typedef struct GeiJoinInfo	JoinInfo;

/* ===========================================================================
 * hash tables
 *	ObjNoMap - maps a Gei Object pointer to a integer.
 *		   this is used to get reproducible results
 *		   where some uniq names need to be generated for
 *		   Gei Objects (e.g. "GeInst%d" variables)
 *	SymTab   - store generated symbol names.
 *		   this is used to generate each symbol only once.
 *	CoordMap - maps coordinates to a string.
 *		   used to map pin coordinates to names of connected nets.
 *	HierMap  - maps hier pin names to coordinates.
 *	StrMap   - maps one string to another.
 * ===========================================================================
 */
static unsigned strhash(register const char* key)
{
    register unsigned hash = 0;
    if(key) while (*key) {
        if(hash &  0xf0000000U) {
           hash %= 0x00fffffdU;     /*  16777213: prime near 16 Meg */
        }
        hash <<= 4;
        hash += *key++;
    }
    return hash;
}

/* -------------------------------------------------------------------------- */
declareHash(  static, ObjNoMap, Obj*, int)
implementHash(static, ObjNoMap, Obj*, int)
typedef struct ObjNoMap ObjNoMap;

static unsigned hashObjPtr(Obj* obj) {
    return (unsigned int)(uintPointer)obj;
}

static int cmpObjPtr(Obj* o1, Obj* o2) {
    return o1 == o2 ? 0 : (o1 > o2 ? 1 : -1);
}

/* -------------------------------------------------------------------------- */
typedef struct {
    const char*     symName;
    const char*     symView;
} SymKey;
declareHash(  static, SymTab, SymKey, int)
implementHash(static, SymTab, SymKey, int)
typedef struct SymTab SymTab;

static unsigned hashSymKey(SymKey k) {
    return strhash(k.symName) + strhash(k.symView);
}

static int cmpSymKey(SymKey s1, SymKey s2) {
    int r = strcmp(s1.symName, s2.symName);
    if (r) return r;
    r = strcmp(s1.symView, s2.symView);
    return r;
}

/* -------------------------------------------------------------------------- */
typedef struct {
    int x, y;
} Coord;
declareHash(  static, CoordMap, Coord, const char*)
implementHash(static, CoordMap, Coord, const char*)
typedef struct CoordMap CoordMap;

static unsigned hashCoord(Coord c) {
    return c.x+17*c.y;
}

static int cmpCoord(Coord c1, Coord c2) {
    int r = c1.x - c2.x;
    if (r) return r;
    r = c1.y - c2.y;
    return r;
}

/* -------------------------------------------------------------------------- */
declareHash(  static, HierMap, const char*, Coord)
implementHash(static, HierMap, const char*, Coord)
typedef struct HierMap HierMap;

/* -------------------------------------------------------------------------- */
declareHash(  static, StrMap, const char*, const char*)
implementHash(static, StrMap, const char*, const char*)
typedef struct StrMap StrMap;

/* -------------------------------------------------------------------------- */
typedef struct {
    const char*     modName;
    const char*     portName;
} PortKey;
declareHash(  static, PortTab, PortKey, const char*)
implementHash(static, PortTab, PortKey, const char*)
typedef struct PortTab PortTab;

static unsigned hashPortKey(PortKey k) {
    return strhash(k.modName) + strhash(k.portName);
}

static int cmpPortKey(PortKey s1, PortKey s2) {
    int r = strcmp(s1.modName, s2.modName);
    if (r) return r;
    r = strcmp(s1.portName, s2.portName);
    return r;
}

/* ===========================================================================
 * StrSpace manages a single linked list of blocks, with increasing
 * sizes.
 *
 * StrSpace_init	 - initialize the given string space
 * StrSpace_free	 - free allocated memory
 * StrSpace_alloc	 - return new memory space with a copy of given string
 * StrSpace_allocLen	 - return new memory space for given length
 *			   A new Mem block is allocated, if necessary.
 * ===========================================================================
 */
typedef struct StrSpace StrSpace;
struct StrSpace {
    struct StrSpace*  next;
    unsigned max;
    unsigned fill;
    char  buf[256-sizeof(void*)-2*sizeof(unsigned)];
};

static void StrSpace_init(StrSpace* ths) {
	ths->next = NULL;
	ths->max  = sizeof(ths->buf);
	ths->fill = 0;
}

static void StrSpace_free(StrSpace* ths) {
	StrSpace* m = ths->next;
	while(m) {
	    StrSpace* m_next = m->next;
	    free(m);
	    m = m_next;
	}
	ths->next = NULL;
}

static char* StrSpace_allocLen(StrSpace* ths, unsigned len) {
	StrSpace* m;		/* pointer to allocated memory block */
	char*     p;		/* char pointer to begin of buffer */
	unsigned  size;		/* the size of the new memory block */
	unsigned  overhead;	/* the size of the StrSpace overhead memory */
	unsigned  up;		/* rounded size */

	if (ths->next == NULL) {
	    /* Check if ths has enough bytes free */
	    if(ths->fill + len < ths->max) {
		char* buf = ths->buf;		/* clang needs this */
		p = buf + ths->fill;
		ths->fill += len;
		return p;
	    }
	    size = 2000;
	} else {
	    enum {depth = 3};		/* search depth */
	    /* Check the "depth" most recently allocated memory blocks 
	     * in reverse order.
	     */
	    StrSpace*	candidates[depth];
	    int		i;

	    /* cache up to "depth" candidates */
	    for (i=0, m = ths->next; i<depth && m; i++, m=m->next) {
		candidates[i]=m;
	    }

	    /* Search in first fit mode in reverse order */
	    while (--i >= 0) {
		m = candidates[i];
		if(m->fill + len < m->max) {
		    char* buf = m->buf;		/* clang needs this */
		    p = buf + m->fill;
		    m->fill += len;
		    return p;
		}
	    }
	    size = m->max * 2;			/* double the size */
	    if (size > 50000) size = 50000;	/* limit to 50k */
	}

	if(len > size) size = len;		/* for very big strings */
	overhead = sizeof(StrSpace) - sizeof(ths->buf);
	size +=overhead;

	if (size < 0x100) {
	    up = 0x20;		     /*start @ 32B boundary*/ /*LCOV_EXCL_LINE*/
	} else if (size < 0x1000) {
	    up = 0x100;		     /*start @ 256B boundary*/
	} else if (size < 0x10000) {
	    up = 0x1000;	     /*start @ 4K boundary*/
	} else {
	    up = 0x10000;	     /*start @ 64K boundary*/ /*LCOV_EXCL_LINE*/
	}
	while (up < size) up <<= 1;
	
	m = (StrSpace*)malloc(up);
	m->next = ths->next;
	m->max = up - overhead;
	m->fill = len;
	ths->next = m;

	return m->buf;
}

static char* StrSpace_alloc(StrSpace* ths, const char* src) {
	char* p;
	if (!src) return NULL;
	if (src[0] == '\0') return (char*)"";	/* don't allocate null string */
	p = StrSpace_allocLen(ths, strlen(src)+1);
	strcpy(p,src);
	return p;
}


/* ===========================================================================
 * global variables for Skill Export
 * ===========================================================================
 */
static char	    LastErrorMsg[512];	/* buffer for error messages */
static StrSpace     Sspace;		/* memory for strings */
static const char*  SkillFname = NULL;	/* file name of main output file */
static FILE*	    MainFp = NULL;	/* main output file */
static FILE*	    SymFp  = NULL;	/* symbol output file (multiFile mode)*/
static SymTab	    Symbols;		/* table of generated symbols */
static PortTab      Ports;		/* table of generated sheetTerms */
static const char*  UserEpilog = NULL;	/* fname of user supplied epilog */
static int	    PrivateCnt = 0;	/* uniq number for private symbols */
static bool	    SpiceUnits = false;	/* convert spice units to skill units */
static bool	    NoSym      = false;	/* do not create any symbols if true */
static bool	    MultiFile  = false;	/* create separate files */
static bool	    NetLabel   = false;	/* make net segment labels visible */
static bool	    NoRangeLb  = false;	/* do not append range to port labels */
static const char*  InstSuffix = NULL;	/* append to all instance names */
static const char** Params     = NULL;	/* names of parameters with pPar() */

/* ===========================================================================
 * strspaceCat - return concated string
 *		  buffer is allocated in global Sspace.
 * ===========================================================================
 */
static const char* strspaceCat(const char* s1, const char* s2) {
    int	    l1 = strlen(s1);
    int	    l2 = strlen(s2);
    char*   buf = StrSpace_allocLen(&Sspace, l1+l2+1);
    strcpy(buf, s1);
    strcat(buf, s2);
    return buf;
}

/* ===========================================================================
 * skillValue - convert Spice Units into Skill units if it is a number.
 *		propertynames will be converted into 'pPar("")' calls.
 *		a small state machine is used to parse the value string
 *		for numbers with optional fraction, exponent and spice unit
 *		string and propertynames.
 *		For other spicedialect there may be more special chars...
 * ===========================================================================
 */
enum TokState {
     StartState = '@',
     EndState   = '#',
     NumState   = 'N',
     FracState  = 'F',
     ExpState   = 'E',
     Exp2State  = '2',
     UnitState  = 'U',
     IdState    = 'I'
};

#define ISEND(c)     ((c)=='\0')
#define ISBLANK(c)   ((c)==' ' || (c)=='\t' || (c)=='\r' || (c)=='\n')
#define ISALPHA(c)   (((c)>='A' && (c)<='Z') || ((c)>='a' && (c)<='z'))
#define ISDIGIT(c)   ((c)>='0' && (c)<='9')
#define ISSIGN(c)    ((c)=='-' || (c)=='+')
#define ISMULT(c)    ((c)=='*' || (c)=='/')
#define ISBRACKET(c) ((c)=='(' || (c)==')' || (c)=='[' || (c)==']')

static char tokBuf[1000];
static int  tokLen =0;

static void tokChar(char c) {
    if (tokLen>=1000) return;
    if (c=='\\' || c=='"') tokBuf[tokLen++] = '\\';
    tokBuf[tokLen++] = c;
}

static void tokStr(const char* str, int n) {
    while(n-- && *str)
	tokChar(*str++);
}

static void appendSpiceUnit(const char* num, const char* unit) {
    if (TO_LOWER(unit[0]) == 'm' &&
	TO_LOWER(unit[1]) == 'i' &&
	TO_LOWER(unit[2]) == 'l')		/* mil */
    {
	static char buf[30];
	char*  endp;
	double val = strtod(num, &endp);
	if (endp[0]) { /* error? */ }
	sprintf(buf, "%g", val * 25.4e-6);
	tokStr(buf, strlen(buf));
    } else {
	tokStr(num,unit-num);
	switch(unit[0]) {
	case 'T': case 't': tokChar('T'); break;
	case 'G': case 'g': tokChar('G'); break;
	case 'K': case 'k': tokChar('K'); break;
	case 'U': case 'u': tokChar('u'); break;
	case 'N': case 'n': tokChar('n'); break;
	case 'P': case 'p': tokChar('p'); break;
	case 'F': case 'f': tokChar('f'); break;
	case 'M': case 'm': /* mega or m */
	    if (TO_LOWER(unit[1]) == 'e' &&
		TO_LOWER(unit[2]) == 'g') /* meg */	tokChar('M');
	    else					tokChar('m');
	    break;
	}
    }
}

static bool isPpar(const char* str, unsigned int len) {
    int i = 0;
    while(Params && Params[i]) {
	if(strlen(Params[i])==len &&
	   strncmp(Params[i], str, len)==0) {
	    return true;
	}
	i++;
    }
    return false;
}

static const char* mapValue(const char* str, bool spiceUnits) {
     const char* c = str;
     const char* unit = NULL;
     const char* num  = NULL;
     const char* id   = NULL;
     enum TokState state = StartState;
     tokLen = 0;
     while(state!=EndState) {
	switch(state) {
	case StartState:
		if (ISEND(*c)) 		{ state = EndState; tokChar('\0'); }
		else if (ISBLANK(*c)) 	{ tokChar(*c++); }
		else if (ISDIGIT(*c))	{ state = NumState;   num=c++; }
		else if (ISSIGN(*c) &&
			 ISDIGIT(c[1]))	{ state = NumState;   num=c++; }
		else if (*c=='.')	{ state = FracState;  num=c++; }
		else if (ISSIGN(*c)  ||
		         ISMULT(*c)  ||
		         ISBRACKET(*c))	{ state = StartState; tokChar(*c++); }
		else 			{ state = IdState;    id=c++; }
		break;
	case NumState:
		if (ISDIGIT(*c))	{ c++; }
		else if (*c=='.')	{ state = FracState;      c++; }
		else if (*c=='E' ||
			 *c=='e')	{ state = ExpState;       c++; }
		else if (ISALPHA(*c))	{ state = UnitState;  unit=c++; }
		else			{ state = StartState;tokStr(num,c-num);}
		break;
	case FracState:
		if (ISDIGIT(*c))	{ c++; }
		else if (*c=='E' ||
			 *c=='e')	{ state = ExpState;       c++; }
		else if (ISALPHA(*c))	{ state = UnitState; unit=c++; }
		else			{ state = StartState;tokStr(num,c-num);}
		break;
	case ExpState:
		if (ISDIGIT(*c) ||
		    ISSIGN(*c))		{ state = Exp2State; c++; }
		else if (ISALPHA(*c))	{ state = UnitState; unit=c++; }
		break;
	case Exp2State:
		if (ISDIGIT(*c))	{ c++; }
		else if (ISALPHA(*c))	{ state = UnitState; unit=c++; }
		else			{ state = StartState;tokStr(num,c-num);}
		break;
	case UnitState:
		if (ISALPHA(*c))	{ c++; }
		else			{ state = StartState;
					  if (spiceUnits)
					    appendSpiceUnit(num,unit);
					  else
					    tokStr(num,c-num);
					}
		break;
	case IdState:
		if (ISEND(*c)   ||
		    ISBLANK(*c) ||
		    ISSIGN(*c)  ||
		    ISMULT(*c)  ||
		    ISBRACKET(*c))	{ bool pPar = isPpar(id,c-id);
					  state = StartState;
					  if (pPar) tokStr("pPar(\"", 6);
					  tokStr(id,c-id);
					  if (pPar) tokStr("\")", 2);
					}
		else 			{ c++; }
		break;
	case EndState:
		break;
	}
     }
     return tokBuf;
}

/* ===========================================================================
 * mapSkill - return valid name. Invalid chars are replaced by
 *	      octal value of char enclosed in underscores.
 *	      if len >=0 only the first len char are converted
 *	      if len ==-1 the whole string ist converted.
 *	      invalid may be NULL: only special chars are escaped then.
 *	      (see virutoso schematic user guide, 
 *	       summary of naming conventions, p 122)
 * ===========================================================================
 */
#define INVALID_SYM  "'\",<>()*:!/\n\r\t "
#define INVALID_NET  "'\",<>()*:!/\n\r\t "
#define INVALID_INST "'\",<>()*:/\n\r\t "
#define ISSPECIAL(c) ((c)<' ' || (c)>126 || (c)=='"' || (c)=='\\')

static const char* mapSkill(const char* str, int len, const char* invalid) {
    int   invalidCnt = 0;
    int   specialCnt = 0;
    char* buf;
    char* d;
    const char* c = str;
    int   l = len;
    while(*c && (len==-1 || l--)) {
	if (invalid && strchr(invalid, *c)) invalidCnt++;
	if (ISSPECIAL(*c))       specialCnt++;
	c++;
    }
    if (invalidCnt==0 && specialCnt==0) {
	if (len>=0) {
	    buf = StrSpace_allocLen(&Sspace, len+1);
	    strncpy(buf, str, len);
	    buf[len] = '\0';
	    return buf;
	}
	return str;
    }

    buf = StrSpace_allocLen(&Sspace, strlen(str)+
				     invalidCnt*(1+3+1)+
				     specialCnt*(1+3)+
				     1);
    d = buf;
    c = str;
    l = len;
    while(*c && (len==-1 || l--)) {
	if (invalid && strchr(invalid, *c)) {
	    sprintf(d, "_%03o_", *c);
	    d+=(1+3+1);
	} else if (ISSPECIAL(*c)) {
	    sprintf(d, "\\%03o", *c);
	    d+=(1+3);
	} else {	
	    *d++ = *c;
	}
	c++;
    }
    *d = '\0';
    return buf;
}

/* ===========================================================================
 * getNum
 *	- helper function, because sscanf does not match
 *	  constants in format correctly on all platforms.
 * analyzeBusName
 *	- split string in basename and a valid range containing ":" and ",".
 *	- returns width if matched, 0 otherwise
 * analyzeBitName
 *	- split string in basename and subscript.
 *	- returns 1 if matched, 0 otherwise
 * mapArrayed
 *	- split busrange/subscript
 *	- map basename
 *	- if width is given, the subscripts must match
 *	- otherwise a new range is appended to the whole mapped string
 *	- return mapped basename with appended range/subscript
 * ===========================================================================
 */
static const char* brackets[] = { "()", "[]", "<>", NULL };

static const char* getNum(const char* str, int* sub) {
    const char*	c = str;
    bool 	fnd = false;
    *sub = 0;
    while(*c && (*c>='0') && (*c<='9')) {
	(*sub) = (*sub) * 10 + (*c - '0');
	c++;
	fnd = true;
    }
    return fnd ? c : NULL;
}

static int analyzeBusName(const char* name, int* len, int* rlen, int comma) {
    int b;
    for (b=0; brackets[b]; b++) {
	const char* begin = strrchr(name, brackets[b][0]);
	if (begin) {
	    int w = 0;
	    int n;
	    const char* num = begin+1;
	    const char* end;
	    int f = -1;
	
	    for(;;) {
		end = getNum(num, &n);
		if (!end) break;
		if (*end == ':') {
		    if (f>=0) break;
		    f = n;
		} else if (*end == ',' && comma) {
		    if (f>=0) { w += (f > n) ? f-n+1 : n-f+1; } else { w++; }
		    f = -1;
		} else if (*end == brackets[b][1]) {
		    if (end[1]) break;
		    if (f>=0) { w += (f > n) ? f-n+1 : n-f+1; } else { w++; }
		    *len  = begin - name;
		    *rlen = end - begin - 1;
		    return w;
		} else {
		    break;
		}
		num = end + 1;
	    }
	}
    }
    return 0;
}

static int analyzeBitName(const char* name, int* len, int* bit) {
    int b;
    for (b=0; brackets[b]; b++) {
	const char* begin = strrchr(name, brackets[b][0]);
	if (begin) {
	    const char* end = getNum(begin+1, bit);
	    if (end && (*end == brackets[b][1]) && !end[1]) {
		*len = begin - name;
		return 1;
	    }
	}
    }
    return 0;
}

static const char* mapArrayed(const char* name, int width,
			      const char* invalid, int comma) {
    int len, rlen;
    int w, bit;

    if (width == 0) width = 1;
    w = analyzeBusName(name, &len, &rlen, comma);
    if (w && (width == -1 || width == w)) {
	const char* base    = mapSkill(name, len, invalid);
	int	    baseLen = strlen(base);
	char*	    busName = StrSpace_allocLen(&Sspace, baseLen+1+rlen+1+1);
	sprintf(busName, "%s<%.*s>", base, rlen, name+len+1);
	return busName;
    }
    w = analyzeBitName(name, &len, &bit);
    if (w && (width == -1 || width == w)) {
	const char* base    = mapSkill(name, len, invalid);
	int	    baseLen = strlen(base);
	char*	    bitName = StrSpace_allocLen(&Sspace, baseLen+1+6+1+1);
	sprintf(bitName, "%s<%d>", base, bit);
	return bitName;
    }
    if (width > 1) {
	const char* base    = mapSkill(name, -1, invalid);
	int	    baseLen = strlen(base);
	char*	    busName = StrSpace_allocLen(&Sspace, baseLen+1+6+1+6+1+1);
	sprintf(busName, "%s<%d:%d>", base, width-1, 0);
	return busName;
	
    }
    return mapSkill(name, -1, invalid);
}

static const char* mapConcat(const char* names[], bool* mapped, int cnt,
			     const char* invalid) {
    int i;
    const char* prevBase = NULL;
    int		prevBit  = -1;
    int		prevDir  = 0;
    const char* concated = "";
    char	sub[20];

    for (i = 0; i < cnt; i++) {
	int	    len, bit = -1;
	const char* base;
	if (analyzeBitName(names[i], &len, &bit)) {
	    if (!mapped || !mapped[i]) {
		base = mapSkill(names[i], len, invalid);
	    } else {
		char* buf = StrSpace_allocLen(&Sspace, len+1);
		strncpy(buf, names[i], len);
		buf[len] = '\0';
		base = buf;
	    }
	    if (!prevBase || strcmp(prevBase, base)!=0) {
		/* basename changed */
		if (prevDir) {
		    sprintf(sub, ":%d", prevBit);
		    concated = strspaceCat(concated, sub);
		}
		if (prevBit!=-1) {
		    concated = strspaceCat(concated, ">");
		}
		if (*concated) concated = strspaceCat(concated, ",");
		concated = strspaceCat(concated, base);
		sprintf(sub, "<%d", bit);
		concated = strspaceCat(concated, sub);
		prevBase = base;
		prevBit  = bit;
		prevDir  = 0;
	    } else {
		/* same basename */
		int dir = bit - prevBit;
		if ((dir == 1 || dir == -1) && (!prevDir || prevDir==dir)) {
		    prevBit = bit;
		    prevDir = dir;
		} else {
		    if (prevDir) {
			sprintf(sub, ":%d", prevBit);
			concated = strspaceCat(concated, sub);
		    }
		    sprintf(sub, ",%d", bit);
		    concated = strspaceCat(concated, sub);
		    prevBit = bit;
		    prevDir = 0;
		}
	    }
	} else { /* single name */
	    if (!mapped || !mapped[i]) base = mapSkill(names[i], -1, invalid);
	    else		       base = names[i];
	    if (prevDir) {
		sprintf(sub, ":%d", prevBit);
		concated = strspaceCat(concated, sub);
	    }
	    if (prevBit!=-1) {
		concated = strspaceCat(concated, ">");
	    }
	    if (*concated) concated = strspaceCat(concated, ",");
	    concated = strspaceCat(concated, base);
	    prevBase = NULL;
	    prevBit  = -1;
	    prevDir  = 0;
	}
    }
    if (prevDir) {
	sprintf(sub, ":%d", prevBit);
	concated = strspaceCat(concated, sub);
    }
    if (prevBit!=-1) {
	concated = strspaceCat(concated, ">");
    }
    return concated;
}

/* ===========================================================================
 * prolog	- constant skill helper code
 * epilog	- cleanup skill helper code
 * ===========================================================================
 */
static const char* prolog[] = {
    "",
    "; justification abbreviations.",
    ";",
    "defvar(GejUL \"upperLeft\")",
    "defvar(GejUC \"upperCenter\")",
    "defvar(GejUR \"upperRight\")",
    "defvar(GejCL \"centerLeft\")",
    "defvar(GejCC \"centerCenter\")",
    "defvar(GejCR \"centerRight\")",
    "defvar(GejLL \"lowerLeft\")",
    "defvar(GejLC \"lowerCenter\")",
    "defvar(GejLR \"lowerRight\")",
    "",
    "; needed layer definitions.",
    ";",
    "defvar(GeWireDrwLyr  list(\"wire\"     \"drawing\"))",
    "defvar(GeWireLabLyr  list(\"wire\"     \"label\"))",
    "defvar(GeDevDrwLyr   list(\"device\"   \"drawing\"))",
    "defvar(GeDevLabLyr   list(\"device\"   \"label\"))",
    "defvar(GeAnnoDrwLyr  list(\"annotate\" \"drawing\"))",
    "defvar(GeAnnoDrw7Lyr list(\"annotate\" \"drawing7\"))",
    "defvar(GeAnnoDrw8Lyr list(\"annotate\" \"drawing8\"))",
    "defvar(GeInstDrwLyr  list(\"instance\" \"drawing\"))",
    "defvar(GeTxtDrwLyr   list(\"text\"     \"drawing\"))",
    "defvar(GePinDrwLyr   list(\"pin\"      \"drawing\"))",
    "defvar(GePinLabLyr   list(\"pin\"      \"label\"))",
    "",
    "; create library if not already exists.",
    ";",
    "putd('GeMakeLib nil)",
    "procedure( GeMakeLib(name dir)",
    "  prog( (fnd path)",
    "    fnd = nil",
    "    foreach( l ddGetLibList()~>name",
    "      if((strcmp(l name) == 0) then fnd = t)",
    "    )",
    "    if((!fnd) then",
    "      sprintf(path \"%s/%s\" dir name)",
    "      printf(\"Creating Library '%s' in dir '%s' \\n\" name path)",
    "      dbCreateLib(name path)",
    "    )",
    "  )",
    ")",
    "",
    "; create cell view.",
    "; return id of new cell view.",
    ";",
    "putd('GeCreateCellView nil)",
    "procedure( GeCreateCellView(cellName viewName viewType)",
    "  prog( (cv)",
    "    if(GeVerbose==1 then",
    "      printf(\"GeCreateCellView %s %s %s\\n\" cellName viewName viewType)",
    "    )",
    "    cv=dbOpenCellViewByType(GeLocalLib cellName viewName viewType \"w\")",
    "    cv~>DBUPerUU  = GeDBUPerUU",
    "    cv~>userUnits = GeUserUnits",
    "    return(cv)",
    "  )",
    ")",
    "",
    "; close cell view.",
    ";",
    "putd('GeSaveCellView nil)",
    "procedure( GeSaveCellView(cv)",
    "  prog( ()",
    "    if(GeVerbose==1 then printf(\"GeSaveCellView\\n\"))",
    "    dbSave(cv)",
    "    dbPurge(cv)",
    "  )",
    ")",
    "",
    "; create instance.",
    ";",
    "putd('GeCreateInst nil)",
    "procedure( GeCreateInst(libName cellName viewName instName "
"sheet x y orient)",
    "  prog( (sym inst)",
    "    if(GeVerbose==1 then",
    "      printf(\"GeCreateInst %s %s %s %s\\n\" "
"libName cellName viewName instName)",
    "    )",
    "    sym = dbOpenCellViewByType(libName cellName viewName nil \"r\")",
    "    inst = dbCreateInst(sheet sym instName x*GeScale:y*GeScale orient 1)",
    "    if(GeVerbose==2 then GeDumpInst(inst))",
    "    dbClose(sym)",
    "    return(inst)",
    "  )",
    ")",
    "",
    "; create a label.",
    ";",
    "putd('GeCreateLabel nil)",
    "procedure( GeCreateLabel(cv lyr x y name just orient size type)",
    "  prog( (label)",
    "    if(GeVerbose==1 then",
    "      printf(\"GeCreateLabel %d %d %s %s %s %d %s\\n\" "
"x y name just orient size type)",
    "    )",
    "    label = dbCreateLabel(cv lyr x*GeScale:y*GeScale name just orient",
    "                          \"stick\" (size*GeScale)*GeFontScale)",
    "    label~>isDrafted = t",
    "    label~>isOverbar = nil",
    "    label~>isVisible = t",
    "    label~>labelType = type",
    "    return(label)",
    "  )",
    ")",
    "",
    "; create a text display.",
    ";",
    "putd('GeCreateTextDisplay nil)",
    "procedure( GeCreateTextDisplay(asso owner lyr x y name just orient size)",
    "  prog( (txtdsp)",
    "    if(GeVerbose==1 then",
    "      printf(\"GeCreateTextDisplay %d %d %s %s %s %d\\n\" "
"x y name just orient size)",
    "    )",
    "    txtdsp = dbCreateTextDisplay(asso owner lyr list(\"font\" \"justify\"",
    "         \"height\" \"orient\" \"drafting\" \"overbar\""
" \"origin\" \"visible\")",
    "         x*GeScale:y*GeScale just orient \"stick\" "
    "(size*GeScale)*GeFontScale t nil t nil t name nil)",
    "    return(txtdsp)",
    "  )",
    ")",
    "",
    "; create a index sheet.",
    "; return sheet id.",
    ";",
    "putd('GeCreateIndex nil)",
    "procedure( GeCreateIndex(cellName)",
    "  prog( (idx)",
    "    printf(\"Creating Index Sheet '%s'\\n\" cellName)",
    "    idx = GeCreateCellView(cellName \"schematic\" \"schematic\")",
    "    dbCreateProp(idx \"schType\" \"string\" \"index\")",
    "    return(idx)",
    "  )",
    ")",
    "",
    "; create a schematic sheet.",
    "; return sheet id.",
    ";",
    "putd('GeCreateSheet nil)",
    "procedure( GeCreateSheet(modName pageNo multiSheet)",
    "  prog( (cellName sheet)",
    "    printf(\"Creating Sheet '%s' %d\\n\" modName pageNo)",
    "    if((multiSheet == t) then",
    "      sprintf(cellName \"%s@sheet%03d\" modName pageNo)",
    "    else",
    "      sprintf(cellName \"%s\" modName)",
    "    )",
    "    sheet = GeCreateCellView(cellName \"schematic\" \"schematic\")",
    "    if((multiSheet == t) then",
    "      dbCreateProp(sheet \"schType\" \"string\" \"sheet\")",
    "    )",
    "    ;thick_line_grp = dbCreateGroup(sheet \"sch_cyan_thickLine_solid\"",
    "    ;   list(\"collection\" \"unordered\" \"uniqueName\" \"deleteLast\"))",
    "    ;disp_pack_grp = dbCreateGroup(sheet \"schDisplayPacketsGroup\"",
    "    ;   list(\"collection\" \"unordered\" \"uniqueName\" \"deleteLast\"))",
    "    ;dbAddObjectToGroup(disp_pack_grp thick_line_grp)",
    "    return(sheet)",
    "  )",
    ")",
    "",
    "; create a msymbol.",
    "; initializes msymbol pin positions.",
    ";",
    "defvar(GeMSymInputX)",
    "defvar(GeMSymInputY)",
    "defvar(GeMSymOutputX)",
    "defvar(GeMSymOutputY)",
    "defvar(GeMSymInOutX)",
    "defvar(GeMSymInOutY)",
    "putd('GeCreateMSymbol nil)",
    "procedure( GeCreateMSymbol(modName pageNo maxCnt)",
    "  prog( (cellName msym x1 y1 x2 y2)",
    "    sprintf(cellName \"%s@sheet%03d\" modName pageNo)",
    "    printf(\"Creating MSymbol '%s'\\n\" cellName)",
    "    msym = GeCreateCellView(cellName \"msymbol\" \"schematicSymbol\")",
    "    dbCreateProp(msym \"schType\" \"string\" \"sheet\")",
    "    GeCreateLabel(msym GeTxtDrwLyr 220 -60 \"[@sheetNumber]\"",
    "                  GejCL \"R0\" 12 \"NLPLabel\")",
    "    x1=0 y1=0 x2=440 y2=-20*(maxCnt+1)",
    "    dbCreateRect(msym GeDevDrwLyr "
" list(x1*GeScale:y1*GeScale x2*GeScale:y2*GeScale))",
    "    dbCreateRect(msym GeInstDrwLyr"
" list(x1*GeScale:y1*GeScale x2*GeScale:y2*GeScale))",
    "    GeMSymInputX =10  GeMSymInputY =-10",
    "    GeMSymOutputX=430 GeMSymOutputY=-10",
    "    GeMSymInOutX =200 GeMSymInOutY =-10",
    "    return(msym)",
    "  )",
    ")",
    "",
    "; create a msymbol pin.",
    "; return pin id.",
    ";",
    "putd('GeCreateMSymPin nil)",
    "procedure( GeCreateMSymPin(msym pinName pinDir isOffSheet)",
    "  prog( (shape net term pin label just orient x y x1 y1 x2 y2 lx ly)",
    "    if(GeVerbose==1 then",
    "      printf(\"GeMSymPin %s %s\\n\" pinName pinDir)",
    "    )",
    "    case(pinDir",
    "      (\"input\"       x=GeMSymInputX y=GeMSymInputY",
    "                     GeMSymInputY=GeMSymInputY-20",
    "                     lx=x+1 ly=y just=GejCL  orient=\"R0\"",
    "      )",
    "      (\"output\"      x=GeMSymOutputX y=GeMSymOutputY",
    "                     GeMSymOutputY=GeMSymOutputY-20",
    "                     lx=x-1 ly=y just=GejCR orient=\"R0\"",
    "      )",
    "      (\"inputOutput\" x=GeMSymInOutX y=GeMSymInOutY",
    "                     GeMSymInOutY=GeMSymInOutY-20",
    "                     lx=x+1 ly=y just=GejCL  orient=\"R0\"",
    "      )",
    "    )",
    "    x1=(x-1)*GeScale y1=(y-1)*GeScale x2=(x+1)*GeScale y2=(y+1)*GeScale",
    "    shape = dbCreateRect(msym GePinDrwLyr list(x1:y1 x2:y2))",
    "    label = dbCreateLabel(msym GePinLabLyr lx*GeScale:ly*GeScale",
    "                          pinName just orient \"stick\" 10*GeScale)",
    "    label~>parent = shape",
    "    net   = dbCreateNet(msym pinName nil)",
    "    term  = dbCreateTerm(net pinName pinDir)",
    "    pin   = dbCreatePin(net shape nil)",
    "    if((isOffSheet == t) then",
    "       dbCreateProp(term \"schPinType\" \"string\" \"offSheet\")",
    "    )",
    "    return(pin)",
    "  )",
    ")",
    "",
    "; create a instance of a msymbol.",
    ";",
    "putd('GeCreateMInst nil)",
    "procedure( GeCreateMInst(x y modName pageNo sheet)",
    "  prog( (cellName instName msym minst)",
    "    sprintf(cellName \"%s@sheet%03d\" modName pageNo)",
    "    sprintf(instName \"SH%d\" pageNo)",
    "    printf(\"Creating MInst '%s'\\n\" instName)",
    "    msym=dbOpenCellViewByType(GeLocalLib cellName \"msymbol\" nil \"r\")",
    "    minst=dbCreateInst(sheet msym instName x*GeScale:y*GeScale \"R0\" 1)",
    "    dbCreateProp(minst \"sheetNumber\" \"integer\" pageNo)",
    "    dbClose(msym)",
    "    return(minst)",
    "  )",
    ")",
    "",
    "; create a term on index sheet.",
    ";",
    "putd('GeCreateMTerm nil)",
    "procedure( GeCreateMTerm(x y pinName dir sheet)",
    "  prog( (term x1 y1 x2 y2 shape label net)",
    "    if(GeVerbose==1 then",
    "      printf(\"GeCreateMTerm %d %d %s %s\\n\" "
"x y pinName dir)",
    "    )",
    "    term = dbFindTermByName(sheet pinName)",
    "    if((term == nil) then",
    "      net = dbFindNetByName(sheet pinName)",
    "      if((net == nil) then",
    "         net = dbCreateNet(sheet pinName nil)",
    "      )",
    "      x1=(x-1)*GeScale y1=(y-1)*GeScale x2=(x+1)*GeScale y2=(y+1)*GeScale",
    "      shape = dbCreateRect(sheet GePinDrwLyr list(x1:y1 x2:y2))",
    "      label = dbCreateLabel(sheet GePinLabLyr x1:y1",
    "                     pinName GejLL \"R90\" \"stick\" 10*GeScale)",
    "      label~>parent = shape",
    "      dbCreateTerm(net pinName dir)",
    "      dbCreatePin(net shape nil)",
    "    )",
    "  )",
    ")",
    "",
    "",
    "; create a pin connection on index sheet.",
    ";",
    "putd('GeCreateMConnect nil)",
    "procedure( GeCreateMConnect(minst pinName sheet)",
    "  prog( (net)",
    "    if(GeVerbose==1 then printf(\"GeCreateMConnect %s\\n\" pinName))",
    "    net = dbFindNetByName(sheet pinName)",
    "    if((net == nil) then",
    "      net = dbCreateNet(sheet pinName nil)",
    "    )",
    "    dbCreateConnByName(net minst pinName)",
    "  )",
    ")",
    "",
    "; create a symbol.",
    "; return symbol id, which need to be saved/closed",
    "; return nil if it already exists",
    ";",
    "putd('GeCreateSymbol nil)",
    "procedure( GeCreateSymbol(cellName viewName ignore schType)",
    "  prog( (sym)",
    "    if(!GeOverwrite && ddGetObj(GeLocalLib cellName viewName) then",
    "      printf(\"Using existing Symbol '%s' '%s'\\n\" cellName viewName)",
    "      sym = nil",
    "    else",
    "      printf(\"Creating Symbol '%s' '%s'\\n\" cellName viewName)",
    "      sym = GeCreateCellView(cellName viewName \"schematicSymbol\")",
    "      if((ignore == t) then",
    "         dbCreateProp(sym \"ignore\"    \"boolean\" t)",
    "         dbCreateProp(sym \"nlAction\"  \"string\"  \"ignore\")",
    "      )",
    "      if((schType != nil) then",
    "         dbCreateProp(sym \"schType\"  \"string\"  schType)",
    "      )",
    "    )",
    "    return(sym)",
    "  )",
    ")",
    "",
    "; create a symbol pin.",
    "; return term id.",
    ";",
    "putd('GeCreateSymPin nil)",
    "procedure( GeCreateSymPin(sym pinName x y stubx stuby neg",
    "                          pinDir pinNo bundle)",
    "  prog( (shape net term d x1 y1 x2 y2 x3 y3)",
    "    if(GeVerbose==1 then",
    "      printf(\"GeCreateSymPin %s %d %d\\n\" pinName x y)",
    "    )",
    "    if((neg == t) then",
    "      if((y == stuby) then",
    "        d=(stubx-x)/2.0",
    "        x1=stubx-d y1=stuby-d/2.0 x2=stubx y2=stuby+d/2.0",
    "        stubx=x1",
    "      else",
    "        d=(stuby-y)/2.0",
    "        x1=stubx-d/2.0 y1=stuby-d x2=stubx+d/2.0 y2=stuby x2=stubx+d/2.0",
    "        stuby=y1",
    "      )",
    "      dbCreateEllipse(sym GeDevDrwLyr"
" list(x1*GeScale:y1*GeScale x2*GeScale:y2*GeScale))",
    "    )",
    "    if(x!=stubx || y!=stuby then",
    "      if((bundle == t) then",
    "        dbCreatePath(sym GeDevDrwLyr list(x*GeScale:y*GeScale",
    "                             stubx*GeScale:stuby*GeScale) 3*GePathScale)",
    "      else",
    "        dbCreateLine(sym GeDevDrwLyr list(x*GeScale:y*GeScale",
    "                                        stubx*GeScale:stuby*GeScale))",
    "      )",
    "    )",
    "    x1=(x-1)*GeScale y1=(y-1)*GeScale",
    "    x2=(x+1)*GeScale y2=(y+1)*GeScale",
    "    shape = dbCreateRect(sym GePinDrwLyr list(x1:y1 x2:y2))",
    "    net = dbFindNetByName(sym pinName)",
    "    if((net == nil) then",
    "       net = dbCreateNet(sym pinName nil)",
    "    )",
    "    term = dbFindTermByName(sym pinName)",
    "    if((term == nil) then",
    "       term  = dbCreateTerm(net pinName pinDir)",
    "    )",
    "    dbCreatePin(net shape pinNo)",
    "    return(term)",
    "  )",
    ")",
    "",
    "; create a symbol bbox.",
    ";",
    "putd('GeCreateBBox nil)",
    "procedure( GeCreateBBox(sym le to ri bo)",
    "  prog( ()",
    "    if(GeVerbose==1 then printf(\"GeCreateBBox %d %d %d %d\\n\" "
"le to ri bo))",
    "    dbCreateRect(sym GeInstDrwLyr"
" list(le*GeScale:bo*GeScale ri*GeScale:to*GeScale))",
    "  )",
    ")",
    "",
    "; create a term on sheet.",
    ";",
    "putd('GeCreateSheetTerm nil)",
    "procedure( GeCreateSheetTerm(sheet netName dir inst pinName offSheet)",
    "  prog( (net term)",
    "    if(GeVerbose==1 then printf(\"GeCreateSheetTerm %s %s\\n\" "
"netName pinName))",
    "    net = dbFindNetByName(sheet netName)",
    "    if((net == nil) then",
    "      net = dbCreateNet(sheet netName nil)",
    "    )",
    "    if((dir != nil) then",
    "      term = dbFindTermByName(sheet netName)",
    "      if((term == nil) then",
    "        term = dbCreateTerm(net netName dir)",
    "      )",
    "    )",
    "    inst~>purpose = \"pin\"",
    "    dbCreatePin(net inst pinName)",
    "    if((offSheet == t) then",
    "      dbCreateProp(inst \"schPinType\" \"string\" \"offSheet\")",
    "    )",
    "  )",
    ")",
    "",
    "; create net pin conn.",
    ";",
    "putd('GeCreateConn nil)",
    "procedure( GeCreateConn(net inst pinName)",
    "  prog( ()",
    "    if(GeVerbose==1 then printf(\"GeCreateConn %s\\n\" pinName))",
    "    dbCreateConnByName(net inst pinName)",
    "  )",
    ")",
    "",
    "; create a segment and add it to net figure.",
    "; if netname is given create a label",
    ";",
    "putd('GeCreateSegment nil)",
    "procedure(GeCreateSegment(cv net x1 y1 x2 y2 netName bundle visible)",
    "  prog( (p l o s offset x y)",
    "    if(GeVerbose==1 then printf(\"GeCreateSegment %d %d %d %d %s\\n\" "
"x1 y1 x2 y2 netName))",
    "    p = list(x1*GeScale:y1*GeScale x2*GeScale:y2*GeScale)",
    "    if((bundle == t) then",
    "      s = dbCreatePath(cv GeWireDrwLyr p 2*GePathScale)",
    "      ;s = dbCreateLine(cv GeWireDrwLyr p)",
    "      ;dbAddObjectToGroup(thick_line_grp s)",
    "      offset = 3*GePathScale",
    "    else",
    "      s = dbCreateLine(cv GeWireDrwLyr p)",
    "      offset = 2*GePathScale",
    "    )",
    "    if((strlen(netName)>0) then",
    "       if((y1 == y2) then",
    "          o = \"R0\"",
    "          x = (x1+x2)/2.0*GeScale y = y1*GeScale+offset",
    "       else",
    "          o = \"R90\"",
    "          x = x1*GeScale-offset   y = (y1+y2)/2.0*GeScale",
    "       )",
    "       l = dbCreateLabel(cv GeWireLabLyr x:y netName GejLC o",
    "                          \"stick\" 6*GeScale)",
    "       l~>isVisible = visible",
    "       l~>parent = s",
    "    )",
    "    dbAddFigToNet(s net)",
    "  )",
    ")",
    "",
    "; search net by name, if not found create it.",
    ";",
    "putd('GeMakeNet nil)",
    "procedure( GeMakeNet(sheet netName)",
    "  prog( (net)",
    "    if(GeVerbose==1 then printf(\"GeMakeNet %s\\n\" netName))",
    "    net = dbFindNetByName(sheet netName)",
    "    if((net == nil) then",
    "      net = dbCreateNet(sheet netName nil)",
    "    )",
    "    return(net)",
    "  )",
    ")",
    "",
    "; create a dot and add it to net figure.",
    ";",
    "putd('GeCreateDot nil)",
    "procedure(GeCreateDot(cv net x y)",
    "  prog( (dot x1 y1 x2 y2)",
    "    if(GeVerbose==1 then printf(\"GeCreateDot %d %d\\n\" x y))",
    "    x1=x*GeScale-3*GePathScale y1=y*GeScale-3*GePathScale",
    "    x2=x*GeScale+3*GePathScale y2=y*GeScale+3*GePathScale",
    "    dot = dbCreateEllipse(cv GeWireDrwLyr list(x1:y1 x2:y2))",
    "    dbAddFigToNet(dot net)",
    "  )",
    ")",
    "",
    "; set a signal global.",
    ";",
    "putd('GeSetGlobalSignal nil)",
    "procedure( GeSetGlobalSignal(sheet netName)",
    "  prog( (sig)",
    "    if(GeVerbose==1 then printf(\"GeSetGlobalSignal %s\\n\" netName))",
    "    sig = dbFindSigByName(sheet netName)",
    "    sig~>isGlobal = t",
    "  )",
    ")",
    "",
    "; create patch cord with given length",
    "; return sym id.",
    ";",
    "putd('GeCreatePatchCord nil)",
    "procedure( GeCreatePatchCord(cellName viewName len)",
    "  prog( (sym)",
    "    sym = ddGetObj(GeLocalLib cellName viewName)",
    "    if(sym == nil then",
    "      printf(\"Creating Symbol '%s' '%s'\\n\" cellName viewName)",
    "      sym = GeCreateCellView(cellName viewName \"schematicSymbol\")",
    "      dbCreateProp(sym \"pcbAction\" \"string\" \"ignore\")",
    "      dbCreateProp(sym \"nlAction\"  \"string\" \"ignore\")",
    "      dbCreateProp(sym \"schType\"   \"string\"  \"patchCord\")",
    "      dbCreatePath(sym GeDevDrwLyr list(0:0 len*GeScale:0) GePathScale)",
    "      GeCreateSymPin(sym \"src\" 0   0 0   0 nil \"input\" \"1\" nil)",
    "      GeCreateSymPin(sym \"dst\" len 0 len 0 nil \"input\" \"2\" nil)",
    "      GeCreateLabel(sym GeAnnoDrw7Lyr len/2 0 \"[@schPatchExpr]\""
" GejLC \"R0\" 2 \"NLPLabel\")",
    "      GeCreateBBox(sym -2 -2 len+2 2)",
    "      GeSaveCellView(sym)",
    "    )",
    "    return(sym)",
    "  )",
    ")",
    "",
    "; disable auto dot generation.",
    ";",
    "defvar(GeOldAutoDot)",
    "putd('GeDisableAutodot nil)",
    "procedure( GeDisableAutodot()",
    "  prog( ()",
    "    if(GeVerbose==1 then printf(\"GeDisableAutodot\\n\"))",
    "    GeOldAutoDot = envGetVal(\"schematic\" \"autoDot\")",
    "    envSetVal(\"schematic\" \"autoDot\" 'boolean nil)",
    "  )",
    ")",
    "",
    "; enable auto dot generation.",
    ";",
    "putd('GeEnableAutodot nil)",
    "procedure( GeEnableAutodot()",
    "  prog( ()",
    "    if(GeVerbose==1 then printf(\"GeEnableAutodot\\n\"))",
    "    GeOldAutoDot = envGetVal(\"schematic\" \"autoDot\")",
    "    envSetVal(\"schematic\" \"autoDot\" 'boolean t)",
    "  )",
    ")",
    "",
    "; restore autodot settings.",
    ";",
    "putd('GeRestoreAutodot nil)",
    "procedure( GeRestoreAutodot()",
    "  prog( ()",
    "    if(GeVerbose==1 then printf(\"GeRestoreAutodot\\n\"))",
    "    envSetVal(\"schematic\" \"autoDot\" 'boolean GeOldAutoDot)",
    "  )",
    ")",
    "",
    "; verify if in non-overwrite mode and module exist.",
    ";",
    "putd('GeVerifyOverwrite nil)",
    "procedure( GeVerifyOverwrite(cellName)",
    "  prog( ()",
    "    if(GeVerbose==1 then printf(\"GeVerifyOverwrite\\n\"))",
    "    if(!GeOverwrite && ddGetObj(GeLocalLib cellName \"schematic\") then",
    "      sprintf(GeErrorMsg \"cell %s already exist in library %s\""
" cellName GeLocalLib)",
    "      error(GeErrorMsg)",
    "    )",
    "  )",
    ")",
    "",
    "; dump instance.",
    ";",
    "putd('GeDumpInst nil)",
    "procedure( GeDumpInst(inst)",
    "  prog( (master cellName lib libName libPath x y p1 p2 x1 y1 x2 y2)",
    "    x = car(inst~>xy)",
    "    y = cadr(inst~>xy)",
    "    p1 = car( inst~>bBox)",
    "    p2 = cadr(inst~>bBox)",
    "    x1 = car(p1) y1 = cadr(p1)",
    "    x2 = car(p2) y2 = cadr(p2)",
    "    master   = inst~>master",
    "    cellName = master~>cellName",
    "    lib      = master~>lib",
    "    libName  = lib~>name",
    "    libPath  = ddGetObjReadPath(lib)",
    "    printf("
    "       \"Inst %-10s %-6s %f (%f,%f) [(%f,%f),(%f,%f)] %-12s %-10s %s\\n\"",
    "            inst~>name inst~>orient inst~>mag x y x1 y1 x2 y2",
    "            cellName libName libPath)",
    "  )",
    ")",
    "",
    "; dummy to be overwritten by userdefined proc.",
    ";",
    "putd('GeFixPage nil)",
    "procedure( GeFixPage(sheet no)",
    "  sheet = no ; please sklint",
    "  no = sheet ; please sklint",
    ")",
    "",
    NULL
};

static const char* epilog[] = {
    "",
    "GeRestoreAutodot()",
    "",
    NULL
};

/* ===========================================================================
 * indent - create indent with current level
 * out    - output a line to skill file
 * ===========================================================================
 */
static bool indent(FILE* fp, int in) {
    in *=2;
    while(in-- > 0) {
	if (putc(' ', fp) == EOF) {			/* LCOV_EXCL_START */
	    sprintf(LastErrorMsg, "indent: %.100s", strerror(errno));
	    return false;
	}						/* LCOV_EXCL_STOP */
    }
    return true;
}

#if (defined(GNUC) || defined(__clang__)) && !defined(MINGW)
static bool out(FILE*fp, int ind, const char* format, ...)
    __attribute__((format (printf, 3, 4)));
#endif

static bool out(FILE*fp, int ind, const char* format, ...) {
    va_list arg_ptr;
    int cnt;

    if (ind) indent(fp, ind);
    va_start(arg_ptr, format);
    cnt = vfprintf(fp, format, arg_ptr);
    va_end(arg_ptr);
    if (cnt<0) {					/* LCOV_EXCL_START */
	sprintf(LastErrorMsg, "out: %.100s", strerror(errno));
	return false;
    }							/* LCOV_EXCL_STOP */
    return true;
}

/* ===========================================================================
 * outHeader	- create version header.
 * outFooter	- create footer.
 * outCode	- output constant skill code.
 * outGlobals	- output some skill globals.
 * outFile	- copy given file into skill code.
 * ===========================================================================
 */
static bool outHeader(FILE* fp) {
    return out(fp, 0, "; created with skill export version '%s'\n;\n", "1.77");
}

static bool outFooter(FILE* fp) {
    return out(fp, 0, "; end of file\n");
}

static bool outCode(FILE* fp, const char* s[]) {
    bool ok = true;
    int i=0;
    while(s[i]) {
	ok = ok && (fprintf(fp, "%s\n", s[i++])>=0);
    }
    if (!ok) {						/* LCOV_EXCL_START */
	sprintf(LastErrorMsg, "outSkill: %.100s", strerror(errno));
    }							/* LCOV_EXCL_STOP */
    return ok;
}

static bool outGlobals(FILE* fp, double scale,
                       const char* localLib, const char* libDir,
                       bool metric, bool overwrite) {
    bool ok = true;
    const char* userunits = "inch";
    float	dbuperuu  = 160.0;
    if (!localLib) localLib = "nlv_lib";
    if (metric) {
	userunits = "centimeter";
	dbuperuu  = 100.0;
	scale	  = scale * 160.0 / 100.0;
    }
    ok = ok && out(fp, 0, "; init settings\n");
    ok = ok && out(fp, 0, ";\n");
    ok = ok && out(fp, 0, "defvar(GeUserUnits \"%s\")\n", userunits);
    ok = ok && out(fp, 0, "defvar(GeDBUPerUU  %9.3f)\n", dbuperuu);
    ok = ok && out(fp, 0, "defvar(GeScale     %g)\n", scale);
    ok = ok && out(fp, 0, "defvar(GePathScale 2.0/GeDBUPerUU)\n");
    ok = ok && out(fp, 0, "defvar(GeFontScale 0.7)\n");
    ok = ok && out(fp, 0, "defvar(GeLocalLib  \"%s\")\n", localLib);
    ok = ok && out(fp, 0, "defvar(GeLibDir    \"%s\")\n", libDir);
    ok = ok && out(fp, 0, "defvar(GeOverwrite %s)\n", overwrite ? "t" : "nil");
    ok = ok && out(fp, 0, "defvar(GeErrorMsg  nil)\n");
    ok = ok && out(fp, 0, "defvar(GeLastCell  nil)\n");
    ok = ok && out(fp, 0, "defvar(GeSym       nil)\n");
#ifdef GE_INTERPOLATE_ARCS
    ok = ok && out(fp, 0, "defvar(GeArc       nil)\n");
#endif
    ok = ok && out(fp, 0, "defvar(GeTerm      nil)\n");
    ok = ok && out(fp, 0, "defvar(GeSheet     nil)\n");
    ok = ok && out(fp, 0, "defvar(GeProp      nil)\n");
    ok = ok && out(fp, 0, "defvar(GeNet       nil)\n");
    ok = ok && out(fp, 0, "defvar(GeIdx       nil)\n");
    ok = ok && out(fp, 0, "defvar(GeMSym      nil)\n");
    ok = ok && out(fp, 0, "defvar(GeMInst     nil)\n");
    ok = ok && out(fp, 0, "\n");
    ok = ok && out(fp, 0, "defvar(GeVerbose 0)\n"); /* 1 = verbose, 2 = debug */
    ok = ok && out(fp, 0, "\n");
    ok = ok && out(fp, 0, "defvar(GeCallFixPage 0)\n"); /* 1 = call GeFixPage */
    ok = ok && out(fp, 0, "\n");
    return ok;
}

static bool outFile(FILE* fp, const char* fname) {
    bool ok = false;
    FILE* src = fopen(fname, "r");
    if (src) {
	char buf[1024];
	for (ok=true; ok; ) {
	    unsigned cnt = fread(buf, sizeof(char), sizeof(buf), src);
	    if (!cnt) break;
	    ok = ok && (fwrite(buf, sizeof(char), cnt, fp)==cnt);
	}
	fclose(src);
    }
    if (!ok) {						/* LCOV_EXCL_START */
	sprintf(LastErrorMsg, "outFile: %.100s %.100s", fname, strerror(errno));
    }							/* LCOV_EXCL_STOP */
    return ok;
}

typedef struct {
    int cnt;
    int lastX, lastY;
    bool midArc;
    int midArcX, midArcY;
} PPState;


#ifdef GE_INTERPOLATE_ARCS

/* ============================================================================
 * roundint - round a double to an int
 * ============================================================================
 */
static int roundint(double f) {
    return (int)((f>0.0) ? floor(f) : ceil(f));
}

/* ===========================================================================
 * outArc - generate arc by some points,
 *	    first and last point is skipped.
 * ===========================================================================
 */
static bool outArc(FILE* fp, Gei* g,
		    int offX,   int offY,
		    int startX, int startY,
		    int midX,   int midY,
		    int endX,   int endY, int* cnt) {
    bool ok = true;
    double centerx, centery, diam, start, span;
    int i, n;

    int isArc = g->calculateArc(startX, startY, midX, midY, endX, endY,
				&centerx, &centery, &diam, &start, &span);
#if 0
    printf("ARC (%d,%d) (%d,%d) (%d,%d) => "
	   "(%lg,%lg) diam=%lg start=%lg span=%lg ok=%d\n",
	   startX, startY, midX, midY, endX, endY,
	   centerx, centery, diam, start, span, isArc);
#endif
    if(!isArc) {
	ok = ok && out(fp,0," %d*GeScale:%d*GeScale",startX+offX,-startY-offY);
	ok = ok && out(fp,0," %d*GeScale:%d*GeScale",endX+offX,  -endY-offY);
	return ok;
    }

    n = roundint(sqrt(2.0 * diam) * (span < 0 ? -span : span) / M_PI);
    if (n<2)  n = 2;
    if (n>20) n = 20;

    for (i=1; i<n; i++) {
	volatile double a = start + span*i/n;
	volatile double x = centerx + diam/2.0*cos(a);
	volatile double y = centery + diam/2.0*sin(a);

#if 0
	/* to get reproducible result with optimizer and on HPUX we round
	 * nearly 0 to -0.0
	 */
	if (fabs(x)<1E-10) x = -0.0;
	if (fabs(y)<1E-10) y = -0.0;
#endif

	if (((++(*cnt)+2) % 3)==0) { ok = ok && out(fp, 0, "\n         "); }
	ok = ok && out(fp, 0, " %.5f*GeScale:%.5f*GeScale", x+offX, -y-offY);
    }
    return ok;
}

#else /* GE_INTERPOLATE_ARCS */

/* ============================================================================
 * normalize_start_span
 *	Arc sweeps (span) in Cadence is always ccw (with Y upwards);
 *	i.e. we have to normalize the span such that it is always [0..2*Pi].
 *	If necessary, modify start angle.
 *   Returns:
 *     start [-Pi..Pi]
 *     span  [  0..2*Pi]
 * ============================================================================
 */
static void normalize_start_span(double* start, double* span) {
    if (*span < 0.0) {
	*start += *span;
	*span = -*span;
    }
    /* normalize start angle to [-2*M_PI..2*M_PI] */
    *start  = fmod(*start, 2*M_PI);

    /* normalize start angle to [-M_PI..M_PI] */
    if (*start >  M_PI) *start -= 2*M_PI;
    if (*start < -M_PI) *start += 2*M_PI;
    assert(-M_PI <= *start && *start <= M_PI);

    assert(0.0 <= *span && *span <= 2*M_PI);
}

/* ===========================================================================
 * outArc - generate circular Arc / Ellipse (full circle) / Line (as fallback)
 * ===========================================================================
 */
static bool outArc(FILE* fp, Gei* g, const char* parent,
		    int offX,   int offY,
		    int startX, int startY,
		    int midX,   int midY,
		    int endX,   int endY) {
    bool ok = true;
    double centerx, centery, diam, start, span;

    int isArc = g->calculateArc(startX, startY, midX, midY, endX, endY,
				&centerx, &centery, &diam, &start, &span);
#if 0
    printf("ARC (%d,%d) (%d,%d) (%d,%d) => "
	   "(%g,%g) diam=%g start=%g span=%g ok=%d\n",
	   startX, startY, midX, midY, endX, endY,
	   centerx, centery, diam, start, span, isArc);
#endif

    /**************************
     *** Arc / Ellipse mode ***
     **************************
     */
    /* The full ellipse bounding box in Nlview coordinates: */
    volatile double ebb_l = centerx - diam/2.0;
    volatile double ebb_t = centery - diam/2.0;
    volatile double ebb_r = ebb_l + diam;
    volatile double ebb_b = ebb_t + diam;

    if (!isArc) {
	ok = ok && out(fp, 3, "dbCreateLine(%s GeDevDrwLyr\n"
		"\tlist(%d*GeScale:%d*GeScale %d*GeScale:%d*GeScale))\n",
		parent,
		startX+offX, -(startY+offY),
		endX  +offX, -(endY  +offY));
	return ok;
    }

    normalize_start_span(&start, &span);

    if (fabs(span-2*M_PI)<1E-10) {
	/* 360deg = full circle => Ellipse */
	ok = ok && out(fp, 3, "dbCreateEllipse(%s GeDevDrwLyr\n"
		"\tlist(%g*GeScale:%g*GeScale %g*GeScale:%g*GeScale))\n",
		parent,
		ebb_l+offX, -(ebb_b+offY), ebb_r+offX, -(ebb_t+offY));
    } else {
	/* We create an initial Arc obj (quarter-circle) and set start-
	 * and stopAngle later by calling Arc obj's Skill methods.
	 * Alternatively, we could have passed a correctly computed Arc,
	 * bbox, but it turns out to be unreliable for small arc sweeps
	 * causing Cadence to complain with:
	 *  "*WARNING* (DB-270000): dbCreateArcByBBox:
	 *   The arc bounding box incorrectly intersects the
	 *   ellipse bounding box"
	 * ...plus we would have to create 2 Arcs if the sweep is > 180deg.
	 */

	/* convert normalized Nlview angles to Cadence angles */
	start = -(start+span);	/* Y-mirror: start becomes prev. stop */
	normalize_start_span(&start, &span);

	ok = ok && out(fp, 3, "GeArc = dbCreateArc(%s GeDevDrwLyr\n"
		"\tlist(%g*GeScale:%g*GeScale %g*GeScale:%g*GeScale)\n"
		"\tlist(%g*GeScale:%g*GeScale %g*GeScale:%g*GeScale))\n",
		parent,
		ebb_l+offX,-(ebb_b+offY),             ebb_r+offX,-(ebb_t+offY),
  ((ebb_l+ebb_r)/2.0)+offX,-((ebb_t+ebb_b)/2.0+offY), ebb_r+offX,-(ebb_t+offY));

	ok = ok && out(fp, 3, "if(GeArc then\n");
	ok = ok && out(fp, 4, "GeArc~>startAngle = %g\n", start);
	ok = ok && out(fp, 4, "GeArc~>stopAngle  = %g\n", start+span);
	ok = ok && out(fp, 3, ")\n");
    }
    return ok;
}
#endif /* GE_INTERPOLATE_ARCS */

/* ===========================================================================
 * outPolyPoint
 *	- helper to output multiple point infos and maintain state.
 *	- state must be initialized with: cnt=-1 and midArc=false.
 * ===========================================================================
 */
static bool outPolyPoint(FILE* fp, Gei* g, const char* parent,
		     int offX, int offY, int width,
		     PointInfo* pi, PPState* state) {
    bool ok = true; 
    switch (pi->type) {
	case GeiPointTBegPin:
	case GeiPointTBegWire:
	case GeiPointTBegT:
	    assert(state->cnt == -1);
	    state->cnt    = 0;
	    state->lastX  = pi->x;
	    state->lastY  = pi->y;
	    state->midArc = false;
	    break;
	case GeiPointTCorner:
	case GeiPointTMidPin:
	case GeiPointTMidT:
	    /* fall through */
	case GeiPointTEndPin:
	case GeiPointTEndWire:
	case GeiPointTEndT:
	    assert(state->cnt >= 0);
#ifdef GE_INTERPOLATE_ARCS			/* backward compatible mode */
	    if (state->lastX!=pi->x || state->lastY!=pi->y || state->midArc) {
		if (state->cnt==0) {
		    if (pi->filled) {
			ok = ok && out(fp, 3, "dbCreatePolygon(%s GeDevDrwLyr ",
					parent);
		    } else {	
			ok = ok && out(fp, 3, "dbCreatePath(%s GeDevDrwLyr ",
					parent);
		    }
		    ok = ok && out(fp, 0, "list(%d*GeScale:%d*GeScale",
					state->lastX+offX, -state->lastY-offY);
		}
		if (state->midArc) {
		    /* add interpolated arc points to Polygon / Path */
		    ok = ok && outArc(fp, g, offX, offY,
				      state->lastX, state->lastY,
				      state->midArcX, state->midArcY,
				      pi->x, pi->y, &state->cnt);
		    state->midArc = false;
		}
		if (((++state->cnt+2) % 3)==0) {
		    ok = ok && out(fp, 0, "\n         ");
		}
		ok = ok && out(fp, 0, " %d*GeScale:%d*GeScale",
						    pi->x+offX, -pi->y-offY);
		state->lastX = pi->x;
		state->lastY = pi->y;
	    }
	    if (pi->type == GeiPointTEndPin  ||
		pi->type == GeiPointTEndWire ||
		pi->type == GeiPointTEndT) {
		if (state->cnt>0) {
		    ok = ok && out(fp, 0, ")"); /* end of list */
		    if (pi->filled) {
			ok = ok && out(fp, 0, ")\n");
		    } else {
			/*Path-Width*/
			ok = ok && out(fp, 0, " %d*GePathScale)\n", width);
		    }
		}
		state->cnt = -1;
	    }
#else
	    (void)width;		/* only Path supports a width */
	    if (state->lastX!=pi->x || state->lastY!=pi->y||state->midArc) {
		if (state->cnt==0) {
		    if (state->midArc) {
			/* create Arc or Ellipse (circle) */
			ok = ok && outArc(fp, g, parent, offX, offY,
					  state->lastX,   state->lastY,
					  state->midArcX, state->midArcY,
					  pi->x,          pi->y);
			state->midArc = false;
			state->cnt = -1;
		    } else {
			ok = ok && out(fp, 3,
				"dbCreateLine(%s GeDevDrwLyr "
				    "list(%d*GeScale:%d*GeScale",
				    parent,
				    state->lastX+offX, -(state->lastY+offY));
		    }
		}
		if (state->midArc) {
		    if (state->cnt>0) {			/* interrupt Line */
			ok = ok && out(fp, 0, ")");	/* end of list */
			ok = ok && out(fp, 0, ")\n");	/* end of Line */
		    }
		    /* create Arc or Ellipse (circle) */
		    ok = ok && outArc(fp, g, parent, offX, offY,
				      state->lastX,   state->lastY,
				      state->midArcX, state->midArcY,
				      pi->x,          pi->y);
		    state->midArc = false;
		    state->cnt = -1;
		}

		/* line wrapping required? */
		if (((++state->cnt+2) % 3)==0) {
		    ok = ok && out(fp, 0, "\n         ");
		}

		if (state->cnt>0) {
		    /* add current point to list for Line */
		    ok = ok && out(fp, 0, " %d*GeScale:%d*GeScale",
						pi->x+offX, -(pi->y+offY));
		}

		state->lastX = pi->x;
		state->lastY = pi->y;
	    }
	    if (pi->type == GeiPointTEndPin  ||
		pi->type == GeiPointTEndWire ||
		pi->type == GeiPointTEndT) {
		if (state->cnt>0) {			/* finish Line */
		    ok = ok && out(fp, 0, ")");		/* end of list */
		    ok = ok && out(fp, 0, ")\n");	/* end of Line */
		}
		state->cnt = -1;
	    }
#endif
	    break;
	case GeiPointTMidArc:
	    state->midArcX = pi->x;
	    state->midArcY = pi->y;
	    state->midArc = true;
	    break;
	case GeiPointTNoCorner:
	case GeiPointTDot:
	case GeiPointTRipL:
	case GeiPointTRipR:
	    break;
    }
    return ok;
}

/* ===========================================================================
 * symType    - return string for given GeiSymType
 * symOrient  - return string for given GeiSymOrient
 * symPinDir  - return pin direction
 * attrJust   - return string for given GeiAttrJust
 * ===========================================================================
 */
static const char* symType(GeiSymType type)
{
    static struct {
	const char*     label;
	GeiSymType	type;
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

static const char* symOrient(GeiSymOrient orient)
{
    static struct {
	const char*  label;
	GeiSymOrient orient;
    } o[] = {
	{ "R0",      GeiSymOrientR0    },
	{ "MY",      GeiSymOrientMY    },
	{ "MX",      GeiSymOrientMX    },
	{ "R180",    GeiSymOrientR180  },
	{ "MYR90",   GeiSymOrientMYR90 },
	{ "R90",     GeiSymOrientR90   },
	{ "R270",    GeiSymOrientR270  },
	{ "MXR90",   GeiSymOrientMXR90 }
    };
    assert(orient == o[orient].orient);
    return o[orient].label;
}

static const char* symPinDir(GeiPinDir dir) {
    int d = dir & GeiPinInOut;
    if (d == GeiPinOutput)     return "output";
    else if (d == GeiPinInput) return "input";
    return "inputOutput";
}

static const char* attrJust(GeiAttrJust just)
{
    static struct {
	const char* label;
	GeiAttrJust just;
    } j[] = {
        { NULL,    0 },
        { "GejUL", GeiAttrFJustUL },
        { "GejUC", GeiAttrFJustUC },
        { "GejUR", GeiAttrFJustUR },
        { "GejCL", GeiAttrFJustCL },
        { "GejCC", GeiAttrFJustCC },
        { "GejCR", GeiAttrFJustCR },
        { "GejLL", GeiAttrFJustLL },
        { "GejLC", GeiAttrFJustLC },
        { "GejLR", GeiAttrFJustLR }
    };
    assert(just < 10 && just == j[just].just);
    return j[just].label;
}

/* ===========================================================================
 * searchSymAttr  - search given sym  attr and return value or NULL
 * searchInstAttr - search given inst attr and return value or NULL
 * searchNetAttr  - search given net  attr and return value or NULL
 * ===========================================================================
 */
static const char* searchSymAttr(Gei* g, Obj* sym, const char* name) {
    Iter*	it;
    const char*	fnd = NULL;

    for (it=g->spropIter(sym); g->Imore(it); g->Inext(it)) {
	AttrInfo	ai;
	const char*	n;
	int l;
	g->IspropInfo(it, &ai);
	n = ai.name;
	l = ai.nameLen;
	if (l>0 && strncmp(n, name, l)==0) {
	    fnd=ai.value;
	    break;
	}
    }
    g->freeIter(it);
    return fnd;
}

static const char* searchInstAttr(Gei* g, Obj* inst, const char* name) {
    Iter*	it;
    const char*	fnd = NULL;

    for (it=g->attrIter(inst); g->Imore(it); g->Inext(it)) {
	AttrInfo	ai;
	const char*	n;
	int l;
	g->IattrInfo(it, &ai);
	n = ai.name;
	l = ai.nameLen;
	if (l>0 && strncmp(n, name, l)==0) {
	    fnd=ai.value;
	    break;
	}
    }
    g->freeIter(it);
    return fnd;
}

static const char* searchNetAttr(Gei* g, Obj* net_or_bun, const char* name) {
    Iter*	it;
    const char*	fnd = NULL;

    for (it=g->nattrIter(net_or_bun); g->Imore(it); g->Inext(it)) {
	AttrInfo	ai;
	const char*	n;
	int		l;
	g->IattrInfo(it, &ai);
	n = ai.name;
	l = ai.nameLen;
	if (l>0 && strncmp(n, name, l)==0) {
	    fnd=ai.value;
	    break;
	}
    }
    g->freeIter(it);
    return fnd;
}

/* ===========================================================================
 * getNetName - get name of single net
 *		use @netName if there is a PG-Symbol and no Port connected
 * ===========================================================================
 */
static const char* getNetName(Gei* g, Obj* net) {
    NetInfo	ni;
    Iter*	cit;
    const char*	netName;
    const char*	pgName = NULL;

    g->netInfo(net, &ni);
    netName = mapArrayed(ni.name, -1, INVALID_NET, 1);

    /* Power/Ground symbol pin overwrite netname, if there is no port */
    for(cit=g->connIter(net); g->Imore(cit); g->Inext(cit)) {
	ConnInfo ci;
	InstInfo ii;
	g->IconnInfo(cit, &ci);
	g->instInfo(ci.inst, &ii);
	switch(ii.type) {
	    case GeiSymTINPORT:
	    case GeiSymTOUTPORT:
	    case GeiSymTINOUTPORT:
	    case GeiSymTPAGEIN:
	    case GeiSymTPAGEOUT:
	    case GeiSymTPAGEINOUT:
				g->freeIter(cit);
				return netName;
	    case GeiSymTPOWER:
	    case GeiSymTGROUND:
	    case GeiSymTNEGPOWER:
				if (!pgName)
				    pgName=searchNetAttr(g, net, "@netName");
				break;
	    case GeiSymTInst:
	    case GeiSymTHier:
				break;
	}
    }
    g->freeIter(cit);
    if (pgName) netName = pgName;
    return netName;
}

/* ===========================================================================
 * mapNetName - loop over all nets and get net name (may be slow)
 * ===========================================================================
 */
static const char* mapNetName(Gei* g, Obj* page, const char* netName) {
    Iter* it;
    for (it=g->netIter(page); g->Imore(it); g->Inext(it)) {
	Obj*	    net = g->Iobj(it);
	NetInfo	    ni;

	g->netInfo(net, &ni);
	if (ni.busWidth > 0) continue;   /* skip unexpected ones */
	if (strcmp(netName, ni.name)==0) {
	    g->freeIter(it);
	    return getNetName(g, net);
	}
    }
    g->freeIter(it);
    return netName;
}

/* ===========================================================================
 * expSegment - helper for expNetOrNbunWire, expSubWires and expOverlayWires
 *		creates a segment from prevPi to curPi (if coords are different)
 *		if begin or end coordinate is equal to a pin coordinate
 *		the net name stored in connTable is used instead of
 *		the given "netName".
 *		ripper == -2: is a straight slash ripper
 *		              -> add artificial "spike" |\ or _\ between
 *		                 subX/Y and busX/Y to avoid a "straight" ripper
 *		ripper == -1: is a straight triangular ripper
 *		              -> add artificial "spike" |\ or _\ between
 *		                 subX/Y and busX/Y to avoid a "straight" ripper
 *		ripper ==  0: do not consider busX, busY, subX, subY
 *		ripper ==  1: is a triangular ripper
 *		              -> subX/Y will be replaced by busX/Y
 *		ripper ==  2: is a slash ripper
 *		              -> add additional segment from subX/Y to busX/Y
 * ===========================================================================
 */
static bool expSegment(FILE* fp, const char* netName, bool bundle,
		       const PointInfo* prevPi, PointInfo* curPi,
		       int ripper, int busX, int busY, int subX, int subY,
		       CoordMap* connTable) {
    bool ok = true;
    bool visible = false;
    int  x1, y1;
    int  x2 = curPi->x;
    int  y2 = curPi->y;

    if (curPi->type == GeiPointTBegPin  ||
	curPi->type == GeiPointTBegWire ||
	curPi->type == GeiPointTBegT	||
	curPi->type == GeiPointTDot	||
	curPi->type == GeiPointTMidArc	||
	curPi->type == GeiPointTRipL	||
	curPi->type == GeiPointTRipR) {
	return true;
    }
    x1 = prevPi->x;	/* deferred initialization of x1,y1 to avoid UMR */
    y1 = prevPi->y;

    if (curPi->type == GeiPointTNoCorner) {		/* LCOV_EXCL_START */
        curPi->x = prevPi->x;
        curPi->y = prevPi->y;
        return true;
    }							/* LCOV_EXCL_STOP */

    if (connTable) {
	const char* conn;
	Coord coord;
	if (prevPi->type==GeiPointTBegPin || prevPi->type==GeiPointTEndPin) {
	    coord.x = x1;
	    coord.y = y1;
	    if (CoordMapFind(connTable, coord, &conn)) {
		ok = ok && out(fp, 4, "; CONN-1: %s\n", conn);
		netName = conn;
	    }
	}
	if (curPi->type==GeiPointTBegPin || curPi->type==GeiPointTEndPin) {
	    coord.x = x2;
	    coord.y = y2;
	    if (CoordMapFind(connTable, coord, &conn)) {
		ok = ok && out(fp, 4, "; CONN-2: %s\n", conn);
		netName = conn;
	    }
	}
    }

    if (ripper && ((x1==subX && y1==subY) || (x2==subX && y2==subY))) {
	if (x2==subX && y2==subY) { 
	    int tmpx=x1, tmpy=y1;
	    x1 = x2;   y1 = y2;
	    x2 = tmpx; y2 = tmpy;
	}
	if (ripper == 2) {
	    /* slash rippers are treated like overlay rippers
	     * -> add additional segment from subX/Y to busX/Y
	     * ...but only if ripper is not zero-length
	     */
	    if (busX-subX || busY-subY) {
		ok = ok && out(fp, 4,
		    "GeCreateSegment(GeSheet GeNet %d %d %d %d \"%s\" %s %s)\n",
		    busX, -busY, subX, -subY, netName, bundle?"t":"nil", "nil");
	    }
	} else if (ripper == -1 || ripper == -2) {
	    int tx, ty;
	    if (x1 == x2) {		/* vertical   ripper */
		tx = x1+3; ty = y1;
	    } else {			/* horizontal ripper __/|__subnet__ */
		tx = x1;   ty = y1-3;
	    }
	    ok = ok && out(fp, 4,
		"GeCreateSegment(GeSheet GeNet %d %d %d %d \"%s\" %s %s)\n",
		busX, -busY, tx, -ty, netName, bundle?"t":"nil", "nil");
	    ok = ok && out(fp, 4,
		"GeCreateSegment(GeSheet GeNet %d %d %d %d \"%s\" %s %s)\n",
		tx, -ty, x1, -y1, netName, bundle?"t":"nil", "nil");
		
	} else {
	    assert(ripper == 1);
	    x1 = busX; y1 = busY;
	}
    }

    if (x1!=x2 || y1!=y2) {
	if (NetLabel && 
	    (curPi ->type==GeiPointTBegPin || curPi ->type==GeiPointTEndPin ||
	     prevPi->type==GeiPointTBegPin || prevPi->type==GeiPointTEndPin)) {
		visible = true;
	}

	ok = ok && out(fp, 4,
	    "GeCreateSegment(GeSheet GeNet %d %d %d %d \"%s\" %s %s)\n",
	    x1, -y1, x2, -y2, netName, bundle?"t":"nil", visible?"t":"nil");
    }
    return ok;
}

/* ===========================================================================
 * expNetOrNbunWires - export net or bundle wire polygons
 * ===========================================================================
 */
static bool expNetOrNbunWires(FILE* fp, Gei* g, Obj* net, const char* netName,
			      bool bundle, CoordMap* connTable) {
    bool	ok = true;
    PointInfo	curPi;
    PointInfo	prevPi;
    Iter*	it;
    int		i;

    /* export segments */
    for (it=g->wireIter(net),i=0; g->Imore(it); g->Inext(it),i++) {
        g->IwireInfo(it, &curPi);
	ok = expSegment(fp, netName, bundle, i>0 ? &prevPi : NULL, &curPi,
			      0, 0, 0, 0, 0, connTable);
	if (!ok) break;
	prevPi = curPi;
    }
    g->freeIter(it);
    return ok;
}

/* ===========================================================================
 * expOverlayWires - export overlay wire polygons
 * ===========================================================================
 */
static bool expOverlayWires(FILE* fp, Gei* g, Iter* ovIter, const char* netName,
			    CoordMap* connTable) {
    bool	ok = true;
    PointInfo	curPi;
    PointInfo	prevPi;
    Iter*	it;
    int		i;

    /* export segments */
    for (it=g->ovWIter(ovIter),i=0; g->Imore(it); g->Inext(it),i++) {
	g->IovWInfo(it, &curPi);
	ok = expSegment(fp, netName, true, i>0 ? &prevPi : NULL, &curPi,
			      0, 0, 0, 0, 0, connTable);
	if (!ok) break;
	prevPi = curPi;
    }
    g->freeIter(it);
    return ok;
}

/* ===========================================================================
 * expOverlayRipper - export overlay ripper as segments
 * ===========================================================================
 */
static bool expOverlayRipper(FILE* fp, Gei* g, Iter* ovIter) {
    bool	ok = true;
    Iter*	it;
    unsigned 	width=0, i=0;
    const char**subNetNames  = NULL;
    bool*	subNetMapped = NULL;

    for (it=g->ovRIter(ovIter); g->Imore(it); g->Inext(it)) {
	RipInfo ri;
	g->IovRInfo(it, &ri);
	if (!subNetNames) {
	    i		= 0;
	    width	= ri.countDown+1;
    	    subNetNames  = malloc(width * sizeof(const char*));
	    subNetMapped = malloc(width * sizeof(bool));
	}
	assert(i<width);
	if (ri.subname) {
	    subNetNames[i]  = ri.subname;
	    subNetMapped[i] = false;
	} else {
    	    subNetNames[i] = getNetName(g, ri.net_or_nbun);
	    subNetMapped[i] = true;
	}
	i++;
	if (ri.countDown == 0) {
	    const char* netName = mapConcat(subNetNames, subNetMapped,
					    width, INVALID_NET);
	    ok = ok && out(fp, 3,
		"GeNet = GeMakeNet(GeSheet \"%s\")\n", netName);
	    /* only create segment if ripper is not zero-length */
	    if (ri.xBus-ri.xSubnet || ri.yBus-ri.ySubnet) {
		if (ri.compass==3 || ri.compass==12) {
		    int tx, ty;
		    /* For straight slash rippers we insert an additional
		     * segment like in expSegment() if needed ...
		     */
		    if (ri.xBus == ri.xSubnet) {/* vertical   ripper */
			assert(ri.compass==12);
			tx = ri.xSubnet+3; ty = ri.ySubnet;
		    } else {			/* horizontal ripper __/|__*/
			assert(ri.yBus == ri.ySubnet);
			assert(ri.compass==3);
			tx = ri.xSubnet;   ty = ri.ySubnet-3;
		    }
		    ok = ok && out(fp, 4,
			"GeCreateSegment(GeSheet GeNet "
			"%d %d %d %d \"%s\" %s %s)\n",
			ri.xBus, -ri.yBus, tx, -ty,
			netName, width>1?"t":"nil", "nil");
		    ok = ok && out(fp, 4,
			"GeCreateSegment(GeSheet GeNet "
			"%d %d %d %d \"%s\" %s %s)\n",
			tx, -ty, ri.xSubnet, -ri.ySubnet,
			netName, width>1?"t":"nil", "nil");
		} else {
		    ok = ok && out(fp, 4,
			"GeCreateSegment(GeSheet GeNet "
			"%d %d %d %d \"%s\" %s %s)\n",
			ri.xBus, -ri.yBus, ri.xSubnet, -ri.ySubnet,
			netName, width>1?"t":"nil", "nil");
		}
	    }
	    free((void*)subNetNames);
	    free((void*)subNetMapped);
	    i		= 0;
	    width	= 0;
	    subNetNames = NULL;
	    subNetMapped= NULL;
	}
    }
    g->freeIter(it);
    return ok;
}

/* ===========================================================================
 * expSubWires - export subnets connecting at a ripper
 * ===========================================================================
 */
static bool expSubWires(FILE* fp, Gei* g, Iter* ripIt, const char* name,
			bool bundle, int xBus, int yBus,
			int xSubnet, int ySubnet,
			bool straightRipper,
			bool slashRipper,
			CoordMap* connTable) {
    bool	ok = true;
    PointInfo	curPi;
    PointInfo	prevPi;
    Iter*	it;
    int		i, ripper = straightRipper ? -1 : 1;
    if (slashRipper) ripper *= 2;

    /* export segments */
    for (it=g->subWIter(ripIt),i=0; g->Imore(it); g->Inext(it), i++) {
	g->IsubWInfo(it, &curPi);
	ok = expSegment(fp, name, bundle, i>0 ? &prevPi : NULL, &curPi,
	      ripper, xBus, yBus, xSubnet, ySubnet, connTable);
	if (!ok) break;
	prevPi = curPi;
    }
    g->freeIter(it);
    return ok;
}

/* ===========================================================================
 * expNbunRippers - export subnets connecting at a ripper
 *		    each ripped bundle gets it's name generated from
 *		    all contained subnets.
 * ===========================================================================
 */
static bool expNbunRippers(FILE* fp, Gei* g, Obj* nbun, CoordMap* connTable) {
    bool	ok = true;
    Iter*	it;
    unsigned 	width=0, i=0;
    const char**subNetNames  = NULL;

    for (it=g->ripIter(nbun); g->Imore(it); g->Inext(it)) {
	RipInfo ri;
	g->IripInfo(it, &ri);
	if (!subNetNames) {
	    i		= 0;
	    width	= ri.countDown+1;
	    subNetNames = malloc(width * sizeof(const char*));
	}
	assert(i<width);
	subNetNames[i++] = ri.subname;
	if (ri.countDown == 0) {
	    bool slashRipper = ri.type == GeiRipTSlash ||
			       ri.type == GeiRipTSlashBus;
	    const char* netName = mapConcat(subNetNames,NULL,width,INVALID_NET);
	    ok = ok && out(fp, 3,
			       "GeNet = GeMakeNet(GeSheet \"%s\")\n", netName);
	    ok = ok && expSubWires(fp, g, it, netName, width>1,
				   ri.xBus,ri.yBus, ri.xSubnet, ri.ySubnet,
				   (ri.compass==3 || ri.compass==12),
				   slashRipper,
				   connTable);
	    free((void*)subNetNames);
	    i		= 0;
	    width	= 0;
	    subNetNames = NULL;
	}
    }
    g->freeIter(it);
    return ok;
}

/* ===========================================================================
 * expSheetTerm - export the sheet terms
 *		- PG instances get purpose=pin set 
 *		  (needed, when PG instance is connected to interface net)
 * ===========================================================================
 */
static bool expSheetTerm(FILE* fp, Gei* g, const char* modName,
			 Obj* inst, const char* netName,
			 ObjNoMap* instNoTable) {
    bool	ok = true;
    bool	port = false;
    const char*	dir = NULL;
    const char*	offSheet = "nil";
    int		instNo;
    InstInfo	ii;

    g->instInfo(inst, &ii);
    if (!ObjNoMapFind(instNoTable, inst, &instNo)) return true;

    switch(ii.type) {
	case GeiSymTINPORT:    dir = "\"input\"";       port = true; break;
	case GeiSymTOUTPORT:   dir = "\"output\"";      port = true; break;
	case GeiSymTINOUTPORT: dir = "\"inputOutput\""; port = true; break;
	case GeiSymTPAGEIN:    dir = "\"input\"";       offSheet="t"; break;
	case GeiSymTPAGEOUT:   dir = "\"output\"";      offSheet="t"; break;
	case GeiSymTPAGEINOUT: dir = "\"inputOutput\""; offSheet="t"; break;
	case GeiSymTPOWER:
	case GeiSymTGROUND:
	case GeiSymTNEGPOWER:  dir = "nil";	    break;
	default:               break;
    }

    ok = ok && out(fp, 3,
      "GeCreateSheetTerm(GeSheet \"%s\" %s GeInst%d \"%d\" %s)\n",
      netName, dir, instNo, instNo, offSheet);

    /**
     * store mapping from portName to netname, used as sheetterm name.
     * this name is used as symbol port name later, to insure that
     * same names are used. E.g. p<2,0> ...
     */
    if (ok && port) {
	PortKey key;
	const char* n = StrSpace_alloc(&Sspace, netName);
	key.modName   = StrSpace_alloc(&Sspace, modName);
	key.portName  = StrSpace_alloc(&Sspace, ii.name);
	PortTabInsert(&Ports, key, &n);
    }

    return ok;
}

/* ===========================================================================
 * getPortWidth
 *	- return width of a portInstance (we hope port has only one pin?)
 * ===========================================================================
 */
static int getPortWidth(Gei* g, Obj* port) {
    Iter*	it;
    int		width = -1;
    int		cnt = 0;
    for (it=g->pinIter(port); g->Imore(it); g->Inext(it)) {
	PinInfo pi;
	g->IpinInfo(it, &pi);
	width = pi.busWidth ? pi.busWidth : 1;
	if (cnt++ > 2) {
	    g->freeIter(it);				/* LCOV_EXCL_LINE */
	    return -1;					/* LCOV_EXCL_LINE */
	}
    }
    g->freeIter(it);
    return width;
}

/* ===========================================================================
 * getPageCnt - return number of pages
 * ===========================================================================
 */
static int getPageCnt(Gei* g) {
    int		cnt = 0;
    Iter*	pit;
    for (pit=g->pageIter(); g->Imore(pit); g->Inext(pit)) {
	cnt++;
    }
    g->freeIter(pit);
    return cnt;
}


/* ===========================================================================
 * expIndex - export index sheet for a module
 *	TODO: there may be a problem if a net is connected to
 *	more than one port with different type.
 * ===========================================================================
 */
static bool expIndex(FILE* fp, Gei* g, const char* modName) {
    bool	ok = true;
    Iter*	it;
    Iter*	iit;
    int		minstX = 0;
    int		minstY = 0;
    int		mtermX = 0;
    int		mtermY = 5;

    ok = ok && out(fp, 2,"GeIdx = GeCreateIndex(\"%s\")\n", modName);
    for (it=g->pageIter(); g->Imore(it); g->Inext(it)) {
	Obj*		page = g->Iobj(it);
	PageInfo	pi;
	StrMap		portNames;
	const char*	dummy = 0;
	int		inCnt = 0, inOutCnt = 0, outCnt = 0;
	int		maxCnt;

	g->pageInfo(page, &pi);

	/*
	 * count ports first, to determine symbol height
	 * port type may be mapped, if the instance is connected
	 * to a net with a port.
	 */
	for (iit=g->instIter(page); g->Imore(iit); g->Inext(iit)) {
	    Obj*	inst = g->Iobj(iit);
	    InstInfo	ii;

	    g->instInfo(inst, &ii);
	    switch (ii.type) {
		case GeiSymTINPORT:
		case GeiSymTPAGEIN:    inCnt++;    break;
		case GeiSymTOUTPORT:
		case GeiSymTPAGEOUT:   outCnt++;   break;
		case GeiSymTINOUTPORT:
		case GeiSymTPAGEINOUT:
		case GeiSymTPOWER:
		case GeiSymTGROUND:
		case GeiSymTNEGPOWER:  inOutCnt++; break;
		default:               continue;
	    }
	}
	g->freeIter(iit);

	/*
	 * create msymbol for each page
	 */
	maxCnt = inCnt;
	if (outCnt>maxCnt)   maxCnt = outCnt;
	if (inOutCnt>maxCnt) maxCnt = inOutCnt;
	ok = ok && out(fp, 2,"GeMSym = GeCreateMSymbol(\"%s\" %d %d)\n",
				     modName, pi.pageNumber, maxCnt);

	/*
	 * create ports.
	 * port type may be mapped, if the instance is connected
	 * to a net with a port.
	 * multiple ports with same name, create only one term.
	 */
	StrMapInit(&portNames, strhash, strcmp);
	for (iit=g->instIter(page); g->Imore(iit); g->Inext(iit)) {
	    Obj*	inst = g->Iobj(iit);
	    InstInfo	ii;
	    const char*	portName;
	    const char*	portDir;
	    int		portWidth;

	    g->instInfo(inst, &ii);
	    switch (ii.type) {
		case GeiSymTINPORT:
		case GeiSymTPAGEIN:    portDir = "input";       break;
		case GeiSymTOUTPORT:
		case GeiSymTPAGEOUT:   portDir = "output";      break;
		case GeiSymTINOUTPORT:
		case GeiSymTPAGEINOUT:
		case GeiSymTPOWER:
		case GeiSymTGROUND:
		case GeiSymTNEGPOWER:  portDir = "inputOutput"; break;
		default: continue;
	    }
	    portWidth = getPortWidth(g, inst);
	    portName = mapArrayed(ii.name, portWidth, INVALID_NET, 1);
	    if (StrMapFind(&portNames, portName, &dummy)) continue;
	    StrMapInsert(&portNames, portName, &dummy);

	    ok = ok && out(fp, 3,
			    "GeCreateMSymPin(GeMSym \"%s\" \"%s\" nil)\n",
			    portName, portDir);
	}
	g->freeIter(iit);
	StrMapFree(&portNames);
	ok = ok && out(fp, 3,"GeSaveCellView(GeMSym)\n");

	/*
	 * create minst for each page
	 */
	ok = ok && out(fp, 2,
			"GeMInst = GeCreateMInst(%d %d \"%s\" %d GeIdx)\n",
			minstX, minstY, modName, pi.pageNumber);
	minstX += 480;

	/*
	 * create term for each non offpage minst pin.
	 * create connection for each port and offpage minst pin.
	 * skip multiple port with same name.
	 */
	StrMapInit(&portNames, strhash, strcmp);
	for (iit=g->instIter(page); g->Imore(iit); g->Inext(iit)) {
	    Obj*	inst = g->Iobj(iit);
	    InstInfo	ii;
	    const char*	portName;
	    const char*	portDir;
	    int		portWidth;
	    bool	off = false;

	    g->instInfo(inst, &ii);
	    switch (ii.type) {
		case GeiSymTINPORT:    portDir="input";                 break;
		case GeiSymTPAGEIN:    portDir="input";       off=true; break;
		case GeiSymTOUTPORT:   portDir="output";                break;
		case GeiSymTPAGEOUT:   portDir="output";      off=true; break;
		case GeiSymTINOUTPORT: portDir="inputOutput";           break;
		case GeiSymTPAGEINOUT: portDir="inputOutput"; off=true; break;
		case GeiSymTPOWER:
		case GeiSymTGROUND:
		case GeiSymTNEGPOWER:  portDir="inputOutput"; off=true; break;
		default: continue;
	    }
	    portWidth = getPortWidth(g, inst);
	    portName = mapArrayed(ii.name, portWidth, INVALID_NET, 1);
	    if (!off)
		ok = ok && out(fp, 3,
			    "GeCreateMTerm(%d %d \"%s\" \"%s\" GeIdx)\n",
			    mtermX, mtermY, portName, portDir);

	    if (StrMapFind(&portNames, portName, &dummy)) continue;
	    StrMapInsert(&portNames, portName, &dummy);

	    ok = ok && out(fp, 3,
			"GeCreateMConnect(GeMInst \"%s\" GeIdx)\n", portName);
	    mtermX += 15;
	}
	g->freeIter(iit);
	StrMapFree(&portNames);
    }
    g->freeIter(it);
    ok = ok && out(fp, 2, "GeSaveCellView(GeIdx)\n");
    return ok;
}

/* ===========================================================================
 * getPermutedNames
 *	- if pins are not swapped return NULL.
 *	- else return string with concated pinnames.
 * ===========================================================================
 */
static const char* getPermutedNames(Gei* g, Obj* inst) {
    char*	names = NULL;
    bool	swapped = false;
    int		len = 0;
    Iter	*it;

    for (it=g->pinIter(inst); g->Imore(it); g->Inext(it)) {
	PinInfo pi;
	const char* pinName;
	g->IpinInfo(it, &pi);
	if (pi.swap_name) { pinName = pi.swap_name; swapped = true; }
	else              { pinName = pi.name; }
	if (!pinName) pinName = "";
	len += strlen(pinName);
    }
    g->freeIter(it);
    if (!swapped) return NULL;
    names    = StrSpace_allocLen(&Sspace, len+1);
    names[0] = '\0';
    for (it=g->pinIter(inst); g->Imore(it); g->Inext(it)) {
	PinInfo pi;
	const char* pinName;
	g->IpinInfo(it, &pi);
	if (pi.swap_name) { pinName = pi.swap_name; }
	else              { pinName = pi.name; }
	strcat(names, pinName);
    }
    g->freeIter(it);
    return names;
}

/* ===========================================================================
 * getSymName
 *	- returns libName, symName and viewName for a Instance
 * ===========================================================================
 */
static void getSymName(Gei* g, Obj*sym, Obj* inst, const char** libName,
		       const char** symName, const char** symView) {
    InstInfo	ii;
    SymInfo	symi;
    const char* cellName;

    g->instInfo(inst, &ii);
    g->symInfo(sym,   &symi);

    if (ii.type == GeiSymTPOWER ||
	ii.type == GeiSymTGROUND ||
	ii.type == GeiSymTNEGPOWER) {
	Iter* it;
	for (it=g->pinIter(inst); g->Imore(it); g->Inext(it)) {
	    PinInfo pi;
	    JoinInfo ji;
	    g->IpinInfo(it, &pi);
	    g->IjoinInfo(it, pi.busWidth?0:-1, &ji);
	    if (ji.net_or_nbun) {
		*libName = searchNetAttr(g, ji.net_or_nbun, "@libName");
		break;
	    }
	}
	g->freeIter(it);
    } else {
	*libName = searchSymAttr(g, sym, "@libName");
	if (!*libName)
	    *libName = searchInstAttr(g, inst, "@libName");
    }
    if (*libName) {
	const char* sname = symi.name;
	cellName = searchSymAttr(g, sym, "@cellName");
	if (!cellName)
	    cellName = searchInstAttr(g, inst, "@cellName");
	if (cellName) sname = cellName;
	*symName = mapSkill(sname,      -1, INVALID_SYM);
	*symView = mapSkill(symi.vname, -1, INVALID_SYM);
    } else {
	const char* permNames = getPermutedNames(g, inst);
	if (permNames) {
#define PREFIX "symbol_permuted_"
	    char* buf = StrSpace_allocLen(&Sspace,
		strlen(PREFIX)+strlen(permNames)+1+strlen(symi.vname)+1);
	    strcpy(buf, PREFIX);
	    strcat(buf, permNames);
	    if (*symi.vname) {
		strcat(buf, "_");
		strcat(buf, symi.vname);
	    }
	    *symName = mapSkill(symi.name,  -1, INVALID_SYM);
	    *symView = mapSkill(buf,        -1, INVALID_SYM);
#undef PREFIX
	} else {
	    *symName = mapSkill(symi.name,  -1, INVALID_SYM);
	    *symView = mapSkill(symi.vname, -1, INVALID_SYM);
	}
    }
    cellName = searchSymAttr(g, sym, "@cell");
    if (!cellName)
	cellName = searchInstAttr(g, inst, "@cell");
    if (cellName) {
	*symName = cellName;
    }
    if (**symView=='\0') *symView = "symbol";
    if (symi.privatized) {
	char* buf = StrSpace_allocLen(&Sspace, strlen(*symView)+30);
	sprintf(buf, "%s_P%d", *symView, PrivateCnt++);
	*symView = buf;
	if (*libName) {					/* LCOV_EXCL_START */
	    printf("Problem with privatized symbol\n"); /* jrr: why printf?! */
	    *libName = NULL;
	}						/* LCOV_EXCL_STOP */
    }
}

/* ===========================================================================
 * expSymbol
 *	- export symbol if not already done.
 *	- if instances have permuted pins, they are appended to
 *	  symbol name to get a uniq name.
 *	- use pin names from inst (to support permuted pins)
 *	- if symName == NULL generate comment graphic only.
 * ===========================================================================
 */
static bool expSymbol(FILE* fp, Gei* g, Obj* sym, Obj* inst,
		      const char* symName, const char* symView) {
    bool	ok = true;
    Iter	*it, *sit, *iit;
    int		pinNo;
    int		offX    = 0, offY    = 0;
    PPState	ppState;
    bool	isPower   = false;
    SymInfo	symi;
    InstInfo	ii = {0,0,0,0,0,0};	/* avoid compiler warning */
    const char*	parent;
    bool	arrayed=false; /* do not append buswidth for ports of
				* symbols used for arrayed instances
				* (@arrayed) attribute
				*/
    int		tokenCnt = 0;  /* estimate the created skill tokens
				* to avoid reaching the limit per block
				*/

    g->symInfo(sym, &symi);
    assert(inst);
    g->instInfo(inst, &ii);
    ok = ok && out(fp, 0, "\n");
    ok = ok && out(fp, 2, "; Symbol '%s' '%s' - '%s' '%s'\n",
		mapSkill(symi.name,-1,NULL),
		symi.vname ? mapSkill(symi.vname,-1,NULL):"",
		symName ? symName : "" , symView ? symView : "");

    if (symName) {
	if (symi.type == GeiSymTPOWER    ||
	    symi.type == GeiSymTGROUND   ||
	    symi.type == GeiSymTNEGPOWER) {
	    isPower = true;
	}
	offX = 0;
	offY = 0;
	parent = "GeSym";
	ok = ok && out(fp, 2,
			"%s = GeCreateSymbol(\"%s\" \"%s\" %s nil)\n",
			parent, symName, symView, isPower ? "t":"nil");
	ok = ok && out(fp, 2, "if(%s then\n", parent);
    } else {
	offX = ii.x;
	offY = ii.y;
	parent = "GeSheet";
    }

    /* export symbol shape polygons/arcs (point by point) */
    ppState.cnt         = -1;
    ppState.midArc      = false;
    for (it=g->shapeIter(sym); g->Imore(it); g->Inext(it)) {
        PointInfo	pi;
        g->IshapeInfo(it, &pi);
	ok = ok && outPolyPoint(fp, g, parent, offX, offY, 1, &pi, &ppState);
	tokenCnt += 10;
    }
    g->freeIter(it);

    /* export pins */
    pinNo = 1;
    sit = g->spinIter(sym);
    iit = g->pinIter(inst);
    while (g->Imore(sit)  && g->Imore(iit)) {
        SymPinInfo	spi;
        PinInfo		ipin;
	const char*	pinName;
	const char*	origPinName;
	const char*	pinDir;

        g->IspinInfo(sit, &spi);
	g->IpinInfo(iit, &ipin);
	if (ipin.hidden || ipin.hidestub) goto nextPin;
	pinName = ipin.swap_name ? ipin.swap_name : ipin.name;
	if (pinName) {
	    /**
	     * look for stored sheetterm name, stored during
	     * creation of module contents;
	     * this only works for a topologically sorted module sequence
	     */
	    PortKey key;
	    const char* n;
	    key.modName  = symName;
	    key.portName = pinName;
	    if (PortTabFind(&Ports, key, &n)) {
		pinName = n;
	    }
	} else {
	    if (spi.hidden || spi.hidestub) goto nextPin;
	    pinName = spi.name;
	}
	if (!pinName || !*pinName) {
	    pinName = "X";
	}

	if (searchInstAttr(g, inst, "@arrayed")) {
	    arrayed = true;
	}
	origPinName = pinName;
        pinName=mapArrayed(pinName,arrayed?-1:(int)spi.busWidth,INVALID_NET,1);

	/* export symbol pin shape(s) */
        if (symName) {
	    pinDir  = isPower ? "inputOutput" : symPinDir(spi.dir);
	    if (spi.polygon_len) {
		const int width = spi.busWidth ? 3 : 1;
		unsigned int i;
		PPState ppState;
		ppState.cnt         = -1; 
		ppState.midArc      = false; 
		for (i=0; i<spi.polygon_len; i++) {
		    ok = ok && outPolyPoint(fp, g, "GeSym", offX, offY, width,
					    spi.polygon+i, &ppState);
		    tokenCnt += 10;
		}
	    }
	    ok = ok && out(fp, 3,
			"GeTerm = GeCreateSymPin(GeSym \"%s\" %d %d %d %d %s"
			" \"%s\" \"%d\" %s)\n",
			pinName, spi.x+offX, -spi.y-offY,
			spi.xStub+offX, -spi.yStub-offY,
			spi.neg ? "t" : "nil",
			pinDir, pinNo, spi.busWidth ? "t" : "nil");
	    tokenCnt += 20;
	}

	/* export symbol pin attribute displays */
	for (it=g->spattrIter(g->Iobj(sit)); g->Imore(it); g->Inext(it)) {
	    SymAttrInfo sattr;
	    g->IsattrInfo(it, &sattr);
	    if (sattr.text) continue;
	    if (strcmp(sattr.name,"@name")==0) {
		ok = ok && out(fp, 3,
				"GeCreateLabel(%s GeAnnoDrw8Lyr %d %d \"%s\""
				" %s \"%s\" %d \"normalLabel\")\n",
				parent,
				sattr.x+offX, -sattr.y-offY,
				NoRangeLb ? origPinName : pinName,
				attrJust(sattr.just),
				sattr.vertical ? "R90" : "R0",
				sattr.size);
		tokenCnt += 15;

	    } else if (symName) {
		ok = ok && out(fp, 3,
			    "GeCreateLabel(%s GeAnnoDrw8Lyr %d %d \"[%s%s-%d]\""
			    " %s \"%s\" %d \"NLPLabel\")\n",
			    parent,sattr.x+offX,-sattr.y-offY,
			    sattr.name[0]=='@'?"":"@",
			    sattr.name, pinNo,
			    attrJust(sattr.just),
			    sattr.vertical?"R90":"R0",
			    sattr.size);
		tokenCnt += 15;
	    }
	}
	g->freeIter(it);

nextPin:
	if (tokenCnt > 20000 && symName) {
	    ok = ok && out(fp, 2, ")\n");
	    ok = ok && out(fp, 2,
		"; start new block, because token limit may be reached soon\n");
	    ok = ok && out(fp, 2, "if(%s then\n", parent);
	    tokenCnt = 0;
	}
	pinNo++;
	g->Inext(sit);
	g->Inext(iit);
    }
    g->freeIter(sit);
    g->freeIter(iit);

    /* export symbol attributes
     * - at port/offpage symbols the @name property is mapped to a TextDisplay
     *   which refers to the (hopefully only one) term
     * - at normal symbols all attribute displays except @value and text
     *   displays are mapped to NLPLabels
     */
    for (it=g->sattrIter(sym); g->Imore(it); g->Inext(it)) {
        SymAttrInfo sattr;
	const char* name;
	enum LabelType {normalLabel=0, NLPLabel=1, ILLabel=2} type= normalLabel;
        g->IsattrInfo(it, &sattr);
	name = sattr.name;
	if (symName) {
	    if (strcmp(name, "@name")==0 &&
		(symi.type != GeiSymTInst && symi.type != GeiSymTHier)) {
		ok = ok && out(fp, 3, "GeCreateTextDisplay(GeTerm GeTerm "
				      "GePinDrwLyr"
				      " %d %d \"name\" %s \"%s\" %d)\n",
			    sattr.x+offX, -sattr.y-offY, attrJust(sattr.just),
			    sattr.vertical ? "R90" : "R0", sattr.size);
		continue;
	    }
	    if (strcmp(name, "@cell")==0) {
		name = "cellName"; type = NLPLabel;
	    } else if (strcmp(name, "@name")==0) {
		name = "name"; type = NLPLabel;
	    } else if (strcmp(name, "@value")==0) {
		name = "cdsParam(1)"; type = ILLabel;
	    } else {
		name = mapSkill(sattr.name, -1, NULL);
		if (name[0] == '@') name++;
		type = sattr.text ? normalLabel : NLPLabel;
	    }
	} else { /* comment graphics symbols */
	    if (strcmp(name, "@cell")==0) {
		name = symi.name;
	    } else {
		if (strcmp(name, "@name")==0) {
		    name = ii.name;
		} else {
		    name = searchInstAttr(g, inst, name);
		}
	    }
	    type = normalLabel;
	}
	if (!name) continue;
	ok = ok && out(fp, 3, "GeCreateLabel(%s GeAnnoDrw7Lyr %d %d"
				   " \"%s%s%s\" %s \"%s\" %d \"%s\")\n",
			parent, sattr.x+offX, -sattr.y-offY, 
			type==NLPLabel?"[@":"", name, type==NLPLabel?"]":"",
			attrJust(sattr.just), sattr.vertical?"R90":"R0",
			sattr.size, 
			type==normalLabel?"normalLabel":
			    (type==NLPLabel?"NLPLabel":"ILLabel"));
    }
    g->freeIter(it);

    if (symName) {
	/* export symbol bbox
	 */
        int l = symi.bboxLeft +offX;
        int r = symi.bboxRight+offX;
        int t = symi.bboxTop  +offY;
        int b = symi.bboxBot  +offY;

        if (l==r) { l--; r++; }	/* M511: minimum width and height is 1 */
        if (t==b) { t--; b++; }

	ok = ok && out(fp, 3, "GeCreateBBox(GeSym %d %d %d %d)\n", l,-t,r,-b);

	/* save symbol
	 */
	ok = ok && out(fp, 3, "GeSaveCellView(GeSym)\n");
	ok = ok && out(fp, 2, ")\n"); /* close of GeCreateSymbol */
    }

    ok = ok && out(fp, 0, "\n");
    return ok;
}

/* ===========================================================================
 * getConnection
 * 	- return connected net or bundle name (or NULL if not connected).
 * ===========================================================================
 */
static const char* getConnection(Gei* g, Iter* pinIt) {
    static int ncCnt = 0;
    PinInfo pi;
    const char* netName = NULL;

    g->IpinInfo(pinIt, &pi);
    if (pi.busWidth) {
	unsigned	bit;
	const char**subNetNames  = malloc(pi.busWidth*sizeof(const char*));
	bool*	    subNetMapped = malloc(pi.busWidth*sizeof(bool));
	bool allNC = true;
	for (bit=0; bit<pi.busWidth; bit++) {
	    JoinInfo ji;
	    g->IjoinInfo(pinIt, bit, &ji);
	    if (ji.net_or_nbun) {
		if (ji.subnetIdx == -1) {
		    subNetNames[bit]  = getNetName(g, ji.net_or_nbun);
		    subNetMapped[bit] = true;
		} else {
		    subNetNames[bit]  = ji.subname;
		    subNetMapped[bit] = false;
		}
		allNC = false;
	    } else {
		char* buf = StrSpace_allocLen(&Sspace, 20);
		sprintf(buf, "NC%d", ncCnt++);
		subNetNames[bit] = buf;
		subNetMapped[bit] = true;
	    }
	}
	if (!allNC)
	    netName=mapConcat(subNetNames,subNetMapped,pi.busWidth,INVALID_NET);
	free((void*)subNetNames);
	free((void*)subNetMapped);
    } else {
	JoinInfo ji;
	g->IjoinInfo(pinIt, -1, &ji);
	if (ji.net_or_nbun) {
	    if (ji.subnetIdx == -1) {
		netName = getNetName(g, ji.net_or_nbun);
	    } else {
		netName = mapArrayed(ji.subname, -1, INVALID_NET, 1);
	    }
	}
    }
    return netName;
}

/* ===========================================================================
 * expAttr - export an inst, pin or net attribute (GeiAttrInfo)
 *
 * The hash table alreadyExported is used to avoid duplicates, since the
 * visible attributes (with a display) are found by the attrDspIters as well
 * as the attrIters.
 * ===========================================================================
 */
static bool expAttr(FILE* fp, AttrInfo* ai, 
    StrMap* alreadyExported, int instNo, int pinNo, const char* association) {
    bool 	ok;
    char	buf[200];
    const char* name, *notUsed;
    int         len = ai->nameLen;

    if (len > 200-1) len = 200-1;	/* Simply cut too long attributes */
    strncpy(buf, ai->name, len);
    buf[len] = '\0';
    name = buf;

    /* Ignore already exported attributes */
    if (StrMapFind(alreadyExported, name, &notUsed)) return true;

    /* Insert complete name into hash table */
    StrMapInsert(alreadyExported, StrSpace_alloc(&Sspace,name), &notUsed);
    
    /* Skip '@' in name */
    if (*name == '@') name++;

    /* skip empty property names */
    if (*name == '\0') return true;

    if (instNo > -1 && pinNo > -1) {	/* Pin attribute */
	ok = out(fp, 3,
		"%sdbCreateProp(GeInst%d \"%s-%d\" \"string\" \"%s\")\n",
		    association?association:"", instNo, name, pinNo, ai->value);

    } else if (instNo > -1) {		/* Inst attribute */
	ok = out(fp, 3,
		"%sdbCreateProp(GeInst%d \"%s\" \"string\" \"%s\")\n",
		    association?association:"", instNo, name, ai->value);

    } else {	/* Net attribute */
	ok = out(fp, 3,"%sdbCreateProp(GeNet \"%s\" \"string\" \"%s\")\n",
			association?association:"", name, ai->value);
    }

    return ok;
}

/* ===========================================================================
 * expInstPins
 * 	- export pin labels for each pin
 *	- skip hidden / hidestub.
 *	- add pin's coordinates and connected net to connTable.
 * ===========================================================================
 */
static bool expInstPins(FILE* fp, Gei* g, Obj* inst, int instNo,
			CoordMap* connTable) {
    bool  ok = true;
    int	  pinNo;
    InstInfo	ii = {0,0,0,0,0,0};	/* avoid (wrong) purify warning */
    Obj* sym;
    Iter* it;
    Iter* sit;
    StrMap attrNames;
    StrMapInit(&attrNames, strhash, strcmp);

    g->instInfo(inst,  &ii);
    sym = ii.sym;

    for (it=g->pinIter(inst), sit=g->spinIter(sym), pinNo=1;
	 g->Imore(it) && g->Imore(sit);
	 g->Inext(it), g->Inext(sit), pinNo++) {
	PinInfo		pi;
	SymPinInfo	spi;
	const char*	netName;
	Iter*           ait; /* Pin attribute iterator */

	g->IpinInfo(it, &pi);
	if (pi.hidden || pi.hidestub) continue;

	spi.x = spi.y = 0;			/* please Windows compiler */

	g->IspinInfo(sit, &spi);

	if (pi.swap_name) {
	    Iter* ssit;
	    for (ssit=g->spinIter(sym); g->Imore(ssit); g->Inext(ssit)) {
		g->IspinInfo(ssit, &spi);
		if (strcmp(spi.name, pi.swap_name)==0) break;
	    }
	    g->freeIter(ssit);
	} else {
	    g->IspinInfo(sit, &spi);
	}

	netName = getConnection(g, it);
	if (netName) {
	    Coord coord;
	    coord.x = ii.x + spi.x;
	    coord.y = ii.y + spi.y;
	    CoordMapInsert(connTable, coord, &netName);
	    ok = ok && out(fp, 3, "; PIN CONN \"%s\" %d %d\n",
			    netName, coord.x, coord.y);
	}

	/* Export all visible pin attributes */
	for (ait=g->pattrDspIter(g->Iobj(it),inst);g->Imore(ait);g->Inext(ait)){
	    SymAttrInfo sai;
	    AttrInfo ai;
	    g->IattrDspInfo(ait, &sai, &ai);
	    if (!sai.name || !ai.name) continue;
	    if (sai.text) continue;

	    /* Export the attribute */
	    ok = ok && expAttr(fp, &ai, &attrNames, instNo, pinNo, "GeProp = ");

	    if (sai.additional && !sai.marks) {
		/* Export the additional attribute display, 
		 * except for marks, these are not supported. 
		 * The created text display is associated to the property 
		 * created above (GeProp) and owned by the current GeInst.
		 */
		ok = ok && out(fp, 3, 
		    "GeCreateTextDisplay(GeProp GeInst%d GeAnnoDrw8Lyr"
					  " %d %d \"%s-%d\" %s \"%s\" %d)\n",
		    instNo, sai.x+ii.x, -sai.y-ii.y, 
		    sai.name[0]=='@' ? sai.name+1 : sai.name, pinNo,
		    attrJust(sai.just), sai.vertical ? "R90" : "R0", sai.size);
	    }
	}
	g->freeIter(ait);

	/* Export the remaining pin attributes */
	for (ait=g->pattrIter(g->Iobj(it)); g->Imore(ait); g->Inext(ait)) {
	    AttrInfo ai;
	    g->IattrInfo(ait, &ai);
	    ok = ok && expAttr(fp, &ai, &attrNames, instNo, pinNo, NULL);
	}
	g->freeIter(ait);

	StrMapFree(&attrNames);	/* Re-init attrNames for the next pin */
    }
    g->freeIter(it);
    g->freeIter(sit);
    return ok;
}

/* ===========================================================================
 * expPatchCord
 *	- create patchCord symbol if needed.
 *	- create patchCord instance at x1/y1.
 * ===========================================================================
 */
static bool expPatchCord(FILE* fp, const char* instName, int pinNo, int width,
			 int x1, int y1, int x2, int y2) {
    bool	ok = true;
    int		dx = x2 - x1;
    int		dy = y2 - y1;
    int		len    = 0;
    char	cellName[10+20];
    char	expr[20];
    const char*	orient = NULL;

    if      (dx >  0 && dy == 0) { orient = "R0";	      len = dx;	}
    else if (dx == 0 && dy >  0) { orient = "R90";	      len = dy; }
    else if (dx <  0 && dy == 0) { orient = "R180"; dx = -dx; len = dx;	}
    else if (dx == 0 && dy <  0) { orient = "R270"; dy = -dy; len = dy; }

    if (!orient) {					/* LCOV_EXCL_START */
	sprintf(LastErrorMsg, "unsupported hierPin orientation\n");
	return false;
    }							/* LCOV_EXCL_STOP */

    sprintf(cellName, "patchCord_%d", len);
    ok = ok && out(fp, 4,
	    "GeCreatePatchCord(\"%s\" \"symbol\" %d)\n",
				cellName, len);
    ok = ok && out(fp, 4, "defvar(GeInst)\n");
    ok = ok && out(fp, 4,
	    "GeInst = GeCreateInst(GeLocalLib \"%s\" \"symbol\" \"%s%s-%d\" "
				"GeSheet %d %d \"%s\")\n",
				cellName, 
				instName, InstSuffix ? InstSuffix : "",
				pinNo, x1, -y1, orient);
    if (width>1) {
	sprintf(expr, "0:%d", width-1);
    } else {
	strcpy(expr, "0");
    }
    ok = ok && out(fp, 4,
	    "dbCreateProp(GeInst \"schPatchExpr\" \"string\" \"%s=%s\")\n",
				expr, expr);
    return ok;
}

/* ===========================================================================
 * expHierPins
 * 	- put all "inner" hier pins in a hash table
 *	- for all "outer" normal pins look in hash table and add patchcord
 *	- add pin's coordinates and connected net to connTable.
 *	- skip hidden / hidestub.
 * ===========================================================================
 */
static bool expHierPins(FILE* fp, Gei* g, Obj* inst, CoordMap* connTable,
			const char* instName) {
    bool	ok = true;
    InstInfo	ii;
    Obj*	sym;
    Iter*	it;
    Iter*	sit;
    int		pinNo;
    HierMap	hierMap;

    g->instInfo(inst,  &ii);
    sym = ii.sym;
    HierMapInit(&hierMap, strhash, strcmp);

    for (it=g->pinIter(inst), sit=g->spinIter(sym);
	 g->Imore(it) && g->Imore(sit);
	 g->Inext(it), g->Inext(sit)) {
	PinInfo		pi;
	SymPinInfo	spi;
	const char*	netName;
	Coord		coord;

	g->IpinInfo(it, &pi);
	if (pi.hidden || pi.hidestub) continue;

	g->IspinInfo(sit, &spi);
	coord.x = ii.x + spi.x;
	coord.y = ii.y + spi.y;

	netName = getConnection(g, it);
	if (netName) {
	    CoordMapInsert(connTable, coord, &netName);
	}
	if (!pi.hier) continue;

	HierMapInsert(&hierMap, pi.name, &coord);
    }
    g->freeIter(it);
    g->freeIter(sit);

    for (it=g->pinIter(inst), sit=g->spinIter(sym), pinNo = 1;
	 g->Imore(it) && g->Imore(sit);
	 g->Inext(it), g->Inext(sit), pinNo++) {
	PinInfo		pi;
	SymPinInfo	spi;
	Coord		coord;
	Coord		hiercoord;

	g->IpinInfo(it, &pi);
	if (pi.hidden || pi.hidestub || pi.hier) continue;

	g->IspinInfo(sit, &spi);
	coord.x = ii.x + spi.x;
	coord.y = ii.y + spi.y;

	if (!HierMapFind(&hierMap, pi.name, &hiercoord)) continue;

	ok = ok && expPatchCord(fp, instName, pinNo, pi.busWidth,
				coord.x, coord.y, hiercoord.x, hiercoord.y);
    }
    g->freeIter(it);
    g->freeIter(sit);
    HierMapFree(&hierMap);
    return ok;
}

/* ===========================================================================
 * expInstAttrs
 *	- the special attributes @type_<name> are used for
 *	  mapping the type of attribute <name>.
 * ===========================================================================
 */
static bool expInstAttrs(FILE* fp, Gei* g, Obj* page, Obj* sym, Obj* inst,
			 int instNo, const char* instName) {
    bool   ok = true;
    bool   modelFnd = false;
    const char* symModel = NULL;
    const char*	notUsed;
    Iter*  it;
    StrMap attrTypes, attrNames;

    StrMapInit(&attrTypes, strhash, strcmp);
    StrMapInit(&attrNames, strhash, strcmp);

    for (it=g->spropIter(sym); g->Imore(it); g->Inext(it)) {
	AttrInfo ai;
	g->IspropInfo(it, &ai);
	if (strncmp(ai.name, "@type_", 6)==0) {
	    char*       n = StrSpace_alloc(&Sspace, ai.name+6);
	    const char* v = StrSpace_alloc(&Sspace, ai.value);
	    n[ai.nameLen-6]='\0';
	    StrMapInsert(&attrTypes, n, &v);
	} else if (strncmp(ai.name, "$MODEL", 6)==0) {
	    symModel = StrSpace_alloc(&Sspace, ai.value);
	}
    }
    g->freeIter(it);
    for (it=g->attrIter(inst); g->Imore(it); g->Inext(it)) {
	AttrInfo ai;
	g->IattrInfo(it, &ai);
	if (strncmp(ai.name, "@type_", 6)==0) {
	    char*       n = StrSpace_alloc(&Sspace, ai.name+6);
	    const char* v = StrSpace_alloc(&Sspace, ai.value);
	    n[ai.nameLen-6]='\0';
	    StrMapInsert(&attrTypes, n, &v);
	}
    }
    g->freeIter(it);

    /* Export all visible instance attributes */
    for (it=g->attrDspIter(inst); g->Imore(it); g->Inext(it)){
	SymAttrInfo sai;
	AttrInfo ai;
	g->IattrDspInfo(it, &sai, &ai);
	if (!sai.name || !ai.name) continue;
	if (sai.text) continue;

	/* Export the attribute */
	ok = ok && expAttr(fp, &ai, &attrNames, instNo, -1, "GeProp = ");

	if (sai.additional && !sai.marks) {
	    /* Export the additional attribute display, 
	     * except for marks, these are not supported. 
	     * The created text display is associated to the property 
	     * created above (GeProp) and owned by the current GeInst.
	     */
	    InstInfo ii;
	    g->instInfo(inst, &ii);
	    ok = ok && out(fp, 3, 
		"GeCreateTextDisplay(GeProp GeInst%d GeAnnoDrw8Lyr"
				      " %d %d \"%s\" %s \"%s\" %d)\n",
		instNo, sai.x+ii.x, -sai.y-ii.y, 
		sai.name[0]=='@' ? sai.name+1 : sai.name,
		attrJust(sai.just), sai.vertical ? "R90" : "R0", sai.size);
	}
    }
    g->freeIter(it);

    /* Export the remaining instance attributes */
    for (it=g->attrIter(inst); g->Imore(it); g->Inext(it)) {
	AttrInfo	ai;
	const char*	type  = NULL;
	const char*	quote = "\"";
	char		buf[200];
	const char*	name;
	const char*	value = NULL;
	int             len;

	g->IattrInfo(it, &ai);
	len = ai.nameLen;
	if (len > 200-1) len = 200-1;	/* Simply cut too long attributes */
	strncpy(buf, ai.name, len);
	buf[len] = '\0';
	name = buf;

	/* Skip visible attributes already exported above.
	 * Note: We assume, that the special attributes handled below are
	 * not skipped here.
	 */
	if (StrMapFind(&attrNames, name, &notUsed)) continue;

	if (strncmp(name, "@libName", 8)==0) continue; /*ignore @libName */
	if (strncmp(name, "@cellName",9)==0) continue; /*ignore @cellName */
	if (strncmp(name, "@value",   6)==0) continue; /*ignore nlview attr*/
	if (strncmp(name, "@arrayed", 8)==0) continue; /*ignore nlview attr*/
	if (strncmp(name, "@type_",   6)==0) continue; /*ignore @type_XXX */
	if (strncmp(name, "@netSet_", 8)==0) {
	    name += 8;
	    type = "netSet";
	    value = mapNetName(g, page, ai.value);
	} else {
	    if (*name == '@' || *name=='$' || *name=='#') name++;
	}

	/* skip empty property names */
	if (*name == '\0') continue;

	if (Strcasecmp(name, "MODEL")==0) {
	    name = "model"; /* lowercase */
	    modelFnd = true;
	}

	/* map attribute type */
	if (!type && !StrMapFind(&attrTypes, name, &type)) {
	    type = "string";
	}
	if (strcmp(type, "float") == 0 || strcmp(type, "int") == 0) {
	    quote = "";
	} else if (strcmp(type, "bulk") == 0) {
	    value = mapNetName(g, page, ai.value);
	    type = "string";
	}

	if (!value) {
	    value = mapValue(ai.value, SpiceUnits);
	}
	ok = ok && out(fp, 3,
			"dbCreateProp(GeInst%d \"%s\" \"%s\" %s%s%s)\n",
			instNo, name, type, quote, value, quote);
    }
    if (!modelFnd && symModel) {
	ok = ok && out(fp, 3,
			"dbCreateProp(GeInst%d \"%s\" \"string\" \"%s\")\n",
			instNo, "model", symModel);
    }
    if (!StrMapFind(&attrNames, "@name", &notUsed)) {
	ok = ok && out(fp, 3,
			"dbCreateProp(GeInst%d \"%s\" \"string\" \"%s\")\n",
			instNo, "name", instName);
    }
    g->freeIter(it);
    StrMapFree(&attrTypes);
    StrMapFree(&attrNames);
    return ok;
}

/* ===========================================================================
 * expInst
 * 	- export the given instance to Skill.
 *	  If a "@libName" attribute exists, it is used as a library
 *	  name of an existing symbol with no pin permutation;
 *	  else a symbol will be generated and referenced.
 * ===========================================================================
 */
static bool expInst(FILE* fp, Gei* g, Obj* page, Obj* inst,
		    int instNo, CoordMap* connTable) {
    bool	ok = true;
    InstInfo	ii;
    SymInfo	symi;
    Obj*	sym;
    const char*	instName;
    const char*	libName = NULL;
    const char*	symName;
    const char*	symView;
    char	buf[20];
    int		offX = 0;
    int		offY = 0;
    GeiSymOrient orient;

    g->instInfo(inst,  &ii);
    sym = ii.sym;
    g->symInfo(sym, &symi);
    orient = symi.orient;
    if (symi.master && !symi.privatized) {
	offX = symi.xmove;
	offY = symi.ymove;
	sym = symi.master;
	g->symInfo(sym, &symi);
    }

    ok = ok && out(fp, 2,
		    "; inst '%s' (%d,%d)+(%d,%d) o=%s sym='%s' v='%s' t='%s'\n",
			mapSkill(ii.name,-1,NULL),
			ii.x, ii.y, offX, offY, symOrient(orient),
			mapSkill(symi.name,-1,NULL),
			mapSkill(symi.vname,-1,NULL), symType(ii.type));

    getSymName(g, sym, inst, &libName, &symName, &symView);
    if (ii.type == GeiSymTPOWER ||
	ii.type == GeiSymTGROUND ||
	ii.type == GeiSymTNEGPOWER ||
	ii.type == GeiSymTINPORT ||
	ii.type == GeiSymTOUTPORT ||
	ii.type == GeiSymTINOUTPORT) {
	/* for power create uniq inst name. */
	sprintf(buf, "portinst_%d", instNo);
	instName = buf;
    } else {
	instName = mapArrayed(ii.name, -1, INVALID_INST, 0);
    }

    if (libName) {
	ok = ok && out(fp, 2, "defvar(GeInst%d)\n", instNo);
	ok = ok && out(fp, 2,
	    "GeInst%d = GeCreateInst(\"%s\" \"%s\" \"%s\" \"%s%s\" "
	    "GeSheet %d %d \"%s\")\n",
	    instNo, libName, symName, symView,
	    instName, InstSuffix ? InstSuffix : "",
	    ii.x+offX, -ii.y-offY, symOrient(orient));
    } else {
	int	dummy = 0;
	SymKey	key;
	if (ii.type == GeiSymTHier) {
	    ok = ok && expSymbol(fp, g, sym, inst, NULL, NULL);
	    ok = ok && expHierPins(fp, g, inst, connTable, instName);
	    return ok;
	} else {
	    key.symName = symName;
	    key.symView = symView;
	    if (!SymTabFind(&Symbols, key, &dummy)) {
		if (!NoSym) {
		    /* create symbol which are not mapped to cadence symbols */
		    ok = ok && expSymbol(MultiFile ? SymFp : MainFp, g,
					sym, inst, symName, symView);
		}
		key.symName = StrSpace_alloc(&Sspace, symName);
		key.symView = StrSpace_alloc(&Sspace, symView);
		SymTabInsert(&Symbols, key, &dummy);
	    }
	    ok = ok && out(fp, 2, "defvar(GeInst%d)\n", instNo);
	    ok = ok && out(fp, 2,
		"GeInst%d = GeCreateInst(%s \"%s\" \"%s\" \"%s%s\" "
		"GeSheet %d %d \"%s\")\n",
		instNo, "GeLocalLib", symName, symView,
		instName, InstSuffix ? InstSuffix : "",
		ii.x+offX, -ii.y-offY,
		symOrient(orient));
	}
    }

    ok = ok && expInstPins (fp, g, inst, instNo, connTable);
    ok = ok && expInstAttrs(fp, g, page, sym, inst, instNo, ii.name);

    return ok;
}

/* ===========================================================================
 * expInsts - export all instances of a page
 * ===========================================================================
 */
static bool expInsts(FILE* fp, Gei* g, Obj* page, ObjNoMap* instNoTable,
					CoordMap* connTable) {
    bool  ok = true;
    int	  instNo;
    Iter* it;

    for (it=g->instIter(page), instNo=0; g->Imore(it); g->Inext(it), instNo++) {
	Obj* inst = g->Iobj(it);

	ObjNoMapInsert(instNoTable, inst, &instNo);
	ok = ok && expInst(fp, g, page, inst, instNo, connTable);
    }
    g->freeIter(it);
    return ok;
}

/* ===========================================================================
 * expNetAttrs - export all attributes of a net or netBundle
 * ===========================================================================
 */
static bool expNetAttrs(FILE* fp, Gei* g, Obj* net) {
    bool	ok = true;
    Iter*       ait;
    StrMap	attrNames;

    StrMapInit(&attrNames, strhash, strcmp);

    /* Export all visible net attributes */
    for (ait=g->nattrDspIter(net);g->Imore(ait);g->Inext(ait)){
	SymAttrInfo sai;
	AttrInfo ai;
	g->IattrDspInfo(ait, &sai, &ai);
	if (!sai.name || !ai.name) continue;
	if (sai.text) continue;

	/* Export the attribute */
	ok = ok && expAttr(fp, &ai, &attrNames, -1, -1, "GeProp = ");
	if (!sai.marks) {
	    /* Export the attributes display.
	     * The created text display is associated to the property 
	     * created above (GeProp) and owned by the current GeNet.
	     */
	    ok = ok && out(fp, 3, 
		"GeCreateTextDisplay(GeProp GeNet GeAnnoDrw8Lyr"
				      " %d %d \"%s\" %s \"%s\" %d)\n",
		sai.x, -sai.y, 
		sai.name[0]=='@' ? sai.name+1 : sai.name,
		attrJust(sai.just), sai.vertical ? "R90" : "R0", sai.size);
	}
    }
    g->freeIter(ait);

    /* Export the remaining net attributes */
    for (ait=g->nattrIter(net); g->Imore(ait); g->Inext(ait)) {
	AttrInfo ai;
	g->IattrInfo(ait, &ai);

	if (strncmp(ai.name, "@libName", 8)==0) continue; /*ignore @libName */
	if (strncmp(ai.name, "@netName", 8)==0) continue; /*ignore @netName */
	if (strncmp(ai.name, "@name",    5)==0) continue; /*ignore @name */

	ok = ok && expAttr(fp, &ai, &attrNames, -1, -1, NULL);
    }
    g->freeIter(ait);

    StrMapFree(&attrNames);
    return ok;
}

/* ===========================================================================
 * expNets - export all nets of a page
 * ===========================================================================
 */
static bool expNets(FILE* fp, Gei* g, Obj* page) {
    bool  ok = true;
    Iter* it;

    for (it=g->netIter(page); g->Imore(it); g->Inext(it)) {
	Obj*	    net = g->Iobj(it);
	NetInfo	    ni;
	const char* netName;

	g->netInfo(net, &ni);
	ok = ok && out(fp, 3,"; net '%s'\n", mapSkill(ni.name,-1,NULL));
	if (ni.busWidth > 0) continue;   /* skip unexpected ones */

	netName = getNetName(g, net);
	ok = ok && out(fp, 3,"GeNet = GeMakeNet(GeSheet \"%s\")\n", netName);

	ok = ok && expNetAttrs(fp, g, net);
	ok = ok && expNetOrNbunWires(fp, g, net, netName, false, NULL);
    }
    g->freeIter(it);
    return ok;
}

/* ===========================================================================
 * create name of bundle
 * ===========================================================================
 */
static const char* getBundleName(Gei* g, Obj* nbun) {
    NetInfo	ni;
    unsigned 	i;
    unsigned	width;
    const char**subNetNames;
    const char* netName;

    g->netInfo(nbun, &ni);
    width = ni.busWidth;
    subNetNames = malloc(width * sizeof(const char*));
    for(i=0; i<width; i++) {
	Iter*	    bcit = g->bconnIter(nbun, i, &subNetNames[i]);
	g->freeIter(bcit);
    }
    netName = mapConcat(subNetNames, NULL, width, INVALID_NET);
    free((void*)subNetNames);
    return netName;
}

/* ===========================================================================
 * create name of overlay bus
 * ===========================================================================
 */
static const char* getOverlayName(Gei* g, Iter* oit) {
    OverInfo    oi;
    unsigned 	i;
    unsigned	width;
    Iter*	it;
    const char**subNetNames;
    bool*	subNetMapped;
    const char* netName;

    g->IoverInfo(oit, &oi);
    width = oi.busWidth;
    if (!width) width = 1;
    subNetNames  = malloc(width * sizeof(const char*));
    subNetMapped = malloc(width * sizeof(bool));
    i = 0;
    for (it=g->ovRIter(oit); g->Imore(it); g->Inext(it)) {
	RipInfo ri;
	g->IovRInfo(it, &ri);
	assert(i<width);
	if (ri.subname) {
	    subNetNames[i]  = ri.subname;
	    subNetMapped[i] = false;
	} else {
    	    subNetNames[i]  = getNetName(g, ri.net_or_nbun);
	    subNetMapped[i] = true;
	}
	i++;
    }
    g->freeIter(it);
    netName = mapConcat(subNetNames, subNetMapped, i, INVALID_NET);
    free((void*)subNetNames);
    free((void*)subNetMapped);
    return netName;
}

/* ===========================================================================
 * expBusses - export all net bundles of a page
 * ===========================================================================
 */
static bool expBusses(FILE* fp, Gei* g, Obj* page, CoordMap* connTable) {
    bool  ok = true;
    Iter* it;

    for (it=g->nbunIter(page); g->Imore(it); g->Inext(it)) {
	Obj* nbun = g->Iobj(it);
	NetInfo	ni;
	const char* netName;

	g->netInfo(nbun, &ni);
	ok = ok && out(fp, 3,"; net '%s' %d\n", mapSkill(ni.name,-1,NULL),
						ni.busWidth);
	netName = getBundleName(g, nbun);
	ok = ok && out(fp, 3,"GeNet = GeMakeNet(GeSheet \"%s\")\n", netName);
	ok = ok && expNetAttrs(fp, g, nbun);
	ok = ok && expNetOrNbunWires(fp, g, nbun, netName, true, connTable);
	ok = ok && expNbunRippers(fp, g, nbun, connTable);
    }
    g->freeIter(it);

    for (it=g->overIter(page); g->Imore(it); g->Inext(it)) {
	OverInfo    oi;
	const char* netName;

	g->IoverInfo(it, &oi);
	if (oi.inst) {
	    InstInfo ii;
	    g->instInfo(oi.inst, &ii);
	    ok = ok && out(fp, 3,"; overlay at '%s' '%s'\n",
			    mapSkill(ii.name,-1,NULL),
			    oi.name ? mapSkill(oi.name,-1,NULL) : "PORT");
	}
	netName = getOverlayName(g, it);
	ok = ok && out(fp, 3,"GeNet = GeMakeNet(GeSheet \"%s\")\n", netName);
	ok = ok && expOverlayWires(fp, g, it, netName, connTable);
	ok = ok && expOverlayRipper(fp, g, it);
    }
    g->freeIter(it);
    return ok;
}

/* ===========================================================================
 * expPorts - export all ports
 * ===========================================================================
 */
static bool expPorts(FILE* fp, Gei* g, const char* modName,
		     Obj* page, ObjNoMap* instNoTable,
		     CoordMap* connTable) {
    bool  ok = true;
    Iter* it;

    for (it=g->instIter(page) ; g->Imore(it); g->Inext(it)) {
	Obj* inst = g->Iobj(it);
	InstInfo	ii;

	g->instInfo(inst, &ii);
	if (ii.type == GeiSymTINPORT    ||
	    ii.type == GeiSymTOUTPORT   ||
	    ii.type == GeiSymTINOUTPORT ||
	    ii.type == GeiSymTPAGEIN    ||
	    ii.type == GeiSymTPAGEOUT   ||
	    ii.type == GeiSymTPAGEINOUT ||
	    ii.type == GeiSymTPOWER     ||
	    ii.type == GeiSymTGROUND    ||
	    ii.type == GeiSymTNEGPOWER) {
	    int	  pinNo;
	    Obj* sym;
	    Iter* pit;
	    Iter* sit;

	    sym = ii.sym;
	    for (pit=g->pinIter(inst), sit=g->spinIter(sym), pinNo=1;
		 g->Imore(pit) && g->Imore(sit);
		 g->Inext(pit), g->Inext(sit), pinNo++) {
		PinInfo		pi;
		SymPinInfo	spi;
		Coord		coord;
		const char*	netName;

		g->IpinInfo(pit, &pi);
		if (pi.hidden || pi.hidestub) continue;
		if (pi.swap_name) {
		    /* We currently don't swap page connector pins. The code
		     * below is just in case we add it in the future.
		     */
		    Iter* ssit;				/* LCOV_EXCL_START */
		    for (ssit=g->spinIter(sym); g->Imore(ssit); g->Inext(ssit)){
			g->IspinInfo(ssit, &spi);
			if (strcmp(spi.name, pi.swap_name)==0) break;
		    }
		    g->freeIter(ssit);
		} else {				/* LCOV_EXCL_STOP */
		    g->IspinInfo(sit, &spi);
		}

		if (ii.type == GeiSymTPOWER     ||
		    ii.type == GeiSymTGROUND    ||
		    ii.type == GeiSymTNEGPOWER) {
		    JoinInfo	ji;
		    NetInfo	ni;
		    bool	portFnd = false;
		    Iter*	cit;

		    g->IjoinInfo(pit, pi.busWidth?0:-1, &ji);
		    if (!ji.net_or_nbun) continue;

		    g->netInfo(ji.net_or_nbun, &ni);
		    if (!ni.isIn && !ni.isOut) continue;

		    /* if isIn or isOut is set
		     * the net which is connected to the PG instance
		     * is an interface, but maybe it's only a Port of hier sym
		     * so look for a real port
		     * To speed this up, we should use a table instead of a loop
		     */
		    for(cit=g->connIter(ji.net_or_nbun);
			g->Imore(cit) && !portFnd; g->Inext(cit)) {
			ConnInfo ci;
			InstInfo ii;
			g->IconnInfo(cit, &ci);
			g->instInfo(ci.inst, &ii);
			switch(ii.type) {
			    case GeiSymTINPORT:
			    case GeiSymTOUTPORT:
			    case GeiSymTINOUTPORT:
			    case GeiSymTPAGEIN:
			    case GeiSymTPAGEOUT:
			    case GeiSymTPAGEINOUT:
				portFnd = true;
				break;
			    case GeiSymTPOWER:
			    case GeiSymTGROUND:
			    case GeiSymTNEGPOWER:
			    case GeiSymTInst:
			    case GeiSymTHier:
				break;
			}
		    }
		    g->freeIter(cit);
		
		    if (!portFnd) continue;
		    
		    /* handle PG instance as terminal pin if it is connected
		     * directly to a interface port
		     */
		}

		g->IspinInfo(sit, &spi);
		coord.x = ii.x + spi.x;
		coord.y = ii.y + spi.y;

		if (!CoordMapFind(connTable, coord, &netName)) {
		    /* use inst name for unconnected ports */
		    netName = mapArrayed(ii.name, pi.busWidth, INVALID_NET, 1);
		}

		ok=ok && expSheetTerm(fp,g,modName,inst,netName,instNoTable);
	    }
	    g->freeIter(pit);
	    g->freeIter(sit);
	}
    }
    g->freeIter(it);
    return ok;
}

/* ===========================================================================
 * expPages
 *	- export all pages of a module
 *	- a mapping from instances to "small" numbers is used
 *	  to generate a Skill variable for each instance.
 *	  (the inst pointer could be used for this, but then
 *	   the Skill output files are not reproducible across platforms
 *	   and time)
 * ===========================================================================
 */
static bool expPages(FILE* fp, Gei* g, const char* modName, bool multiSheet) {
    bool     ok = true;
    Iter*     it;

    for (it=g->pageIter(); g->Imore(it); g->Inext(it)) {
	Obj*	 page = g->Iobj(it);
	PageInfo pi;
	ObjNoMap instNoTable;
	CoordMap connTable;

	ObjNoMapInit(&instNoTable, hashObjPtr, cmpObjPtr);
	CoordMapInit(&connTable,   hashCoord,  cmpCoord);
	g->pageInfo(page, &pi);
	ok = ok && out(fp, 0, "\n");
	ok = ok && out(fp, 2,"; page %d\n", pi.pageNumber);
	ok = ok && out(fp, 2,"defvar(GeSheet)\n");
	ok = ok && out(fp, 2,"GeSheet = GeCreateSheet(\"%s\" %d %s)\n",
                                 modName, pi.pageNumber, multiSheet?"t":"nil");
	ok = ok && expInsts (fp, g, page, &instNoTable, &connTable);
	ok = ok && expNets  (fp, g, page);
	ok = ok && expBusses(fp, g, page, &connTable);
	ok = ok && expPorts (fp, g, modName, page, &instNoTable, &connTable);
	ok = ok && out(fp, 2, "if(GeCallFixPage==1 then\n");
	ok = ok && out(fp, 2, "   GeFixPage(GeSheet %d)\n", pi.pageNumber);
	ok = ok && out(fp, 2, ")\n");
	ok = ok && out(fp, 2, "GeSaveCellView(GeSheet)\n");
	ok = ok && out(fp, 2, "\n");
	CoordMapFree(&connTable);
	ObjNoMapFree(&instNoTable);
    }
    g->freeIter(it);
    return ok;
}

/* ===========================================================================
 * createFile
 *	- create file with version header
 *	- append part name (if not NULL)
 *	- return false and set LastErrorMsg on error.
 * ===========================================================================
 */
static bool createFile(FILE* mainFp, const char* part, FILE** fp) {
    int   l1 = strlen(SkillFname);
    int   l2 = part ? strlen(part) : 0;
    char* fname;

    fname = StrSpace_allocLen(&Sspace, l1+5+l2+1);
    if (part) {
	const char* ext = strrchr(SkillFname, '.');
	if (ext) {
	    sprintf(fname, "%.*s_inc_%s%s", (int)(ext-SkillFname),
					    SkillFname, part, ext);
	} else {
	    sprintf(fname, "%s_inc_%s", SkillFname, part);
	}
    } else {
	strcpy(fname, SkillFname);
    }
    *fp = fopen(fname, "w");
    if (!*fp) {
	sprintf(LastErrorMsg, "%.100s: %.100s", fname, strerror(errno));
	return false;
    }
    outHeader(*fp);
    if (mainFp) {
	const char* slash = strrchr(fname, '/');
	if (!slash) slash = strrchr(fname, '\\');
	out(mainFp, 0, "load(\"%s\")\n", slash ? slash+1 : fname);
    }
    return true;
}

/* ===========================================================================
 * closeFile
 *	- append footer and close file
 *	- return false and set LastErrorMsg on error.
 * ===========================================================================
 */
static bool closeFile(FILE* fp) {
    bool  ok = true;

    if (fp) {
	ok = outFooter(fp);
	if (fclose(fp) == EOF) {
							/* LCOV_EXCL_START */
	    sprintf(LastErrorMsg, "fclose: %.100s", strerror(errno));
	    ok = false;
	}						/* LCOV_EXCL_STOP */
    }
    return ok;
}

/* ===========================================================================
 * GexSkillStart
 *	- open output file
 *	  initialize all global variables
 * ===========================================================================
 */
int/*bool*/ GexSkillStart(const char* skillFname,
			  const char* libName,
			  const char* libDir,
			  const char* userProlog,
			  const char* userEpilog,
			  double scale,
			  bool metric, bool overwrite, bool spiceUnits,
			  bool noSym,
			  bool multiFile,
			  bool netLabel,
			  bool noRangeLabel,
			  const char* instSuffix) {
    bool ok = true;

    if (MainFp) GexSkillEnd(); /* end missing, we do it here automatically */

    StrSpace_init(&Sspace);
    SymTabInit(&Symbols, hashSymKey, cmpSymKey);
    PortTabInit(&Ports, hashPortKey, cmpPortKey);
    PrivateCnt = 0;
    SpiceUnits = spiceUnits;
    NoSym      = noSym;
    MultiFile  = multiFile;
    NetLabel   = netLabel;
    NoRangeLb  = noRangeLabel;
    InstSuffix = instSuffix ? StrSpace_alloc(&Sspace,instSuffix) : NULL;
    SkillFname = StrSpace_alloc(&Sspace, skillFname);
    UserEpilog = StrSpace_alloc(&Sspace, userEpilog);

    ok = createFile(NULL, NULL, &MainFp);

    ok = ok && outGlobals(MainFp, scale, libName, libDir, metric, overwrite);

    if (MultiFile) {
	FILE* utilsFp = NULL;
	ok = ok && out(MainFp, 0, "; load utility code\n");
	ok = ok && createFile(MainFp, "utils", &utilsFp);
	if (ok) {
	    outCode(utilsFp, prolog);
	    ok = closeFile(utilsFp);
	}
	if(userProlog) {
	    ok = ok && createFile(MainFp, "userprolog", &utilsFp);
	    ok = ok && outFile(utilsFp, userProlog);
	    ok = closeFile(utilsFp);
	}
    } else {
	ok = ok && outCode(MainFp, prolog);
	if(userProlog) {
	    ok = ok && outFile(MainFp, userProlog);
	}
    }

    ok = ok && out(MainFp, 0, "\n");
    ok = ok && out(MainFp, 0, "; generate schematics\n");
    ok = ok && out(MainFp, 0, "GeEnableAutodot()\n");
    ok = ok && out(MainFp, 0, "GeMakeLib(GeLocalLib GeLibDir)\n");
    ok = ok && out(MainFp, 0, "\n");

    if (MultiFile && !NoSym) {
	ok = ok && createFile(MainFp, "symbols", &SymFp);
    }
    return ok;
}

/* ===========================================================================
 * GexSkillModule
 *	- export one module
 * ===========================================================================
 */
int/*bool*/ GexSkillModule(const char* module, Gei* g, const char** params) {
    bool	ok = true;
    int		pageCnt;
    const char* modName;
    FILE*	fp = NULL;
    int		i;

    if (!MainFp) {					/* LCOV_EXCL_START */
	sprintf(LastErrorMsg, "\"start\" missing");
	return false;
    }							/* LCOV_EXCL_STOP */

    /*
     * check GEI magic and version
     */
    if (g->magic != GeiMagic) {				/* LCOV_EXCL_START */
	sprintf(LastErrorMsg, "bad magic in Nlview GEI pointer %p", (void*)g);
	return false;
    }
    if (g->geiVersion() != GeiVersion) {
	sprintf(LastErrorMsg, "bad Nlview GEI version %d (%d expected)",
	    g->geiVersion(), GeiVersion);
	return false;
    }							/* LCOV_EXCL_STOP */

    Params = params;

    modName = mapSkill(module,  -1, INVALID_SYM);

    if (MultiFile) {
	ok = createFile(MainFp, modName, &fp);
    } else {
	fp = MainFp;
    }
    ok = ok && out(fp, 2, "; creating module %s\n", mapSkill(modName,-1,NULL));
    i = 0;
    while(Params && Params[i]) {
	ok = ok && out(fp, 2, ";   param: %s\n", mapSkill(Params[i++],-1,NULL));
    }
    ok = ok && out(fp, 2, "GeVerifyOverwrite(\"%s\")\n", modName);
    ok = ok && out(fp, 2, "GeLastCell = \"%s\"\n", modName);
        
    pageCnt = getPageCnt(g);
    if (pageCnt>1)
	ok = ok && expIndex(fp, g, modName);
    ok = ok && expPages(fp, g, modName, pageCnt>1);
    if (MultiFile) {
	ok = closeFile(fp) && ok;
    }
    return ok;
}

/* ===========================================================================
 * GexSkillEnd
 *	- close output file and cleanup data.
 * ===========================================================================
 */
int/*bool*/ GexSkillEnd() {
    bool ok = true;

    if (SymFp) {
	ok = closeFile(SymFp) && ok;
    }
    if (MainFp) {
	ok = ok && outCode(MainFp, epilog);
	if (MultiFile) {
	    if(UserEpilog) {
		FILE* utilsFp = NULL;
		ok = ok && createFile(MainFp, "userepilog", &utilsFp);
		ok = ok && outFile(utilsFp, UserEpilog);
		ok = closeFile(utilsFp);
	    }
	} else {
	    if(UserEpilog) {
		ok = ok && outFile(MainFp, UserEpilog);
	    }
	}
	ok = closeFile(MainFp) && ok;
    }

    SymFp  = NULL;
    MainFp = NULL;
    StrSpace_free(&Sspace);
    SymTabFree( &Symbols);
    PortTabFree(&Ports);
    return ok;
}

/* ===========================================================================
 * GexSkillLastErr
 *	- return detailed error message, when
 *	  GexSkillStart(), GexSkillModule(), GexSkillEnd() returned
 *	  with false.
 * ===========================================================================
 */
const char* GexSkillLastErr() {
    return LastErrorMsg;
}
