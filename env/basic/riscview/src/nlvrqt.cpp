/*  nlvrqt.cpp 1.92 2018/12/21

    Copyright 2003-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	Qt Renderer implementation (C Interface)

    Description:
	This module implements Nlview's NlvRend C interface.
	It is used by the Qt wrapper to draw on the screen / printer
	(in general onto any QPaintDevice).

    Known problems:
        - the text bounding boxes are not 100% correct for zoom factors other
          than 1 as long as we don't have smooth font scaling -> overlaps
        - screen resolutions with more than 32768 pixels per axis are
          not supported due to a bug in Qt < 4. Doesn't apply to Qt >= 4.x.
        - Anti-aliased text rendering conflicts with Nlview's
	  behavior of drawing text above already existing text
	  (causes unwanted distortion).
	  Therefore, we disable anti-aliasing of text (Qt4 only),
	  but only if there is no clipping rectangle available.
	  Proper clipping solves the problem.
    Fixed problems:
        - Since Qt 4 dashed lines may have different dash "offsets".
          This might result in unexpected dash overlaps that cause dashed
	  lines to become solid in the end during scrolling/panning
	  (where new and old content overlaps).
	  See also Qt task tracker id 142471. This is fixed in Qt 4.3.0.
	- Qt 4.1.x - 4.2.x: Qt task tracker id 140952:
	  When drawing text on a rotated QPainter, the text might get
          clipped on some setups. This is noticeable on Solaris systems
          with no Xrender and FontConfig support.

    ===========================================================================
*/
#if (defined(_WIN32) || defined(__CYGWIN32__)) && !defined(WIN)
#define WIN
#endif

#include "nlvos.h"
#include "nlvcore.h"
#include "nlvrqt.h"
#include <math.h>		// sqrt, cos, sin

#include <qglobal.h>
#if QT_VERSION >= 0x040000	// Qt4+
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPaintDevice>
#include <QPainter>
#include <QPen>
#include <QPolygon>
#include <QRadialGradient>
#include <QRect>
#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <QPicture>
#else				// Qt 2.x - 3.x
#include <qcolor.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qnamespace.h>
#include <qpaintdevicemetrics.h>
#include <qpaintdevice.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpointarray.h>
#include <qrect.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpicture.h>

// make source code upward compatible with Qt4+
typedef class QPointArray QPolygon;
#define setPointSizeF setPointSizeFloat

#if !defined(Q_ASSERT)	// Q_ASSERT is called ASSERT in Qt 2.x
#define Q_ASSERT(x) ASSERT(x)
#endif

#endif // QT_VERSION


/**
 * Qt 3.0.0 thru 3.1.2 has a known bug in rotating text more than 45 degrees
 * at a time: it resulted in truncated vertical text.
 */
#if (QT_VERSION >= 300) && (QT_VERSION <= 0x030102)
#define DO_ROTATE_90(rotate) { \
    rotate(-30); \
    rotate(-30); \
    rotate(-30); }
#else
#define DO_ROTATE_90(rotate) rotate(-90);
#endif

#if ((QT_VERSION >= 0x040805) && (QT_VERSION < 0x040807)) || \
    ((QT_VERSION >= 0x050300) && (QT_VERSION < 0x050302))
/* Walk around QTBUG-31579 present in (4.8.5..4.8.6) and (5.3.0..5.3.1) */
#define AVOIDQTBUG31579
#endif


#ifdef Q_NO_BOOL_TYPE
#define true  1
#define false 0
#endif

////////////////////////////////////////////////////////////////////////
// Implementation of private class ImageMap
//
ImageMap::ImageMap()
{
#if QT_VERSION >= 0x040000
    map = new QHash<QString,ImageMapValue>;
#else
    map = new QDict<ImageMapValue>;
    map->setAutoDelete(true);
#endif
}

ImageMap::~ImageMap()
{
#if QT_VERSION >= 0x040000
    foreach (ImageMapValue v, *map) {
	if (v.must_delete) delete v.u.paintdevice;
    }
#else
    QDictIterator<ImageMapValue> it(*map);
    for(; it.current(); ++it) {
	ImageMapValue* v = it.current();
	if (v->must_delete) delete v->u.paintdevice;
    }
#endif
    delete map;
    map = NULL;
}

void ImageMap::add(const QString& iname, QPaintDevice* img,
	enum ImageMapValue::Type t, bool limitscale, bool free)
{
    remove(iname);		// potentially free existing image
#if QT_VERSION >= 0x040000
    ImageMapValue v;
    v.u.paintdevice  = img;
    v.type           = t;
    v.must_delete    = free;
    v.imgwidth       = img->width();
    v.imgheight      = img->height();
    v.limitscale     = limitscale;
    (*map)[iname]    = v;
#else
    ImageMapValue* v = new ImageMapValue;
    v->u.paintdevice = img;
    v->type          = t;
    v->must_delete   = free;
    v->limitscale    = limitscale;

    switch (t) {
	case ImageMapValue::PIXMAP: {
	    v->imgwidth  = v->u.pix->width();
	    v->imgheight = v->u.pix->height();
	    break;
	}
	case ImageMapValue::IMAGE: {
	    v->imgwidth  = v->u.img->width();
	    v->imgheight = v->u.img->height();
	    break;
	}
	case ImageMapValue::PICTURE: {
#if QT_VERSION >= 300
	    QRect b = v->u.pic->boundingRect();
	    v->imgwidth  = b.width();
	    v->imgheight = b.height();
#else
	    // We cannot measure the size of QPicture in Qt 2.x!
	    // The QPaintDeviceMetrics are hard-wired to 640x480
#endif
	    break;
	}
    }
    map->replace(iname, v);
#endif
}

void ImageMap::remove(const QString& iname)
{
#if QT_VERSION >= 0x040000
    if (!map->contains(iname)) return;
    ImageMapValue v = map->take(iname);
    if (v.must_delete) delete v.u.paintdevice;
#else
    ImageMapValue* v = map->find(iname);
    if (v) {
	if (v->must_delete) delete v->u.paintdevice;
	map->remove(iname);
    }
#endif
}

bool ImageMap::contains(const QString& iname) const
{
#if QT_VERSION >= 0x040000
    return map->contains(iname);
#else
    return map->find(iname) != NULL;
#endif
}

ImageMapValue ImageMap::value(const QString& iname) const
{
    Q_ASSERT(contains(iname));
#if QT_VERSION >= 0x040000
    return (*map)[iname];
#else
    return *map->find(iname);
#endif
}



