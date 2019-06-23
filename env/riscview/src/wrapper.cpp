/*  wrapper.cpp 1.173 2018/12/21
  
    Copyright 1998-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
        Qt Wrapper Implementation
    Author:
        Jochen Roemmler
    ===========================================================================
*/
#define DECLARE_KERNEL_FUNC
#include "wrapper.h"
#include "nlvcore.h"
#include "nlvos.h"
#undef DECLARE_KERNEL_FUNC

#if QT_VERSION >= 0x040000	// Qt4+
#include <QCoreApplication>
#include <QMouseEvent>
#include <QPaintDevice>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QPicture>
#include <QImage>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTimer>
#include <QWheelEvent>
#ifndef WIN
#if QT_VERSION < 0x050000	// Qt5+: QX11Info class is gone
#include <QX11Info>
#endif
#endif
#else				// Qt 2.x - 3.x
#include <qapplication.h>
#include <qpaintdevice.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpicture.h>
#include <qimage.h>
#include <qscrollbar.h>
#include <qtimer.h>
#endif // QT_VERSION

#include "nlvrqt.h"


// In order to have a single source file that is source-code (API) compatible 
// with Qt 2.x, 3.x and 4.x we have to manipulate some code places...

#if QT_VERSION >= 0x040000	// Qt 4+
#  define SETUPDATESENABLED(A) setUpdatesEnabled(A)
#  define ISSLIDERDOWN         isSliderDown
#  define SETMINIMUM           setMinimum
#  define SETMAXIMUM           setMaximum
#  define MODIFIERS            modifiers
#else				// Qt 2.x - 3.x
#  define SETUPDATESENABLED(A) /* N/A under this Qt version */
#  define ISSLIDERDOWN         draggingSlider
#  define SETMINIMUM           setMinValue
#  define SETMAXIMUM           setMaxValue
#  define MODIFIERS            state
#endif // QT_VERSION


// The version number format of QT_VERSION changed in Qt 3.0.5
// It is now 0xmmiibb (m = major, i = minor, b = bugfix).
// Qt 3.0.5's QT_VERSION is 0x030005.

#if QT_VERSION >= 300   /* Qt 3.0.0 and up */
// Qt3.x: Standard-Cursors moved into class "Qt"
// Qt4.x: Standard-Cursors moved into namespace "Qt"
#  define WAIT_CURSOR         Qt::WaitCursor
#  define SIZE_ALL_CURSOR     Qt::SizeAllCursor
#  define ARROW_CURSOR        Qt::ArrowCursor
#  define SIZEVER_CURSOR      Qt::SizeVerCursor  
#  define SIZEHOR_CURSOR      Qt::SizeHorCursor
#  define SIZEBDIAG_CURSOR    Qt::SizeBDiagCursor
#  define SIZEFDIAG_CURSOR    Qt::SizeFDiagCursor
#  define CENTERAT_CURSOR     Qt::PointingHandCursor
#  if QT_VERSION >= 0x040200
#    define MOVABLE_CURSOR    Qt::OpenHandCursor
#    define MOVING_CURSOR     Qt::ClosedHandCursor
#  else
#    define MOVABLE_CURSOR    Qt::PointingHandCursor
#    define MOVING_CURSOR     Qt::PointingHandCursor
#  endif
#else
#  define WAIT_CURSOR           WaitCursor
#  define SIZE_ALL_CURSOR       SizeAllCursor
#  define ARROW_CURSOR          ArrowCursor
#  define SIZEVER_CURSOR        SizeVerCursor
#  define SIZEHOR_CURSOR        SizeHorCursor
#  define SIZEBDIAG_CURSOR      SizeBDiagCursor
#  define SIZEFDIAG_CURSOR      SizeFDiagCursor
#  define CENTERAT_CURSOR       PointingHandCursor
#  define MOVABLE_CURSOR        PointingHandCursor
#  define MOVING_CURSOR         PointingHandCursor
#  if !defined(Q_ASSERT)	// Q_ASSERT is called ASSERT in Qt 2.x
#    define Q_ASSERT(x) ASSERT(x)
#  endif
#endif // QT_VERSION


// QSTRING_TO_ASCII: a macro to convert a QString into a term. ASCII string.
// Attention: the lifetime of the resulting const char* is bound to its QString!
#if   QT_VERSION >= 0x050000
#define QSTRING_TO_ASCII(qstr) qstr.toLocal8Bit().constData()
#elif QT_VERSION >= 0x040000
#define QSTRING_TO_ASCII(qstr) qstr.toAscii().constData()
#else
#define QSTRING_TO_ASCII(qstr) qstr.latin1()
#endif // QT_VERSION


/**
 * This struct extends 'struct NlvWidget' from the C core with private data
 * needed for the C core-callback functions for *this* wrapper (Qt).
 * It is all internally used (private to the wrapper).
 */
struct WidgetImpl {

    /**
     * This struct is the base "class".
     */
    struct NlvWidget widget;

    /**
     * private struct to access Nlview low-level (core) functions.
     */
    struct NlvCore* nlvcore;

    /**
     * private struct to the Qt renderer.
     */
    struct NlvRend* nlvrend;

    /**
     * Pointer to the object which owns this struct
     * (to be able to call some of its member functions).
     */
    NlvQWidget* obj;

    /**
     * Pixmap used to double-buffer the output (flicker-free update).
     */
    QPixmap* pixmap;

    /**
     * Transformation applied to the widget as a whole: 
     * to compute screen coordinates (X,Y) from 
     * widget coordinates (x,y), do the following:
     * <pre>
     * X = x - xOrigin; Y = y - yOrigin; 
     * </pre>
     * Widget coordinates corresponding to upper-left corner of the
     * drawing window, given in widget pixel units.
     */
    int xOrigin, yOrigin;

    /** Remember the scroll-region's geometry.
     */
    int scrollX, scrollY, scrollW, scrollH;

    /**
     * Define some internal flags.
     */
    enum {
	/**
	 * If this bit is set, the pixmap is not up-to-date and should be
	 * updated as part of the next display operation.
	 */
	DAMAGED = 0x1,
      
	/**
	 * If this bit is set, panning is active.
	 */
	PAN_IN_PROCESS = 0x2
    };

    /* a flag-value storing any flags from the enum above */
    int flags;
    
    /**
     * Coordinates only needed when panning the widget.
     */
    int panX, panY;
};

void NlvQWidget::deleteQPixmap(QPixmap* pixmap)
{
    delete pixmap;
}

QPixmap* NlvQWidget::createQPixmap(int w, int h)
{
    return new QPixmap(w, h);
}


// As long as the Nlview core does not support multi-threading,
// we must maintain a static table of failed draw requests.
//
static struct {
    unsigned count;
    enum { max = 20 };
    NlvQWidget* tab[max];
} failed_draw = {
    0, {0}
};



// This code goes here because we don't want to pollute the NlvQWidget
// namespace

#if QT_VERSION >= 0x040000
static NlvCoreModifiers modifierOf( Qt::KeyboardModifiers state ) {
    NlvCoreModifiers			mods = 0;
    if (state & Qt::ShiftModifier)	mods |= NlvCoreModifier_SHIFT;
    if (state & Qt::ControlModifier)	mods |= NlvCoreModifier_CTRL;
    if (state & Qt::AltModifier)	mods |= NlvCoreModifier_ALT;
    return mods;
}

static NlvCoreButton buttonmaskOf( Qt::MouseButton btn ) {
    switch (btn) {
	case Qt::LeftButton:  return NlvCoreButton_1;
	case Qt::MidButton:   return NlvCoreButton_2;
	case Qt::RightButton: return NlvCoreButton_3;
	default: return NlvCoreButton_NO;
    }
}

#else // Qt 2.x - 3.x:

static NlvCoreModifiers modifierOf( Qt::ButtonState state ) {
    NlvCoreModifiers			mods = 0;
    if (state & Qt::ShiftButton)	mods |= NlvCoreModifier_SHIFT;
    if (state & Qt::ControlButton)	mods |= NlvCoreModifier_CTRL;
    if (state & Qt::AltButton)		mods |= NlvCoreModifier_ALT;
    return mods;
}

static NlvCoreButton buttonmaskOf( Qt::ButtonState state ) {
    switch (state & Qt::MouseButtonMask) {
	case Qt::LeftButton:  return NlvCoreButton_1;
	case Qt::MidButton:   return NlvCoreButton_2;
	case Qt::RightButton: return NlvCoreButton_3;
	default: return NlvCoreButton_NO;
    }
}
#endif // QT_VERSION



static int propertyReceive_int( NlvQWidget* nlv, const char* name ) {
    bool ok;
    const char* res = nlv->command(&ok, "property", name);
    return ok ? NlviewList::toInt(res) : 0;
}



static void propertySend_int( NlvQWidget* nlv, const char* name, int v ) {
    NlviewArgs a;
    a << "property" << name << v;
    nlv->command(NULL, a);
}



static bool propertyReceive_bool( NlvQWidget* nlv, const char* name ) {
    return !!propertyReceive_int( nlv, name );
}



static void propertySend_bool( NlvQWidget* nlv, const char* name, bool v ) {
    propertySend_int( nlv, name, v );
}



static double propertyReceive_double( NlvQWidget* nlv, const char* name ) {
    bool ok;
    const char* res = nlv->command(&ok, "property", name);
    return ok ? NlviewList::toDouble(res) : 0.0;
}



static void propertySend_double( NlvQWidget* nlv, const char* name, double v ) {
    NlviewArgs a;
    a << "property" << name << v;
    nlv->command(NULL, a);
}



/* ======================================================================
 * nlvColor - map QColor -> NlvColor
 * ======================================================================
 */
static NlvColor nlvColor(const QColor& qcol)
{
    NlvColor c;
    c.red     = qcol.red();
    c.green   = qcol.green();
    c.blue    = qcol.blue();
#if QT_VERSION >= 0x040000
    c.transp  = 0xff - qcol.alpha();
#else
    c.transp  = 0;
#endif // QT_VERSION
    return c;
}

/* ======================================================================
 * castColor - map NlvColor -> QColor
 * ======================================================================
 */
static QColor castColor(const NlvColor* c)
{
#if QT_VERSION >= 0x040000
    return QColor(c->red, c->green, c->blue, 0xff - c->transp);
#else
    return QColor(c->red, c->green, c->blue);
#endif
}

