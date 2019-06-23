/*  gscene.cpp 1.16 2013/07/11

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

	It requires Qt 4.4 or later, because the Qt "Graphics View Framework"
	has been release first in Qt 4.2: see
	http://doc.trolltech.com/4.4/graphicsview.html
    ===========================================================================
*/
#include <qglobal.h>
#if QT_VERSION >= 0x040400

#include "gscene.h"
#include "gei.h"
#include <QtGui>
#include <QString>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <math.h>		// atan2 and friends...
#include <string.h>


#define Obj   struct GeiObject
#define Iter  struct GeiIter
typedef const struct GeiAccessFunc Gei;

static QString LastErrorMsg;			// buffer for error messages
static int zorder;


// some hard-wired colors of our choice
//
// (0) Graphics Scene colors...
//
static const QColor GexColorSceneBG       = Qt::black;

// (1) shape colors...
//
static const QColor GexColorRipper        = Qt::cyan;
static const QColor GexColorOverlay       = Qt::darkGray;
static const QColor GexColorInst          = Qt::cyan;
static const QColor GexColorInstFill      = QColor(0xaa,0x55,0x88, 0xa0);
static const QColor GexColorNet           = Qt::yellow;
static const QColor GexColorNetFill       = Qt::yellow;		// dots
static const QColor GexColorNetBundle     = Qt::darkYellow;
static const QColor GexColorNetBundleFill = Qt::darkYellow;	// dots
static const QColor GexColorConn          = Qt::cyan;
static const QColor GexColorConnFill      = Qt::transparent;    // no bg
static const QColor GexColorConnBus       = Qt::darkCyan;
static const QColor GexColorConnBusFill   = Qt::transparent;    // no bg

// (2) text/name colors...
//
static const QColor GexColorAtNameBG      = Qt::transparent;	// no bg
static const QColor GexColorAtCellBG      = Qt::transparent;	// no bg
static const QColor GexColorAttrFG        = Qt::white;
static const QColor GexColorAttrBG        = Qt::transparent;	// no bg
static const QColor GexColorNameConnFG    = GexColorNet;
static const QColor GexColorNameConnBusFG = GexColorNetBundle;


// some hard-wired (default) appearance parameters of our choice
//
static const int GexWidthInst           = 1;
static const int GexWidthPin            = 1;
static const int GexWidthPinBus         = 3;
static const int GexWidthRipTriangle    = 3;
static const int GexWidthRipTriangleBus = 4;
static const int GexWidthRipSlash       = 1;
static const int GexWidthRipSlashBus    = 3;
static const int GexWidthNet            = 1;
static const int GexWidthNetBundle      = 3;
static const int GexWidthOverlay        = 3;
static const int GexWidthConn           = GexWidthNet;
static const int GexWidthConnBus        = GexWidthNetBundle;

static const qreal GexSizeNegPins       = 6.0;	// bubble size of .neg SymPin


// which GEI objects should be put in a QGraphicsItemGroup?
//
static bool GexGroupNet       = false;
static bool GexGroupNetBundle = false;
static bool GexGroupOverlay   = false;


/* --------------------------------------------------------------------------
 * calculate_arc - similar to gskill.c:calculate_arc
 * angle, range, distance, between are all mathematic helpers
 * --------------------------------------------------------------------------
 */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const qreal TWO_PI = 2.0 * M_PI;


static inline qreal ABS(qreal x) { return x<0 ? -x : x; }

static qreal angle(qreal p1x, qreal p1y, qreal p2x, qreal p2y)
{
    const qreal e  = 1E-24;
    qreal       dx = p2x - p1x;
    qreal       dy = p2y - p1y;
    return (ABS(dx)<e && ABS(dy)<e) ? 0 : atan2(-dy,dx);
}

static qreal range(qreal a1, qreal a2)
{
    qreal r;
    if((a1 >= 0.0 && a2 >= 0.0) || (a1 < 0.0 && a2 < 0.0)) {
	/* both angle have same sign */
	r = a2 - a1;
    } else {
	/* both angle differ in sign */
	r = fmod(a2 - a1 + TWO_PI, TWO_PI);
    }
    if(r < 0.0) {
	r += TWO_PI;
    } else if(r > TWO_PI) {
	r -= TWO_PI;
    }
    return fmod(r, TWO_PI);
}

static bool between(qreal start, qreal mid, qreal end)
{
    if (ABS(start-end) < 1E-10) return true;

    if ((start >= 0.0 && end >= 0.0) ||
        (start <  0.0 && end <  0.0)) {
	return (start < end) ?
	       (mid > start && mid < end) : (mid > start || mid < end);
    } else if (start >= 0.0 && end < 0.0) {
	if (mid > start || mid < end) return true;
    } else if (start <  0.0 && end >= 0.0) {
	if (mid > start && mid < end) return true;
    }
    return false;
}

static qreal distance(qreal p1x, qreal p1y, qreal p2x, qreal p2y)
{
    const qreal e  = 1E-24;
    qreal       dx = p1x - p2x;
    qreal       dy = p1y - p2y;
    if (ABS(dx)<e || ABS(dy)<e) {
	qreal sum = ABS(dx);
	sum += ABS(dy);
	return sum;
    } else {
	return sqrt(dx*dx + dy*dy);	// good ol'Pythagoras...
    }
}

static bool calculate_arc(qreal* centerx, qreal* centery, qreal* diam,
	qreal* start, qreal* span,
	qreal p1x, qreal p1y, qreal p2x, qreal p2y, qreal p3x, qreal p3y)
{
    qreal A,B,C,D,E,F,G, mid, end, cx, cy;

    A = p2x - p1x;
    B = p2y - p1y;
    C = p3x - p1x;
    D = p3y - p1y;
    E =  A*(p1x + p2x) + B*(p1y + p2y);
    F =  C*(p1x + p3x) + D*(p1y + p3y);
    G = (A*(p3y - p2y) - B*(p3x - p2x)) * 2.0;

    if (ABS(G) < 1E-10) {
	/**
	 * here we have  p1---center  in parallel to  p3---center:
	 * i.e. the 3 points are collinear ("in line"), check for full circle
	 */
	if (distance(p1x,p1y, p3x,p3y) < 1E-10) {
	    /**
	     * special case: we have p1 == p3 => return full circle
	     */
            *centerx = (p1x + p2x) * 0.5;
            *centery = (p1y + p2y) * 0.5;
            *diam    = distance(p1x,p1y, p2x,p2y);
            *start   = angle(*centerx,*centery, p1x,p1y);
            *span    = TWO_PI;
	    return true;
        } else {
	    *centerx = *centery = *diam = *start = *span = 0.0;
	    return false;
	}
    }
    *centerx  = cx = (D*E - B*F) / G;
    *centery  = cy = (A*F - C*E) / G;
    *diam     = distance(cx,cy, p1x,p1y) * 2.0;
    *start    = angle(   cx,cy, p1x,p1y);
    mid       = angle(   cx,cy, p2x,p2y);
    end       = angle(   cx,cy, p3x,p3y);
    *span     = range(*start, end);
    if (!between(*start, mid, end)) {
	*span -= TWO_PI;
    }
    if (*span < 0.0) {			/* swap start and end angles */
	qreal tmp = *start;
	*start = end;
	end = tmp;
	*span = -(*span);
    }
    *span  = fmod(*span, TWO_PI);

    /* Finally, we MUST assure that the start angle refers to the first
     * point (x1,y1), because we rely on the sequence of points
     * (the arc will be added to a QPainterPath with arcTo, which implicitly
     * connects to the arc's starting point).
     */
    qreal sx = (*centerx) + (*diam)/2.0 * cos(*start);
    qreal sy = (*centery) + (*diam)/2.0 * -sin(*start);
    qreal xdiff = ABS(p1x-sx);
    qreal ydiff = ABS(p1y-sy);
    if (xdiff + ydiff > 1.0) {	// rev. arc dir: p3->p2->p1 ==> p1->p2->p3
	*start += *span;
	*span   = -*span;
    }
    return true;
}