/**
 * extend struct NlvRend with private data needed for the implementation (POD).
 */
struct QtRend {

    struct NlvRend rend;       /* "base" class */

    int           xOrigin;
    int           yOrigin;

    QFont*        baseFont;
    QPainter*     curPainter;
    QColor*       _cachedLineColor;
    QColor*       _cachedFontColor;
    float         _cachedFontSize;
    const ImageMap* imageMap;
    bool	  perfect_fontscale;

    struct {
	bool rotate;
	char mode;
    } printer;

};
typedef struct QtRend QtRend;


/* ======================================================================
 * various helper functions
 * ======================================================================
 */

/* ======================================================================
 * apply_justify - a text service function
 *
 * parameters:
 *   xoff      : x-offset return value
 *   yoff      : y-offset return value
 *   w	       : text width (in Pixel)
 *   ascent    : font ascent
 *   descent   : font descent
 *   justify   : text alignment, see below
 *
 * The following text alignments are supported by Nlview:
 *
 *           1---2---3
 *           |   |   |
 *           4---5---6
 *           |   |   |
 *           7---8---9
 *
 * ======================================================================
 */
static void apply_justify(float* xoff, float* yoff,
			  float w, float h,
			  enum NlvRendJustify justify)
{
    switch(justify & NlvJ_JUSTMASK) {

	case NlvJ_UPPERLEFT:     // alignment #1
	    *yoff = 0;
	    *xoff = 0;
	    break;
	case NlvJ_UPPERCENTER:   // alignment #2
	    *yoff = 0;
	    *xoff = w/2;
	    break;
	case NlvJ_UPPERRIGHT:    // alignment #3
	    *yoff = 0;
	    *xoff = w;
	    break;

	case NlvJ_CENTERLEFT:    // alignment #4
	    *yoff = -h/2;
	    *xoff = 0;
	    break;
	case NlvJ_CENTERCENTER:  // alignment #5
	    *yoff = -h/2;
	    *xoff = w/2;
	    break;
	case NlvJ_CENTERRIGHT:   // alignment #6
	    *yoff = -h/2;
	    *xoff = w;
	    break;

	case NlvJ_LOWERLEFT:     // alignment #7
	    *yoff = -h;
	    *xoff = 0;
	    break;
	case NlvJ_LOWERCENTER:   // alignment #8
	    *yoff = -h;
	    *xoff = w/2;
	    break;
	case NlvJ_LOWERRIGHT:    // alignment #9
	    *yoff = -h;
	    *xoff = w;
	    break;

	default:
	    *yoff = 0;
	    *xoff = 0;
	    break;
    }
}



/* ======================================================================
 * castColor - map NlvColor -> QColor
 * ======================================================================
 */
static QColor castColor(QtRend* ths, NlvColor c)
{
    if (ths->printer.mode == 'M' && c.transp == 0) return Qt::black;
    NlvCore_applyPrinterColormode(ths->printer.mode, &c, NULL);

#if QT_VERSION >= 0x040000
    return QColor(c.red, c.green, c.blue, 0xff - c.transp);
#else
    return QColor(c.red, c.green, c.blue);
#endif
}


/* ======================================================================
 * castFillStyle - map enum NlvRendFillStyle -> enum Qt::BrushStyle
 * ======================================================================
 */
inline static enum Qt::BrushStyle castFillStyle(enum NlvRendFillStyle fs)
{
    switch (fs & NlvRendFS_PATTERNS) {
	case NlvRendFS_SOLID:       return Qt::SolidPattern;
	case NlvRendFS_DIAGCROSS:   return Qt::DiagCrossPattern;
	case NlvRendFS_CROSS:       return Qt::CrossPattern;
	case NlvRendFS_BDIAGONAL:   return Qt::BDiagPattern;
	case NlvRendFS_FDIAGONAL:   return Qt::FDiagPattern;
	case NlvRendFS_VERTICAL:    return Qt::VerPattern;
	case NlvRendFS_HORIZONTAL:  return Qt::HorPattern;
	case NlvRendFS_TRANSPARENT:
	default:                    return Qt::NoBrush;
    }
}


/* ======================================================================
 * castLineStyle - map enum NlvRendLineStyle -> enum Qt::PenStyle
 * ======================================================================
 */
inline static enum Qt::PenStyle castLineStyle(enum NlvRendLineStyle style) {
    switch(style) {
#if QT_VERSION >= 0x040000
#define DASHLINE DashLine
#else
#define DASHLINE DotLine	/* comments in bk 1.20 */
#endif
	case NlvRendLS_SOLID:   return Qt::SolidLine;   /* ____________ */
	case NlvRendLS_DASHED:	return Qt::DASHLINE;	/* _ _ _ _ _ _  */
	case NlvRendLS_DASHED2:	return Qt::DashDotLine;	/* _ . _ . _ .  */
	case NlvRendLS_DASHED50:return Qt::DASHLINE;	/* _ _ _ _ _ _  */
	case NlvRendLS_DOTTED:	return Qt::DotLine;	/* ...........  */
    }
    return Qt::SolidLine;	/* please compiler */
}


/**
 * ----------------------------------------------------------------------
 * setCustomDashPattern
 *
 * This routine is setting a custom line dash pattern, which does
 * user-space dashing (scaling with Nlview's zoom factor), but independent
 * from the current QPen width (which is the default unit).
 * The scaling of the dash pattern depends on a Nlview core-internally
 * computed value 'gw' (gap-width), which is not 100% linear to Nlview's
 * zoom factor to avoid extreme cases.
 *
 * The dash offset (phase-shift) is also set per dash pattern for best
 * visual results (requires Qt >= 4.3).
 *
 * Plus it supports extra dash patterns like NlvRendLS_DASHED50.
 *
 * It requires Qt >= 4.1, but there are some bugs in the initial Qt dash
 * pattern implementation that are fixed in later versions, e.g.
 *    Qt 4.1.0 shifts dashed lines one Y coordinate unit too high
 *    at certain zoom factors	(seems to be fixed in Qt 4.2.0)
 *
 *    Qt 4.3.0 fixes the problem with shifting offsets into dashed
 *    patterns with negative X values (142471).
 *
 *    Qt 4.5.1 fixes a long-standing performance bottleneck for very
 *    long lines having custom dash-patterns (160617, 164676, 246573).
 * ----------------------------------------------------------------------
 */