static QColor propertyReceive_QColor( NlvQWidget* nlv, const char* name ) {
    bool ok;
    const char* res = nlv->command(&ok, "property", name);
    if (ok) {
	NlvColor col;
	if (NlvColor_init(&col, res))
	    return castColor(&col);
    }
    return Qt::black;	/* we should never get here */
}



static void propertySend_QColor( NlvQWidget* nlv, const char* name,
				 const QColor& qcol ) {
    char value[20];
    NlvColor c = nlvColor(qcol);
    NlvColor_print(c, value);
    nlv->command(NULL, "property", name, value);
}



#define PROPDEF_ANY(type, name, def,  qtp) \
 type NlvQWidget::get##qtp() const \
   { return propertyReceive_##type( (NlvQWidget*) this, name ); } \
 void NlvQWidget::set##qtp(const type v) \
   { propertySend_##type( (NlvQWidget*) this, name, v ); }

#define PROPDEF_ANYREF(type, name, def,  qtp) \
 type NlvQWidget::get##qtp() const \
   { return propertyReceive_##type( (NlvQWidget*) this, name ); } \
 void NlvQWidget::set##qtp(const type& v) \
   { propertySend_##type( (NlvQWidget*) this, name, v ); }

#define PROPDEF_Bool(name,m,def,to,tc,qtp)  PROPDEF_ANY(bool,name,def,qtp)
#define PROPDEF_Short(name,m,def,to,tc,qtp) PROPDEF_ANY(int,name,def,qtp)
#define PROPDEF_Int(name,m,def,to,tc,qtp)   PROPDEF_ANY(int,name,def,qtp)
#define PROPDEF_Float(name,m,def,to,tc,qtp) PROPDEF_ANY(double,name,def,qtp)
#define PROPDEF_Color(name,m,def,to,tc,qtp) PROPDEF_ANYREF(QColor,name,def,qtp)
#define PROPDEF_ColAr(name,m,def,to,tc,qtp) PROPDEF_ANYREF(QColor,name,def,qtp)

#include "propdef.h"

#undef PROPDEF_Bool
#undef PROPDEF_Short
#undef PROPDEF_Int
#undef PROPDEF_Float
#undef PROPDEF_Color
#undef PROPDEF_ColAr
#undef PROPDEF_ANY
#undef PROPDEF_ANYREF

extern "C" {
    static void vGuiVersion(struct NlvWidget*, char buf[64]);
}


NlvQWidget::NlvQWidget( QWidget* parent,
#if QT_VERSION >= 0x050000				// Qt5+
		        Qt::WindowFlags f,
#else							// Qt2..4
		        Qt::WFlags f,
#endif
			int qtrend )
    : QWidget( parent,
#if QT_VERSION >= 0x040000
	       f
#else
	       "NlviewQT", f | WRepaintNoErase | WResizeNoErase
#endif // QT_VERSION
	     )
     , damageX1(0)
     , damageY1(0)
     , damageX2(0)
     , damageY2(0)
     , logFile("")
     , hScroll(NULL)
     , vScroll(NULL)
     , widi(NULL)
     , perfectfs(qtrend == 2)
     , imageMap(NULL)
     , minimaps(NULL)
{
    setMinimumSize( 50, 50 );
    setMouseTracking(true);	// required for elidetext feature

#if QT_VERSION >= 0x040000
    setAttribute(Qt::WA_StaticContents);
    setAttribute(Qt::WA_NoBackground);
    setAttribute(Qt::WA_NoSystemBackground);
#endif // QT_VERSION

    widi = (WidgetImpl*)NlvAlloc(sizeof(WidgetImpl));

    // fill function vector for NlvWidget struct
    widi->widget.guiVersion            = vGuiVersion;
    widi->widget.define_scrollregion   = vDefineScrollregion;
    widi->widget.undefine_scrollregion = vUndefineScrollregion;
    widi->widget.damageAll             = vDamageAll;
    widi->widget.damage                = vDamage;
    widi->widget.resetAll              = vResetAll;
    widi->widget.centerAt              = vCenterAt;
    widi->widget.moveCursorTo          = vMoveCursorTo;
    widi->widget.own_selection         = vOwnSelection;

    widi->widget.page_notify           = vPageNotify;
    widi->widget.selection_notify      = vSelectionNotify;
    widi->widget.hierarchy_godown      = vHierarchyGodown;
    widi->widget.hierarchy_goup        = vHierarchyGoup;
    widi->widget.bindcallback          = vBindcallback;

    widi->widget.message_output        = vMessageOutput;
    widi->widget.progress_notify       = vProgressNotify;
    widi->widget.property_notify       = vPropertyNotify;
    widi->widget.wait_in_mainloop      = vWaitInMainloop;
    widi->widget.get_elided_text       = NULL;	/* can be changed later
						 * with setCustomEliding
						 */
    widi->widget.busy_cursor           = vBusyCursor;
    widi->widget.std_cursor            = vStdCursor;

    widi->widget.start_pan             = vStartPan;
    widi->widget.pan_to                = vPanTo;
    widi->widget.stop_pan              = vStopPan;
    widi->widget.invalidate            = vInvalidate;

    // fill widi's private data
    widi->pixmap = createQPixmap(1,1);
    Q_ASSERT( widi->pixmap );

    widi->nlvrend = NlvRQt_construct(perfectfs);
    Q_ASSERT( widi->nlvrend );
    NlvRQt_set_basefont(widi->nlvrend, font());

#ifdef NLV_TCLCOMPAT
    NlvCore_tclcompat(1);
#endif
    widi->nlvcore = NlvCore_construct(&widi->widget,widi->nlvrend);
    NlvCore_zoomlock(widi->nlvcore, 0/* tell nlvcore: I support zoomlock */);

    widi->obj     = this;
    widi->flags   = 0;
    widi->xOrigin = 0;
    widi->yOrigin = 0;
    widi->scrollX = 0;
    widi->scrollY = 0;
    widi->scrollH = 0;
    widi->scrollW = 0;
    widi->panX    = 0;
    widi->panY    = 0;

    dontUpdateSBValues  = false;

    // initialize local background color property with value from core
    local_shownetattr     = getShownetattr();
    local_showgrid        = getShowgrid();

#if QT_VERSION < 0x040000
    // nlview has other default background settings than qt
    QWidget::setBackgroundColor(getBackgroundcolor());
#endif

    NlvCore_visible_area(widi->nlvcore, 0,0, 0,0, 0,0, NULL);

#if QT_VERSION < 0x050000
    connect(this, SIGNAL(imageMapChanged()), SLOT(imageMapChangedSlot()));
#else
    /* Use new signal-slot connections with pointer-to-member function */
    connect(this, &NlvQWidget::imageMapChanged,
	    this, &NlvQWidget::imageMapChangedSlot);
#endif

    Q_ASSERT(NlvPageNotifyDPage         == (int)PageNotifyDPage);
    Q_ASSERT(NlvPageNotifyDPageCnt      == (int)PageNotifyDPageCnt);
    Q_ASSERT(NlvPageNotifyDModule       == (int)PageNotifyDModule);
    Q_ASSERT(NlvPageNotifyDCleared      == (int)PageNotifyDCleared);
    Q_ASSERT(NlvPageNotifyDLayout       == (int)PageNotifyDLayout);
    Q_ASSERT(NlvPageNotifyDMouse        == (int)PageNotifyDMouse);
    Q_ASSERT(NlvPageNotifyDMagnifyClose == (int)PageNotifyDMagnifyClose);
    Q_ASSERT(NlvPageNotifyDMagnifyOpen  == (int)PageNotifyDMagnifyOpen);

    Q_ASSERT(NlvCoreObj_Error      == (int)NlviewList::O_Error);
    Q_ASSERT(NlvCoreObj_Port       == (int)NlviewList::O_Port);
    Q_ASSERT(NlvCoreObj_PortBus    == (int)NlviewList::O_PortBus);
    Q_ASSERT(NlvCoreObj_Pin        == (int)NlviewList::O_Pin);
    Q_ASSERT(NlvCoreObj_PinBus     == (int)NlviewList::O_PinBus);
    Q_ASSERT(NlvCoreObj_Segm       == (int)NlviewList::O_Segm);
    Q_ASSERT(NlvCoreObj_Segm1io    == (int)NlviewList::O_Segm1io);
    Q_ASSERT(NlvCoreObj_Segm0io    == (int)NlviewList::O_Segm0io);
    Q_ASSERT(NlvCoreObj_Inst       == (int)NlviewList::O_Inst);
    Q_ASSERT(NlvCoreObj_Net        == (int)NlviewList::O_Net);
    Q_ASSERT(NlvCoreObj_NetVector  == (int)NlviewList::O_NetVector);
    Q_ASSERT(NlvCoreObj_NetBundle  == (int)NlviewList::O_NetBundle);
    Q_ASSERT(NlvCoreObj_Symbol     == (int)NlviewList::O_Symbol);
    Q_ASSERT(NlvCoreObj_Attr       == (int)NlviewList::O_Attr);
    Q_ASSERT(NlvCoreObj_Text       == (int)NlviewList::O_Text);
    Q_ASSERT(NlvCoreObj_Image      == (int)NlviewList::O_Image);
    Q_ASSERT(NlvCoreObj_Button     == (int)NlviewList::O_Button);
    Q_ASSERT(NlvCoreObj_HierPin    == (int)NlviewList::O_HierPin);
    Q_ASSERT(NlvCoreObj_HierPinBus == (int)NlviewList::O_HierPinBus);
    Q_ASSERT(NlvCoreObj_PageSubNet == (int)NlviewList::O_PageSubNet);
    Q_ASSERT(NlvCoreObj_CGraphic   == (int)NlviewList::O_CGraphic);
    Q_ASSERT(NlvCoreObj_NetWire    == (int)NlviewList::O_NetWire);
    Q_ASSERT(NlvCoreObj_Last       == (int)NlviewList::O_Last);
}



NlvQWidget::~NlvQWidget() {
    if (imageMap) delete imageMap;
    imageMap = NULL;
    NlvRQt_setImageMap(widi->nlvrend, NULL);	// no emit imageMapChanged here

    //
    // tell all minimaps, which are still registered, that Nlview is gone.
    //
    if (minimaps) {
	int idx = (int)minimaps->count();
	while (--idx >=0) {
	    NlviewMinimap* minimap = minimaps->at(idx);
	    minimap->linkNlview(NULL);
	}
	delete minimaps;
	minimaps = NULL;
    }

    NlvRQt_destruct(widi->nlvrend);
    NlvCore_destruct(widi->nlvcore);
    deleteQPixmap(widi->pixmap);
    NlvFree(widi);

    /*
     * Check failed_draw table and remove pointer to this NlvQWidget object.
     */
    unsigned i;
    NlvQWidget** p = failed_draw.tab;
    for(i=0; i < failed_draw.count; i++) {
        if( failed_draw.tab[i] != this ) *p++ = failed_draw.tab[i];
    }
    failed_draw.count = p - failed_draw.tab;
}



// public static member functions' implementation

const char** NlvQWidget::splitList(const char* string, int* argc) {
    return NlvCore_splitList(string, argc);
}



char* NlvQWidget::mergeList(int argc, const char** argv) {
    return NlvCore_mergeList(argc, argv);
}



void NlvQWidget::freeList(const char** argv) {
    NlvCore_splitFree(argv);
}



void NlvQWidget::freeMerged(char* merged) {
    NlvCore_mergeFree(merged);
}



// public member functions' implementation

const char* NlvQWidget::commandLine( bool* ret, const char* cmd)  {
    const char* result;
    bool	ok;
    SETUPDATESENABLED(false);
    ok = NlvCore_command_line(widi->nlvcore, cmd, &result);
    if (ret) *ret = ok;
    SETUPDATESENABLED(true);
    checkFailedDraw();
    return result;
}


const char* NlvQWidget::command( bool* ret, const char* cmd,
				 const char* arg1, const char* arg2,
				 const char* arg3, const char* arg4,
				 const char* arg5 ) {
    const char* argv[8];
    const char* result;
    bool	ok;
    int argc = 0;
    argv[ argc++ ] =  cmd;
    if( arg1 ) argv[ argc++ ] = arg1;
    if( arg2 ) argv[ argc++ ] = arg2;
    if( arg3 ) argv[ argc++ ] = arg3;
    if( arg4 ) argv[ argc++ ] = arg4;
    if( arg5 ) argv[ argc++ ] = arg5;

    SETUPDATESENABLED(false);
    ok = NlvCore_command(widi->nlvcore, argc, argv, 0, &result);
    if (ret) *ret = ok;
    SETUPDATESENABLED(true);
    checkFailedDraw();
    return result;
}



const char* NlvQWidget::command( bool* ret, int argc, const char** argv) {
    const char* result;
    bool	ok;
    SETUPDATESENABLED(false);
    ok = NlvCore_command(widi->nlvcore, argc, argv, 0, &result);
    if (ret) *ret = ok;
    SETUPDATESENABLED(true);
    checkFailedDraw();
    return result;
}



const char* NlvQWidget::command( bool* ret ) {
    return command(ret, args);
}



const char* NlvQWidget::command( bool* ret, NlviewArgs& that) {
    const char* result;
    bool	ok;
    SETUPDATESENABLED(false);
    ok = NlvCore_command(widi->nlvcore, that.length(), that(), 0, &result);
    if (ret) *ret = ok;
    that.reset();
    SETUPDATESENABLED(true);
    checkFailedDraw();
    return result;
}



void NlvQWidget::setLogfile( const QString& s ) {
    logFile = s;
    args << "logfile";
    if (!logFile.isEmpty()) args << logFile;
    command(NULL);
}



bool NlvQWidget::Print(QPainter* p,
		       const QRect& printarea,
		       int pageNum,
		       enum PrintMode m, enum PrintScale s,
		       enum PrintOrientation o,
		       bool hi,
		       int* xoff, int* yoff, double* zoom,
		       bool fillbg)
{
    bool ret = true;
    if (p == NULL) return false;
    if (!p->isActive()) return false; // painter must be properly initialized
    if (pageNum < -1)   return false; // pageNum out of range. Must be >= -1

    char 		mode = 'M';
    enum NlvPrintScale  scale = NlvPrint_FULL;
    enum NlvPrintOrient orient = NlvPrint_AUTO;
    switch(m) {
		case MONO:	mode = 'M';	break;
		case COLOR:	mode = 'C';	break;
		case COLORINV:  mode = 'I';	break;
		case COLORINV2: mode = '2';	break;
    }
    switch(s) {
		case FULL:	scale = NlvPrint_FULL;    break;
		case FULLFIT:	scale = NlvPrint_FULLFIT; break;
		case VISIBLE:	scale = NlvPrint_VISIBLE; break;
		case VIEW:	scale = NlvPrint_VIEW;    break;
    }
    switch(o) {
		case AUTO:	orient = NlvPrint_AUTO;      break;
		case LANDSCAPE:	orient = NlvPrint_LANDSCAPE; break;
		case PORTRAIT:	orient = NlvPrint_PORTRAIT;  break;
		case ROTATE:	orient = NlvPrint_ROTATE;    break;
		case NOTROTATE:	orient = NlvPrint_NOTROTATE; break;
    }
    NlvRend* printer = NlvRQt_construct(perfectfs);
    p->setFont(font());
    NlvRQt_setImageMap(printer, imageMap);
    NlvRQt_initPrinter(printer, p, &printarea, mode);
    ret = NlvCore_print(widi->nlvcore, printer, pageNum, 
			scale, orient,
			printarea.left(),  printarea.top(),
			printarea.width(), printarea.height(),
			hi, fillbg,
			xoff, yoff, zoom, NULL, NULL, NULL/*result*/);

    NlvRQt_finit(printer);
    NlvRQt_destruct(printer);
    return ret;
}


void NlvQWidget::setHScrollBar( QScrollBar* scrollBar ) {
    if( hScroll )
	hScroll->disconnect( this );
    if( scrollBar ) {
#if QT_VERSION < 0x050000
	connect( scrollBar, SIGNAL(valueChanged( int )),
		 SLOT(hSBarValueChanged( int ) ) );
#else
	connect( scrollBar, &QScrollBar::valueChanged,
		 this,      &NlvQWidget::hSBarValueChanged );
#endif
    }
    hScroll = scrollBar;
}



void NlvQWidget::setVScrollBar( QScrollBar* scrollBar ) {
    if( vScroll )
	vScroll->disconnect( this );
    if( scrollBar ) {
#if QT_VERSION < 0x050000
	connect( scrollBar, SIGNAL(valueChanged( int )),
		 SLOT(vSBarValueChanged( int ) ) );
#else
	connect( scrollBar, &QScrollBar::valueChanged,
		 this,      &NlvQWidget::vSBarValueChanged );
#endif
    }
    vScroll = scrollBar;
}



void NlvQWidget::setFont(const QFont& font) {
    QWidget::setFont(font);
    NlvRQt_set_basefont(widi->nlvrend, font);
}



void NlvQWidget::checkFailedDraw() {
    while (failed_draw.count) {
	/* There have been failed draw calls - because
	 * most probably, they have been called recursively e.g. thru
	 * show-->notify_callback-->update().
	 * Here we re-schedule another call to draw for them.
	 */
	NlvQWidget* w = failed_draw.tab[--failed_draw.count];
	QRect rect(w->damageX1 - w->widi->xOrigin,
		   w->damageY1 - w->widi->yOrigin,
		   w->damageX2 - w->damageX1,
		   w->damageY2 - w->damageY1);
	w->update(rect);
    }
}



void NlvQWidget::setOrigin( int newx, int newy ) {
    widi->xOrigin = newx;
    widi->yOrigin = newy;
    QRect va = QRect(widi->xOrigin, widi->yOrigin, width(), height());
    int coords[4];
    int coordsok = NlvCore_visible_area(widi->nlvcore,
			 widi->xOrigin, widi->yOrigin,
			 va.left(), va.top(), va.width(), va.height(),
			 coords);
    updateScrollbars();
    if (coordsok)
	emit viewportNotify(coords[0], coords[1],
		coords[0]+coords[2], coords[1]+coords[3]);
    emit viewportChanged(coordsok ? va : QRect(),
			 QPoint(widi->xOrigin, widi->yOrigin));
}



void NlvQWidget::scrollTo( int newx, int newy ) {
    int dx = newx - widi->xOrigin;
    int dy = newy - widi->yOrigin;

    setOrigin( newx, newy );

    if (dx == 0 && dy == 0) return;

    copy_pixmap( dx, dy );
    update();
    NlvCore_zoomlock(widi->nlvcore, 0/*stop*/);
}



/* ============================================================================
 * copy_pixmap()
 *
 *      This procedure is used by scrollTo() for scrolling the
 *      offscreen buffer (pixmap) accordingly.
 *      We can achieve this in two ways:
 *
 *      (1) scrolling the pixmap by (dx,dy) and
 *          enlarging the damaged area (== area to be repaired later)
 *      (2) damage complete pixmap by calling pixmapDamageAll()
 *
 *      Note: the damaged area will always be an overestimate, especially when
 *            scrolling both vertically and horizontally (e.g. during panning).
 * ============================================================================
 */
void NlvQWidget::copy_pixmap(int dx, int dy)
{
    if (local_shownetattr & (0x08|0x10|0x40) || local_showgrid & 0x4) {
        pixmapDamageAll();	// (2)
        return;
    }

    // 1.a) scroll the pixmap by (dx, dy):
    //
#if QT_VERSION >= 0x040000 || (QT_VERSION < 300 && defined(WIN))

#if QT_VERSION >= 0x040600	/* QT 4.6+ */
    QRegion exposed;
    widi->pixmap->scroll(-dx, -dy, widi->pixmap->rect(), &exposed);

    // 1.b) enlarge the damaged area by newly exposed region
    //
    exposed.translate(widi->xOrigin, widi->yOrigin);
    QRect bb = exposed.boundingRect();
    pixmapDamage(bb.left(), bb.top(), bb.right()+1, bb.bottom()+1);
    return;
#else	/* QT [4.0..4.6) */
    QPixmap src = *widi->pixmap;	// work-around: cannot copy onto itself
    QPainter p(widi->pixmap);
    p.drawPixmap(-dx, -dy, src);
#endif

#else	/* QT [2.x..3.x.0) */
    QPainter p(widi->pixmap);		// no work-around required for older Qt
    p.drawPixmap(-dx, -dy, *widi->pixmap);
#endif

    // 1.b) enlarge the damaged area by newly exposed region
    //
    int left   = widi->xOrigin;
    int top    = widi->yOrigin;
    int right  = left + width();
    int bottom = top  + height();

    if (dx > 0) {	// scrolled LEFT : damage right window edge
        pixmapDamage(right-dx, top, right, bottom);
    } else if (dx <0 ) {// scrolled RIGHT: damage left  window edge
        pixmapDamage(left, top, left-dx, bottom);
    }
    if (dy > 0) {	// scrolled UP   : damage lower window edge
        pixmapDamage(left, bottom-dy, right, bottom);
    } else if (dy < 0) {// scrolled DOWN : damage upper window edge
        pixmapDamage(left, top, right, top-dy);
    }
}


void NlvQWidget::updateScrollbars() {
    dontUpdateSBValues = true;

    int x = widi->xOrigin;	// set scrollbar value to stored x/yOrigin
    int y = widi->yOrigin;

    int vl = widi->xOrigin;
    int vt = widi->yOrigin;
    int vw = widi->pixmap->width();
    int vh = widi->pixmap->height();
    
    if( hScroll ) {
	int min = widi->scrollX;
	int max = widi->scrollX + widi->scrollW - vw;
	int lineStep = (int) (vw * 0.1);
	int pageStep = vw;
	
	// Special case: window width exceeds widget width:
	if (widi->scrollW <= vw) {
	    // scroll region cut off at left side?
	    if (vl > widi->scrollX) {
		max = min + vl - widi->scrollX;
	    } else {                   
		// scroll region cut off at right side!
		min = vl;
		max = min + widi->scrollX + widi->scrollW - (vw + vl);
		x = min;
	    }
	    if( max < min ) max = min;
	} else {
	    if( max < min ) max = min;
	    // adjust scrollbar range if x (left side) is out of SB range
	    if (!hScroll->ISSLIDERDOWN() && (min != max)) {
		if( x < min ) min = x;    // expand scroll-range to the left
		if( x > max ) max = x;    // expand scroll-range to the right
	    }
	}
	// apply the changes to the scrollbar
	hScroll->setRange( min, max );
#if QT_VERSION >=0x040000
	hScroll->setSingleStep( lineStep );
	hScroll->setPageStep( pageStep );
#else
	hScroll->setSteps( lineStep, pageStep );
#endif // QT_VERSION
	hScroll->setValue( x );
    }
	
    if( vScroll ) {
	int min = widi->scrollY;
	int max = widi->scrollY + widi->scrollH - vh;
	int lineStep = (int) (vh * 0.1);
	int pageStep = vh;

	// Special case: window height exceeds widget height:
	if (widi->scrollH <= vh) {
	    // scroll region cut off at top?
	    if (vt > widi->scrollY) {
		max = min + vt - widi->scrollY;
	    } else {                        
		// scroll region cut off at bottom!
		min = vt;
		max = min + widi->scrollY + widi->scrollH - (vh + vt);
		y = min;
	    }
	    if( max < min ) max = min;
	} else {
	    if( max < min ) max = min;
	    // adjust scrollbar range if y (top) is out of SB range
	    if (!vScroll->ISSLIDERDOWN() && (min != max)) {
		if( y < min ) min = y;    // expand scroll-range to top
		if( y > max ) max = y;    // expand scroll-range to bottom
	    }
	}
	// apply the changes to the scrollbar
	vScroll->setRange( min, max );
#if QT_VERSION >=0x040000
	vScroll->setSingleStep( lineStep );
	vScroll->setPageStep( pageStep );
#else
	vScroll->setSteps(lineStep, pageStep);
#endif // QT_VERSION
	vScroll->setValue( y );
    }
	
    dontUpdateSBValues = false;
}



void NlvQWidget::pixmapDamageAll() {
    pixmapDamage(widi->xOrigin,           widi->yOrigin,
                 widi->xOrigin + width(), widi->yOrigin + height());
}



/* ============================================================================
 * pixmapDamage()
 *
 *      Arrange for parts or all of the pixmap to be repaired within
 *      the next redraw cycle.
 *
 * Arguments:
 *      x1, y1          Upper left corner of area to repair.
 *                      Pixels on edge are redrawn.
 *      x2, y2          Lower right corner of area to repair.
 *                      Pixels on edge are NOT redrawn.
 * ============================================================================
 */
void NlvQWidget::pixmapDamage(int x1,int y1,int x2,int y2) {
    if((x1 >= x2) || (y1 >= y2)) return;

    if (widi->flags & WidgetImpl::DAMAGED) {
	if (x1 <= damageX1) damageX1 = x1;
	if (y1 <= damageY1) damageY1 = y1;
	if (x2 >= damageX2) damageX2 = x2;
	if (y2 >= damageY2) damageY2 = y2;
    } else {
	damageX1 = x1;
	damageY1 = y1;
	damageX2 = x2;
	damageY2 = y2;
	widi->flags |= WidgetImpl::DAMAGED;
    }
}



void NlvQWidget::emitPageNotify(unsigned details,int page_no, int pagecnt) {
    emit pageNotify(details, page_no, pagecnt);
}



void NlvQWidget::emitSelectionNotify(unsigned length) {
    emit selectionNotify(length);
}



void NlvQWidget::emitHierarchyDown(const char* instname) {
    emit hierarchyDown( instname );
}



void NlvQWidget::emitHierarchyUp() {
    emit hierarchyUp();
}



void NlvQWidget::emitBindcallback(const char* binding_type,
		  int down_x, int down_y, int up_x, int up_y,
		  int wdelta, const char* oid) {
    emit bindCallback(binding_type, down_x, down_y, up_x, up_y, wdelta, oid);
}



void NlvQWidget::emitMessageOutput(const char* id, const char* txt) {
    emit messageOutput( id, txt );
}



void NlvQWidget::emitProgressNotify(int cnt, float percent, const char* c) {
    emit progressNotify( cnt, (double) percent, c );
}



void NlvQWidget::waitInMainloop(int ms)
{
    elapsed = false;
    QTimer::singleShot(ms, this, SLOT(timeout()));
    while (!elapsed) {
	/* handle at least paint and timer events in a 'local mainloop'
	 */
#if QT_VERSION >= 0x040000
	QCoreApplication::instance()->processEvents(
	    QEventLoop::ExcludeUserInputEvents
	);
#else
	qApp->processEvents();
#endif // QT_VERSION
    }
}



void NlvQWidget::getCustomElidedText(char*,const char*,int,int,
	enum NlviewList::OType,const char*,bool) {
    // the base implementation of custom text eliding returns null strings.
}

void NlvQWidget::setCustomEliding(bool custom) {
    widi->widget.get_elided_text = custom ? vGetElidedText : NULL;
}
bool NlvQWidget::getCustomEliding() const {
    return widi->widget.get_elided_text != NULL;
}



// -------------------- Implementation of static functions for the widget

extern "C" {	// MSVC6 (-GX) requires this, despite of previous declaration

static void vGuiVersion(struct NlvWidget* ths, char buf[64]) {
    (void)ths;
    NlvSprintf(buf, "QT:%.60s", QT_VERSION_STR);
}

static void vDefineScrollregion(struct NlvWidget* ths, WBox rect) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    // adjust the scrollable area (usually larger than visible area)
    widi->scrollX = rect.left;
    widi->scrollY = rect.top;
    widi->scrollW = rect.right - rect.left;
    widi->scrollH = rect.bot   - rect.top;
    
    // set scrollbars to new scrollable area, but do NOT alter their
    // value (new value is taken from untouched widi->xOrigin, widi->yOrigin)
    widi->obj->updateScrollbars();
    widi->obj->emitMapChanged(QRect(widi->scrollX, widi->scrollY,
				    widi->scrollW, widi->scrollH));
}



static void vUndefineScrollregion(struct NlvWidget* ths) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    
    widi->scrollX = widi->xOrigin;
    widi->scrollY = widi->yOrigin;
    widi->scrollW = widi->pixmap->width();
    widi->scrollH = widi->pixmap->height();
    
    widi->obj->setDontUpdateSBValues(true);
    
    QScrollBar* sb = widi->obj->getHScrollBar();
    if( sb ) {
		sb->SETMINIMUM( 0 );
		sb->SETMAXIMUM( 0 );
		sb->setValue( 0 );
    }
    sb = widi->obj->getVScrollBar();
    if( sb ) {
		sb->SETMINIMUM( 0 );
		sb->SETMAXIMUM( 0 );
		sb->setValue( 0 );
    }
    
    widi->obj->setDontUpdateSBValues(false);
    widi->obj->emitMapChanged(QRect());
}



static void vDamageAll(struct NlvWidget* ths) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->pixmapDamageAll();
    widi->obj->update();
}



static void vDamage(struct NlvWidget* ths, WBox rect) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->pixmapDamage(rect.left, rect.top, rect.right, rect.bot);
    QRect r(rect.left-widi->xOrigin, rect.top-widi->yOrigin,
	    rect.right - rect.left, rect.bot - rect.top);
    widi->obj->update(r);
}



static void vResetAll(struct NlvWidget* ths) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->pixmapDamageAll();
    widi->obj->update();
}



