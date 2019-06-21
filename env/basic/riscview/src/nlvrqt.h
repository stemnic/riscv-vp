/*  nlvrqt.h 1.24 2018/12/12

    Copyright 2003-2018 by Concept Engineering GmbH
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	THIS IS A PRIVATE INTERFACE of NlviewQT.
	It implements the NlvRend C-interface for printing and drawing in Qt.

    Description:
        Qt's drawing mechanism uses a QPainter class to draw on a
        QPaintDevice subclass. These subclasses include
             QWidget  -- draws on a screen
             QPixmap  -- draws into a pixel-based buffer (like QWidget does)
             QPrinter -- draws on a printer
             QPicture -- draws onto a device independent surface
             ... and many more (depends on Qt version).

        We translate the internal drawing functions to QPainter's member
        functions and draw on a given QPaintDevice object.
        This Qt approach separates the drawing code from the underlying
	device being drawn on.

    Author:
	Jochen Roemmler
    ===========================================================================
*/

#ifndef NLVRQT_H
#define NLVRQT_H

/* forward declarations */
class QPainter;
class QPaintDevice;
class QPixmap;
class QImage;
class QPicture;
class QRect;

#include <qglobal.h>
#if QT_VERSION >= 0x040000
#include <QHash>
#include <QString>
#include <QFont>
#else
#include <qdict.h>
#include <qstring.h>
#include <qfont.h>
#endif

/*!
 * \internal
 * used as value for hash map in class ImageMap.
 */
extern "C" {			// POD
struct ImageMapValue {
    union {			// this union saves us from casting explicitly
	QPaintDevice* paintdevice;
	QPixmap*      pix;
	QImage*       img;
	QPicture*     pic;
    } u;
    bool must_delete;
    enum Type {PIXMAP, IMAGE, PICTURE} type;
    int  imgwidth;
    int  imgheight;
    bool limitscale;		// limit scaling to max image's natural size
};
}

/*!
 * \internal
 * helper class used as a container for images of different types:
 * a hash table that maps names to values (holding pointers to images).
 */
class ImageMap {

 public:
    ImageMap();
    ~ImageMap();

    void add(const QString& iname, QPaintDevice* img,
	     enum ImageMapValue::Type t, bool limitscale, bool free);
    void remove(const QString& iname);
    bool contains(const QString& iname) const;
    ImageMapValue value(const QString& iname) const;

private:
#if QT_VERSION >= 0x040000
    QHash<QString,ImageMapValue>* map;
#else
    QDict<ImageMapValue>*         map;
#endif
};


/*************************************************************************
 * Qt Renderer - Implements the NlvRend Interface
 * ==============================================
 *
 * The Qt Renderer's interface is C instead of C++ to fit easily into the
 * existing built-in (core) renderer. Implementation however, will be of
 * course in C++ since it contains Qt code.
 *
 * NlvRQt_construct()  - allocates the memory required for the Qt Renderer.
 * NlvRQt_destruct()   - frees     the memory required for the Qt Renderer.
 * 
 * NlvRQt_init()        - initialize on an external QPainter
 * NlvRQt_finit()       - reset internal state
 *
 *************************************************************************
 */
struct NlvRend* NlvRQt_construct(bool perfect_fontscale);
void NlvRQt_destruct(struct NlvRend*);


/************************************************************************
 * NlvRQt_init()        - starts the draw cycle
 * NlvRQt_initPrinter() - starts the draw cycle (in print mode)
 * ============================================================
 *
 * The QPainter must be initialized on a QPaintDevice before passed in.
 * (by calling QPainter::begin() or by using QPainter's constructor that
 * takes a QPaintDevice); The QPainter* to use must be created/destroyed
 * by the caller.
 *
 ************************************************************************
 */
void NlvRQt_init(struct NlvRend*, QPainter*, const QRect* clipRect = 0);
void NlvRQt_initPrinter(struct NlvRend*,
			QPainter*,
			const QRect*  printarea,
			char          printmode);


/************************************************************************
 * NlvRQt_finit() - reset internal state
 * =====================================
 *
 * The drawing cycle is not finished, yet.
 * The caller can still continue to use the QPainter.
 *
 ************************************************************************
 */
void NlvRQt_finit(struct NlvRend*);

/************************************************************************
 * NlvRQt_setImageMap() - pass list of registered images for
 * 			  NlvRend.draw_image and NlvRend.get_image
 * ===============================================================
 *
 * This function can be called outside of _init/_finit cycle.
 *
 ************************************************************************
 */
void NlvRQt_setImageMap(struct NlvRend*, const ImageMap* imageMap);

/************************************************************************
 * NlvRQt_set_basefont() - set the font to use for both
 * 			   rendering (NlvRend.draw_text) and
 * 			   text metrics (NlvRend.get_text_*)
 * =====================================================================
 *
 * This function can be called outside of _init/_finit cycle.
 *
 ************************************************************************
 */
void NlvRQt_set_basefont(struct NlvRend* rend, const QFont& font);

#endif  /* NLVRQT_H */