#if QT_VERSION >= 0x040200
inline static void setCustomDashPattern(QPen* pen, float w,
					enum NlvRendLineStyle style, float gw) {
    QVector<qreal> dashes;
    qreal	   phase = 0;

    /* rounding with 'floor' to avoid jitter in Qt combining dash sequences
     * (non anti-aliasing) 
     */
#define RELWIDTH(pixel) floor(gw*((float)(pixel)/3.0F))
#define RELWIDNZ(pixel) NlvMax(RELWIDTH(pixel),1.0)	/* not zero */

    NlvSETMAX(w, 1.0F);	/* just to be sure to avoid a 'division by zero' */
    switch(style) {
	case NlvRendLS_SOLID:
	    return;
	case NlvRendLS_DASHED:		/* 7:3 */
	    dashes << RELWIDNZ(7)/w << RELWIDNZ(3)/w;
	    phase = RELWIDTH(3.5)/w;
	    break;
	case NlvRendLS_DASHED2:		/* 7:2:1:2 */
	    dashes << RELWIDNZ(7)/w << RELWIDNZ(2)/w
		   << RELWIDNZ(1)/w << RELWIDNZ(2)/w;
	    phase = RELWIDTH(3.5)/w;
	    break;
	case NlvRendLS_DASHED50:	/* 5:5 */
	    dashes << RELWIDNZ(5)/w << RELWIDNZ(5)/w;
	    phase = RELWIDTH(2.5)/w;
	    break;
	case NlvRendLS_DOTTED:		/* 1:5 */
	    dashes << RELWIDNZ(1)/w << RELWIDNZ(5)/w;
	    phase = RELWIDTH(3.5)/w;
	    break;
    }
#undef RELWIDTH
#undef RELWIDNZ
    pen->setDashPattern(dashes);
#if QT_VERSION >= 0x040300
    pen->setDashOffset(phase);
#endif
}
#endif // QT_VERSION is >= 4.2.0

/* ======================================================================
 * swap*() - rotate 90 degrees counterclockwise (y is top-to-bottom)
 * ======================================================================
 */
inline static void swapI(int* xP, int* yP) {
    int newx = *yP;
    *yP = -*xP;
    *xP = newx;
}

inline static void swapD(double* xP, double* yP)
{
    double newx = *yP;
    *yP = -*xP;
    *xP = newx;
}

inline static void swapF(float* xP, float* yP)
{
    float newx = *yP;
    *yP = -*xP;
    *xP = newx;
}

/* ============================================================================
 * Define the coordinate range and addPoint function for NlvTrimPoly/NlvClipLine
 * ============================================================================
 */
#if QT_VERSION < 0x040000
/**
 * Qt < 4: We have to clip the coordinate space to 16 bits prior to passing
 * coords to the QPainter::draw...() functions to walk around a bug
 * (internal coordinate overflow [short=int; ?]) in the Qt library.
 * Using QPainter::setClipRect doesn't help either.
 */
enum CoordRange { minCoord = -32768, maxCoord = 32767 };

static void addPoint(QPoint** pv, int x, int y) 
{ 
	(*pv)->setX(x); 
	(*pv)->setY(y); 
	(*pv)++; 
}
#define CUTCOORDS NlvTrimPoly
#define CUTFACTOR 2	/* Max number of cuts per line segment. */

#else
// There is no coordinate range problem in Qt >= 4.x
//
enum CoordRange { minCoord = 0, maxCoord = 0 };			/* not used */
static void addPoint(QPoint**, int, int){ Q_ASSERT(0); }	/* not used */

static int copyCoords(const int* xt, const int* yt, int len,
	int/*bool*/,
	void* buf, unsigned maxlen, NlvAddPointFunc,
	int, int, int xOrigin, int yOrigin, int rotate)
{
	QPoint* pv = (QPoint*)buf;
	int x, y;
	unsigned i;
	for (i=0; i<(unsigned)len; i++) {
	    if (i==maxlen) { Q_ASSERT(0); break; }
	    x = xt[i] - xOrigin; 	
	    y = yt[i] - yOrigin;
	    if (rotate) {
		pv->setX(y);
		pv->setY(-x);
	    } else {
		pv->setX(x);
		pv->setY(y);
	    }
	    pv++;
	}
	return i;
}
#define CUTCOORDS copyCoords
#define CUTFACTOR 1	/* No cuts done */

#endif // QT_VERSION


/* ===========================================================================
 *
 * ("inherited") NlvRend-interface implementation
 *
 * The following "static" functions' addresses are used in
 * struct NlvRend's function vector table to implement the Nlview core
 * rendering interface.
 * ===========================================================================
 */

extern "C" {
static void set_origin(struct NlvRend*, int xOrigin, int yOrigin, int rot);
static void set_path(struct NlvRend*,int,NlvColor,enum NlvRendLineStyle, float);
static void set_font(struct NlvRend*, float size, NlvColor color);
static void draw_line(struct NlvRend*, int x1, int y1, int x2, int y2);
static void draw_rectangle(struct NlvRend*, WBox);
static void draw_poly(struct NlvRend*, int n, const int* xt, const int* yt);
static void draw_circle(struct NlvRend* rend, int x, int y, int diam);
static void draw_arc(struct NlvRend*, double,double, double, double,double);
static void draw_filled_rectangle(struct NlvRend*, WBox, const NlvPaint*);
static void draw_filled_poly(struct NlvRend*, int n, const int*, const int*,
				    const NlvPaint* paint);
static void draw_filled_circle(struct NlvRend* rend, int x, int y, int diam,
				    const NlvPaint* paint);
static int  draw_image(struct NlvRend* rend, const char* name,
		       enum NlvRendOrient, WBox);
static NlvColor* get_image(const struct NlvRend* rend, const char* name,
			   unsigned* w, unsigned* h, int/*bool*/* limitscale);
static void draw_text(struct NlvRend*, const char*, int, float x, float y,
				    enum NlvRendJustify justify);
static void get_text_extend(const struct NlvRend*, float, const char*, int len,
			    enum NlvRendJustify justify, WBox*, int);
static int get_text_width(const struct NlvRend*, const char*, float);
static float get_text_linefeed(const struct NlvRend*);
}



static void set_origin(struct NlvRend* rend, int xOrigin, int yOrigin, int rot)
{
    /* We transform x,y (widget coords) to x',y' (window coords) this way:
     *          x' = x - xOrig
     *          y' = y - yOrig
     * but, if "rotate" == true (only supported for printer), then
     * transform x,y this way:
     *          x' =   y - yOff
     *          y' = -(x - xOff) 
     */
    QtRend* ths = (QtRend*)(void*)rend;
    ths->xOrigin = xOrigin;
    ths->yOrigin = yOrigin;
    ths->printer.rotate = rot>0;

    /* fill pattern is relative to schematic, not relative to window:
     */
    ths->curPainter->setBrushOrigin(-xOrigin, -yOrigin);
}