static void vCenterAt(struct NlvWidget* ths, int x, int y){
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->setOrigin(x - widi->pixmap->width()/2,
			 y - widi->pixmap->height()/2);
    widi->obj->pixmapDamageAll();
    widi->obj->update();
}



static void vMoveCursorTo(struct NlvWidget* ths, int x, int y) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    QCursor::setPos(widi->obj->mapToGlobal(QPoint(x - widi->xOrigin,
						  y - widi->yOrigin)));
}



static void vOwnSelection(struct NlvWidget*) {
    // NOT supported
}



static void vPageNotify(struct NlvWidget* ths, unsigned d, int p, int c) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->emitPageNotify(d, p, c);
}



static void vSelectionNotify(struct NlvWidget* ths, unsigned length) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->emitSelectionNotify(length);
}



static void vHierarchyGodown(struct NlvWidget* ths, const char* instname) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->emitHierarchyDown( instname );
}



static void vHierarchyGoup(struct NlvWidget* ths) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->emitHierarchyUp();
}



static void vBindcallback(struct NlvWidget* ths, const char* binding_type,
			  int down_x, int down_y, int up_x, int up_y,
			  int wdelta, const char* oid) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->emitBindcallback(binding_type, down_x, down_y, up_x, up_y,
				wdelta, oid);
}



