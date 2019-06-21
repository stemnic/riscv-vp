/*  ghash.h 1.9 2018/09/11

    Copyright 2004-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	Hash Table Template
    ===========================================================================
*/

#ifndef ghash_h
#define ghash_h

#if (defined(_WIN32) || defined(__CYGWIN32__))&& !defined(WIN)
#define WIN
#endif

/* ===========================================================================
 * GeHashGetPrime   - private to hash table
 * GeHashFreeMemory - private to hash table
 * GeHashMoreMemory - private to hash table
 *
 * GeStrhash - compute hash key from given string
 *            This function (together with GeStrcmp) can be used for
 *	      hash tables with string key.
 * GeStrcasehash - compute hash key from given string case insensitive
 *            This function (together with GeStrcasecmp) can be used for
 *	      hash tables with string key.
 * ===========================================================================
 */
struct GeHashMemory;
unsigned GeHashGetPrime(unsigned nBuckets);
void     GeHashFreeMemory(struct GeHashMemory*);
void*	 GeHashMoreMemory(struct GeHashMemory**, int esize, unsigned count);


/* ===========================================================================
 * Each hash table struct "T" must get initialized by ..Init() before it
 * can be used.  Calling ..Alloc() is optional and can be used to
 * preset a bucket size.  The hash table increases its bucket size
 * automatically when required.  At the end, the table memory should be
 * freed by calling ..Free() - this function does of course not free the
 * hash table struct.
 *
 * A hash table stores key-value pairs.  The ..Insert() function adds
 * a key-value pair to the hash table (without checking for duplicate keys).
 * If the ..Find() function finds the given key, then it returns true
 * and returns the associated value; the ..FindV() function works similar,
 * except it returns a pointer to the associated value - it points to
 * memory inside the hash table - or NULL.
 *
 * The ..Init() function needs two function pointer arguments that are
 * used for the hash key generation and for equality comparison. The
 * "hashFunc" should return an unsigned number 0...MAXINT (from given key)
 * and the "compFunc" should return 0 if the given keys are equal (and != 0
 * otherwise).  If the hash key is "const char*" (common case), then
 * GeStrhash and GeStrcmp can be used for "hashFunc" and "compFunc".
 * ===========================================================================
 */
#if defined(__STDC__) || defined(WIN) || defined(HPUX)
#define HCONCAT(a, b) a##b
#else
#define HCONCAT(a, b) a/**/b
#endif

/* ===========================================================================
 * declareHash	- define struct and function prototypes for hash table
 *
 * The T is the table name, it is used as a prefix to the hash table access
 * functions.  The K and V are the key and value types.  The S is either
 * empty or "static" to make all access function static functions.
 * The table function are ..Init, ..Alloc and ..Free
 * plus the access functions ..Insert, ..Find
 *
 * Example:
 *	struct Node {...};
 *	declareHash(,Reg,const char*,struct Node);
 *
 *	struct Reg  reg;
 *	struct Node node;
 *	...
 *	RegInit(&reg, GeStrhash, GeStrcmp);
 *	RegInsert(&reg, "text1", &node);
 *	RegInsert(&reg, "text2", &node);
 *	if (RegFind(&reg, "text1", &node)) {
 *		return value is in "node"
 *	}
 *      RegFree(&reg);
 * ===========================================================================
 */
#define declareHashStruct(T,K,V) \
struct T { \
    struct HCONCAT(T,Entry) ** bucketList; \
    unsigned    nBuckets; \
    unsigned    count; \
    unsigned (*hashFunc)(K); \
    int      (*compFunc)(K,K); \
    struct GeHashMemory* memoryBuffer; \
    struct HCONCAT(T,Entry)* freeList; \
};

#define declareHashProto(S,T,K,V) \
struct HCONCAT(T,Entry) { \
    struct HCONCAT(T,Entry)* next; \
    K  key; \
    V  value; \
}; \
 \
S void HCONCAT(T,Init)( struct T*, unsigned (*hash)(K), int (*comp)(K,K)); \
S void HCONCAT(T,Alloc)(struct T*, unsigned nBuckets); \
S void HCONCAT(T,Free)( struct T*); \
S V*          HCONCAT(T,Insert)(struct T*, K, V*); \
S int/*bool*/ HCONCAT(T,Find)(  const struct T*, K, V*); \

#define declareHash(S,T,K,V) \
declareHashStruct(T,K,V) \
declareHashProto(S,T,K,V)