static void set_path(struct NlvRend* rend,
    int w, NlvColor color, enum NlvRendLineStyle style, float gw)
{
    QtRend* ths = (QtRend*)(void*)rend;
    *ths->_cachedLineColor = castColor(ths, color);

    Q_ASSERT(ths->curPainter);
    // create a new pen and pass it to the current QPainter
#if QT_VERSION >=0x040000
    // default Qt4 join-style is: pen.setJoinStyle(Qt::BevelJoin);
    // default Qt4 cap-style  is: pen.setCapStyle(Qt::SquareCap);
    enum Qt::PenCapStyle capstyle = Qt::FlatCap;
#if defined(WIN) && (QT_VERSION <= 0x040001) && defined(DEBUG)
    // Walk around a Qt bug that raises an assertion in DEBUG mode
    // while using a Qt::FlatCap pen: revert to suboptimal Qt::SquareCap
    // This Qt bug only exists in Qt version 4.0.0 thru 4.0.1
    capstyle = Qt::SquareCap;
#endif // QT_VERSION is [4.0.0 - 4.0.1]

#if defined(WIN) && (QT_VERSION < 0x040101)
    if (w<=1) {
	// Walk around a Qt bug that does not render diagonal lines of
	// width<=1 during printing on a PCL-based QPrinter device
	// (Qt task tracker ID: 89466)
	// Unfortunately we don't know if we're drawing or printing - so we
	// have to do it in either case.
	// (jrr, Feb 17 2006): This has been fixed in Qt/Win 4.1.1.
	w = 2; // increase minimum line width (thicker)
    }
#endif // QT_VERSION is [4.0.0 - 4.1.1)
    QPen pen(*ths->_cachedLineColor, w, castLineStyle(style), capstyle,
		Qt::MiterJoin);
#if QT_VERSION >= 0x040200
    setCustomDashPattern(&pen, w, style, gw);
#else
    (void)gw;	/* user-space dashing not supported */
#endif
#if QT_VERSION >= 0x040100
    pen.setMiterLimit(0.25);	// M880: avoid discrepancy of raster vs. native
#endif
#else  // Qt 2.x - 3.x
    // default join-style is: pen.setJoinStyle(Qt::MiterJoin);
    // default cap-style  is: pen.setCapStyle(Qt::FlatCap);
    QPen pen(*ths->_cachedLineColor, w, castLineStyle(style));
    (void)gw;	/* user-space dashing not supported */
#endif
    ths->curPainter->setPen(pen);
}



static void set_font(struct NlvRend* rend, float size, NlvColor color)
{
    QtRend* ths = (QtRend*)(void*)rend;

    Q_ASSERT(ths->curPainter);
    *ths->_cachedFontColor = castColor(ths, color);
    ths->_cachedFontSize = size;
}



/**
 * Draws a line from (x1, y1) to (x2, y2) and sets the current pen position
 * to (x2, y2).
 */
static void draw_line(struct NlvRend* rend, int x1, int y1, int x2, int y2)
{
    QtRend* ths = (QtRend*)(void*)rend;

#if QT_VERSION < 0x040000
    if (!NlvClipLine(&x1, &y1, &x2, &y2, minCoord, maxCoord,
		     ths->xOrigin, ths->yOrigin, ths->printer.rotate))
	return;			/* complete line is clipped */
#else
    x1 -= ths->xOrigin;
    y1 -= ths->yOrigin;
    x2 -= ths->xOrigin;
    y2 -= ths->yOrigin;
    if (ths->printer.rotate) {
	swapI(&x1, &y1);
	swapI(&x2, &y2);
    }
#endif
    ths->curPainter->drawLine(x1, y1, x2, y2);
}



static void draw_rectangle(struct NlvRend* rend, WBox rect)
{
    QtRend* ths = (QtRend*)(void*)rend;
    int l = rect.left  - ths->xOrigin;
    int t = rect.top   - ths->yOrigin;
    int r = rect.right - ths->xOrigin;
    int b = rect.bot   - ths->yOrigin;

    Q_ASSERT(l<=r);
    Q_ASSERT(t<=b);

    if(ths->printer.rotate) {
	swapI(&l, &t);
	swapI(&r, &b);
    }
#ifdef AVOIDQTBUG31579
    ths->curPainter->drawLine(l, t, r, t);
    ths->curPainter->drawLine(r, t, r, b);
    ths->curPainter->drawLine(r, b, l, b);
    ths->curPainter->drawLine(l, b, l, t);
#else
    ths->curPainter->drawRect(l, t, r-l, b-t);
#endif
}



static void draw_poly(struct NlvRend* rend, int n, const int* xt, const int* yt)
{
    QtRend* ths = (QtRend*)(void*)rend;
    if(n < 2) return;

    QPolygon buf(n*CUTFACTOR);
    n = CUTCOORDS(xt, yt, n, 0/*no autoclose*/, buf.data(), n*CUTFACTOR, 
		    (NlvAddPointFunc)addPoint, minCoord, maxCoord,
		    ths->xOrigin, ths->yOrigin, ths->printer.rotate);
    buf.resize(n);

#ifdef AVOIDQTBUG31579
    for (int i=1; i<n; i++) {
	const QPoint& from = buf.at(i-1);
	const QPoint& to   = buf.at(i);
	ths->curPainter->drawLine(from, to);
    }
#else
    if (xt[0]==xt[n-1] && yt[0]==yt[n-1])
	ths->curPainter->drawPolygon(buf);
    else ths->curPainter->drawPolyline(buf);
#endif
}



static void draw_circle(struct NlvRend* rend, int x, int y, int diam)
{
    QtRend* ths = (QtRend*)(void*)rend;
    if (diam==0) return;	// prevent a crash seen in Qt 4.8.2/Win64

    x -= ths->xOrigin;
    y -= ths->yOrigin;
    if(ths->printer.rotate) swapI(&x,&y);

    ths->curPainter->save();
    ths->curPainter->setBrush(Qt::NoBrush);
    ths->curPainter->drawEllipse(x-diam/2, y-diam/2, diam, diam);
    ths->curPainter->restore();
}