static void vMessageOutput(struct NlvWidget* ths, const char* id,
			   const char* txt){
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->emitMessageOutput( id, txt );
}



static void vProgressNotify(struct NlvWidget* ths, int cnt, float percent,
			    const char* c) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->emitProgressNotify( cnt, (double) percent, c );
}



/**
 * This function will be called by the core every time a property's
 * value has changed to inform the widget about this update. 
 * For all locally stored properties we also have to apply the changes
 * and can safely ignore all other property-changes.
 */
static void vPropertyNotify(struct NlvWidget* ths,
			    const char* name,
			    const char* value)
{
    WidgetImpl* widi = (WidgetImpl*)ths;
    if (!name || !value) return;

    if (NlvStrcmp(name, "shownetattr") == 0) {
	// update local copy of shownetattr
	widi->obj->local_shownetattr = NlviewList::toInt(value);

    } else if (NlvStrcmp(name, "showgrid") == 0) {
	// update local copy of showgrid
	widi->obj->local_showgrid = NlviewList::toInt(value);

    } else {
	// we ignore other property changes, since the Qt wrapper doesn't store
	// copies of them.
    }

    /* redraw widget (every time) to reflect property changes */
    vDamageAll(ths);
}



static void vWaitInMainloop(struct NlvWidget* ths, int ms)
{
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->waitInMainloop(ms);
}