/* ===========================================================================
 * declareHashIter - define struct and function prototypes for table iterator
 *
 * It uses the same arguments as declareHash above.  The iterator
 * struct is a concatenated name from the table name plus "Iter".
 * The iterator function are ..IterInit, ..IterMore and ..IterNext
 * plus the access functions ..IterKey and ..IterValue.
 *
 * Example:
 *	declareHashIter(,Reg,const char*,struct Node);
 *
 *	struct Reg reg;
 *	...
 *	struct RegIter it;
 *	for (RegIterInit(&it, &reg); RegIterMore(&it); RegIterNext(&it)) {
 *		const char* key  = RegIterKey(&it);
 *		struct Node node = *RegIterValue(&it);
 *	}
 *
 * The alternative ..InitKey initializes the iterator to loop only over
 * duplicate entries of the given key.
 * ===========================================================================
 */
#define declareHashIter(S,T,K,V) \
struct HCONCAT(T,Iter) { \
    const struct T*  tab; \
    unsigned int     slot; \
    int/*bool*/      dupl_only; /* set by InitKey - duplicates only */ \
    struct HCONCAT(T,Entry)* cur; \
}; \
S void        HCONCAT(T,IterInit)( struct HCONCAT(T,Iter)*, const struct T*); \
S void     HCONCAT(T,IterInitKey)( struct HCONCAT(T,Iter)*, const struct T*,K);\
S int/*bool*/ HCONCAT(T,IterMore)( const struct HCONCAT(T,Iter)*); \
S int/*bool*/ HCONCAT(T,IterNext)( struct HCONCAT(T,Iter)*); \
S K           HCONCAT(T,IterKey)(  const struct HCONCAT(T,Iter)*); \
S V*          HCONCAT(T,IterValue)(const struct HCONCAT(T,Iter)*);


/* ===========================================================================
 * implementHash - implement functions from declareHash
 *
 * It uses the same arguments as declareHash.
 * ===========================================================================
 */
#define avoidCompilerWarning(T) \
    /* ref to Find avoid compiler warning */ \
    (void)(HCONCAT(T,Find));

#define implementHash(S,T,K,V) \
S void HCONCAT(T,Init)(struct T* t, unsigned (*hash)(K), int (*comp)(K,K)) { \
    avoidCompilerWarning(T) \
    t->nBuckets  = 0; \
    t->count     = 0; \
    t->hashFunc  = hash; \
    t->compFunc  = comp; \
    t->bucketList   = NULL; \
    t->memoryBuffer = NULL; \
    t->freeList     = NULL; \
} \
 \
S void HCONCAT(T,Alloc)(struct T* t, unsigned nBuckets) { \
    struct HCONCAT(T,Entry) **newBucketList, *entry, *newEntry; \
    unsigned slot, newSlot; \
 \
    nBuckets = GeHashGetPrime(nBuckets); \
    if (t->nBuckets == nBuckets) return; \
 \
    newBucketList=(struct HCONCAT(T,Entry)**)malloc(nBuckets*sizeof(void*)); \
    memset(newBucketList, '\0', nBuckets*sizeof(void*)); \
 \
    /* loop over old "t->bucketList" (if available) and for each \
     * entry, recompute the hash key and insert the entry into the \
     * "newBucketList" \
     */ \
    for (slot=0; slot < t->nBuckets; slot++) { \
        for (entry = t->bucketList[slot]; entry; ) { \
            newSlot = (t->hashFunc)(entry->key) % nBuckets; \
            newEntry = entry; \
            entry = entry->next; \
            newEntry->next = newBucketList[newSlot]; \
            newBucketList[newSlot] = newEntry; \
        } \
    } \
    if (t->nBuckets) free(t->bucketList); \
    t->bucketList = newBucketList; \
    t->nBuckets = nBuckets; \
} \
  \
S void HCONCAT(T,Free)( struct T* t) { \
    if (t->nBuckets) free(t->bucketList); \
    t->bucketList = NULL; \
    t->nBuckets = 0; \
    GeHashFreeMemory(t->memoryBuffer); \
    t->memoryBuffer = NULL; \
    t->freeList = NULL; \
    t->count = 0; \
} \
  \