static void draw_arc(struct NlvRend* rend, double xm,double ym, double diam, 
		     double start,double span)
{
    QtRend* ths = (QtRend*)(void*)rend;
    if (diam==0) return;	// prevent a crash seen in Qt 4.8.2/Win64

    xm -= ths->xOrigin;
    ym -= ths->yOrigin;
    if(ths->printer.rotate) {
	swapD(&xm,&ym);
	start -= NlvPI/2.0;
    }
    int startAngleDeg16th = NlvRound(-start/NlvPI*180*16);
    int spanAngleDeg16th  = NlvRound(-span /NlvPI*180*16);

#if (QT_VERSION >= 0x040000)
    ths->curPainter->drawArc(QRectF(xm-diam/2.0, ym-diam/2.0, diam, diam),
			     startAngleDeg16th, spanAngleDeg16th);
#else
    int xp = NlvRound(xm - diam/2);
    int yp = NlvRound(ym - diam/2);
    int di = NlvRound(diam);
    ths->curPainter->drawArc(xp,yp, di,di, startAngleDeg16th, spanAngleDeg16th);
#endif
}



/* =========================================================================
 * Draw filled objects
 *
 * All filled objects get a NULL pen and a new colored+styled brush:
 *   pushFilledBrushPen - creates them and
 *   popFilledBrushPen  - restores the previous brush and pen.
 * =========================================================================
 */
static void pushFilledBrushPen(QtRend* ths, const NlvPaint* paint,
	bool firstcall, bool* again)
{
    ths->curPainter->save();
    ths->curPainter->setPen(Qt::NoPen);

    *again = false;

    if (paint->fillstyle == NlvRendFS_SOLID) {
	ths->curPainter->setBrush(castColor(ths, paint->fg));	/* solid */
	return;
    }
    if (paint->fillstyle == NlvRendFS_SOLID2) {
	ths->curPainter->setBrush(*ths->_cachedLineColor);	/* solid */
	return;
    }
    if (paint->fillstyle & NlvRendFS_LGRADIENT) {
#if (QT_VERSION >= 0x040000)
	QLinearGradient gradient(paint->startx, paint->starty,
				 paint->stopx,  paint->stopy);
	gradient.setColorAt(0, castColor(ths, paint->fg));
	gradient.setColorAt(1, castColor(ths, paint->bg));
	ths->curPainter->setBrush(gradient);
#else
	ths->curPainter->setBrush(castColor(ths, paint->fg));
#endif
	return;
    }

    Q_ASSERT(paint->fillstyle != NlvRendFS_TRANSPARENT);

    /* We have a pattern: draw solid background in 1st call if the paint's
     * background color is set and not fully transparent.
     */
    if (firstcall && paint->bg.transp != 0xff) {
	ths->curPainter->setBrush(castColor(ths, paint->bg));	/* solid */
	*again = true;
	return;
    }

    QBrush brush(castColor(ths, paint->fg), castFillStyle(paint->fillstyle));
    ths->curPainter->setBrush(brush);
}

static void popFilledBrushPen(QtRend* ths)
{
    ths->curPainter->restore();
}

static void draw_filled_rectangle(struct NlvRend* rend, WBox rect,
				  const NlvPaint* paint)
{
    QtRend* ths = (QtRend*)(void*)rend;
    bool again, first = true;
    int l = rect.left  - ths->xOrigin;
    int t = rect.top   - ths->yOrigin;
    int r = rect.right - ths->xOrigin;
    int b = rect.bot   - ths->yOrigin;

    if (paint->nomono && ths->printer.mode == 'M') return;
#if QT_VERSION < 0x040000
    if (paint->noalpha) return;
#endif

    Q_ASSERT(l<=r);
    Q_ASSERT(t<=b);

    if(ths->printer.rotate) {
	swapI(&l, &t);
	swapI(&r, &b);
    }

    do {
	pushFilledBrushPen(ths, paint, first, &again);
	ths->curPainter->fillRect(l,t,r-l,b-t, ths->curPainter->brush());
	popFilledBrushPen(ths);
    } while (first=false, again);
}



static void draw_filled_poly(struct NlvRend* rend, 
			     int n, const int* xt, const int* yt,
			     const NlvPaint* paint)
{
    QtRend* ths = (QtRend*)(void*)rend;
    bool again, first = true;
    if (paint->nomono && ths->printer.mode == 'M') return;
#if QT_VERSION < 0x040000
    if (paint->noalpha) return;
#endif
    if (n < 2) return;

    QPolygon buf((n+1/*for autoclose*/)*CUTFACTOR);
    n = CUTCOORDS(xt, yt, n, 1/*autoclose*/, buf.data(), (n+1)*CUTFACTOR, 
		    (NlvAddPointFunc)addPoint, minCoord, maxCoord,
		    ths->xOrigin, ths->yOrigin, ths->printer.rotate);
    buf.resize(n);

    do {
	pushFilledBrushPen(ths, paint, first, &again);
	ths->curPainter->drawPolygon(buf);
	popFilledBrushPen(ths);
    } while (first=false, again);
}



static void draw_filled_circle(struct NlvRend* rend, int x, int y, int diam,
			       const NlvPaint* paint)
{
    QtRend* ths = (QtRend*)(void*)rend;
    bool again, first = true;

    if (diam==0) return;	// prevent a crash seen in Qt 4.8.2/Win64
    if (paint->nomono && ths->printer.mode == 'M') return;
#if QT_VERSION < 0x040000
    if (paint->noalpha) return;
#endif

    x = x - ths->xOrigin;
    y = y - ths->yOrigin;
    if(ths->printer.rotate) swapI(&x,&y);

    do {
	pushFilledBrushPen(ths, paint, first, &again);
	ths->curPainter->drawEllipse(x-diam/2, y-diam/2, diam, diam);
	popFilledBrushPen(ths);
    } while (first=false, again);
}



static int draw_image(struct NlvRend* rend, const char* name,
		       enum NlvRendOrient orient, WBox rect)
{
    QtRend* ths = (QtRend*)(void*)rend;
    QString iname = QString(name);

    // see also NlviewJA's drawImage()
    //
    if (!ths->imageMap || !ths->imageMap->contains(iname)) {
	if (name[0]!='@') qDebug("draw_image %s --> not registered", name);
	return 0;
    }
    ImageMapValue val = ths->imageMap->value(iname);

    int l = rect.left;
    int t = rect.top;
    int r = rect.right;
    int b = rect.bot;

    int L = l - ths->xOrigin;
    int T = t - ths->yOrigin;
    float W = r-l;
    float H = b-t;

    ths->curPainter->save();
    ths->curPainter->translate(L+W/2, T+H/2);