static void vGetElidedText(struct NlvWidget* ths, char* result,
	const char* text, int len, int maxlen,
	int otype, const char* aname, int plan)
{
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->getCustomElidedText(result, text, len, maxlen,
	(enum NlviewList::OType)(enum NlvCoreObj)otype, aname, plan);
}



static void vBusyCursor(struct NlvWidget* ths) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->setCursor( WAIT_CURSOR );
}



static void vStdCursor(struct NlvWidget* ths) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    widi->obj->setCursor( ARROW_CURSOR );
}



static void vStartPan(struct NlvWidget* ths, int x, int y) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    
    if( widi->flags & WidgetImpl::PAN_IN_PROCESS )
	return;
    
    widi->flags |= WidgetImpl::PAN_IN_PROCESS;
    widi->panX = x;
    widi->panY = y;
    widi->obj->oldCursor = widi->obj->cursor();
    widi->obj->setCursor( SIZE_ALL_CURSOR );
}



static void vPanTo(struct NlvWidget* ths, int x, int y) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    if( !(widi->flags & WidgetImpl::PAN_IN_PROCESS) )
    	return;
    
    int dx = x - widi->panX;
    int dy = y - widi->panY;
    if( !dx && !dy )
    	return;
    
    widi->obj->scrollTo( widi->xOrigin - dx, widi->yOrigin - dy );
    widi->panX = x;
    widi->panY = y;
}



static void vStopPan(struct NlvWidget* ths, int x, int y) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    if( !(widi->flags & WidgetImpl::PAN_IN_PROCESS) )
    	return;

    int dx = x - widi->panX;
    int dy = y - widi->panY;

    widi->obj->setOrigin( widi->xOrigin - dx, widi->yOrigin - dy );
    widi->flags &= ~WidgetImpl::PAN_IN_PROCESS;
    vDamageAll(ths);
    widi->obj->setCursor( widi->obj->oldCursor );
    NlvCore_zoomlock(widi->nlvcore, 0/*stop*/);
}



static void vInvalidate(struct NlvWidget* ths, WBox rect) {
    WidgetImpl* widi = (WidgetImpl*)ths;
    QRect r(rect.left-widi->xOrigin, rect.top-widi->yOrigin,
	    rect.right - rect.left, rect.bot - rect.top);
    widi->obj->update(r);
}

} // extern "C"


// protected member functions' implementation:

void NlvQWidget::mousePressEvent( QMouseEvent* e ) {
    NlvCoreButton but = buttonmaskOf( e->button() );
#if QT_VERSION >= 0x040000
    if( updatesEnabled() == false ) return;	// ignore while busy
#endif // QT_VERSION
    if( but == NlvCoreButton_NO ) return;
    NlvCore_mouse_press(widi->nlvcore, but,
			modifierOf( e->MODIFIERS() ),
			e->x(), e->y());
}



void NlvQWidget::mouseReleaseEvent( QMouseEvent* e ) {
    NlvCoreButton but = buttonmaskOf( e->button() );
    if( but == NlvCoreButton_NO ) return;
    NlvCore_mouse_release(widi->nlvcore, but,
			  modifierOf( e->MODIFIERS() ),
			  e->x(), e->y());
}



void NlvQWidget::mouseDoubleClickEvent( QMouseEvent* e ) {
    NlvCoreButton but = buttonmaskOf( e->button() );
    if( but == NlvCoreButton_NO ) return;
    NlvCore_mouse_dblclick(widi->nlvcore, but,
			   modifierOf( e->MODIFIERS() ),
			   e->x(), e->y());
}


#if QT_VERSION >= 0x040000
#define ANY_MOUSE_BUTTON_DOWN(e) ((e)->buttons() != Qt::NoButton)
#else  // Qt 2.x - 3.x
#define ANY_MOUSE_BUTTON_DOWN(e) ((e)->state() & Qt::MouseButtonMask)
#endif

void NlvQWidget::mouseMoveEvent( QMouseEvent* e ) {
    NlvCore_mouse_move2(widi->nlvcore, ANY_MOUSE_BUTTON_DOWN(e),
		       modifierOf( e->MODIFIERS() ),
		       e->x(), e->y());
}



void NlvQWidget::wheelEvent(QWheelEvent* e) {
#if defined(WIN) && (QT_VERSION < 300)
    // walk around a bug on MS Windows: e->pos() not relative as advertised
    QPoint rel_mousept = mapFromGlobal(e->globalPos());
#else
    QPoint rel_mousept = e->pos();
#endif
    int wdelta = 0;
#if QT_VERSION < 0x050200
    wdelta = -e->delta();
#else	/* Qt 5.2.0 and later: */
    if (e->phase() == Qt::ScrollBegin ||
	e->phase() == Qt::ScrollEnd) return;

    if (!e->angleDelta().isNull()) {
	wdelta = -e->angleDelta().y();	// Nlview only supports 1 dimension. Y?
    } else if (!e->pixelDelta().isNull() && height()) {
	// best guess (for vertical scrolling only):
	// 15deg (==15*8 wdelta units) correspond to 10% of vertical line step
	wdelta = -e->pixelDelta().y() * 1200 / height();
    }
#endif
    NlvCore_mouse_wheel(widi->nlvcore, ANY_MOUSE_BUTTON_DOWN(e),
		       modifierOf(e->MODIFIERS()),
		       rel_mousept.x(), rel_mousept.y(), wdelta);
    e->accept();
}



void NlvQWidget::paintEvent( QPaintEvent* e ) {
    /**
     * A QPainter is used to 
     *   1. draw Nlview schematic into off-screen buffer (if using Qt renderer)
     *   2. copy the updated off-screen buffer to screen.
     *   3. Possibly draw a pending mouse stroke on top (unbuffered!)
     */
    QRect    invalid = e->rect();

    if( widi->flags & WidgetImpl::DAMAGED ) {

	// Limit the damage area to the screen area (without borders)
	//
	int ok = 1;
	limit(damageX1, damageY1, damageX2, damageY2);

	if( (damageX1 < damageX2) && (damageY1 < damageY2) ) {
	    QRect clipRect(				/* Window coords */
		    QPoint(damageX1-widi->xOrigin, damageY1-widi->yOrigin),
		    QSize(damageX2-damageX1, damageY2-damageY1));
	    QPainter p(widi->pixmap);
	    p.setFont(font());
	    NlvRQt_init(widi->nlvrend, &p, &clipRect);

	    widi->nlvrend->set_origin(  widi->nlvrend,
					widi->xOrigin,
					widi->yOrigin, 0);
	    WBox drawwin;
	    drawwin.left  = damageX1-1;		// missing some pixels !?
	    drawwin.top   = damageY1-1;
	    drawwin.right = damageX2+1;
	    drawwin.bot   = damageY2+1;
	    ok = NlvCore_draw(widi->nlvcore, drawwin,
		    (widi->flags & WidgetImpl::PAN_IN_PROCESS) != 0);

	    NlvRQt_finit( widi->nlvrend );
	}
	if( ok ) {
	    widi->flags &= ~WidgetImpl::DAMAGED;
	} else {
	    /* draw() failed; this may happen if we are called
	     * recursively, e.g. thru progress_notify.  The recursive
	     * call may also come from another instance of Nlview
	     */
	    unsigned i;
	    for(i=0; i< failed_draw.count; i++ ) {
		if( failed_draw.tab[i] == this ) goto skip;
	    }
	    if(failed_draw.count < failed_draw.max) {
		failed_draw.tab[failed_draw.count++] = this;
	    }
	    skip: ;
	}
    }

    /**
     * Copy off-screen buffer to screen. Do not use bitBlt here, because
     * otherwise we might lose the QWidget::grab capturing functionality.
     */
    QPainter painter_screen(this);	// will use QWidget::font()
    Q_ASSERT(painter_screen.isActive());
    painter_screen.drawPixmap(
		 invalid.x(), invalid.y(), *widi->pixmap,
		 invalid.x(), invalid.y(),
		 invalid.width(), invalid.height());

    // finally draw the rubberband on top if needed
    if (NlvCore_stroke_pending(widi->nlvcore)) {
	NlvRQt_init(widi->nlvrend, &painter_screen, NULL);
	widi->nlvrend->set_origin(widi->nlvrend,widi->xOrigin,widi->yOrigin,0);
	NlvCore_draw_stroke(widi->nlvcore);
	NlvRQt_finit(widi->nlvrend);
    }
}


void NlvQWidget::limit(int& x1, int& y1, int& x2, int& y2, int w)
{
    // Limit the given area to the screen area (w = border width)
    //
    int x1min = widi->xOrigin + w;
    int y1min = widi->yOrigin + w;
    int x2max = widi->xOrigin + width() - w;
    int y2max = widi->yOrigin + height() - w;
    if( x1 < x1min) x1 = x1min;
    if( y1 < y1min) y1 = y1min;
    if( x2 > x2max) x2 = x2max;
    if( y2 > y2max) y2 = y2max;
}


