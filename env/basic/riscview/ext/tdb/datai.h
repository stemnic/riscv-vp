/* datai.h 1.2 2008/01/11
    Copyright 2007-2008 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Description:
	Declare the DataBase root in data.c and datatrans.c
  ===================================================================
*/
#ifndef datai_h
#define datai_h

extern struct DB* DataRoot;		/* implemented in data.c */
extern struct DB* DataTransRoot;	/* implemented in datatrans.c */

#endif