    if(orient & NlvRendO_FswapXY) {
	ths->curPainter->rotate(-90);

	float tmp = W;
	W = H; H = tmp;		// unrotated W/H
    }

#if QT_VERSION >= 0x040000
    Q_ASSERT(val.imgwidth  == val.u.paintdevice->width());
    Q_ASSERT(val.imgheight == val.u.paintdevice->height());
#endif
    float iw = val.imgwidth;
    float ih = val.imgheight;

    float sx = W/iw;  float sx_error = 1/iw;
    float sy = H/ih;  float sy_error = 1/ih;

    // Check if we have a slightly different scaling for x and y.
    // If so, fix potential rounding problems that may have led to this;
    // this means, make sx==sy if they are close enough.
    //
    if (sx + sx_error >= sy - sy_error &&
	sx - sx_error <= sy + sy_error) {
	sx = sy = (sx + sy) / 2;
    }

    if (val.limitscale) {
	// limit to maximum scaling to 1
	if (sx > 1) sx = 1;
	if (sy > 1) sy = 1;
    }
    if (sx != sy || sx < 0.95 || sx > 1.05) ths->curPainter->scale(sx, sy);

    ths->curPainter->translate(-iw/2, -ih/2);

    switch (val.type) {
	case ImageMapValue::PIXMAP:
		ths->curPainter->drawPixmap(0,0, *val.u.pix);
		break;
	case ImageMapValue::IMAGE:
		ths->curPainter->drawImage( 0,0, *val.u.img);
		break;
	case ImageMapValue::PICTURE: {
#if QT_VERSION >= 0x040000
	    /* We need to account for resolution mismatch of the QPicture
	     * and the target paint device manually? Is this a Qt4 bug?
	     * There are even more (unresolved) QPicture bugs seen in Qt 4.2.0.
	     */
	    const QPaintDevice* src = val.u.paintdevice;
	    const QPaintDevice* tgt = ths->curPainter->device();
	    double resScaleX = (double)src->logicalDpiX()/tgt->logicalDpiX();
	    double resScaleY = (double)src->logicalDpiY()/tgt->logicalDpiY();
	    ths->curPainter->scale(resScaleX,resScaleY);
	    ths->curPainter->drawPicture(0, 0, *val.u.pic);
#elif QT_VERSION >= 300
	    ths->curPainter->drawPicture(0, 0, *val.u.pic);
#else
	    // We cannot measure the size of QPicture in Qt 2.x, i.e.
	    // we cannot scale properly => so we do at least clip:
	    ths->curPainter->setClipRect(L,T,r-l,b-t);
	    ths->curPainter->setClipping(true);
	    ths->curPainter->drawPicture(*val.u.pic);
#endif
	    break;
	}
    }
    ths->curPainter->restore();
    return 1;
}


/* WARNING: This function will be called asynchronously (outside init/finit)
 *          (without a valid QPainter/QPaintDevice)
 */
static NlvColor* get_image(const struct NlvRend* rend, const char* name,
    unsigned* width, unsigned* height, int/*bool*/* limitscale)
{
    const QtRend* ths = (const QtRend*)(void*)rend;
    QString iname = QString(name);

    if (!ths->imageMap || !ths->imageMap->contains(iname)) return NULL;
    ImageMapValue v = ths->imageMap->value(iname);
    *width	= v.imgwidth;
    *height	= v.imgheight;
    *limitscale	= v.limitscale;
    QImage im;
    switch (v.type) {
	case ImageMapValue::PIXMAP: {
#if QT_VERSION >= 0x040000
	    im = v.u.pix->toImage();
#else
	    im = v.u.pix->convertToImage();
#endif
	    if (im.isNull()) break;	// QPixmap => QImage conversion failed

	process_im:
	    // Because Qt prefers a pre-multiplied alpha channel format, we
	    // must convert the image to a format with a separate alpha channel:
	    //
#if QT_VERSION >= 0x040000
	    if (!(im.format() == QImage::Format_ARGB32 ||
		  im.format() == QImage::Format_RGB32)) {
		im = im.convertToFormat(im.hasAlphaChannel() ?
			QImage::Format_ARGB32 :
			QImage::Format_RGB32);
	    }
#else
	    im = im.convertDepth(32/*bpp*/);
#endif
	    unsigned const pixel_cnt = (*width) * (*height);
	    NlvColor* colors = (NlvColor*)NlvMalloc(pixel_cnt*sizeof(NlvColor));
	    NlvColor* p = colors;
	    for (unsigned y=0; y<*height; y++ ) {
#if QT_VERSION >= 0x040700
		const QRgb* pixel = (const QRgb*)im.constScanLine(y);
#else
		const QRgb* pixel = (const QRgb*)im.scanLine(y);
#endif
		for (unsigned x=0; x<*width; x++, pixel++, p++) {
		    p->transp = 0xff - qAlpha(*pixel);
		    p->red    =        qRed(  *pixel);
		    p->green  =        qGreen(*pixel);
		    p->blue   =        qBlue( *pixel);
		}
	    }
	    NlvASSERT((unsigned)(p-colors) == pixel_cnt);
	    return colors;
	}
	case ImageMapValue::IMAGE: {
	    im = *v.u.img;
	    goto process_im;
	}
	case ImageMapValue::PICTURE: {
#if QT_VERSION < 300
	    // We cannot measure the size of QPicture in Qt 2.x!
	    // The QPaintDeviceMetrics are hard-wired to 640x480
	    break;
#else
	    // rasterize the picture using triple resolution for good quality
	    *width  *= 3;
	    *height *= 3;
#  if QT_VERSION >= 0x040000
	    im = QImage((int)*width, (int)*height,
#    if QT_VERSION >= 0x040200
			QImage::Format_ARGB32);
#    else
			QImage::Format_ARGB32_Premultiplied);
#    endif
	    if (im.isNull()) break;		// cannot create QImage
	    QPainter painter(&im);
	    if (!painter.isActive()) break;	// cannot paint on QImage
	    painter.scale(3,3);
	    im.fill(Qt::transparent);
	    painter.drawPicture(0, 0, *v.u.pic);
	    painter.end();
#  else	// Qt3 or earlier:
	    // Because in Qt3 the class QImage does not derive from
	    // QPaintDevice, we cannot directly paint on it like above.
	    // So we have to use a QPixmap. Unfortunately, we cannot
	    // reliably initialize the QPixmap to be "transparent"
	    // (with QPixmap::fill(Qt::transparent)) we are using an
	    // opaque background (Qt::white) as a fallback.
	    QPixmap pm = QPixmap((int)*width, (int)*height, 32/*bpp*/);
	    if (pm.isNull()) break;		// cannot create QPixmap
	    pm.fill();				// ..default: white (opaque)
	    QPainter painter(&pm);
	    if (!painter.isActive()) break;	// cannot paint on QPixmap
	    painter.scale(3,3);
	    painter.drawPicture(0, 0, *v.u.pic);
	    painter.end();
	    // convert the QPixmap to QImage like above
	    im = pm.convertToImage();
	    if (im.isNull()) break;	// QPixmap => QImage conversion failed
#  endif
	    goto process_im;
#endif
	}
    }

    *width = *height = *limitscale = 0;
    return NULL;
}