inline static enum Qt::PenStyle castLineStyle(enum GeiLineStyle style)
{
    switch (style) {
	case GeiLineSSolid:   return Qt::SolidLine;
	case GeiLineSDashed:  return Qt::DashLine;
	case GeiLineSDashed2: return Qt::DashDotLine;
	case GeiLineSDefault: return Qt::SolidLine;
    }
    return Qt::SolidLine;	/* not reached */
}

inline static enum Qt::PenStyle mapAtStyle2PenStyle(const char* atstyle)
{
    if (qstrcmp(atstyle, "dashed")  == 0) return Qt::DashLine;
    if (qstrcmp(atstyle, "dashed2") == 0) return Qt::DashDotLine;
    return Qt::SolidLine;
}


/* ---------------------------------------------------------------------------
 * class NlviewGraphicsPathItem - helper class used by class NlvShape.
 *
 * It is used to overwrite QGraphicsPathItem::shape to return an accurate
 * shape for exact hit-tests. Since net wires are usually complex polygons
 * (e.g. concave outline) the bounding rectangle is certainly not the shape
 * for hit-tests. We use the QPainterPathStroker shape (the outline)
 * for the outline shape (and eventually add the body if the object is filled);
 * this approach may be slow, but it's accurate.
 * ---------------------------------------------------------------------------
 */
class NlviewGraphicsPathItem : public QGraphicsPathItem {
  public:
    NlviewGraphicsPathItem(const QPainterPath& path, QGraphicsItem* parent=0)
	: QGraphicsPathItem(path, parent)
    {};

    virtual QPainterPath shape () const
    {
	QPen pen = QGraphicsPathItem::pen();

	QPainterPathStroker ps;
	ps.setCapStyle(pen.capStyle());
	ps.setWidth(qMax(pen.widthF(), qreal(4.0))); // easier to drag...
	ps.setJoinStyle(pen.joinStyle());
	ps.setMiterLimit(pen.miterLimit());

	QPainterPath p = ps.createStroke(path());
	if (brush() != QBrush()) {
	    p.addPath(path());			// add body of filled paths
	}
	return p;
    };
};

/* ---------------------------------------------------------------------------
 * class NlvToolTip - helper class to represent a string with escaping support
 * ---------------------------------------------------------------------------
 */
class NlvToolTip : public QString {

 public:
    NlvToolTip() {}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString toHtmlEscaped() const { return Qt::escape(*this); }
#endif

    NlvToolTip& operator<< (const char* str) {
	append(QLatin1String(str));
	return *this;
    }
    NlvToolTip& operator<< (const QString& str) {
	append(str);
	return *this;
    }
};


/* ---------------------------------------------------------------------------
 * class NlvShape - helper class to create graphic view items from
 *		    GeiPointInfo structs.
 * ---------------------------------------------------------------------------
 */
class NlvShape {
  public:
    NlvShape(Gei* _g, int _w, enum Qt::PenStyle _st, const QColor& _fg,
	const QColor& _bg, const NlvToolTip& _tt)
       : g(_g)
       , width(_w)
       , style(_st)
       , fg(_fg)
       , bg(_bg)
       , tooltip(_tt)
    {};

    NlvShape& operator<<(struct GeiPointInfo* pi) {
	piList.append(*pi);
	return *this;
    };

    void addTo(QGraphicsItemGroup* group) const {
	addToSceneOrGroup(0, group);
    };

    void addTo(QGraphicsScene* scene) const {
	addToSceneOrGroup(scene, 0);
    };

  private:
    void addToSceneOrGroup(QGraphicsScene*, QGraphicsItemGroup*) const;

    Gei* g;
    int width;			// native width of the shape (may be 0)
    enum Qt::PenStyle style;	// native style of the shape (solid, dashed...)
    const QColor& fg;		// native foreground color of the shape
    const QColor& bg;		// native background color of the shape
    const NlvToolTip& tooltip;	// the text associated with the shape
    QList<struct GeiPointInfo> piList;
};


