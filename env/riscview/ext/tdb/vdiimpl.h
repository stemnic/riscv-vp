/* vdiimpl.h 1.3 2008/01/11
    Copyright 2007-2008 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Description:
	Declare the vdiimpl function vector
  ===================================================================
*/
#ifndef vdiimpl_h
#define vdiimpl_h

#ifdef __cplusplus
extern "C" {
#endif

extern struct VdiAccessFunc* VdiImpl;	/* implemented in vdiimpl.c and .cpp */

#ifdef __cplusplus
}
#endif

#endif
