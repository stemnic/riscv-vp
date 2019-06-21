/*  gassert.h 1.4 2012/04/13

    Copyright 2003-2012 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	Replace system assert.h to avoid linker problems on some platforms
    ===========================================================================
*/

#ifndef gassert_h
#define gassert_h

#if defined(_MSC_VER) && (_MSC_VER <= 1400)
#pragma warning(disable : 4206)
#endif


#ifdef NDEBUG

#define assert(e) ((void)0)

#else /* NDEBUG */
#ifdef __STDC__
#define assert(e) ((e) ? (void)0 : \
    (fprintf(stderr,"%.50s:%d - %.100s\n",__FILE__,__LINE__,#e), abort()))
#else   /* K&R Style */
#define assert(e) ((e) ? (void)0 : \
    (fprintf(stderr,"%.50s:%d - %.100s\n",__FILE__,__LINE__,"e"), abort()))
#endif

#endif /* NDEBUG */

#endif	/* gassert_h */