void NlvShape::addToSceneOrGroup(
	QGraphicsScene*     scene,
	QGraphicsItemGroup* group) const
{
    // some frequently used 'fixed' shapes ...
    //
    QRectF    dot = QRectF(-2,-2,4,4);
    QPolygonF ripL, ripR;
    ripL << QPointF(0,3) << QPointF(-6,0) << QPointF(0,-3) << QPointF(0,3);
    ripR << QPointF(0,3) << QPointF(+6,0) << QPointF(0,-3) << QPointF(0,3);

    bool filled = false;
    QPainterPath* path = NULL;
    int i;
    for (i=0; i < piList.size(); i++) {
	const struct GeiPointInfo& point = piList.at(i);
	switch (point.type) {
	    case GeiPointTBegPin:
	    case GeiPointTBegWire:
	    case GeiPointTBegT: {
		filled = point.filled;
		path = new QPainterPath(QPointF(point.x, point.y));
		break;
	    }

	    case GeiPointTEndPin:
	    case GeiPointTEndWire:
	    case GeiPointTEndT: {
		if (filled != (bool)point.filled) goto bad;
		if (!path) goto bad;
		path->lineTo(point.x, point.y);
		NlviewGraphicsPathItem* pathitem =
		    new NlviewGraphicsPathItem(*path, group);
		if (!group) {
		    pathitem->setFlag(QGraphicsItem::ItemIsMovable, true);
		    scene->addItem(pathitem);
		}
		if (width>0) {
		    QPen pen = QPen(QBrush(fg), width, style,
				    Qt::FlatCap, Qt::MiterJoin);
		    pathitem->setPen(pen);
		} else {
		    pathitem->setPen(Qt::NoPen);
		}
		if (filled) {
		    pathitem->setBrush(QBrush(bg));
		}
		pathitem->setToolTip(tooltip.toHtmlEscaped());
		pathitem->setZValue(zorder);
		zorder++;
		delete path;
		path = NULL;
		filled = false;
		break;
	    }

	    case GeiPointTCorner:
	    case GeiPointTNoCorner:
	    case GeiPointTMidPin:
	    case GeiPointTMidT: {
		if (!path) goto bad;
		path->lineTo(point.x, point.y);
		break;
	    }

	    case GeiPointTMidArc: {
		if (i+1 >= piList.size()) goto bad;
		const struct GeiPointInfo& pi0 = piList.at(i-1);
		const struct GeiPointInfo& pi1 = piList.at(i);
		const struct GeiPointInfo& pi2 = piList.at(i+1);
		qreal cx, cy, diam, start, span;
		if (calculate_arc(&cx, &cy, &diam, &start, &span,
			    pi0.x, pi0.y, pi1.x, pi1.y, pi2.x, pi2.y))
		{
		    start  *= 180.0/M_PI;	// rad2deg
		    span   *= 180.0/M_PI;	// rad2deg
		    qreal r = diam/2.0;
		    QRectF arcrect = QRectF(-r,-r, diam, diam);
		    arcrect.moveCenter(QPointF(cx,cy));
		    path->arcTo(arcrect, start, span);
		    // Don't skip following (arc) end-point (i++), it might be
		    // a GeiPointTEnd*-type which finally adds the QPainterPath
		} else {
		    if (!path) goto bad;
		    // fallback, if we could not calculate arc parameters
		    path->lineTo(point.x, point.y);
		}
		break;
	    }

	    case GeiPointTDot: {
		dot.moveCenter(QPointF(point.x, point.y));
		QGraphicsRectItem* rectitem =
		    new QGraphicsRectItem(dot, group);
		if (!group) {
		    rectitem->setFlag(QGraphicsItem::ItemIsMovable, true);
		    scene->addItem(rectitem);
		}
		if (width>0) {
		    QPen pen = QPen(QBrush(fg), width, style,
				    Qt::FlatCap, Qt::MiterJoin);
		    rectitem->setPen(pen);
		} else {
		    rectitem->setPen(Qt::NoPen);
		}
		rectitem->setBrush(QBrush(fg));
		rectitem->setToolTip(tooltip.toHtmlEscaped());
		rectitem->setZValue(zorder);
		zorder++;
		break;
	    }

	    case GeiPointTRipL: {
		ripL.translate(QPointF(point.x, point.y));
		QGraphicsPolygonItem* polyitem =
		    new QGraphicsPolygonItem(ripL, group);
		if (!group) {
		    polyitem->setFlag(QGraphicsItem::ItemIsMovable, true);
		    scene->addItem(polyitem);
		}
		if (width>0) {
		    QPen pen = QPen(QBrush(GexColorRipper), width, style,
				    Qt::FlatCap, Qt::MiterJoin);
		    polyitem->setPen(pen);
		} else {
		    polyitem->setPen(Qt::NoPen);
		}
		polyitem->setBrush(QBrush(GexColorRipper));
		polyitem->setToolTip(tooltip.toHtmlEscaped());
		polyitem->setZValue(zorder);
		zorder++;
		break;
	    }

	    case GeiPointTRipR: {
		ripR.translate(QPointF(point.x, point.y));
		QGraphicsPolygonItem* polyitem =
		    new QGraphicsPolygonItem(ripR, group);
		if (!group) {
		    polyitem->setFlag(QGraphicsItem::ItemIsMovable, true);
		    scene->addItem(polyitem);
		}
		if (width>0) {
		    QPen pen = QPen(QBrush(GexColorRipper), width, style,
				    Qt::FlatCap, Qt::MiterJoin);
		    polyitem->setPen(pen);
		} else {
		    polyitem->setPen(Qt::NoPen);
		}
		polyitem->setBrush(QBrush(GexColorRipper));
		polyitem->setToolTip(tooltip.toHtmlEscaped());
		polyitem->setZValue(zorder);
		zorder++;
		break;
	    }
	}
	continue;

      bad:
	LastErrorMsg =
	    QString("unexpected point, type=%1, (%2,%3), filled=%4").
	    	arg(point.type).arg(point.x).arg(point.y).arg(point.filled);
	qWarning() << LastErrorMsg;
	return;
    }
}


/* ---------------------------------------------------------------------------
 * isConnector - return true for connector instances and false for an ordinary
 *		 instance (GeiSymTInst or GeiSymTHier).
 * ---------------------------------------------------------------------------
 */
static bool isConnector(enum GeiSymType type)
{
    switch (type) {
	/* primary I/O port/portBus or page/bus connector */
	case GeiSymTINPORT:
	case GeiSymTOUTPORT:
	case GeiSymTINOUTPORT:
	case GeiSymTPAGEIN:
	case GeiSymTPAGEOUT:
	case GeiSymTPAGEINOUT:
	    return true;

	/* ordinary instance pin/pinBus */
	case GeiSymTInst:
	case GeiSymTHier:
	default:
	    return false;
    }
}


/* ---------------------------------------------------------------------------
 * symType2Str - returns a string representation of the enumerators
 *		 in enum GeiSymType.
 * ---------------------------------------------------------------------------
 */
static const char* symType2Str(enum GeiSymType type, bool isBus)
{
    switch (type) {
	case GeiSymTINPORT:    return isBus ? "input portBus" : "input port";
	case GeiSymTOUTPORT:   return isBus ? "output portBus": "output port";
	case GeiSymTINOUTPORT: return isBus ? "inout portBus" : "inout port";
	case GeiSymTPAGEIN:    return isBus ? "pagein bus-connector"
					    : "pagein connector";
	case GeiSymTPAGEINOUT:
	case GeiSymTPAGEOUT:   return isBus ? "pageout bus-connector"
					    : "pageout connector";
	case GeiSymTInst:      return isBus ? "bus-instance" : "instance";
	case GeiSymTHier:      return isBus ? "hierarchical bus-instance"
					    : "hierarchical instance";
	case GeiSymTPOWER:     return isBus ? "power bus-stub"  : "power stub";
	case GeiSymTGROUND:    return isBus ? "ground bus-stub" : "ground stub";
	case GeiSymTNEGPOWER:  return isBus ? "negpower bus-stub"
					    : "negpower stub";
	default:               return isBus ? "<unknown>-bus" : "<unknown>";
    }
}

/* ---------------------------------------------------------------------------
 * searchInstAttrByName - search the given inst for an attribute by name
 *
 *  returns true : if     found and copy to given struct GeiAttrInfo
 *  returns false: if not found and leave struct GeiAttrInfo untouched
 * ---------------------------------------------------------------------------
 */
static bool searchInstAttrByName(Gei* g,
	Obj* inst,
	const char* attrname,
	struct GeiAttrInfo* callers_ai)
{
    bool found = false;

    // for each instance attribute ...
    //
    Iter* it;
    for (it=g->attrIter(inst); g->Imore(it); g->Inext(it)) {
	struct GeiAttrInfo ai;
	g->IattrInfo(it, &ai);
	if (qstrncmp(attrname, ai.name, ai.nameLen)==0) {	// found!
	    *callers_ai = ai;
	    found = true;
	    break;
	}
    }
    g->freeIter(it);
    return found;
}


/* ---------------------------------------------------------------------------
 * getPinByName
 *
 * Searches the passed inst for the named pin - account for pin permutation.
 * The pin will be searched linearly at its instance.
 * ---------------------------------------------------------------------------
 */
static Obj* getPinByName(Gei* g, Obj* inst, const char* spin_name)
{
    Obj* pin = NULL;

    // for each instance pin ...
    //
    Iter* it;
    for (it=g->pinIter(inst); g->Imore(it); g->Inext(it)) {
	struct GeiPinInfo pi;
	g->IpinInfo(it, &pi);
	if (!pi.name) continue;    // happens for connectors like GeiSymTINPORT

	// We have a certain display location of a symbol pin called
	// $spin_name. Now, we want to know which pin should be displayed there:
	// (a) If the pin is NOT permuted, it is the pin called $spin_name.
	// (b) If the pin is     permuted, it is the pin having
	//                                            $swap_name == $spin_name
	if (pi.swap_name) {
	    if (qstrcmp(pi.swap_name, spin_name) != 0) continue;
	} else {
	    if (qstrcmp(pi.name,      spin_name) != 0) continue;
	}

	pin = g->Iobj(it);
	break;
    }
    g->freeIter(it);
    return pin;
}