static void limit_fontmetrics(float fontsize, float* ascent, float* descent)
{
    /* Limit FontMetric's ascent+descent to given fontsize (we need this
     * trick because the FontMetrics.ascent often includes "internal leading"
     * above the character glyphs.
     */
    float dy = *ascent + *descent - fontsize * 1.1F;
    if (dy > 0) {
	/* shrink ascent  by 100% of dy, 
	 * shrink descent by   0% of dy, 
	 * this makes ascent+descent == fontsize * 1.1
	 */
	*descent -= dy* 5/100;
	*ascent  -= dy*95/100;
    }
}

#define FONTSIZE4METRICS 18
#define FONTBOOSTER 1.1F

static void draw_text(struct NlvRend* rend, 
		      const char* txt, int len, float x, float y,
		      enum NlvRendJustify justify)
{
    QtRend* ths = (QtRend*)(void*)rend;

    bool rotate = ths->printer.rotate;
    float ffac;
    QFont font = ths->curPainter->font();

    if (len <= 0 || NlvRound(ths->_cachedFontSize) <= 0) return;

    if (ths->perfect_fontscale) {
	font.setPixelSize(FONTSIZE4METRICS);
	ths->curPainter->setFont(font);
	ffac = ths->_cachedFontSize*FONTBOOSTER / FONTSIZE4METRICS;
    } else {
	font.setPixelSize(NlvRound(ths->_cachedFontSize*FONTBOOSTER));
	ths->curPainter->setFont(font);
	ffac = 1.0F;		/* just to please the compiler */
    }

    x = x - ths->xOrigin;
    y = y - ths->yOrigin;

    Q_ASSERT(NlvStrchr(txt, '\n') == NULL);

#if QT_VERSION >= 0x040000
    const QFontMetricsF m(font);
#else
    const QFontMetrics m(font);
#endif
    float w       = m.width(txt);
    float ascent  = m.ascent();
    float descent = m.descent();

#if 0
    const QFontMetrics mold = ths->curPainter->fontMetrics();
    Q_ASSERT(w       == mold.width(txt));
    Q_ASSERT(ascent  == mold.ascent());
    Q_ASSERT(descent == mold.descent());
#endif

    if (ths->perfect_fontscale) {
	w       *= ffac;
	ascent  *= ffac;
	descent *= ffac;
    }

    limit_fontmetrics(ths->_cachedFontSize, &ascent, &descent);
    float h = ascent + descent;

    // to undo painter transformations and restore line pen:
    ths->curPainter->save();

    // set pen's color to current text color
    QPen pen(ths->curPainter->pen());
    pen.setColor(*ths->_cachedFontColor);
    ths->curPainter->setPen(pen);

    float xoff, yoff;
    apply_justify(&xoff, &yoff, w, h, justify);
    if (rotate) swapF(&x, &y);
    // move the origin to where we want to render the text
    ths->curPainter->translate(x,y);
    if (rotate) DO_ROTATE_90(ths->curPainter->rotate)

    if(justify & NlvJ_VERTICAL) {
	// to draw text vertically we must rotate the QPainter's coordinate
	// system temporarily by 90 degrees clockwise
	DO_ROTATE_90(ths->curPainter->rotate)
    }

    // now, ths->curPainter's (0,0) is at text label's anchor point
    // translate by anchor offset and call drawText(0,0) 
    //
    ths->curPainter->translate(-xoff,yoff+ascent);
    if (ths->perfect_fontscale) ths->curPainter->scale(ffac,ffac);
    ths->curPainter->drawText(0,0, QString(txt).left(len));
    ths->curPainter->restore();
}



/* WARNING: This function *can* be called asynchronously
 *          (without a valid QPainter/QPaintDevice)
 */
static void get_text_extend(const struct NlvRend* rend, 
			    float fontsize, const char* txt, int len,
			    enum NlvRendJustify justify, WBox* wbox, int linear)
{
    QtRend* ths = (QtRend*)(void*)rend;

    float xoff, yoff;
    float w, ascent, descent;
    QPainter* p = ths->curPainter;
    QFont font;
    float ffac;
    
    (void)linear;
    if (fontsize < 1) {
	wbox->left = wbox->top = wbox->right = wbox->bot = 0;
	return;
    }

    if (p) {
	font = p->font();
    } else {
	font = *ths->baseFont;
    }
    Q_ASSERT(!p || p->isActive());

    if (ths->perfect_fontscale) {
	font.setPixelSize(FONTSIZE4METRICS);
	ffac = fontsize*FONTBOOSTER / FONTSIZE4METRICS;
    } else {
	font.setPixelSize(NlvRound(fontsize*FONTBOOSTER));
	ffac = 1.0F;	/* just to please the compiler */
    }

    // Qt4.1+: very slow in QFontMetrics::width() if \n or \t in txt
    // so we truncate txt @ len instead of passing 2nd length arg to width()
    // This has also been reported as Qt task tracker id 130366:
    // "QFontMetrics::boundingRect() slow when Qt is not using FontConfig"
    //
    QString qtxt = QString(txt).left(len);

#if QT_VERSION >= 0x040000
    QFontMetricsF m(font);
    w	    = m.width(qtxt);
    ascent  = m.ascent();
    descent = m.descent();
#else
    if (!p) {
	QFontMetrics m(font);
	w	= m.width(qtxt);
	ascent  = m.ascent();
	descent = m.descent();
    } else {
	p->setFont(font); // apply changes
	QFontMetrics m = p->fontMetrics();
	w	= m.width(qtxt);
	ascent  = m.ascent();
	descent = m.descent();
    }
#endif

    if (ths->perfect_fontscale) {
	w	*= ffac;
	ascent	*= ffac;
	descent	*= ffac;
    }

    limit_fontmetrics(fontsize, &ascent, &descent);
    float h = ascent + descent;

    apply_justify(&xoff, &yoff, w, h, justify);

