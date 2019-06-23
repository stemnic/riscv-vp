/* tdb.c 1.5 2018/09/11
    Copyright 2007-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Description:
	For SOURCE RELEASES only (mfc/qt/wx)

	This is a mini-wrapper around data.c, datatrans.c and vdiimpl.c.
	It is used to implement a native Windows DLL, used for the
	source releases mfc/qt/wx

	Note: tk/ja/web use their own mini-wrapper to fit the
	GUI system's requirements.
  ===================================================================
*/
#include "tdb.h"
#include "datai.h"
#include "vdiimpl.h"

struct VdiAccessFunc;
struct VdiDB;

/* ============================================================================
 * TdbInit	- initialize the Tdb (for Gate- or Transistor-Level)
 * ============================================================================
 */
struct VdiDB* TdbInit(struct VdiAccessFunc** func, int/*bool*/ transistor)
{
    *func  = VdiImpl;
    return transistor ? (void*)DataTransRoot : (void*)DataRoot;
}
