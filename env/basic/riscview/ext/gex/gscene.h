/*  gscene.h 1.6 2013/07/11

    Copyright 2009-2013 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	Populate a Qt QGraphicsScene thru Nlview's Graphics Export Interface

    Author:
	Jochen Roemmler

    Description:
	The C++ module Gscene collects schematic data of the current Nlview
	module of the given Nlview page number thru
	Nlview's Graphics Export Interface (GEI) and adds it to a given
	instance of QGraphicsScene Qt class.

	For more details about accessing GEI, see comments in gdump.h.
    ===========================================================================
*/
#ifndef gscene_h
#define gscene_h

#include <qglobal.h>
#if QT_VERSION >= 0x040400

struct GeiAccessFunc;
class QGraphicsScene;
class QString;

#if   (defined(_WIN32) || defined(__CYGWIN32__)) && defined(NLVIEW_DLLEXPORT)
#define DLL __declspec(dllexport)
#elif (defined(_WIN32) || defined(__CYGWIN32__)) && defined(NLVIEW_DLLIMPORT)
#define DLL __declspec(dllimport)
#else
#define DLL /**/
#endif



/* ===========================================================================
 * All functions return true on success or false on error. If they
 * return false, then GexSceneLastErr() will return the error message.
 * ===========================================================================
 */
DLL bool GexScene(const struct GeiAccessFunc*,class QGraphicsScene*,int pageNo);
DLL const QString& GexSceneLastErr();

#endif /* QT_VERSION */

#endif	/* gscene_h */
