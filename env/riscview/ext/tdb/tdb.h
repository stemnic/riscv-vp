/* tdb.h 1.10 2012/04/03
    Copyright 2007-2012 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Description:

	This is the native DLL interface to data.c, datatrans.c and vdiimpl.c.
	It declares the init function:

	    extern struct VdiDB* TdbInit(struct VdiAccessFunc** func, int);

	On Windows, this should be compiled with
	    -DNLVIEW_DLLEXPORT	- for building this tdb.DLL
	or
	    -DNLVIEW_DLLIMPORT	- for building tapp if importing the tdb.DLL
	or
	    nothing 		- for building tapp statically linked
  ===================================================================
*/
#ifndef tdb_h
#define tdb_h

#ifdef __cplusplus
extern "C" {
#endif

struct VdiAccessFunc;
struct VdiDB;

#if defined(_WIN32) || defined(__CYGWIN32__)
#  ifdef NLVIEW_DLLEXPORT
    __declspec(dllexport)
#  endif
#  ifdef NLVIEW_DLLIMPORT
    __declspec(dllimport)
#  endif
#endif

extern struct VdiDB* TdbInit(struct VdiAccessFunc**, int/*bool*/transistorMode);

#ifdef __cplusplus
}
#endif

#endif