    if (justify & NlvJ_VERTICAL) {
	// WBoxSetLTWH(wbox,  yoff, xoff-w, h, w);
	wbox->left  = NlvRound(yoff);
	wbox->top   = NlvRound(xoff - w);
	wbox->right = NlvRound(yoff + h);
	wbox->bot   = NlvRound(xoff);
    } else {
	// WBoxSetLTWH(wbox, -xoff, yoff,   w, h);
	wbox->left  = NlvRound(-xoff);
	wbox->top   = NlvRound( yoff);
	wbox->right = NlvRound(w - xoff);
	wbox->bot   = NlvRound(h + yoff);
    }
}

/* WARNING: This function *can* be called asynchronously
 *          (without a valid QPainter/QPaintDevice)
 */
static float get_text_linefeed(const struct NlvRend* rend)
{
    (void)rend;
    return 1.1F;	/* see also limit_fontmetrics: we make sure, that
			 * font's bbox height is fontsize * 1.1 (unless
			 * the font's bbox is unexpected small).
			 */
}

/* WARNING: This function *can* be called asynchronously
 *          (without a valid painter)
 */
static int get_text_width(const struct NlvRend* rend, 
			  const char* txt, float fontsize)
{
    WBox wbox;
    if (txt == 0 || txt[0] == 0) return 0;
    get_text_extend(rend, fontsize, txt,NlvStrlen(txt), NlvJ_LOWERLEFT,&wbox,0);
    return wbox.right - wbox.left;
}


/* ===========================================================================
 * Qt Renderer interface implementation
 * ===========================================================================
 */
struct NlvRend* NlvRQt_construct(bool perfect_fontscale) {
    QtRend* ths = (QtRend*)NlvAlloc(sizeof(QtRend));
    Q_ASSERT(ths);

    /* base initialization (struct NlvRend) */
    ths->rend.set_origin            = set_origin;
    ths->rend.set_path              = set_path;
    ths->rend.set_font              = set_font;
    ths->rend.draw_line             = draw_line;
    ths->rend.draw_rectangle        = draw_rectangle;
    ths->rend.draw_poly             = draw_poly;
    ths->rend.draw_circle           = draw_circle;
    ths->rend.draw_filled_rectangle = draw_filled_rectangle;
    ths->rend.draw_filled_poly      = draw_filled_poly;
    ths->rend.draw_filled_circle    = draw_filled_circle;
    ths->rend.draw_text             = draw_text;
    ths->rend.draw_image            = draw_image;
    ths->rend.get_image             = get_image;
    ths->rend.get_text_width        = get_text_width;
    ths->rend.get_text_extend       = get_text_extend;
    ths->rend.get_text_linefeed     = get_text_linefeed;

    /* private data initialization */
    ths->curPainter        = 0;
    ths->_cachedLineColor  = 0;
    ths->_cachedFontColor  = 0;
    ths->_cachedFontSize   = 0.0f;
    ths->xOrigin           = 0;
    ths->yOrigin           = 0;
    ths->printer.rotate    = false;
    ths->printer.mode      = 'C';	/* COLOR - re-set in NlvRQt_init() */
    ths->perfect_fontscale = perfect_fontscale;

    QWidget  dummyq;
    return (struct NlvRend*)(void*)ths;
}



void NlvRQt_destruct(struct NlvRend* rend) {
    QtRend* ths = (QtRend*)(void*)rend;
    if (ths->baseFont) delete ths->baseFont;
    NlvFree(ths);
}



void NlvRQt_setImageMap(struct NlvRend* rend, const ImageMap* imageMap)
{
    QtRend* ths = (QtRend*)(void*)rend;
    ths->imageMap = imageMap;
}



void NlvRQt_set_basefont(struct NlvRend* rend, const QFont& font)
{
    QtRend* ths = (QtRend*)(void*)rend;
    if (ths->baseFont) delete ths->baseFont;
    ths->baseFont = new QFont(font);
}



void NlvRQt_init(struct NlvRend* rend, 
		 QPainter*       painter,
		 const QRect*    clipRect)
{
    QtRend* ths = (QtRend*)(void*)rend;
    Q_ASSERT( painter );        	/* You MUST pass a valid QPainter    */
    Q_ASSERT( painter->isActive() );
    if (ths->curPainter) {
	qWarning("Warning: Qt Renderer already initialized, call "
		 "NlvRQt_finit first");
    }
    ths->curPainter = painter;
    ths->_cachedLineColor = new QColor(ths->curPainter->pen().color());
    ths->_cachedFontColor = new QColor(ths->curPainter->pen().color());
    ths->printer.mode = 'C';	// default is color mode

    Q_ASSERT( ths->_cachedLineColor );
    Q_ASSERT( ths->_cachedFontColor );
#if QT_VERSION >= 0x040000
    if (!clipRect) {
	ths->curPainter->setRenderHint(QPainter::TextAntialiasing, false);
    }
#ifdef NLVIEW_USE_AA_SHAPES
    /* docs: "Indicates that the engine should antialias edges of primitives if
     * possible." It is disabled by default, because it decreases readability.
     */
    ths->curPainter->setRenderHint(QPainter::Antialiasing, true);
#endif
#endif // QT_VERSION

    // set clipping on, if clipRect is given:
    if (clipRect) {
	ths->curPainter->setClipRect(*clipRect);
	ths->curPainter->setClipping(true);
    }
}



void NlvRQt_initPrinter(struct NlvRend* rend, 
			QPainter*       painter,
			const QRect*    printArea,
			char            printMode)
{
    int     dpiY;
    QtRend* ths = (QtRend*)(void*)rend;
    NlvRQt_init(rend, painter, printArea/*clip to printArea*/);
    ths->printer.mode = printMode;

#if QT_VERSION >= 0x040000
    dpiY = ths->curPainter->device()->logicalDpiY();
#else
    dpiY = QPaintDeviceMetrics(ths->curPainter->device()).logicalDpiY();
#endif // QT_VERSION

    if (dpiY > 200) {
	// use precise arcs when printing to higher resolution QPaintDevice...
	ths->rend.draw_arc = draw_arc;
    }
}



void NlvRQt_finit(struct NlvRend* rend) {
    QtRend* ths = (QtRend*)(void*)rend;
    if (!ths->curPainter) {
	qWarning("Warning: Qt Renderer: called NlvRQt_finit() without "
		 "NlvRQt_init()");
	return; 
    }
    delete ths->_cachedLineColor;
    delete ths->_cachedFontColor;

    ths->_cachedLineColor = 0;
    ths->_cachedFontColor = 0;
    ths->curPainter       = 0;
}