static void getStyleAndColorFromNet(Gei* g, Obj* net_or_nbun,
	int*               w,
	enum Qt::PenStyle* st,
	QColor*            fg,
	QColor*            bg)
{
    Iter* it;
    if (!net_or_nbun) return;

    // for each net/nbun attr ...
    //
    for(it=g->nattrIter(net_or_nbun); g->Imore(it); g->Inext(it)) {
	struct GeiAttrInfo ai;
	g->IattrInfo(it, &ai);
	if        (qstrncmp(ai.name, "@color",     ai.nameLen)==0) {
	    QColor c(ai.value);
	    if (c.isValid()) *fg = c;
	} else if (qstrncmp(ai.name, "@fillcolor", ai.nameLen)==0) {
	    QColor c(ai.value);		/* only works for simple cases */
	    if (c.isValid()) *bg = c;
	} else if (qstrncmp(ai.name, "@width",     ai.nameLen)==0) {
	    bool ok;
	    int i = QString(ai.value).toInt(&ok);
	    if (ok && i>0) *w = i;	/* @width=0 not useful for net/nbun */
	} else if (qstrncmp(ai.name, "@style",     ai.nameLen)==0) {
	    *st = mapAtStyle2PenStyle(ai.value);
	}
    }
    g->freeIter(it);
}


/* ---------------------------------------------------------------------------
 * apply_justify - a text service function
 *
 * parameters:
 *   xoff      : x-offset return value
 *   yoff      : y-offset return value
 *   just      : text alignment (one of enum GeiAttrJust), see below
 *
 * The following text alignments are supported by Nlview:
 *
 *           1---2---3
 *           |   |   |
 *           4---5---6
 *           |   |   |
 *           7---8---9
 *
 * ---------------------------------------------------------------------------
 */
static void apply_justify(const QGraphicsTextItem* ti,
			  qreal* xoff, qreal* yoff,
			  enum GeiAttrJust just)
{
    QRectF bbox = ti->boundingRect();		// untransformed bounding box
    qreal descent = QFontMetrics(ti->font()).descent();
    qreal h = bbox.height();
    qreal w = bbox.width();

    switch(just) {
	case GeiAttrFJustUL:	// alignment #1
	    *yoff = 0;
	    *xoff = 0;
	    break;
	case GeiAttrFJustUC:	// alignment #2
	    *yoff = 0;
	    *xoff = w/2;
	    break;
	case GeiAttrFJustUR:	// alignment #3
	    *yoff = 0;
	    *xoff = w;
	    break;

	case GeiAttrFJustCL:	// alignment #4
	    *yoff = -h/2;
	    *xoff = 0;
	    break;
	case GeiAttrFJustCC:	// alignment #5
	    *yoff = -h/2;
	    *xoff = w/2;
	    break;
	case GeiAttrFJustCR:	// alignment #6
	    *yoff = -h/2;
	    *xoff = w;
	    break;

	case GeiAttrFJustLL:	// alignment #7
	    *yoff = -(h-descent);
	    *xoff = 0;
	    break;
	case GeiAttrFJustLC:	// alignment #8
	    *yoff = -(h-descent);
	    *xoff = w/2;
	    break;
	case GeiAttrFJustLR:	// alignment #9
	    *yoff = -(h-descent);
	    *xoff = w;
	    break;

	default:
	    *yoff = 0;
	    *xoff = 0;
	    break;
    }
}


/* ---------------------------------------------------------------------------
 * addAttr - adds the given text attribute (struct GeiSymAttrInfo)
 *           to the given group or scene (if group == NULL).
 * ---------------------------------------------------------------------------
 */