void NlvQWidget::resizeEvent( QResizeEvent* e ) {
#if QT_VERSION >= 0x040000
    // Qt4 no longer supports QPixmap::resize(). We need to re-create it.
    deleteQPixmap(widi->pixmap);
    widi->pixmap = createQPixmap(width(), height());
    Q_ASSERT( widi->pixmap );
#else  // Qt 2.x - 3.x
    /* adjust the size of the offscreen buffer to the
       current widget size */
    widi->pixmap->resize( width(), height() );
#endif // QT_VERSION
    
    /* call inherited method */
    QWidget::resizeEvent( e );
    vResetAll(&widi->widget);
    /* adjust visible area & scrollbars */
    setOrigin( widi->xOrigin, widi->yOrigin );
    NlvCore_zoomlock(widi->nlvcore, 1/*adjust*/);
}


struct NlvCore* NlvQWidget::core() { return widi->nlvcore; }


void NlvQWidget::registerImage(const QString& iname, QPixmap* img,
    bool limitscale, bool free) {
    if (!imageMap) imageMap = new ImageMap;
    imageMap->add(iname, img, ImageMapValue::PIXMAP, limitscale, free);
    emit imageMapChanged();
}

#if QT_VERSION >= 0x040000
void NlvQWidget::registerImage(const QString& iname, QImage* img,
    bool limitscale, bool free) {
    if (!imageMap) imageMap = new ImageMap;
    imageMap->add(iname, img, ImageMapValue::IMAGE, limitscale, free);
    emit imageMapChanged();
}
#endif

void NlvQWidget::registerImage(const QString& iname, QPicture* img,
    bool limitscale, bool free) {
    if (!imageMap) imageMap = new ImageMap;
    imageMap->add(iname, img, ImageMapValue::PICTURE, limitscale, free);
    emit imageMapChanged();
}

void NlvQWidget::unregisterImage(const QString& iname) {
    if (!imageMap) return;
    imageMap->remove(iname);
    emit imageMapChanged();
}

void NlvQWidget::unregisterImages() {
    if (imageMap) delete imageMap;
    imageMap = NULL;
    emit imageMapChanged();
}


// public slots' implementation

void NlvQWidget::cancel() {
    float f;
    NlvCore_progress(widi->nlvcore, 1, &f);
    // has been aborted and is not called by kernel
    widi->widget.damageAll(&widi->widget);
}



// private slots' implementation

void NlvQWidget::hSBarValueChanged( int value ) {
    if (getDontUpdateSBValues()) return;
    scrollTo( value, widi->yOrigin );
}



void NlvQWidget::vSBarValueChanged( int value ) {
    if (getDontUpdateSBValues()) return;
    scrollTo( widi->xOrigin, value );
}



void NlvQWidget::imageMapChangedSlot() {
    NlvRQt_setImageMap(widi->nlvrend, imageMap);
}



// NlviewMinimap support implementation

void NlvQWidget::linkMinimap(NlviewMinimap* minimap) {
    if (!minimap) return;
    unlinkMinimap(minimap);	// avoid sending duplicate signals to minimap
#if QT_VERSION >= 0x040000
    if (!minimaps) minimaps = new QList<NlviewMinimap*>();
#elif QT_VERSION >= 300		/* QList is not generally available in Qt 3 */
    if (!minimaps) minimaps = new QPtrList<NlviewMinimap>();
#else
    if (!minimaps) minimaps = new QList<NlviewMinimap>();
#endif
    minimaps->append(minimap);

#if QT_VERSION < 0x050000
    connect(this,  SIGNAL(mapChanged(const QRect&)),
	    minimap, SLOT(mapChanged(const QRect&)));
    connect(this,  SIGNAL(viewportChanged(const QRect&,const QPoint&)),
	    minimap, SLOT(viewportChanged(const QRect&,const QPoint&)));
#else
    connect(this,    &NlvQWidget::mapChanged,
	    minimap, &NlviewMinimap::mapChanged);
    connect(this,    &NlvQWidget::viewportChanged,
	    minimap, &NlviewMinimap::viewportChanged);
#endif

    minimap->linkNlview(this);

    emit mapChanged(
	QRect(widi->scrollX, widi->scrollY,
	      widi->scrollW, widi->scrollH));

    emit viewportChanged(
	QRect(widi->xOrigin/*left*/, widi->yOrigin/*top*/, width(), height()),
	QPoint(widi->xOrigin, widi->yOrigin));
}

void NlvQWidget::unlinkMinimap(NlviewMinimap* minimap) {
    if (!minimap || !minimaps || minimaps->count() == 0) return;

    // check, if minimap has been registered before and unregister it
    int idx = (int)minimaps->count();
    while (--idx >=0) {
	if (minimap == minimaps->at(idx)) break;
    }
    if (idx < 0) return;	// minimap not found/registered
#if QT_VERSION >= 0x040000
    minimaps->removeAt(idx);
#else
    minimaps->remove(idx);
#endif

    /* disconnect all of my signals to any of minimap's slots */
    disconnect(this, 0, minimap, 0);
    minimap->linkNlview(NULL);	/* tell Minimap to break the link to Nlview */
}

void NlvQWidget::emitMapChanged(const QRect& scrollr) {
    emit mapChanged(scrollr);
}


////////////////////////////////////////////////////////////////////////
// Implementation of service class NlviewMinimap
//
NlviewMinimap::NlviewMinimap(QWidget* parent,
#if QT_VERSION >= 0x050000				// Qt5+
			     Qt::WindowFlags f
#else							// Qt2..4
			     Qt::WFlags f
#endif
    )
    : QWidget( parent,
#if QT_VERSION >= 0x040000
	       f
#else
	       "NlviewMinimap", f | WRepaintNoErase | WResizeNoErase
#endif // QT_VERSION
	     )
    , hiselvis(false)
    , vpcolor_set(false)
    , nlview(NULL)
    , map(NULL)
    , gripsz(7)
    , old_zoom(0.0)
    , max_zoom(0.0)
    , ratioM(0.0)
{
    printer = NlvRQt_construct(false);
    setMouseTracking(true);
#if QT_VERSION >= 0x040000
    setFocusPolicy(Qt::ClickFocus);	// to receive keyPressEvent
#else
    setFocusPolicy(QWidget::ClickFocus);
#endif // QT_VERSION
    linkNlview(NULL);			// initialize remaining members
}

NlviewMinimap::~NlviewMinimap()
{
    if (nlview) nlview->unlinkMinimap(this);
    if (map) delete map;
    NlvRQt_destruct(printer);
}

void NlviewMinimap::linkNlview(NlvQWidget* _nlview)
{
    // switch to different Nlview: unlink from previous Nlview first!
    if (_nlview && nlview && (_nlview != nlview)) nlview->unlinkMinimap(this);

    nlview    = _nlview;

    // initialize internal state to defaults
    what      = A_NONE;
    scrollr   = QRect();
    viewport  = QRect();		// QRect::isNull() == true
    origin    = QPoint();
    w2m_scale = 0.0;
    map_xoff  = 0;
    map_yoff  = 0;
    map_zoom  = 1.0;
    map_scrollregionM = QRect();
    mapdirty  = true;
    update();
}

void NlviewMinimap::mousePressEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton) return; // only accept left mouse button
    if (viewportInvalid()) return;
    QPoint pt = e->pos();
    what = update_cursor_shape(pt, true);

    clickptM = pt;
    QPoint clickptW = map2widget(clickptM);
    old_origin = origin;

    switch (what) {
	case A_NONE: {
	    QPoint vp_centerM = widget2map(viewport.center());
	    QPoint nsp = old_origin + (clickptW-viewport.center());
	    nlview->args << "scrollpos" << nsp.x() << nsp.y();
	    nlview->command(NULL);	// ignore err

	    what = A_MOVING;		// enter A_MOVING state
	    // pretend the user originally clicked into the
	    // viewport center (important for cancel):
	    clickptM = vp_centerM;
	    setCursor(MOVING_CURSOR);
	    break;
	}
	case A_MOVING:
	    break;
	default: {	// all types of A_SCALING_*
	    old_viewportM = widget2map(viewport);
	    if (old_viewportM.isEmpty()) {
		what = A_NONE;
		break;   // viewport too small in minimap: ignore
	    }
	    old_zoom = map_zoom/w2m_scale;
	    max_zoom = nlview->getMaxzoom();
	    ratioM = (double)old_viewportM.width()/old_viewportM.height();
	    break;
	}
    }
}

void NlviewMinimap::mouseMoveEvent(QMouseEvent* e)
{
    if (viewportInvalid()) return;
    QPoint pt = e->pos();
    QPoint mouseptW = map2widget(pt);
    switch (what) {
	case A_NONE: {
	    if (!ANY_MOUSE_BUTTON_DOWN(e))
		update_cursor_shape(pt, false);
	    return;
	}
	case A_MOVING: {
	    QPoint clickptW = map2widget(clickptM);
	    QPoint nsp = old_origin + (mouseptW-clickptW);
	    nlview->args << "scrollpos" << nsp.x() << nsp.y();
	    nlview->command(NULL);	// ignore err
	    break;
	}
	default: {
	    int dx=0;
	    switch (what) {
		case A_SCALING_UL:
		case A_SCALING_UC:
		case A_SCALING_UR: dx=(int)(ratioM*(clickptM.y()-pt.y()));break;
		case A_SCALING_LL:
		case A_SCALING_LC:
		case A_SCALING_LR: dx=(int)(ratioM*(pt.y()-clickptM.y()));break;
		case A_SCALING_CL: dx=clickptM.x() - pt.x(); break;
		case A_SCALING_CR: dx=pt.x() - clickptM.x(); break;
		default: return;
	    }
	    double new_zoom = old_zoom*1.0/(1.0+2.0*dx/old_viewportM.width());
	    if (new_zoom > max_zoom) new_zoom = max_zoom;
	    if (new_zoom <= 0.0) break;
	    nlview->args << "zoom" << new_zoom;
	    nlview->command(NULL);	// ignore err
	    break;
	}
    }
}

void NlviewMinimap::mouseReleaseEvent(QMouseEvent* e)
{
    what = A_NONE;
    update_cursor_shape(e->pos(), false);
}