S V* HCONCAT(T,Insert)(struct T* t, K key, V* valPtr) { \
    struct HCONCAT(T,Entry) *entry; \
    unsigned slot; \
    if (t->count >= t->nBuckets*5) { \
        /* enlarge by 4X (because Hash's Alloc rounds up to ~ 2^n) */ \
        HCONCAT(T,Alloc)(t, t->nBuckets*3); \
    } \
    if (t->freeList == NULL) { \
	t->freeList=(struct HCONCAT(T,Entry)*)GeHashMoreMemory( \
        &t->memoryBuffer, (int)sizeof(*entry), t->nBuckets); \
    } \
 \
    /* grab Entry memory from freeList and store key and value in it */ \
    entry = t->freeList; \
    t->freeList = entry->next; \
    entry->key = key; \
    entry->value = *valPtr; \
 \
    /* insert Entry at front of bucket list */ \
    slot = (*t->hashFunc)(key) % t->nBuckets; \
    entry->next = t->bucketList[slot]; \
    t->bucketList[slot] = entry; \
    t->count++; \
    return &entry->value; \
} \
 \
S int/*bool*/ HCONCAT(T,Find)(const struct T* t, K key, V* valPtr) { \
    struct HCONCAT(T,Entry)* entry; \
    unsigned slot; \
    if (t->nBuckets == 0) return 0/*false*/; \
    slot = (*t->hashFunc)(key) % t->nBuckets; \
    for (entry = t->bucketList[slot]; entry; entry = entry->next) { \
        if ((*t->compFunc)(key, entry->key)==0) { \
	    *valPtr = entry->value; \
            return 1/*true*/; \
        } \
    } \
    return 0/*false*/; \
}

/* ===========================================================================
 * implementHashIter - implement functions from declareHashIter
 *
 * It uses the same arguments as declareHashIter.
 * ===========================================================================
 */
#define avoidCompilerWarning2(T) \
    /* ref to IterInitKey, IterKey and IterValue to avoid compiler warning */ \
    (void)(HCONCAT(T,IterInitKey)); \
    (void)(HCONCAT(T,IterKey)); (void)(HCONCAT(T,IterValue)); \
    \
    /* ref to IterInit, IterKey and IterValue to avoid compiler warning */ \
    (void)(HCONCAT(T,IterInit)); \
    (void)(HCONCAT(T,IterKey)); (void)(HCONCAT(T,IterValue));

#define implementHashIter(S,T,K,V) \
S void HCONCAT(T,IterInit)(struct HCONCAT(T,Iter)* iter, const struct T* t) { \
    avoidCompilerWarning2(T) \
    iter->tab = t; \
    iter->cur   = 0; \
    iter->dupl_only = 0; \
    for(iter->slot=0; iter->slot < iter->tab->nBuckets; iter->slot++) { \
        iter->cur = t->bucketList[iter->slot]; \
        if(iter->cur) break; \
    } \
} \
S void HCONCAT(T,IterInitKey)(struct HCONCAT(T,Iter)* iter, const struct T* t, \
    K key) { \
    struct HCONCAT(T,Entry)* e; \
    unsigned slot; \
    avoidCompilerWarning2(T) \
    iter->tab = t; \
    iter->cur   = 0; \
    iter->dupl_only = 1; \
    if (t->nBuckets == 0) return; \
    slot = (*t->hashFunc)(key) % t->nBuckets; \
    for (e = t->bucketList[slot]; e; e = e->next) { \
        if ((*t->compFunc)(key, e->key) == 0) break; \
    } \
    iter->cur = e; \
} \
 \
S int/*bool*/ HCONCAT(T,IterMore)(const struct HCONCAT(T,Iter)* iter) { \
    return iter->cur != 0; \
} \
 \
S int/*bool*/ HCONCAT(T,IterNext)(struct HCONCAT(T,Iter)* iter) \
{ \
    if(iter->cur) { \
	if(iter->dupl_only) { \
	    struct HCONCAT(T,Entry)* e; \
	    for (e = iter->cur->next; e; e = e->next) { \
		if ((*iter->tab->compFunc)(iter->cur->key, e->key) == 0) break;\
	    } \
	    iter->cur = e; \
	} else { \
	    iter->cur = iter->cur->next; \
	    while(!iter->cur && iter->slot < iter->tab->nBuckets-1) { \
		iter->cur = iter->tab->bucketList[++iter->slot]; \
	    } \
	} \
    } \
    return iter->cur != 0; \
} \
 \
S K  HCONCAT(T,IterKey)(const struct HCONCAT(T,Iter)* iter) { \
    return iter->cur->key; \
} \
S V* HCONCAT(T,IterValue)(const struct HCONCAT(T,Iter)* iter) { \
    return &iter->cur->value; \
}

#endif /* ghash_h */