static void addAttr(Gei* g,
	QGraphicsScene*     scene,	// the scene
	QGraphicsItemGroup* group,	// optional parent group
	const struct GeiSymAttrInfo* sattr,
	const char* txt,		// displayed text, may include \n
	const QColor& fg,
	const QColor& bg,
	qreal scale)
{
    Q_UNUSED(g);

    // correction factor is required, because the fonts appear
    // slightly too big on my screen. Does it depend on the subsystem's DPI?
    //
    qreal fontsize = sattr->size*0.75;
    if (fontsize<=0.0001) return;   // avoid setting font size 0 (not allowed)

    if (sattr->marks) return;       // graphical marks are internal to Nlview

    NlvToolTip tooltip;
    if (sattr->text) {
	tooltip << "static text label: " << txt;
    } else {
	tooltip << "attribute " << sattr->name << "=" << txt;
    }

    QGraphicsTextItem* ti = new QGraphicsTextItem(txt, group);
    if (!group) {
	ti->setFlag(QGraphicsItem::ItemIsMovable, true);
	scene->addItem(ti);
    }

    ti->setDefaultTextColor(fg);
    ti->setToolTip(tooltip.toHtmlEscaped());

    QFont font = ti->font();
    font.setPointSizeF(fontsize);
    ti->setFont(font);

    QTransform m;
    m.translate(sattr->x, sattr->y);		// move to attribute location
    m.scale(scale, scale);			// apply font scale (e.g. 2.0)
    if (sattr->vertical) m.rotate(-90);		// rotate vertical text
    qreal xoff,yoff;
    apply_justify(ti, &xoff, &yoff, sattr->just);
    m.translate(-xoff, +yoff);			// apply justification offset
    ti->setTransform(m);

    if (bg != Qt::transparent) {		// add a background rectangle
	QRectF bgrect = ti->boundingRect();	// untransformed text bbox
	QGraphicsRectItem* tb = new QGraphicsRectItem(bgrect, group);
	if (!group) {
	    tb->setFlag(QGraphicsItem::ItemIsMovable, true);
	    scene->addItem(tb);
	}
	QPen pen = QPen(QBrush(bg.lighter()), 1,
			Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	tb->setTransform(m);
	tb->setPen(pen);
	tb->setBrush(QBrush(bg));
	tb->setZValue(zorder);
	zorder++;
    }

    ti->setZValue(zorder);	// defer setting to be on top of bg rect.
    zorder++;
}

/* ---------------------------------------------------------------------------
 * addNetOrNetBundleLabels - loops over all visible net/nbun labels and
 *                           adds them to the given group or scene
 *                           (if group == NULL).
 * ---------------------------------------------------------------------------
 */
static void addNetOrNetBundleLabels(Gei* g,
	QGraphicsScene*     scene,	// the scene
	QGraphicsItemGroup* group,	// parent group
	Obj* net_or_nbun,
	const struct GeiNetInfo* ni)	// avoids extra call of GEI's netInfo
{
    Q_UNUSED(g);

    // add all visible net/nbun labels using nattrDspIter
    //
    Iter* it;
    for (it=g->nattrDspIter(net_or_nbun); g->Imore(it); g->Inext(it)) {
        struct GeiSymAttrInfo sattr;
        struct GeiAttrInfo    ai;
        g->IattrDspInfo(it, &sattr, &ai);

	if (!ai.name || ai.nameLen<=0 || !ai.value) continue;
	if (sattr.marks) continue;	// skip @marks attribute

	Q_ASSERT(qstrncmp(sattr.name, ai.name, ai.nameLen)==0);

	QColor afg;
	QColor abg;

	if (sattr.text) {
	    Q_ASSERT(qstrcmp(ai.value, sattr.name)==0);
	    ai.value = sattr.name;
	    afg = GexColorRipper;
	    abg = Qt::transparent;
	} else if (qstrcmp("@name", sattr.name)==0) {
	    afg = ni->busWidth > 0 /*nbun*/ ? GexColorNetBundle : GexColorNet;
	    abg = Qt::transparent;
	} else {			// all other attributes...
	    afg = GexColorAttrFG;
	    abg = GexColorAttrBG;
	}

	if (ai.fmtForeground[0]) afg = QColor(ai.fmtForeground);
	if (ai.fmtBackground[0]) abg = QColor(ai.fmtBackground);
	if (abg != Qt::transparent) abg.setAlpha(200);

	addAttr(g, scene, group, &sattr, ai.value, afg, abg, ai.fmtFontscale);
    }
    g->freeIter(it);
}

/* ---------------------------------------------------------------------------
 * addComp - adds the given comp symbol shape to the scene as a group
 * ---------------------------------------------------------------------------
 */
static bool addComp(Gei* g, QGraphicsScene* scene, Obj* inst)
{
    struct GeiInstInfo ii;
    g->instInfo(inst, &ii);

    bool isConn = isConnector(ii.type);
    bool isBusConn = false;

    Obj* sym = ii.sym;
    if (!sym)
	return true;	// should not happen: nothing to display w/o a symbol!

    QGraphicsItemGroup* group = new QGraphicsItemGroup(0/*parent*/);
    group->setZValue(zorder);
    group->setPos(ii.x, ii.y);
    group->setFlag(QGraphicsItem::ItemIsMovable, true);
    scene->addItem(group);
    zorder++;

    struct GeiSymInfo symi;
    g->symInfo(sym, &symi);

    Iter* it;

    // symbol body graphic context ...
    //
    QColor sfg, sbg;
    int sw;
    enum Qt::PenStyle sst = Qt::SolidLine;
    if (symi.linestyle) sst = castLineStyle(symi.linestyle);

    if (isConn) {
	// determine if it's a bussed connector, or not
	// (check 1st inst-pin busWidth, NOT sym-pin!)
	//
	it = g->pinIter(inst);
	if (g->Imore(it)) {
	    struct GeiPinInfo pi;
	    g->IpinInfo(it, &pi);
	    isBusConn = pi.busWidth > 0;
	}
	g->freeIter(it);
	
	sfg = isBusConn ? GexColorConnBus     : GexColorConn;
	sbg = isBusConn ? GexColorConnBusFill : GexColorConnFill;
	sw  = isBusConn ? GexWidthConnBus     : GexWidthConn;
    } else {
	sfg = GexColorInst;
	sbg = GexColorInstFill;
	sw  = GexWidthInst;
    }
    if (symi.linewidth >= 0) sw = symi.linewidth;

    // Before drawing any wires, we need to search the instance
    // for some special attributes:
    //    @width     - overrides the default outline stroke width (w)
    //    @color     - overrides the default foreground color (fg)
    //    @fillcolor - overrides the default background color (bg)
    //    @style     - overrides the default line style (solid/dashed/dashed2)
    GeiAttrInfo ai;
    if (searchInstAttrByName(g, inst, "@width", &ai)) {
	bool ok;
	int i = QString(ai.value).toInt(&ok);
	if (ok && i>=0) sw = i;
    }
    if (searchInstAttrByName(g, inst, "@color", &ai)) {
	QColor c(ai.value);
	if (c.isValid()) sfg = c;
    }
    if (searchInstAttrByName(g, inst, "@fillcolor", &ai)) {
	QColor c(ai.value);	/* only works for non-pattern/non-gradient */
	if (c.isValid()) sbg = c;
    }
    if (searchInstAttrByName(g, inst, "@style", &ai)) {
	sst = mapAtStyle2PenStyle(ai.value);
    }

    // build up the inst tooltip message ...
    //
    NlvToolTip tooltip;
    tooltip << symType2Str(ii.type, isBusConn) << " " << ii.name;

    // add symbol properties to tooltip if available
    //
    for (it=g->spropIter(sym); g->Imore(it); g->Inext(it)) {
	struct GeiAttrInfo sprop;
	g->IspropInfo(it,  &sprop);
	QString name = QString::fromLatin1(sprop.name, sprop.nameLen);
	tooltip << "\n  property " << name << "=" << sprop.value;
    }
    g->freeIter(it);


    // foreach symbol "wire" point ...
    //
    NlvShape symshape(g, sw, sst, sfg, sbg, tooltip);
    for (it=g->shapeIter(sym); g->Imore(it); g->Inext(it)) {
        struct GeiPointInfo point;
        g->IshapeInfo(it,  &point);
	symshape << &point;
    }
    g->freeIter(it);
    symshape.addTo(group);	// creates items, parent is group

    // for each symbol pin ...
    //
    for (it=g->spinIter(sym); g->Imore(it); g->Inext(it)) {
        struct GeiSymPinInfo spin;
        g->IspinInfo(it,  &spin);
	if (spin.hidden || spin.hidestub) continue;

	Obj* pin = getPinByName(g, inst, spin.name);
	if (!pin) continue;	// error

	// symbol pin graphic context ...
	//
	QColor pfg = sfg;		// inherit pin color     from instance
	QColor pbg = sbg;		// inherit pin fillcolor from instance
	int pw = spin.busWidth ? GexWidthPinBus : GexWidthPin;
	if (spin.linewidth >= 0) pw = spin.linewidth;
	enum Qt::PenStyle pst = sst;	// inherit pin style     from instance
	if (spin.linestyle) pst = castLineStyle(spin.linestyle);

	// build up the pin tooltip message ...
	//
	NlvToolTip ptt;
	ptt << (spin.busWidth ? "pinBus" : "pin") << " "
	    << ii.name << " " << spin.name;
	//
	// add all available pin attributes to tooltip, except special ones:
	Iter* pit;
	for (pit=g->pattrIter(pin); g->Imore(pit); g->Inext(pit)) {
	    struct GeiAttrInfo ai;
	    g->IattrInfo(pit, &ai);
	    QString name = QString::fromLatin1(ai.name, ai.nameLen);
	    if (name == "@width") {
		bool ok;
		int i = QString(ai.value).toInt(&ok);
		if (ok && i>=0) pw = i;
	    } else if (name == "@color") {
		QColor c(ai.value);
		if (c.isValid()) pfg = c;
	    } else if (name == "@fillcolor") {
		QColor c(ai.value);  // only works for non-pattern/non-gradient
		if (c.isValid()) pbg = c;
	    } else if (name == "@style") {
		pst = mapAtStyle2PenStyle(ai.value);
	    } else {
		ptt << "\n  pinAttr " << name << "=" << ai.value;
	    }
	}
	g->freeIter(pit);

	NlvShape stub(g, pw, pst, pfg, pbg, ptt);
	if (spin.polygon_len>0) {	// a polygon shaped pin ...
	    unsigned i;
	    for (i=0; i<spin.polygon_len; i++)
		stub << spin.polygon+i;
	} else {			// a regular pin stub ...
	    struct GeiPointInfo point;
	    point.filled = 0;

	    // add 1st pin stub point...
	    point.x      = spin.x;
	    point.y      = spin.y;
	    point.type   = GeiPointTBegWire;
	    stub << &point;

	    // add 2nd pin stub point...
	    point.x      = spin.xStub;
	    point.y      = spin.yStub;
	    point.type   = GeiPointTEndWire;
	    if (spin.neg) {			// bubble neg symbol pins
		qreal diff_x = ABS(spin.x - spin.xStub);
		qreal diff_y = ABS(spin.y - spin.yStub);
		qreal len    = diff_x > diff_y ? diff_x : diff_y;
		qreal BUB_SIZE = symi.scale*GexSizeNegPins;
		if (len<BUB_SIZE) BUB_SIZE = len;	// don't exceed stub len

		if(       spin.x >= spin.xStub+BUB_SIZE) {	/* dir: right */
		    point.x += (int)BUB_SIZE;
		} else if(spin.x <= spin.xStub-BUB_SIZE) {	/* dir: left */
		    point.x -= (int)BUB_SIZE;
		} else if(spin.y >= spin.yStub+BUB_SIZE) {	/* dir: bot */
		    point.y += (int)BUB_SIZE;
		} else if(spin.y <= spin.yStub-BUB_SIZE) {	/* dir: top */
		    point.y -= (int)BUB_SIZE;
		}		// don't support .neg at diagonal pin stubs

		if (ABS(point.x - spin.xStub)<1E-10 ||
		    ABS(point.y - spin.yStub)<1E-10)
		{
		    point.type = GeiPointTNoCorner;	// use as 1st arc point
		    stub << &point;

		    int start_x = point.x;		// remember for 3rd pt
		    int start_y = point.y;

		    // add the 2nd arc point...
		    point.x      = spin.xStub;
		    point.y      = spin.yStub;
		    point.type   = GeiPointTMidArc;
		    stub << &point;

		    // add the 3rd arc point...
		    point.x      = start_x;
		    point.y      = start_y;
		    point.type   = GeiPointTEndWire;
		    stub << &point;
		} else {
		    stub << &point;
		}
	    } else {
		stub << &point;
	    }
	}
	stub.addTo(group);

	// add all visible pin labels using pattrDspIter;
	// this will include all dynamically created pinattrdsp.
	//
	Iter* sit;
	for (sit=g->pattrDspIter(pin,inst); g->Imore(sit); g->Inext(sit)) {
	    struct GeiSymAttrInfo sattr;
	    struct GeiAttrInfo    ai;
	    g->IattrDspInfo(sit, &sattr, &ai);

	    if (!ai.name || ai.nameLen<=0 || !ai.value) continue;

	    Q_ASSERT(qstrncmp(sattr.name, ai.name, ai.nameLen)==0);

	    QColor afg = GexColorAttrFG;
	    QColor abg = GexColorAttrBG;

	    if (qstrcmp("@name", sattr.name)==0) {
		afg = sfg;
		abg = GexColorAtNameBG;
	    }

	    if (ai.fmtForeground[0]) afg = QColor(ai.fmtForeground);
	    if (ai.fmtBackground[0]) abg = QColor(ai.fmtBackground);
	    if (abg != Qt::transparent) abg.setAlpha(200);

	    addAttr(g, scene, group, &sattr, ai.value, afg,abg,ai.fmtFontscale);
	}
	g->freeIter(sit);

    }
    g->freeIter(it);


    // add all visible instance labels using attrDspIter;
    // this will include all dynamically created attrdsp.
    //
    for (it=g->attrDspIter(inst); g->Imore(it); g->Inext(it)) {
        struct GeiSymAttrInfo sattr;
        struct GeiAttrInfo    ai;
        g->IattrDspInfo(it, &sattr, &ai);

	if (!ai.name || ai.nameLen<=0 || !ai.value) continue;
	if (sattr.marks) continue;

	Q_ASSERT(qstrncmp(sattr.name, ai.name, ai.nameLen)==0);

	QColor afg;
	QColor abg;

	if (sattr.text) {
	    Q_ASSERT(qstrcmp(ai.value, sattr.name)==0);
	    ai.value = sattr.name;
	    afg = GexColorInst;
	    abg = Qt::transparent;
	} else if (qstrcmp("@name", sattr.name)==0) {
	    if (isConn) {
		afg = isBusConn ? GexColorNameConnBusFG : GexColorNameConnFG;
	    } else {
		afg = sfg.lighter();
	    }
	    abg = GexColorAtNameBG;
	} else if (qstrcmp("@cell", sattr.name)==0) {
	    afg = sfg;
	    abg = GexColorAtCellBG;
	} else {			// all other attributes...
	    afg = GexColorAttrFG;
	    abg = GexColorAttrBG;
	}

	if (ai.fmtForeground[0]) afg = QColor(ai.fmtForeground);
	if (ai.fmtBackground[0]) abg = QColor(ai.fmtBackground);
	if (abg != Qt::transparent) abg.setAlpha(200);

	addAttr(g, scene, group, &sattr, ai.value, afg, abg, ai.fmtFontscale);
    }
    g->freeIter(it);

    return group;
}


/* ---------------------------------------------------------------------------
 * addNet - adds the given single-bit net to the scene
 * ---------------------------------------------------------------------------
 */
static bool addNet(Gei* g, QGraphicsScene* scene, Obj* net)
{
    struct GeiNetInfo ni;
    g->netInfo(net, &ni);

    NlvToolTip tooltip;
    tooltip << "net " << ni.name;

    QGraphicsItemGroup* group = NULL;
    if (GexGroupNet) {
	group = new QGraphicsItemGroup(0/*parent*/);
	group->setZValue(zorder);
	group->setFlag(QGraphicsItem::ItemIsMovable, true);
	scene->addItem(group);
	zorder++;
    }

    Iter* it;
    int               w  = GexWidthNet;
    enum Qt::PenStyle st = Qt::SolidLine;
    QColor            fg = GexColorNet;
    QColor            bg = GexColorNetFill;

    getStyleAndColorFromNet(g, net, &w, &st, &fg, &bg);

    // foreach net "wire" point ...
    //
    NlvShape netshape(g, w, st, fg, bg, tooltip);
    for (it=g->wireIter(net); g->Imore(it); g->Inext(it)) {
        struct GeiPointInfo point;
	g->IwireInfo(it, &point);
	netshape << &point;
    }
    g->freeIter(it);
    if (GexGroupNet) netshape.addTo(group);
    else             netshape.addTo(scene);

    addNetOrNetBundleLabels(g, scene, group, net, &ni);

    return true;
}


/* ---------------------------------------------------------------------------
 * addRipper - adds the ripper (Triangle/Bus or Slash/Bus) to the scene
 * ---------------------------------------------------------------------------
 */
static void addRipper(Gei* g, QGraphicsScene* scene, QGraphicsItemGroup* group,
	const GeiRipInfo* ri, const NlvToolTip& tip, bool isOverlayRipper)
{
    struct GeiPointInfo point;
    int w = 0;

    switch (ri->type) {
	case GeiRipTTriangle:    w = GexWidthRipTriangle;    break;
	case GeiRipTTriangleBus: w = GexWidthRipTriangleBus; break;
	case GeiRipTSlash:       w = GexWidthRipSlash;       break;
	case GeiRipTSlashBus:    w = GexWidthRipSlashBus;    break;
	default: {
	    LastErrorMsg =
		QString("unexpected ripper type=%1 @ bus-side=(%2,%3)").
		arg(ri->type).arg(ri->xBus).arg(ri->yBus);
	    qWarning() << LastErrorMsg;
	    return;
	}
    }

    NlvShape ripper(g, w, Qt::SolidLine, GexColorRipper, GexColorRipper, tip);

    point.filled = 0;
    point.x      = ri->xBus;
    point.y      = ri->yBus;

    if (ri->type == GeiRipTTriangle ||
	ri->type == GeiRipTTriangleBus)
    {
	point.type   = ri->xSubnet > ri->xBus ? GeiPointTRipR : GeiPointTRipL;
	ripper << &point;
    } else {		// GeiRipTSlash or GeiRipTSlashBus
	point.type   = GeiPointTBegWire;
	ripper << &point;

	point.x      = ri->xSubnet;
	point.y      = ri->ySubnet;
	point.type   = GeiPointTEndWire;
	ripper << &point;
    }
    if (isOverlayRipper) {
	if (GexGroupOverlay)   ripper.addTo(group);
	else                   ripper.addTo(scene);
    } else {
	if (GexGroupNetBundle) ripper.addTo(group);
	else                   ripper.addTo(scene);
    }
}


/* ---------------------------------------------------------------------------
 * addNetBundle - adds the given netBundle to the scene
 * ---------------------------------------------------------------------------
 */
static bool addNetBundle(Gei* g, QGraphicsScene* scene, Obj* netBundle)
{
    struct GeiNetInfo ni;
    g->netInfo(netBundle, &ni);

    NlvToolTip tooltip;
    tooltip << "netBundle " << ni.name;

    QGraphicsItemGroup* group = NULL;
    if (GexGroupNetBundle) {
	group = new QGraphicsItemGroup(0/*parent*/);
	group->setZValue(zorder);
	group->setFlag(QGraphicsItem::ItemIsMovable, true);
	scene->addItem(group);
	zorder++;
    }

    Iter* it;
    int               w  = GexWidthNetBundle;
    enum Qt::PenStyle st = Qt::SolidLine;
    QColor            fg = GexColorNetBundle;
    QColor            bg = GexColorNetBundleFill;

    getStyleAndColorFromNet(g, netBundle, &w, &st, &fg, &bg);

    // foreach netBundle "wire" point ...
    //
    NlvShape nbunshape(g, w, st, fg, bg, tooltip);
    for (it=g->wireIter(netBundle); g->Imore(it); g->Inext(it)) {
        struct GeiPointInfo point;
	g->IwireInfo(it, &point);
	nbunshape << &point;
    }
    g->freeIter(it);
    if (GexGroupNetBundle) nbunshape.addTo(group);
    else                   nbunshape.addTo(scene);

    // for each netBundle ripper ...
    //
    QStringList slist;
    for (it=g->ripIter(netBundle); g->Imore(it); g->Inext(it)) {
	struct GeiRipInfo ri;
	g->IripInfo(it, &ri);

	// collect the subnet names (and indices) in slist
	//
	if (ri.subname) {
	    QString subname;
	    if (ri.subnetIdx != -1)
		subname = QString("[#%1] ").arg(ri.subnetIdx);
	    subname.append(ri.subname);
	    slist.append(subname);
	}

	// skip all except last repetition @ GeiRipTSlashBus/GeiRipTTriangleBus
	if (ri.countDown > 0) continue;

	// (a) export connecting subnet/subbus wire polygons
	//
	QColor sfg, sbg;
	int sw;
	enum Qt::PenStyle sst;
	bool isSubbus = (ri.type == GeiRipTTriangleBus) ||
			(ri.type == GeiRipTSlashBus);
	sfg = isSubbus ? GexColorNetBundle     : GexColorNet;
	sbg = isSubbus ? GexColorNetBundleFill : GexColorNetFill;
	sw  = isSubbus ? GexWidthNetBundle     : GexWidthNet;
	sst = isSubbus ? st : Qt::SolidLine;

	if (!isSubbus) {
	    // we could ask this single-bit sub-net for @color, ... but ...
	    // GEI restriction: there's no way to get hold of the subnet
	    //                  Obj* while looping over the rippers.
	    //                  Thus, we cannot ask a subnet for attributes
	    //                  (subnets are not returned by netIter and we
	    //                  cannot start a nattrIter...)
	    //
	    //getStyleAndColorFromNet(g, subnet, &sw, &sst, &sfg, &sbg);
	}
	NlvToolTip subnettt;
	subnettt << tooltip;
	if (isSubbus) {
	    subnettt << QString("\n%1-bit sub-bus:").arg(slist.size());
	    subnettt << "\n  " << slist.join("\n  ");
	} else {
	    subnettt << "\nsubnet " << slist.at(0);
	}
	slist.clear();

	NlvShape subnetshape(g, sw, sst, sfg, sbg, subnettt);
	Iter* wit;
	for (wit=g->subWIter(it); g->Imore(wit); g->Inext(wit)) {
	    struct GeiPointInfo point;
	    g->IsubWInfo(wit, &point);
	    subnetshape << &point;
	}
	g->freeIter(wit);
	if (GexGroupNetBundle) subnetshape.addTo(group);
	else                   subnetshape.addTo(scene);

	// (b) export ripper shape
	//
	addRipper(g, scene, group, &ri, subnettt, false);
    }
    g->freeIter(it);

    addNetOrNetBundleLabels(g, scene, group, netBundle, &ni);

    return true;
}



/* ---------------------------------------------------------------------------
 * addOverlay - adds the given overlay bundle to the scene
 * ---------------------------------------------------------------------------
 */
static bool addOverlay(Gei* g, QGraphicsScene* scene, Iter* cur_overlay)
{
    NlvToolTip tooltip;
    tooltip << "overlay bus";

    QGraphicsItemGroup* group = NULL;
    if (GexGroupOverlay) {
	group = new QGraphicsItemGroup(0/*parent*/);
	group->setZValue(zorder);
	group->setFlag(QGraphicsItem::ItemIsMovable, true);
	scene->addItem(group);
	zorder++;
    }

    // nested ovWIter - print wire info at this overlay bundle
    //
    Iter* ovWit;
    NlvShape overshape(g, GexWidthOverlay, Qt::SolidLine,
	GexColorOverlay, GexColorOverlay, tooltip);
    for (ovWit=g->ovWIter(cur_overlay); g->Imore(ovWit); g->Inext(ovWit)) {
	struct GeiPointInfo point;
	g->IovWInfo(ovWit, &point);
	overshape << &point;
    }
    g->freeIter(ovWit);
    if (GexGroupOverlay) overshape.addTo(group);
    else                 overshape.addTo(scene);

    // nested ovRIter - loops over the GeiRipTSlash/SlashBus rippers of overlay
    //
    Iter* ovRit;
    QStringList slist;
    for (ovRit=g->ovRIter(cur_overlay); g->Imore(ovRit); g->Inext(ovRit)) {
	struct GeiRipInfo ri;
	g->IovRInfo(ovRit, &ri);

	// collect the subnet names (and indices) in slist
	//
	if (ri.subname) {
	    QString subname;
	    if (ri.subnetIdx != -1)
		subname = QString("[#%1] ").arg(ri.subnetIdx);
	    subname.append(ri.subname);
	    slist.append(subname);
	}

	// skip all except last repetition at GeiRipTSlashBus
	if (ri.countDown > 0) continue;

	bool isBusRip = ri.type == GeiRipTSlashBus;

	// create an informative tooltip
	//
	struct GeiNetInfo ni;
	g->netInfo(ri.net_or_nbun, &ni);
	NlvToolTip subnettt;
	subnettt << tooltip << " ripping ";
	if (isBusRip) {
	    if ((unsigned)slist.size() == ni.busWidth) {// full netBundle rip
		subnettt << "full netBundle " << ni.name;
	    } else {					// partial netBundle rip
		subnettt << QString("%1-bits").arg(slist.size())
			 << " of netBundle " << ni.name << ":\n  "
			 << slist.join("\n  ");
	    }
	} else {
	    if (ni.busWidth==0) {			// single net rip
		subnettt << "net " << ni.name;
	    } else {					// single sub-net rip
		subnettt << "sub-net " << slist.at(0)
			 << " of netBundle " << ni.name;
	    }
	}
	slist.clear();

	addRipper(g, scene, group, &ri, subnettt, true);
    }
    g->freeIter(ovRit);
    return true;
}


/* ---------------------------------------------------------------------------
 * addCoordinateSystem - adds a (debug) coordinate system into the scene
 * ---------------------------------------------------------------------------
 */
static void addCoordinateSystem(QGraphicsScene* scene)
{
    QBrush cbrush = QBrush(Qt::gray);
    QPen   cpen   = QPen(cbrush, 1);
    QPen   cpen3  = QPen(cbrush, 3);
    const int len = 200;
    QString unit;
    unit.setNum(len);

    // X axis:
    //
    scene->addLine(-len,0,len,0,cpen);
    for (int x=0; x<=len; x += 10) {
	scene->addLine(x, -3, x, 3, x % 50 == 0 ? cpen3 : cpen);
    }
    for (int x=0; x>=-len; x -= 10) {
	scene->addLine(x, -3, x, 3, x % 50 == 0 ? cpen3 : cpen);
    }
    scene->addLine(len-5, -3, len, 0, cpen);	// >
    scene->addLine(len-5, +3, len, 0, cpen);

    QGraphicsSimpleTextItem* xleg = scene->addSimpleText("X");
    xleg->setBrush(QBrush(cbrush));
    xleg->setPos(len+5, -xleg->sceneBoundingRect().height()/2);

    QGraphicsSimpleTextItem* xunit = scene->addSimpleText(unit);
    xunit->setBrush(QBrush(cbrush));
    xunit->setPos(len-xunit->sceneBoundingRect().width()/2,
		     -xunit->sceneBoundingRect().height());

    // Y axis:
    //
    scene->addLine(0,-len,0,len,cpen);
    for (int y=0; y<=len; y += 10) {
	scene->addLine(-3, y, 3, y, y % 50 == 0 ? cpen3 : cpen);
    }
    for (int y=0; y>=-len; y -= 10) {
	scene->addLine(-3, y, 3, y, y % 50 == 0 ? cpen3 : cpen);
    }
    scene->addLine(-3, len-5, 0, len, cpen);	// v
    scene->addLine(+3, len-5, 0, len, cpen);

    QGraphicsSimpleTextItem* yleg = scene->addSimpleText("Y");
    yleg->setBrush(QBrush(cbrush));
    yleg->setPos(-yleg->sceneBoundingRect().width()/2, len+5);

    QGraphicsSimpleTextItem* yunit = scene->addSimpleText(unit);
    yunit->setBrush(QBrush(cbrush));
    yunit->setPos(6, len-yunit->sceneBoundingRect().height()/2);
}


/* ---------------------------------------------------------------------------
 * addPage - add the given schematic page to the scene
 * ---------------------------------------------------------------------------
 */
static bool addPage(Gei* g, QGraphicsScene* scene, Obj* page)
{
    // Set the scene background color
    //
    scene->setBackgroundBrush(GexColorSceneBG);

    if (false) addCoordinateSystem(scene);   // enable this for coord-debugging

    // for each instance...
    //
    Iter* it;
    for (it=g->instIter(page); g->Imore(it); g->Inext(it)) {
	Obj* inst = g->Iobj(it);
	bool ok = addComp(g, scene, inst);
	if (!ok) goto bad;
    }
    g->freeIter(it);

    // for each net ...
    //
    for (it=g->netIter(page); g->Imore(it); g->Inext(it)) {
	Obj* net = g->Iobj(it);
	bool ok = addNet(g, scene, net);
	if (!ok) goto bad;
    }
    g->freeIter(it);

    // for each netBundle ...
    //
    for (it=g->nbunIter(page); g->Imore(it); g->Inext(it)) {
	Obj* nbun = g->Iobj(it);
	bool ok = addNetBundle(g, scene, nbun);
	if (!ok) goto bad;
    }
    g->freeIter(it);

    // for each (artificial) overlay bundle ...
    //
    for (it=g->overIter(page); g->Imore(it); g->Inext(it)) {
	bool ok = addOverlay(g, scene, it);
	if (!ok) goto bad;
    }
    g->freeIter(it);
    return true;

  bad:
    g->freeIter(it);
    return false;
}


/* ===========================================================================
 * Gscene Main Functions:
 * ======================
 *
 * GexScene           - Populate a given Qt QGraphicsScene with the schematic
 *                      data of the current Nlview module on
 *                      the given Nlview page number thru Nlview's GEI.
 *
 * GexSceneLastErr    - return last error message
 *
 * ===========================================================================
 */

bool GexScene(Gei* g, QGraphicsScene* scene, int pageNo)
{
    LastErrorMsg = QString("");
    if (g->magic != GeiMagic) {
	LastErrorMsg =
	    QString("bad magic value in Nlview GEI pointer");
	return false;
    }
    if (g->geiVersion() != GeiVersion) {
	LastErrorMsg =
	    QString("bad Nlview GEI version %1 (%2 expected)").
	    arg(g->geiVersion()).arg(GeiVersion);
	return false;
    }

    /* Loop over all schematic pages in current module.
     */
    zorder = 0;
    Iter* it;
    Obj* page = NULL;
    for (it=g->pageIter(); g->Imore(it); g->Inext(it)) {
	Obj*  testpage = g->Iobj(it);
	GeiPageInfo pi;
	g->pageInfo(testpage, &pi);
	if (pi.pageNumber == pageNo) {
	    page = testpage;
	    break;
	}
    }
    g->freeIter(it);

    if (page) {
	return addPage(g, scene, page);
    } else {
	LastErrorMsg = QString("no such page number: %1").arg(pageNo);
	return false;
    }
}


/* ===========================================================================
 * Return last error message
 * ===========================================================================
 */
const QString& GexSceneLastErr()
{
    return LastErrorMsg;
}

#endif /* QT_VERSION */