void NlviewMinimap::wheelEvent(QWheelEvent* e)
{
    if (viewportInvalid()) return;
    QPoint pt = e->pos();
    double numSteps = (double)e->delta() / (8*15); // support hires wheel-mice

#if QT_VERSION >= 0x040000
    bool CtrlPressed  = !!(e->modifiers() & Qt::ControlModifier);
    bool ShiftPressed = !!(e->modifiers() & Qt::ShiftModifier);
#else  // Qt 2.x - 3.x
    bool CtrlPressed  = !!(e->state() & Qt::ControlButton);
    bool ShiftPressed = !!(e->state() & Qt::ShiftButton);
#if defined(WIN) && (QT_VERSION < 300)
    // walk around a bug on windows: e->pos() not relative as advertised
    pt = mapFromGlobal(e->globalPos());
#endif
#endif
    if (CtrlPressed) {
	QPoint mouseptWIN = map2widget(pt) - origin;	// WINDOW coordinates!

	char zbuf[64];
	// use "locale"-independent NlvSnprintf with %+g to force sign
	NlvSnprintf(zbuf, sizeof(zbuf), "%+g", numSteps*0.2);
	nlview->args	<< "zoom" << zbuf
			<< "-x" << mouseptWIN.x()
			<< "-y" << mouseptWIN.y();
	nlview->command(NULL);
    } else {
	QPoint new_scrollpos = origin;
	Qt::Orientation orient = ShiftPressed ? Qt::Horizontal : Qt::Vertical;
#if QT_VERSION >= 300
	if (e->orientation() == Qt::Horizontal) orient = Qt::Horizontal;
#endif
	if (orient == Qt::Horizontal) {
	    new_scrollpos.rx() -= int(viewport.width()*numSteps*0.1);
	} else {
	    new_scrollpos.ry() -= int(viewport.height()*numSteps*0.1);
	}
	nlview->args << "scrollpos" << new_scrollpos.x() << new_scrollpos.y();
	nlview->command(NULL);
    }
    update_cursor_shape(pt, false);
}

void NlviewMinimap::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) cancel();
    else QWidget::keyPressEvent(e);
}

void NlviewMinimap::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (!map || size() != map->size()) {
	/* resize off-screen image 'map' to window dimension */
	if (map) delete map;
	map = new QPixmap(size());
	mapdirty = true;
    }
    if (!map) return;        	// headless ?
    if (mapdirty) update_map();
    if (mapdirty) return;	// update_map() failed: still dirty. Try later.

    // draw the captured schematic (map) first:
    //
    painter.drawPixmap(0,0, *map);

    // draw the visible area (viewport) rectangle on top:
    //
    if (viewportInvalid()) { unsetCursor(); return; }
    QRect vpM = widget2map(viewport);
#if QT_VERSION >= 0x040000	// Qt4+: use alpha blending effects
#ifdef NLVIEW_FANCY_SHAPES	// either 'torch light' effect ...
    QColor c0   = Qt::black; c0.setAlpha(0);
    QColor c60  = Qt::black; c60.setAlpha(40);
    QColor c100 = Qt::black; c100.setAlpha(100);
    QRadialGradient gradient(vpM.center(),
			     (vpM.height()+vpM.width())/2,
			     vpM.center());
    gradient.setColorAt(0.0, c0);
    gradient.setColorAt(0.6, c60);
    gradient.setColorAt(1.0, c100);
    painter.fillRect(map->rect(), gradient);
#else
    QColor fillcolor = vpcolor;
    fillcolor.setAlpha(127);	// ... or semi-transparent filling
#if QT_VERSION >= 0x040300
    QLinearGradient gradient(vpM.topLeft(), vpM.bottomRight());
    gradient.setColorAt(0, fillcolor.lighter());
    gradient.setColorAt(1, fillcolor.darker());
    painter.fillRect(vpM, gradient);
#else
    painter.fillRect(vpM, fillcolor);
#endif	// Qt >= 4.3.0

#endif	// NLVIEW_FANCY_SHAPES
#endif
    painter.setPen(vpcolor);
    painter.drawRect(vpM);

    // draw the sizer rectangles last:
    //
    scaleGrip(vpM);
    if (gripsz==0) return;
#if QT_VERSION >= 0x040300
    QColor gripcolor = vpcolor.lighter();
    gripcolor.setAlpha(200);
    QPen pen = QPen(gripcolor);
#else
    QPen pen = QPen(vpcolor.light());
#endif
    pen.setWidth(gripsz);
    painter.setPen(pen);
    QPoint sz[8];
    sz[0] =  vpM.bottomRight();
    sz[1] =  vpM.bottomLeft();
    sz[2] =  vpM.topRight();
    sz[3] =  vpM.topLeft();
    sz[4] = (vpM.bottomRight() + vpM.bottomLeft()) /2;
    sz[5] = (vpM.topRight()    + vpM.bottomRight())/2;
    sz[6] = (vpM.topLeft()     + vpM.bottomLeft()) /2;
    sz[7] = (vpM.topRight()    + vpM.topLeft())    /2;
    for (int i=0;i<8; i++) {
#if QT_VERSION >= 0x040000
	painter.drawPoint(sz[i]);
#else
	painter.translate(sz[i].x(), sz[i].y());
	painter.fillRect(QRect(-gripsz/2,-gripsz/2,gripsz,gripsz), pen.color());
	painter.translate(-sz[i].x(), -sz[i].y());      // undo translation
#endif
    }
}

/* This method will try to undo a moving or scaling operation which
 * may currently be performed on the minimap by restoring the original
 * x/yOrigin (scrollpos) and zoom factor.
 * The original (old) values are expected to be set at the time the mouse
 * button was depressed. Cursor will be restored on mouse button release.
 */
void NlviewMinimap::cancel() {
    if (viewportInvalid()) return;
    switch (what) {
	case A_NONE: return;
	case A_MOVING: {
	    nlview->args << "scrollpos" << old_origin.x() << old_origin.y();
	    nlview->command(NULL);			// ignore err
	    break;
	}
	default: {      // all types of A_SCALING_*
	    nlview->args << "zoom" << old_zoom;
	    nlview->command(NULL);			// ignore err
	    // also reset scrollpos to account for inaccuracies
	    nlview->args << "scrollpos" << old_origin.x() << old_origin.y();
	    nlview->command(NULL);			// ignore err
	    break;
	}
    }
    what = A_NONE;
}

void NlviewMinimap::viewportChanged(const QRect& vp, const QPoint& orig)
{
    viewport = vp;
    origin = orig;
    update();
}

void NlviewMinimap::mapChanged(const QRect& scrollregion)
{
    scrollr = scrollregion;
    mapdirty = true;
    update();
}

enum NlviewMinimap::action NlviewMinimap::update_cursor_shape(
	const QPoint& mpt, bool pressed)
{
    if (viewportInvalid()) {
	unsetCursor();
	return A_NONE;
    }
    QRect vpM = widget2map(viewport);

    scaleGrip(vpM);
    QRect sizer = QRect(0,0,gripsz,gripsz);
    QRect ul = sizer; ul.moveCenter(vpM.topLeft());
    QRect uc = sizer; uc.moveCenter((vpM.topRight() + vpM.topLeft())/2);
    QRect ur = sizer; ur.moveCenter(vpM.topRight());
    QRect cl = sizer; cl.moveCenter((vpM.topLeft()  + vpM.bottomLeft())/2);
    QRect cr = sizer; cr.moveCenter((vpM.topRight() + vpM.bottomRight())/2);
    QRect ll = sizer; ll.moveCenter(vpM.bottomLeft());
    QRect lc = sizer; lc.moveCenter((vpM.bottomRight()+vpM.bottomLeft())/2);
    QRect lr = sizer; lr.moveCenter(vpM.bottomRight());

    if (ul.contains(mpt)) { setCursor(SIZEFDIAG_CURSOR);return A_SCALING_UL; }
    if (uc.contains(mpt)) { setCursor(SIZEVER_CURSOR);  return A_SCALING_UC; }
    if (ur.contains(mpt)) { setCursor(SIZEBDIAG_CURSOR);return A_SCALING_UR; }
    if (cl.contains(mpt)) { setCursor(SIZEHOR_CURSOR);  return A_SCALING_CL; }
    if (cr.contains(mpt)) { setCursor(SIZEHOR_CURSOR);  return A_SCALING_CR; }
    if (ll.contains(mpt)) { setCursor(SIZEBDIAG_CURSOR);return A_SCALING_LL; }
    if (lc.contains(mpt)) { setCursor(SIZEVER_CURSOR);  return A_SCALING_LC; }
    if (lr.contains(mpt)) { setCursor(SIZEFDIAG_CURSOR);return A_SCALING_LR; }
    if (vpM.contains(mpt)) {
	setCursor(pressed ? MOVING_CURSOR : MOVABLE_CURSOR);
	return A_MOVING;
    }
    setCursor(CENTERAT_CURSOR);
    return A_NONE;
}

bool NlviewMinimap::viewportInvalid() const
{
    return !nlview || scrollr.isNull() || viewport.isNull()||viewport.isEmpty();
}

void NlviewMinimap::scaleGrip(const QRect& vpM)
{
    int w = (int)(0.2*vpM.width());
    int h = (int)(0.2*vpM.height());
    int m = w>h ? h : w;
    gripsz = m > 9 ? 9 : (m<5 ? 5 : m);		// gripsz range is [5..9]
}

void NlviewMinimap::forceUpdate()
{
    mapdirty = true;
    update();
}

void NlviewMinimap::setVportColor(const QColor& color)
{
    vpcolor = color;
    vpcolor_set = true;
}

bool NlviewMinimap::drawBackground(QPainter*, const QRect&)
{
    return false; // no custom background, we will "paint" the map opaquely
}

