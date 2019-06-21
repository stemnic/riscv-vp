/*  nlvos.h 1.77 2018/12/01

    Copyright 1996-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	Operating System Calls and Service Functions

    Description:
	Re-implementation of Clib functions to get predictable results
	on regression tests.  These functions do not depend on "locales"
 	($LC_LOCALE et al) and is significantly faster that most system
	clib implementations.

    ===========================================================================
*/

#ifndef nlvos_h
#define nlvos_h

#ifdef __cplusplus
extern "C" {
#endif


/* ============================================================================
 * NlvUint64    - Integral type for 64bit unsigned integer (portability)
 * ============================================================================
 */
#if defined(_MSC_VER) && _MSC_VER < 1400
/* (unsigned) long long is defined by C99 standard,
 * but Microsoft Visual C/C++ < 8.0 does not support this.
 * To work around this limitation, we can use a Microsoft specific
 * 64-bit integer data type (_int64) instead:
 */
typedef unsigned _int64    NlvUint64;
#else
typedef unsigned long long NlvUint64;
#endif


/* ============================================================================
 * NlvUintPointer    - Integral for Pointer size (for portable cast)
 * ============================================================================
 */
#ifdef _WIN64
#ifndef _MSC_VER
#include <stdio.h>	/* MINGW: required for __int64 type definition */
#endif
    /* for P64 data model (Microsoft calls it LLP64) */
    typedef unsigned __int64 NlvUintPointer;
#else
    /* for IPL32 and LP64 data models */
    typedef unsigned long NlvUintPointer;
#endif



/* ============================================================================
 * Allocate Memory Chunks
 */
void*   NlvMalloc(unsigned size);            /* alloc uninit memory */
void*   NlvAlloc(unsigned size);             /* alloc cleared memory */
void*   NlvRealloc(void*, unsigned size);    /* grow memory, new mem is uninit*/
void    NlvFree(void* ptr);


/* ============================================================================
 * Snap up the given size to get a nice malloc block size.
 *
 * return a size large enough for the requested size,
 * but rounded up a multiple of 1.0 or 1.5 * power of 2.
 * Minimum is 24 byte (which holds 6 32bit words).
 * This function is used internally in sglist for the list
 * sizes, but can be used to achieve the same block sizes
 * for other dynamic growing memory.
 */
unsigned NlvBestSize4Malloc(unsigned reqSize);

/* ============================================================================
 * Fast Memory Functions
 */
void	NlvMemMove(void* dest, const void* src, unsigned int nbytes);
void	NlvMemCopy(void* dest, const void* src, unsigned int nbytes);
void	NlvMemClear(void* dest, unsigned int nbytes);


/*=============================================================================
 * Math functions
 */
#define NlvPI	  3.14159265358979323846

#define NlvMin(a,b) ((a)<(b)?(a):(b))
#define NlvMax(a,b) ((a)>(b)?(a):(b))

#define NlvSETMIN(a,b) if((a)>(b)) (a)=(b)
#define NlvSETMAX(a,b) if((a)<(b)) (a)=(b)

int	NlvRound(double x);
int	NlvAbsoluteI(int x);
double  NlvAbsoluteD(double x);

/* ============================================================================
 * Special string compare function
 * and glob-style pattern match
 */
int	NlvStrcmpAlphaNum(const char*, const char*);
int	NlvStrcasecmpAlphaNum(const char*, const char*);
int	NlvStrmatch(const char*, const char* pattern, int caseSensitive);
int	NlvStrmatch2(const char*, const char* pattern, int caseSen, int glob2);

/* ============================================================================
 * String to Hash key functions
 */
unsigned NlvStrhash(const char* s);
unsigned NlvStrcasehash(const char* s);

/* ============================================================================
 * Reimplemented libc functions
 */
int 	NlvStrcmp(const char* s1, const char* s2);
int 	NlvStrncmp(const char* s1, const char* s2, unsigned n);
int 	NlvStrcasecmp(const char* s1, const char* s2);
int 	NlvStrncasecmp(const char* s1, const char* s2, unsigned n);

unsigned NlvStrlen(const char* str);
char*    NlvStrcat(char* s, const char* append);
char*    NlvStrcpy(char* to, const char* from);
char*    NlvStrncpy(char* dst, const char* src, unsigned n);
char*    NlvStrdup(const char* src);

char*    NlvStrchr(const char* p, const char ch);
char*    NlvStrrchr(const char* p, const char ch);
unsigned NlvStrcspn(const char* s1, const char* s2);
char*    NlvStrstr(const char* s, const char* find);
char*    NlvStrcasestr(const char* s, const char* find);
char*    NlvStrsep(char** stringp, const char* delim);

const char*   NlvStrLastError(void);

long          NlvStrtol(const char* nptr, char** endptr, int base);
unsigned long NlvStrtoul(const char* nptr, char** endptr, int base);
NlvUint64     NlvStrtoull(const char* nptr, char** endptr, int base);
void*         NlvStrtoPtr(const char* nptr, char** endptr);
double        NlvStrtod(const char* nptr, char** endptr);
double        NlvStrtodSIDecMult(const char* nptr, char** endptr);
char*         NlvPtrtoStr(char buf[24], const void* val);

struct NlvRandState {
    unsigned long state[31];
    unsigned long *fptr;
    unsigned long *rptr;
};

void	NlvRandomInit(struct NlvRandState*);
void	NlvSrandom(struct NlvRandState*, unsigned long seed);
long	NlvRandom(struct NlvRandState*);

int NlvIsAscii(int);
int NlvIsPrint(int);
int NlvIsAlpha(int);
int NlvIsDigit(int);
int NlvIsAlnum(int);
int NlvIsCntrl(int);
int NlvIsLower(int);
int NlvIsSpace(int);
int NlvIsUpper(int);
int NlvIsXDigit(int);

/* ============================================================================
 * service functions
 */
unsigned long NlvGetTime(void);
char*         NlvGetCTime(char buf[32]);
char*         NlvGetFmtTime(char* buf, unsigned maxlen, const char* fmt);

long          Nlv_get_cpu_time_ms(void);
const char*   Nlv_get_username(void);


/* ===========================================================================
 * C-arrays	- sort and binary search
 *
 * The compare function NlvCmpFunc get two pointer to the entities to compare
 * and must return <0, ==, or >0
 * ===========================================================================
 */
typedef int (*NlvCmpFunc)(const void*,const void*);
void* NlvBsearch(const void* key, const void*, unsigned, unsigned, NlvCmpFunc);
#ifdef WIN
void  NlvQsort(const void* base, int nel, int elsize, NlvCmpFunc);
#else
void  NlvQsort(void* base, int nel, int elsize, NlvCmpFunc);
#endif


/* ============================================================================
 * NlvASSERT() Macro
 * ============================================================================
 */
#ifdef NDEBUG
#define NlvASSERT(e) ((void)0)
#else /* NDEBUG */
#if defined(__STDC__) || defined(WIN32) || defined(WIN)
#define NlvASSERT(e) ((e) ? (void)0 : NlvAssertionError(__FILE__,__LINE__,#e))
#else   /* PCC */
#define NlvASSERT(e) ((e) ? (void)0 : NlvAssertionError(__FILE__,__LINE__,"e"))
#endif
#endif /* NDEBUG */
void NlvAssertionError(const char* file, int line, const char* expr);
void NlvPrintError(const char* file, int line, const char* expr, int terminate);


/* ============================================================================
 * define TLS_PREFIX 
 * ============================================================================
 */
#ifdef  TLS
#define TLS_PREFIX TLS
#else
#define TLS_PREFIX /**/
#endif


/* ============================================================================
 * Reimplemented libc stdio functions, impl in osprintf.c and osscanf.c
 * ============================================================================
 */
#if defined(GNUC) && !defined(MINGW)
#define FMT_CHECK(a,b)  __attribute__((format (printf, a, b)))
#define SCF_CHECK(a,b)  __attribute__((format (scanf, a, b)))
#else
#define FMT_CHECK(a,b)  /* */
#define SCF_CHECK(a,b)  /* */
#endif

extern int NlvSprintf( char*,           const char*, ...) FMT_CHECK(2,3);
extern int NlvSnprintf(char*, unsigned, const char*, ...) FMT_CHECK(3,4);
extern int NlvSscanf(  const char*,     const char*, ...) SCF_CHECK(2,3);


#ifdef __cplusplus
}
#endif

#endif