void NlviewMinimap::update_map()
{
#if QT_VERSION >= 0x040000	// Qt4+: use alpha blending effects
    map->fill(Qt::transparent);	// initialize to fully transparent
#else
    map->fill();		// initialize to fully opaque (white)
#endif

    QPainter painter(map);
    QRect printarea = map->rect();
    mapdirty = false;

    bool fillbg = !drawBackground(&painter, printarea);

    if (!nlview) return;				// unlinked?!

    // look up Nlview property rubberbandcolor, it might have changed...
    if (!vpcolor_set) vpcolor = nlview->getRubberbandcolor();

    // "print" schematic into image
    painter.setFont(nlview->font());
    NlvRQt_setImageMap(printer, nlview->imageMap);
    NlvRQt_initPrinter(printer, &painter, &printarea, 'C'/*color mode*/);
    int scr[4];
    int ret = NlvCore_print(nlview->core(), printer, -1/*lowres*/, 
			NlvPrint_MINIMAP, NlvPrint_NOTROTATE,
			printarea.left(),  printarea.top(),
			printarea.width(), printarea.height(),
			hiselvis, fillbg,
			&map_xoff, &map_yoff, &map_zoom, NULL, scr,NULL/*res*/);
    map_scrollregionM = ret ? QRect(scr[0],scr[1],scr[2],scr[3]) : QRect();
    NlvRQt_finit(printer);

    if (ret) {
	if (scrollr.isNull() || scrollr.isEmpty() ||
	    map_scrollregionM.isNull() || map_scrollregionM.isEmpty())
	{
	    w2m_scale = 0.0;
	} else {
	    w2m_scale = (double)map_scrollregionM.width() / scrollr.width();
	}
    } else forceUpdate();	// "print" failed, because Nlview is busy;
				// re-schedule a paint event & try later.
}

QPoint NlviewMinimap::map2widget(const QPoint& mappixel) const {
    if (w2m_scale > 0.0) {
	return QPoint((int)((mappixel.x()-map_xoff)/w2m_scale),   
		      (int)((mappixel.y()-map_yoff)/w2m_scale));
    }
    return QPoint(0,0);
}

QPoint NlviewMinimap::widget2map(const QPoint& w) const {
    if (w2m_scale > 0.0) {
	return QPoint(map_xoff+(int)(w.x()*w2m_scale),
		      map_yoff+(int)(w.y()*w2m_scale));
    }
    return QPoint(0,0);
}

QRect NlviewMinimap::widget2map(const QRect& wrect) const {
    return QRect(widget2map(wrect.topLeft()),
		 widget2map(wrect.bottomRight()));
}

////////////////////////////////////////////////////////////////////////
// Implementation of helper class NlviewStrSpace
//
struct MemBlock {
    struct MemBlock* next;
    unsigned         fill;
    char             buf[1];
};

NlviewStrSpace::NlviewStrSpace(const char* s) {
    root = NULL;
    append(s);
}

NlviewStrSpace::~NlviewStrSpace() {
    struct MemBlock* mem = root;
    while (mem) {
	struct MemBlock* disposable = mem;
	mem = mem->next;
	NlvFree(disposable);
    }
}

void NlviewStrSpace::insertBlock(unsigned minsize) {
    unsigned size = DEFAULT_BLOCK_SIZE;
    if (minsize > DEFAULT_BLOCK_SIZE) size = minsize;

    struct MemBlock* newBlock;
    newBlock = (struct MemBlock*)NlvMalloc(sizeof(struct MemBlock)+size);
    newBlock->next = NULL;
    newBlock->fill = 0;

    // insert new block at front of linked memory block-list
    if (!root) {
	root = newBlock;
    } else {
	newBlock->next = root;
	root = newBlock;
    }
}

void NlviewStrSpace::clear() {
    if (!root) return;
#if 1
    // optional: free all memory blocks and keep only the (empty) root block
    struct MemBlock* mem = root->next;
    while (mem) {
	struct MemBlock* disposable = mem;
	mem = mem->next;
	NlvFree(disposable);
    }
    root->next = NULL;
#endif
    root->fill = 0;
}

const char* NlviewStrSpace::append(const char* str) {
    if (!str) return NULL;
    int len = NlvStrlen(str)+1; // len includes the terminating '\0' character
    if (!root || (root->fill+len > DEFAULT_BLOCK_SIZE))
	insertBlock(len);

    char* dst = root->buf+root->fill;
    NlvMemCopy(dst, str, len);
    root->fill += len;
    return dst;
}



////////////////////////////////////////////////////////////////////////
// Implementation of helper class NlviewArgs
//
NlviewArgs::NlviewArgs(): argv(NULL), argc(0), size(0) {
}

NlviewArgs::NlviewArgs(const NlviewArgs& other): argv(NULL), argc(0), size(0) {
    for (int i=0; i<other.argc; i++)
	*this << other.argv[i];
};

NlviewArgs::~NlviewArgs() {
    if (argv) NlvFree(argv);
}

NlviewArgs& NlviewArgs::operator= (const NlviewArgs& other) {
    reset();
    for (int i=0; i<other.argc; i++)
	*this << other.argv[i];
    return *this;
}

NlviewArgs& NlviewArgs::operator<< (const char* arg) {
    const char* buffered_arg = buf.append(arg);	// may be NULL
    if (argc >= size) {
	/*
	 * Enlarge argv vectors to fit more elements.
	 * The new vector size will be stored in size member.
	 */
	size = (argc+16)*2;
	argv = (const char**)NlvRealloc(argv, sizeof(const char*)*size);
	if (!argv) return *this;	/* out of memory */
    }
    argv[argc++] = buffered_arg;
    return *this;
};

NlviewArgs& NlviewArgs::operator<< (const std::string& arg) {
    return *this << arg.c_str();
};

NlviewArgs& NlviewArgs::operator<< (const QString& arg) {
    return *this << QSTRING_TO_ASCII(arg);
};

NlviewArgs& NlviewArgs::operator<< (const NlviewArgs& args) {
    char* str = NlvCore_mergeList(args.argc, args.argv);
    *this << str;		// appends merged 'str' as a single argument
    NlvCore_mergeFree(str);
    return *this;
};

NlviewArgs& NlviewArgs::operator<< (int v) {
    char str[64];
    NlvSnprintf(str, sizeof(str), "%d", v);
    return *this << str;
}

NlviewArgs& NlviewArgs::operator<< (double v) {
    char str[64];
    NlvSnprintf(str, sizeof(str), "%g", v);
    return *this << str;
}

const char* NlviewArgs::operator[] (int index) const {
    return (index>=0 && index<argc) ? argv[index] : NULL;
}

const char** NlviewArgs::operator()() const {
    return argc > 0 ? argv : NULL;
}

int NlviewArgs::length() const {
    return argc;
}

void NlviewArgs::reset() {
    buf.clear();
    argc = 0;
}



////////////////////////////////////////////////////////////////////////
// Implementation of helper class NlviewList
//
NlviewList::NlviewList(const char* s): buf(), argc(0), argv(NULL) {
    list = buf.append(s);
}

NlviewList::NlviewList(const NlviewList& other)
    : buf()
    , argc(0)
    , argv(NULL)
{
    list = buf.append(other.list);
}

NlviewList::NlviewList(const NlviewArgs& args)
    : buf()
    , argc(0)
    , argv(NULL)
{
    char* str = NlvCore_mergeList(args.length(), args());
    list = buf.append(str);
    NlvCore_mergeFree(str);
}

NlviewList::~NlviewList() {
    if (argv) NlvCore_splitFree(argv);
}

NlviewList& NlviewList::operator= (const NlviewList& other) {
    buf.clear();
    if (argv) NlvCore_splitFree(argv);
    argc = 0;
    argv = NULL;
    list = buf.append(other.list);
    return *this;
}

NlviewList& NlviewList::operator= (const NlviewArgs& args) {
    buf.clear();
    if (argv) NlvCore_splitFree(argv);
    argc = 0;
    argv = NULL;
    char* str = NlvCore_mergeList(args.length(), args());
    list = buf.append(str);
    NlvCore_mergeFree(str);
    return *this;
}

NlviewList& NlviewList::operator+= (const NlviewList& other) {
    if (!other.list || other.list[0]=='\0')
	return *this;	/* keep 'this': nothing to append from 'other' */

    if (!list || list[0]=='\0') {
	*this = other;	/* keep 'other': nothing to append to 'this' */
    } else {
	int len1 = NlvStrlen(list);
	int len2 = NlvStrlen(other.list);
	char* tmp = (char*)NlvMalloc(len1+1/*blank*/+len2+1/*'\0'*/);
	NlvStrcpy(tmp, list);
	tmp[len1] = ' ';
	NlvStrcpy(tmp+len1+1, other.list);

	buf.clear();
	if (argv) NlvCore_splitFree(argv);
	argc = 0;
	argv = NULL;
	list = buf.append(tmp);
	NlvFree(tmp);
    }
    return *this;
}

const char* NlviewList::operator[] (int index) const {
    if (!argv) split();
    return (index>=0 && index<argc) ? argv[index] : NULL;
}

bool NlviewList::operator==(const NlviewList& other) const {
    return NlvStrcmp(list?list:"", other.list?other.list:"") == 0;
}

int NlviewList::length() const {
    if (!argv) split();
    return argc;
}

int NlviewList::split() const {
    NlviewList* t = (NlviewList*)this;	/* de-const */
    if (argv) return argc;
    if (!list) return 0;
    t->argv = NlvCore_splitList(list, &t->argc);
    if (argv) return argc;

    /* Badly formatted lists returns argv=NULL (and argc=0) */
    Q_ASSERT(argc==0);
    return -1;
}

bool NlviewList::isEmpty() const {
    const char* p;
    if (!list || !list[0]) return true;
    for (p=list; *p; p++) if (!NlvIsSpace(*p)) return false;
    return true;
}

enum NlviewList::OType NlviewList::objType() const {
	if (!argv) split();

	if (argv && argc > 0) {
	    return (OType)NlvCore_lookupObjType(argv[0]);
	} else {
	    return O_Error;
	}
}

int NlviewList::toInt(const char* arg, const char** endp)
{
    return NlvStrtoul(arg, (char**)endp, 0);
}

double NlviewList::toDouble(const char* arg, const char** endp)
{
    return NlvStrtod(arg, (char**)endp);
}



#undef WAIT_CURSOR
#undef SIZE_ALL_CURSOR
#undef ARROW_CURSOR
#undef SIZEVER_CURSOR
#undef SIZEHOR_CURSOR
#undef SIZEBDIAG_CURSOR
#undef SIZEFDIAG_CURSOR
#undef CENTERAT_CURSOR
#undef MOVABLE_CURSOR
#undef MOVING_CURSOR
#undef QSTRING_TO_ASCII
#undef ANY_MOUSE_BUTTON_DOWN
