#define QT_FATAL_ASSERT /* stop program, if an assertion failed using ASSERT */

#include "riscview.h"
#include "nlvhandler.hpp"

#include <QLinearGradient>
#include <QPainterPath>
#include <QPicture>
#include <stdlib.h>
#include <stdio.h>	// for sscanf
#include <iostream>

#ifdef CKEY
#include <time.h>
#include "nl/licrypt3.h"
#elif defined(NLVLIC)
#include "nlvcookie.h"
#endif

#ifdef TDB
#include <tdb.h>
#endif

#ifdef VVDI
#include <vvdi.h>
#include <veri_file.h>
#include <Netlist.h>
#include <Message.h>
#endif

#ifdef GEX
#include <gdump.h>
#include <gskill.h>
#include <gscene.h>
#endif

#if QT_VERSION >= 0x050000
#define toAscii toLocal8Bit
#endif

/* for IPL32 and LP64 data models */
typedef unsigned long uintPointer;
#define PRINTF_PTR_FMT "0x%lx"

// **********************************************************************
// **********************************************************************
// Nlview - derived from the NlviewQT class "NlvQWidget"
// **********************************************************************
// **********************************************************************

// The OKEMIT macro is used to check for Nlview command() error cases,
// assuming Nlview.command() returned an "ok" boolean and left the
// error message in the macro argument "r".  OKEMIT emits a signal
// and returns from the function call.
//
#define OKEMIT(r) if (!ok) { emit messageOutput("ERR", r); return; }

Nlview::Nlview(bool perfectfs_, QWidget* parent)
    : NlvQWidget(parent, 0, perfectfs_ ? 2:1)
    , withTooltips(false)
{
    createActions();
    setAcceptDrops(true);
    setFocusPolicy(Qt::ClickFocus);
    connect(this, &NlvQWidget::selectionNotify, this, &Nlview::selectionNotify);
    connect(this, &NlvQWidget::pageNotify,      this, &Nlview::pageNotify);

    copyAct->setEnabled(false);
    deleteAct->setEnabled(false);
    magnifyAct->setEnabled(false);
    zoomSelAct->setEnabled(false);
    prevPageAct->setEnabled(false);
    nextPageAct->setEnabled(false);
    // The following binding allows dragging the Magnify Window by its title
    args << "bind" << "stroke-1-magnify cgraphic titlebar"
	<< "-action" << "instdrag"
	<< "-echo_type" << "instdrag"
	<< "-autoscroll";
    command(NULL);

}

Nlview::~Nlview() {}

void Nlview::selectionNotify(unsigned n_items)
{
    bool ok = n_items > 0;	// enable QActions if > 0 items are selected

    copyAct->setEnabled(ok);
    deleteAct->setEnabled(ok);
    magnifyAct->setEnabled(ok);
    zoomSelAct->setEnabled(ok);
}

void Nlview::pageNotify(unsigned details, int curpage, int npages)
{
    if (details & (NlvQWidget::PageNotifyDPage|
		   NlvQWidget::PageNotifyDPageCnt|
		   NlvQWidget::PageNotifyDModule|
		   NlvQWidget::PageNotifyDCleared)) {

	// The following actions require a "current page"
	//
	fullfitAct->setEnabled(curpage > 0);
	fullscreenAct->setEnabled(curpage > 0);
	zoom1Act->setEnabled(curpage > 0);
	zoomInAct->setEnabled(curpage > 0);
	zoomOutAct->setEnabled(curpage > 0);
	optimizeAct->setEnabled(curpage > 0);

	// enable/disable our page navigation actions
	//
	prevPageAct->setEnabled(curpage > 1);
	nextPageAct->setEnabled(curpage > 0 && curpage < npages);

	// The following actions require a "current module"
	//
	bool haveCurMod;
	if (curpage > 0) {
	    haveCurMod = true;		// "current page" => "current module"
	} else {
	    bool ok;
	    NlviewList res = command(&ok, "module", "info");
	    haveCurMod = ok && !res.isEmpty();
	}
	regenerateAct->setEnabled(haveCurMod);
    }
}

void Nlview::enableTooltips(bool t) { withTooltips = t; }

// **********************************************************************
// (a) Implement drag&drop by overwriting dragEnterEvent, dropEvent,
//     mousePressEvent and mouseMoveEvent.
//
void Nlview::dragEnterEvent( QDragEnterEvent* e )
{
    if (e->mimeData()->hasText()) {	// "text/plain" available?
	// validate the string-based Nlview OID we see attached to this event
	// before we accept the proposed drag event action:
	//
	NlviewList oiddrop = e->mimeData()->text().toAscii().constData();
	// Expect "Tcl-list" structure with at least two elements: type + name.
	// We accept only some (well-known) types of object_ids:
	if (oiddrop.split()>1) {
	    switch (oiddrop.objType()) {
		case NlviewList::O_Pin:
		case NlviewList::O_PinBus:
		case NlviewList::O_HierPin:
		case NlviewList::O_HierPinBus:
		case NlviewList::O_Port:
		case NlviewList::O_PortBus:
		case NlviewList::O_Inst:
		case NlviewList::O_Net:
		case NlviewList::O_NetVector:
		case NlviewList::O_NetBundle:
		case NlviewList::O_NetWire: {
		    // We could perform a "search -exists" command to validate
		    // OID further...
		    e->acceptProposedAction();
		    break;
		}
		default:	// do not accept (ignore) everything else
		    break;
	    }
	}
    } else {
	NlvQWidget::dragEnterEvent( e );
    }
}

void Nlview::dropEvent( QDropEvent* e )
{
    switch (e->dropAction()) {
	case Qt::MoveAction:
	case Qt::CopyAction: {
	    // assume attached OID has already been validated in dragEnterEvent
	    //
	    NlviewList r;
	    QByteArray oid = e->mimeData()->text().toAscii();
	    bool ok;
	    QPoint rel_mousept = e->pos();
	    args << "center" << oid.constData()
		 << "-x" << rel_mousept.x()
		 << "-y" << rel_mousept.y();
	    r = command(&ok);					OKEMIT(r)
	    args << "highlight" << "-itemized" << oid.constData();
	    r = command(&ok);					OKEMIT(r)
	    e->acceptProposedAction();
	    break;
	}
	default: NlvQWidget::dropEvent( e );
    }
}

void Nlview::mousePressEvent( QMouseEvent* e )
{
    dragStartPosition = e->pos();
    // call base class to handle the built-in behavior for that event
    NlvQWidget::mousePressEvent( e );
}

void Nlview::mouseMoveEvent( QMouseEvent* e )
{
    bool CtrlOnlyPressed = (e->modifiers() == Qt::ControlModifier);
    if ((e->buttons() & Qt::LeftButton) && CtrlOnlyPressed) {
	if ((e->pos() - dragStartPosition).manhattanLength()
	    < QApplication::startDragDistance()) return;  // below threshold?

	bool 	    ok;
	const char* r;

	// stop poss. Nlview mouse stroke
	r = command(&ok, "bind", "-cancel");			OKEMIT(r)

	// check what the user originally clicked on
	args << "pick" << "-shortfmt" << "-hier" << "*"
					<< dragStartPosition.x()
					<< dragStartPosition.y();
	NlviewList oid = command(&ok);
	if (!ok || oid.isEmpty()) return;	// error or nothing clicked on

	QDrag*     drag     = new QDrag(this);
	QMimeData* mimeData = new QMimeData;
	mimeData->setText(oid.getText());
	drag->setMimeData(mimeData);

	// just for fun: attach a (scaled) screendump of the obj to drag
	NlviewList bboxstr = command(&ok, "bbox", oid);
	if (ok && bboxstr.split()==4) {
	    int left  = NlviewList::toInt(bboxstr[0]);
	    int top   = NlviewList::toInt(bboxstr[1]);
	    int right = NlviewList::toInt(bboxstr[2]);
	    int bot   = NlviewList::toInt(bboxstr[3]);
	    QRect bbox(QPoint(left,top), QPoint(right,bot));
	    bbox = bbox.intersected(rect()).adjusted(-1,-1,+1,+1);
	    QPixmap pm = grab(bbox);
	    // make background transparent, but *may* cause flicker,
	    // due to Qt::WA_NoSystemBackground==false on top level
	    pm.setMask(pm.createMaskFromColor(getBackgroundcolor(),
					      Qt::MaskInColor));
	    const int max = 100;  // shrink to 100x100px if necessary
	    if (pm.width()>max || pm.height()>max) {
		pm = pm.scaled(QSize(max,max),
			       Qt::KeepAspectRatio,
			       Qt::SmoothTransformation);
	    }
	    drag->setPixmap(pm);
	}

	drag->exec(Qt::CopyAction|Qt::MoveAction, Qt::CopyAction);
    } else {
	// call base class to handle the built-in behavior for that event
	NlvQWidget::mouseMoveEvent( e );
    }
}

// **********************************************************************
// (b) Implement tooltips by overwriting event() to catch QHelpEvent
//
//	From the Qt4 docs (http://qt-project.org/doc/qt-4.8/qtooltip.html):
//	"It is also possible to show different tool tips for different regions
//	of a widget, by using a QHelpEvent of type QEvent::ToolTip.
//	Intercept the help event in your widget's event() function
//	and call QToolTip::showText() with the text you want to display."
//
bool Nlview::event( QEvent* event )
{
    if (event->type() == QEvent::ToolTip && withTooltips) {

	QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);

	bool ok;
	args << "pick" << "-regular" << "-hier" << "*"
	     << helpEvent->x() << helpEvent->y();
	QString obj = command(&ok);
	if (ok && obj.length() > 0) {
	    if (obj.length()>50) { // tooltip's length max. 50 chars
		obj.truncate(50);
		obj.append("...");
	    }
	    QToolTip::showText(helpEvent->globalPos(), obj, this);
	}
    }
    // call base class to handle the built-in behavior for that event
    return NlvQWidget::event(event);
}

// **********************************************************************
// (c) Implement popup context menu by overwriting contextMenuEvent()
//     createActions creates the QActions needed for contextMenuEvent().
//     The popup context menu may send the signal deleteSelection to Demo
//     or may call the local methods copy, fullfit, zoom, regenerate etc.
//
void Nlview::createActions()
{
    copyAct = new QAction(tr("&Copy"), this);
    copyAct->setStatusTip(tr("Copy IDs of current selection to clipboard"));

    deleteAct = new QAction(tr("&Delete/ictrl less"), this);
    deleteAct->setStatusTip(tr("Remove selected object(s) from schematic"));

    magnifyAct = new QAction(tr("&Magnify"), this);
    magnifyAct->setStatusTip(tr("Open Nlview's Magnify inline-Window"));

    fullfitAct = new QAction(tr("&Fullfit"), this);
    fullfitAct->setStatusTip(tr("Fit the schematic into window"));

    fullscreenAct = new QAction(tr("Fullscreen"), this);
    fullscreenAct->setStatusTip(tr("Fit the full page into window"));

    zoomSelAct = new QAction(tr("&Zoom selected object(s)"), this);
    zoomSelAct->setStatusTip(tr("Zoom to the current selection"));

    zoom1Act = new QAction(tr("Zoom &1:1"), this);
    zoom1Act->setStatusTip(tr("Zoom to native size (1)"));

    zoomInAct = new QAction(tr("Zoom &in"), this);
    zoomInAct->setStatusTip(tr("Zoom into the schematic"));

    zoomOutAct = new QAction(tr("Zoom out"), this);
    zoomOutAct->setStatusTip(tr("Zoom out of the schematic"));

    regenerateAct = new QAction(tr("&Regenerate"), this);
    regenerateAct->setStatusTip(tr("Regenerate the current schematic page"));

    optimizeAct = new QAction(tr("&Optimize"), this);
    optimizeAct->setStatusTip(tr("Optimize the current schematic page"));

    prevPageAct = new QAction(tr("Show prev page"), this);
    prevPageAct->setStatusTip(tr("Generate/display the prev schematic page"));

    nextPageAct = new QAction(tr("Show next page"), this);
    nextPageAct->setStatusTip(tr("Generate/display the next schematic page"));

    generateCodeAct = new QAction(tr("Generate scheme from random source"), this);
    generateCodeAct->setStatusTip(tr("Generate funny stuff"));

    QAction* bindCancelAct = new QAction(this);

    connect(copyAct,       &QAction::triggered, this, &Nlview::copy);
    connect(deleteAct,     &QAction::triggered, this, &Nlview::deleteSelection);
    connect(magnifyAct,    &QAction::triggered, this, &Nlview::magnify);
    connect(fullfitAct,    &QAction::triggered, this, &Nlview::fullfit);
    connect(fullscreenAct, &QAction::triggered, this, &Nlview::fullscreen);
    connect(zoomSelAct,    &QAction::triggered, this, &Nlview::zoomSel);
    connect(zoom1Act,      &QAction::triggered, this, &Nlview::zoom1);
    connect(zoomInAct,     &QAction::triggered, this, &Nlview::zoomIn);
    connect(zoomOutAct,    &QAction::triggered, this, &Nlview::zoomOut);
    connect(regenerateAct, &QAction::triggered, this, &Nlview::regenerate);
    connect(optimizeAct,   &QAction::triggered, this, &Nlview::optimize);
    connect(prevPageAct,   &QAction::triggered, this, &Nlview::prevPage);
    connect(nextPageAct,   &QAction::triggered, this, &Nlview::nextPage);
    connect(bindCancelAct, &QAction::triggered, this, &Nlview::bindCancel);


#if QT_VERSION >= 0x040200
    copyAct->setShortcut(QKeySequence::Copy);
    deleteAct->setShortcuts(QList<QKeySequence>() << QKeySequence::Delete
						  << QKeySequence("Shift+D"));
    magnifyAct->setShortcut(QKeySequence("m"));
    fullfitAct->setShortcut(QKeySequence("f"));
    fullscreenAct->setShortcut(QKeySequence("Shift+F"));
    zoomSelAct->setShortcut(QKeySequence("z"));
    zoom1Act->setShortcut(QKeySequence("1"));
    zoomInAct->setShortcuts(QList<QKeySequence>() << QKeySequence::ZoomIn
						  << QKeySequence("i"));
    zoomOutAct->setShortcuts(QList<QKeySequence>() << QKeySequence::ZoomOut
						  << QKeySequence("o"));
    regenerateAct->setShortcut(QKeySequence("Shift+R"));
    optimizeAct->setShortcut(QKeySequence("Shift+O"));
    prevPageAct->setShortcut(QKeySequence::MoveToPreviousPage);
    nextPageAct->setShortcut(QKeySequence::MoveToNextPage);
    bindCancelAct->setShortcut(Qt::Key_Escape);

    // add all actions to an active widget to make the actions' shortcuts work
    addAction(copyAct);
    addAction(deleteAct);
    addAction(magnifyAct);
    addAction(fullfitAct);
    addAction(fullscreenAct);
    addAction(zoomSelAct);
    addAction(zoom1Act);
    addAction(zoomInAct);
    addAction(zoomOutAct);
    addAction(regenerateAct);
    addAction(optimizeAct);
    addAction(prevPageAct);
    addAction(nextPageAct);
    addAction(bindCancelAct);
#endif
}

void Nlview::contextMenuEvent( QContextMenuEvent* e )
{
    QMenu menu(this);
    menu.addAction(copyAct);
    menu.addAction(deleteAct);
    menu.addAction(magnifyAct);
    menu.addSeparator();
    menu.addAction(fullfitAct);
    menu.addAction(fullscreenAct);
    menu.addAction(zoomSelAct);
    menu.addAction(zoom1Act);
    menu.addAction(zoomInAct);
    menu.addAction(zoomOutAct);
    menu.addSeparator();
    menu.addAction(regenerateAct);
    menu.addAction(optimizeAct);
    menu.addSeparator();
    menu.addAction(prevPageAct);
    menu.addAction(nextPageAct);
    menu.exec(e->globalPos());
}

void Nlview::copy()
{
    bool 	ok;
    const char* r = command(&ok, "selection", "-regular");	OKEMIT(r)
    QClipboard* clip = qApp->clipboard();
    clip->setText(r);
}

void Nlview::magnify()
{
    bool 	ok;
    const char* r = command(&ok, "last_selection", "-shortfmt");OKEMIT(r)

    args << "magnify" << r;
    r = command(&ok);						OKEMIT(r)
}

void Nlview::fullfit()    { command(NULL, "fullfit");    }
void Nlview::fullscreen() { command(NULL, "fullscreen"); }

void Nlview::zoomSel()
{
    bool 	ok;
    const char* r;
    r = command(&ok, "selection", "-shortfmt");			OKEMIT(r)

    args << "center_objects" << "-zoom_to" << "-all" << r;
    r = command(&ok);						OKEMIT(r)
}

void Nlview::zoom1()   { command(NULL, "zoom", "1");  }
void Nlview::zoomIn()  { command(NULL, "zoom", "+1"); }
void Nlview::zoomOut() { command(NULL, "zoom", "-1"); }

void Nlview::regenerate()
{
    bool ok;
    const char* r = command(&ok, "regenerate", "-curpage");	OKEMIT(r)
}

void Nlview::optimize()
{
    bool ok;
    const char* r = command(&ok, "regenerate", "-optimize");	OKEMIT(r)
}

void Nlview::generateCode()
{
    bool ok;
    std::cout << command(&ok, "clear") << std::endl;
    std::cout << command(&ok, "regenerate", "-optimize") << std::endl;
}

void Nlview::prevPage()   { command(NULL, "show", "--"); }
void Nlview::nextPage()   { command(NULL, "show", "++"); }
void Nlview::bindCancel() { command(NULL, "bind", "-cancel"); }

// **********************************************************************
// (e) Listen to left-mouse double-click and send signal incrMore to
//     Demo to implement VDI-based Incremental Schematic
//
void Nlview::mouseDoubleClickEvent( QMouseEvent* e )
{
    // call base class to handle the built-in behavior for that event
    NlvQWidget::mouseDoubleClickEvent( e );

    if (e->button() & Qt::LeftButton) {
	emit incrMore(e->modifiers(), e->x(), e->y());
    }
}


// **********************************************************************
// **********************************************************************
// MyComboBox - extend QComboBox by history & navigation
// **********************************************************************
// **********************************************************************

MyComboBox::MyComboBox(QWidget* parent)
    : QComboBox( parent) {
#if QT_VERSION >= 0x050700
    /* By default, for an editable combo box, a QCompleter that performs
     * case insensitive inline completion is automatically created.
     */
#else
    setAutoCompletion( true );
#endif /* QT_VERSION */
    setMaxCount( 200 );
    setInsertPolicy( QComboBox::InsertAtBottom );
    setMaxVisibleItems( 20 );
    setEditable( true );
    addItem("");
}

void MyComboBox::keyPressEvent(QKeyEvent* e) {
    bool CtrlOnlyPressed = (e->modifiers() == Qt::ControlModifier);
    if (e->key()==Qt::Key_Enter ||
	e->key()==Qt::Key_Return) {
	e->accept();
	QString txt = currentText();
	if (!txt.isEmpty()) {
	    setItemText(count()-1, txt);
	    addItem("");
	    setCurrentIndex(count()-1);
	    clearEditText();
	    emit ReturnPressed(txt);
	}
    } else if (e->key()==Qt::Key_D && CtrlOnlyPressed) {
		qApp->quit();
		return;
    } else if (e->key()==Qt::Key_Up) {
		int curitem = currentIndex();
		if (curitem>0) setCurrentIndex(curitem-1);
    } else if (e->key()==Qt::Key_Down) {
		int curitem = currentIndex();
		if (curitem<count()-1) setCurrentIndex(curitem+1);
    } else {
    	QComboBox::keyPressEvent(e);
    }
}



// **********************************************************************
// **********************************************************************
// Demo - the application's MainWindow
// **********************************************************************
// **********************************************************************

static const char *tapp_icon[] = {
/* width height num_colors chars_per_pixel */
"    53    48        5            1",
/* colors */
". c #00d1d1",
"# c #00d1eb",
"a c #332288",
"b c None",
"c c #8fffff",
/* pixels */
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbcbbbbbcbbbccccbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbcbbbbbcbbbcbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbcbbbbbcbbcbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbcbbbbbcbbccccbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbcbbbbbcbbcbbbcbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbcbbbbbcbbbbbbcbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbcbbbbbcbbbbbbcbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbb###bbbbbbbcbbbcbbbcbbbcbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbb#bb#bbbbbbbcccbbbbbcccbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbb#bb#bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbb####bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbb#bb#bb.........................bbbbbbbbbbbbbbbbbbb",
"bbb#bb#bb.aaaaaaaaaaaaaaaaaaaaaaa....bbbbbbbbbbbbbbbb",
"bbb###bbb.aaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbbbbbbbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbbbbbbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbb###bbbb",
"#########.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbb#bbb#bbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbb#bbb#bbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbb#bbb#bbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bb#bbb#bbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bb#bbb#bbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bb###bbbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbb",
"bbbb#bbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa##########",
"bbb#b#bbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbb",
"bbb#b#bbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbb",
"bb#bbb#bb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbb",
"bb#####bb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbbb",
"bb#bbb#bb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbbb",
"b#bbbbb#b.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbbbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbbbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbbbbb",
"#########.aaaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbbbbbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbbbbbbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaaaaaa.bbbbbbbbbbbbbbb",
"bbbbbbbbb.aaaaaaaaaaaaaaaaaaaaaaa....bbbbbbbbbbbbbbbb",
"bbbbbbbbb.........................bbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
};

Demo::Demo(const char* argv0, bool perfectfs_)
  : appName(argv0)
  , perfectfs(perfectfs_)
  , printer(QPrinter::HighResolution)
  , withTooltips(false)
  , withTransistors(false)
  , stopAtOperators(false)
  , ictrlMoreLess(true)
  , verilogTopModule(NULL)
{
    setWindowIcon(QPixmap(tapp_icon));
    setWindowTitle(appName);

    // create nlview
    nlview = new Nlview(perfectfs);

    // create inputLine
    inputLine = new MyComboBox;

    // create logWindow
    logWindow = new LogWindow;
    logWindow->setLineWrapMode(LogWindow::WidgetWidth);
    logWindow->setWordWrapMode(QTextOption::WordWrap);
    logWindow->setReadOnly(true);
    logWindow->setMinimumHeight(40);

    createMenus();
    // addMinimapDock(); create a minimap dock window only via Options menu

    // add a progress bar and cancel button in status bar
    progressBar = new QProgressBar( statusBar() );
    progressBar->setRange( 0, 100 );
    progressBar->setVisible(false);
    cancelButton = new QPushButton(tr("Cancel"));
    cancelButton->setVisible(false);
    cancelButton->setToolTip(tr("Interrupt a running Nlview command"));
    connect(cancelButton, &QPushButton::clicked, nlview, &NlvQWidget::cancel);
    attachLabel = NULL;
    statusBar()->setSizeGripEnabled( true );
    statusBar()->addWidget(progressBar);
    statusBar()->addWidget(cancelButton);

    // build-up the GUI central window
    setCentralWidget( createCentral() );

    logthis('i', "This is a C++/Qt test application for NlviewQT"
		 " - find mouse bindings in the Help menu");

}

Demo::~Demo() {
    unlicense();
	ncs.quit();
}

// **********************************************************************
// (1) create Qt GUI objects
//
void Demo::createMenus()
{
    QAction* a;

    // File menu entries...
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    a = fileMenu->addAction(tr("&Open nlv..."), this, SLOT(openNetList()),
	tr("F3"));
    a->setStatusTip(tr("Open a Nlview netlist (command file)"));

    fileMenu->addSeparator();

    a = fileMenu->addAction(tr("S&ave image..."), this,SLOT(saveImage()),
	tr("F2"));
    a->setStatusTip(tr("Save the schematic as bitmap image"));

    fileMenu->addSeparator();

    a = fileMenu->addAction(tr("&Print..."), this, SLOT(print()), tr("Ctrl+P"));
    a->setStatusTip(tr("Print the schematic"));

    a = fileMenu->addAction(tr("Print Pre&view..."), this,SLOT(printPreview()));
    a->setStatusTip(tr("Display preview of schematic printout"));

    a = fileMenu->addAction(tr("Page &Setup..."), this, SLOT(pageSetup()));
    a->setStatusTip(tr("Change page properties for printing"));
    fileMenu->addSeparator();
    a = fileMenu->addAction(tr("&Gdump..."), this, SLOT(gdump()));
    a->setStatusTip(tr("Dump the schematic using GEI"));

    a = fileMenu->addAction(tr("Gs&kill..."), this, SLOT(gskill()));
    a->setStatusTip(tr("Write a SKILL script using GEI"));
    a = fileMenu->addAction(tr("Gsc&ene..."), this,SLOT(gscene()),tr("Ctrl+E"));
    a->setStatusTip(tr("Create a Qt QGraphicsScene using GEI"));
    fileMenu->addSeparator();

    a = fileMenu->addAction(tr("&Quit"), qApp, SLOT(quit()), tr("Ctrl+Q"));
    a->setStatusTip(tr("Quits this program"));



    // File menu entries...
    QMenu* custMenu = menuBar()->addMenu(tr("&Custom"));

    a = custMenu->addAction(tr("&Do a specific custom action"), this, SLOT(doCustomAction()));
    a->setStatusTip(tr("This is a status tip for cust action"));

    custMenu->addSeparator();


    // Option menu entries...
    optionsMenu = menuBar()->addMenu(tr("&Options"));

    a = optionsMenu->addAction(tr("&Show Tooltips"),
				this, SLOT(toggleTooltips()), tr("Ctrl+S"));
    a->setStatusTip(tr("Toggle context sensitive tooltips"));
    a->setCheckable(true);
    a->setChecked(withTooltips);

    a = optionsMenu->addAction(tr("&Add MiniMap"), this,SLOT(addMinimapDock()));
    a->setStatusTip(tr("Create a new MiniMap dock window"));
    optionsMenu->addSeparator();

#if defined(TDB) || defined(VVDI)
    // Ictrl menu entries...
    QMenu* ictrlMenu = menuBar()->addMenu(tr("&Ictrl"));

    a = ictrlMenu->addAction(tr("Attach with &Loadmodule"),
				this, SLOT(attach_loadmodule()), tr("Ctrl+L"));
    a->setStatusTip(tr("Attach VDI-based DB, run ictrl in full-module mode"));

    a = ictrlMenu->addAction(tr("Attach for &More/Less"),
				this, SLOT(attach_moreless()), tr("Ctrl+M"));
    a->setStatusTip(tr("Attach VDI-based DB, run ictrl in increment mode"));

    ictrlMenu->addSeparator();

    a = ictrlMenu->addAction(tr("&Detach"), this, SLOT(detach()));
    a->setStatusTip(tr("Detach from VDI-based DB"));
#endif

#ifdef TDB
    ictrlMenu->addSeparator();

    a =ictrlMenu->addAction(tr("&Transistor Level"), this, SLOT(toggleTrans()));
    a->setStatusTip(tr("Switch between transistor-level and gate-level TDB"));
    a->setCheckable(true);
    a->setChecked(withTransistors);
#endif
#ifdef VVDI
    ictrlMenu->addSeparator();

    a =ictrlMenu->addAction(tr("stop at &Operators"), this, SLOT(toggleOper()));
    a->setStatusTip(tr("Stop hierarchy at operator level (needs new Attach)"));
    a->setCheckable(true);
    a->setChecked(stopAtOperators);
#endif

    // Page actions... a little bit page navigation
    prevPageAct= menuBar()->addAction(tr("<<"), nlview, SLOT(prevPage()));
    nextPageAct= menuBar()->addAction(tr(">>"), nlview, SLOT(nextPage()));
    nextPageAct->setStatusTip(tr("Show next schematic page"));
    prevPageAct->setStatusTip(tr("Show previous schematic page"));
    prevPageAct->setEnabled( false );
    nextPageAct->setEnabled( false );

    // Help menu entries...
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    a = helpMenu->addAction(tr("Mouse &Strokes"), this, SLOT(helpStrokes()));
    a->setStatusTip(tr("Display information about how to use Mouse strokes"));

    a = helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
    a->setStatusTip(tr("Display information about Qt"));

    a = helpMenu->addAction(tr("&About"), this, SLOT(about()), tr("F1"));
    a->setStatusTip(tr("Display information about this program"));
}

QWidget* Demo::createCentral() const
{
    // =========================================================
    // This is the UPPER splitter part (Nlview, h-/v-scrollbars)
    //
    QWidget* upper = new QWidget;
    QGridLayout* upper_grid = new QGridLayout(upper);

    Q_ASSERT(nlview);		// created in Demo constructor
    QScrollBar* hScrollBar = new QScrollBar( Qt::Horizontal );
    QScrollBar* vScrollBar = new QScrollBar( Qt::Vertical );

    nlview->setHScrollBar( hScrollBar );
    nlview->setVScrollBar( vScrollBar );

    upper_grid->addWidget( nlview,     0, 0 );
    upper_grid->addWidget( hScrollBar, 1, 0 );
    upper_grid->addWidget( vScrollBar, 0, 1 );
#if QT_VERSION >= 0x040300
    upper_grid->setContentsMargins(3, 0, 0, 0);
#else
    upper_grid->setMargin(0);
#endif
    upper_grid->setSpacing(0);
    upper_grid->setRowStretch(    0, 1 );
    upper_grid->setColumnStretch( 0, 1 );

    // =========================================================
    // This is the LOWER splitter part (commandline, label, log)
    //
    QWidget* lower = new QWidget;
    QGridLayout* lower_grid = new QGridLayout(lower);

    // --- COMMANDLINE ---
    Q_ASSERT(inputLine);		// created in Demo constructor

    // --- LABEL ---
    QLabel* label = new QLabel(tr("&Nlview API command:"));
    label->setBuddy(inputLine);

    // --- LOG ---
    Q_ASSERT(logWindow);		// created in Demo constructor

    lower_grid->addWidget( label,     0, 0 );
    lower_grid->addWidget( inputLine, 0, 1 );
    lower_grid->addWidget( logWindow, 1, 0, -1, -1); // take up all cells
#if QT_VERSION >= 0x040300
    lower_grid->setContentsMargins(3, 0, 3, 0);
#else
    lower_grid->setMargin(0);
#endif
    lower_grid->setRowStretch(    1, 1 );
    lower_grid->setColumnStretch( 1, 1 );


    // =========================================================
    // Create & set up a QSplitter as the main layout manager
    //
    QSplitter* splitter = new QSplitter( Qt::Vertical);
    splitter->addWidget(upper);
    splitter->addWidget(lower);
    splitter->setCollapsible(0/*upper*/, false);
    splitter->setOpaqueResize( true );
    splitter->setStretchFactor( 0/*upper*/, 100/*keep size*/ );
    splitter->setStretchFactor( 1/*lower*/, 20 );

    // we connect to the following signals emitted by Nlview:
    connect(nlview, &NlvQWidget::progressNotify, this,&Demo::progressNotify);
    connect(nlview, &NlvQWidget::selectionNotify,this,&Demo::selectionNotify);
    connect(nlview, &NlvQWidget::pageNotify,     this,&Demo::pageNotify);
    connect(nlview, &NlvQWidget::messageOutput,  this,&Demo::messageOutput);
    connect(nlview, &NlvQWidget::hierarchyDown,  this,&Demo::hierarchyDown);
    connect(nlview, &NlvQWidget::hierarchyUp,    this,&Demo::hierarchyUp);
    connect(nlview, &NlvQWidget::bindCallback,   this,&Demo::bindCallback);

    connect(nlview, &Nlview::deleteSelection,    this,&Demo::deleteSelection);
    connect(nlview, &Nlview::incrMore,           this,&Demo::incrMore);

    inputLine->setFocus();

    connect( inputLine, SIGNAL(ReturnPressed(const QString&)),
	     this,      SLOT  (evalCommand  (const QString&)) );

    return splitter;
}

// The OK macro is used to check for Nlview command() error cases,
// similar to OKEMIT above - but here, we call failure() directly
// to print the error message to the log window.  In case of an
// error, OK terminates the void-returning procedure.
//
// The OKF macro is identical to OK, except it returns false for
// bool-returning functions.
//
#define OK(r) if (!ok) { failure(r); return; }
#define OKF(r) if (!ok) return failure(r);


// **********************************************************************
// (2) setup Nlview license
//
bool Demo::license()
{
    const char* widgetname = "Nlview";
    bool 	ok;
    const char* result;
    unsigned    challenge;
    char        buffer[1024];

    result = nlview->command(&ok, "license", "-salt");
    if (!ok) goto error;
    challenge = strtoul(result, NULL, 0);

#ifdef CKEY
    /* HERE: Do the mandatory Application License Checking */

    /* Here, we call licrypt3() using the secret CKEY to create a
     * valid cookie.
     */
#    undef  STRINGIFY
#    define STRINGIFY(a) STRINGIFY2(a)
#    define STRINGIFY2(a) #a
    licrypt3(buffer, challenge, widgetname, STRINGIFY(CKEY));

#elif defined(NLVLIC)
{
    /* Here, we call NlviewCookie() to use FLEXnet to checkout
     * a license for FEATURE "nlview". If that succeeds, then NlviewCookie()
     * will return a valid cookie.
     */
    int flags = NlviewCookieFFlex|NlviewCookieFNoCache;
    ok = NlviewCookie(buffer, widgetname, flags, challenge);
    if (!ok) { result = buffer; goto error; }
}
#else
    /**
     * If we get here, we have neither a CKEY (customer key) nor the
     * nlvlic package (for FLEXnet licensing).
     *
     *   Warning: NlviewQT is not licensed.
     *   You must compile the test application either with define
     *     -DCKEY=yourkey (customer key) or with
     *     -DNLVLIC (FlexLM licensing)
     */
    (void)challenge;
    sprintf(buffer, "NlviewQT widget `%.255s' could NOT be licensed!\n"
	    "Neither CKEY (customer key) nor NLVLIC (FlexLM licensing)\n"
	    "was defined during compilation!", widgetname);
    result = buffer;
    goto error;
#endif
    /* here, the "buffer" stores the cookie from NlviewCookie() or licrypt3() */

    result = nlview->command(&ok, "license", widgetname, "-salt", buffer);
    if (!ok) goto error;
    std::cout << "license ok!" << std::endl;
    return true/*ok*/;

  error:
    logthis('w', result);
    qWarning("%s", result);
    return false/*error*/;
}

bool Demo::unlicense()
{
#ifdef CKEY
    return true;
#elif defined(NLVLIC)
    char msg_buf[1024];
    return (NlviewUncheck(msg_buf) != 0);
#else
    return true;
#endif
}

// **********************************************************************
// (3) deal with logging (to log window)
//
bool Demo::executeCommandLine(const char* cmdline)
{
    logthis('l', cmdline);

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    bool       ok;
    NlviewList result = nlview->commandLine(&ok, cmdline);
    if (ok) {
	if (!result.isEmpty()) logthis('r', result);
    } else {
	logthis('e', result);
    }
    return ok;
}

static void writeLog(Demo::LogWindow* log, const char* color, bool bold,
    QString txt)
{
    if (txt.length() > 1024) {
	txt = txt.left(1024).append("...");
    }
    QString    html = Qt::convertFromPlainText(txt, Qt::WhiteSpaceNormal);
    if (bold)  html = QString("<b>%1</b>").arg(html);
    if (color) html = QString("<font color=%1>%2</font>").arg(color, html);

#if QT_VERSION >= 0x040400
    log->appendHtml(html);
#else
    log->append(html);
#endif

#if QT_VERSION >= 0x040200
    log->moveCursor(QTextCursor::End);
#else
    QTextCursor tc = log->textCursor();
    tc.movePosition(QTextCursor::End);
    log->setTextCursor(tc);
#endif
    log->ensureCursorVisible();
}

void Demo::logthis(char severity, const char* msg) const
{
    const char* color;
    bool 	b = false;
    switch (severity) {
	// Qt supports color names defined by the W3C:
	// http://www.w3.org/TR/SVG/types.html#ColorKeywords
 	case 'e': color = "red";	b=true;	break;
	case 'r': color = "darkblue";		break;		/* OK result */
	case 'w': color = "darkorange";	b=true;	break;
	case 'l': color = "black";		break;
	case 'i': color = "slategray";		break;
	default: Q_ASSERT(0); return;
    }
    writeLog(logWindow, color, b, msg);
}

bool Demo::failure(const char* result)
{
    logthis('e', result);
    return false;
}

// **********************************************************************
// (4) Implementations for the Nlview notify signals
//
void Demo::progressNotify( int cnt, double percentage, const char* )
{
    if (cnt == -1) {
	progressBar->reset();
	progressBar->setVisible(false);
	cancelButton->setVisible(false);
	nlview->setUpdatesEnabled(true);
    } else if (cnt >= 0) {
	progressBar->setVisible(true);
	progressBar->setValue( (int) percentage );
	cancelButton->setVisible(true);
	/**
	 * In order to see a progress-bar update, we must process pending
	 * paint events.
	 * This will also process user input events; so be careful, because
	 * NlvQWidget does not support reentrancy. But we need user
	 * interaction to capture "Cancel" button press-events.
	 * If you do not need this, you can prevent processing user input
	 * events by using:
	 *
	 *	qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
	 *
	 * The call to setUpdatesEnabled(false) is required to avoid screen
	 * flicker. Usually, updates are disabled while a Nlview command is
	 * running, but some Qt versions seem to enable it again internally
	 * before the command has finished. So we disable updates again here.
	 */
	nlview->setUpdatesEnabled(false);
	qApp->processEvents();
    }
}

void Demo::selectionNotify(unsigned n_items)
{
    if (n_items > 0) {
	bool ok;
	NlviewList selList = nlview->command(&ok, "selection", "-regular");
	OK(selList)
	if (ok && !selList.isEmpty()) {
	    statusBar()->showMessage(selList.getText());
	    return;
	}
    }
    statusBar()->clearMessage();
}

void Demo::pageNotify(unsigned details, int curpage, int npages)
{
    if (details & (NlvQWidget::PageNotifyDPage|
		   NlvQWidget::PageNotifyDPageCnt|
		   NlvQWidget::PageNotifyDModule|
		   NlvQWidget::PageNotifyDCleared)) {

	// enable/disable our page navigation buttons: << and >>
	//
	prevPageAct->setEnabled( curpage > 1 );
	nextPageAct->setEnabled( curpage > 0 && curpage < npages );
    }
}

void Demo::messageOutput( const char* id, const char* txt )
{
    // take severity level from id
    char severity;
    if (qstrcmp(id, "ERR") == 0) severity = 'e';
    else if (id[0] == 'W')	 severity = 'w';
    else			 severity = 'i';

    logthis(severity, txt);
}

void Demo::bindCallback(const char* bindingType,
			  int downX, int downY,
			  int upX,   int upY, int wdelta, const char* oid)
{
    NlviewArgs items;
    items << bindingType << downX << downY << upX << upY << wdelta
	  << (oid ? oid : "-");
    logthis('i', NlviewList(items).getText());
}

// **********************************************************************
// (5) Implementations for some basic GUI functions
//
bool Demo::evalCommand(const QString& cmdline) {// inputLine's ReturnPressed

    // For "shell-convenience", we support some non-Nlview API commands
    // that can be entered in our inputline widget: exit pwd cd ls
    //
    if (cmdline.trimmed() == "exit") {
	qApp->quit();
	return true;
    }
    if (cmdline.trimmed() == "pwd") {
	logthis('i', cmdline.toAscii().constData());
	logthis('r', QDir::currentPath().toAscii().constData());
	return true;
    }
    if (cmdline.trimmed().left(3) == "cd " || cmdline.trimmed() == "cd") {
	logthis('i', cmdline.toAscii().constData());
	QString newDir;
	if (cmdline.trimmed() == "cd") {		// "cd" => exe-path
	    newDir = QCoreApplication::applicationDirPath();
	} else {
	    newDir = cmdline.mid(3).trimmed();		// "cd <dir>" => dir
	    if (newDir == "~") newDir = QDir::homePath();// "cd ~" => HOME
	    else if (newDir.startsWith("~/"))
		newDir.replace(0,1,QDir::homePath());	// "cd ~/tmp"=>HOME/tmp
	    else if (newDir.startsWith("~"))
		newDir.replace(0,1,QDir::homePath()+"/../"); // cd ~u2=>HOME/u2
	}
	bool ok = QDir::setCurrent(newDir);
	if (!ok)
	    logthis('e', QString(newDir+" not found").toAscii().constData());
	return ok;
    }
    if (cmdline.trimmed().left(3) == "ls " || cmdline.trimmed() == "ls") {
	logthis('i', cmdline.toAscii().constData());
	QFileInfo fi;
	if (cmdline.trimmed().left(3) == "ls ") {	// "ls <arg>"
	    QString arg = cmdline.mid(3).trimmed();
	    if (arg == "~") arg = QDir::homePath();	// "~" => HOME
	    else if (arg.startsWith("~/"))
		arg.replace(0,1,QDir::homePath());	// "~/tmp" => HOME/tmp
	    else if (arg.startsWith("~"))
		arg.replace(0,1,QDir::homePath()+"/../"); // "~u2" => HOME/u2
	    fi = QFileInfo(arg);
	} else {					// "ls"
	    fi = QFileInfo(".");
	}
	QString     dirPath;
	QStringList nameFilters;			// empty filter
	if (fi.isDir()) {
	    dirPath = fi.filePath();			// "ls dir" => ls dir
	} else {
	    dirPath = fi.dir().path();			// "ls fo/baz* -> ls fo
	    nameFilters << fi.fileName();		// filter: ["baz*"]
	}
	QDir dir(dirPath);
	dir.setNameFilters(nameFilters);
	dir.setFilter(QDir::Dirs | QDir::Files | QDir::CaseSensitive
#if QT_VERSION >= 0x040200
	    | QDir::NoDotAndDotDot
#endif
	);
	dir.setSorting(QDir::Name | QDir::DirsFirst);

	QFileInfoList flist = dir.entryInfoList();
	for (int i=0; i < flist.size(); i++) {
	    QFileInfo fi = flist.at(i);
	    logthis(fi.isDir() ? 'l':'r', fi.fileName().toAscii().constData());
	}
	return true;
    }

    // ...everything else is treated as a Nlview API command-line:
    return executeCommandLine(cmdline.toAscii().constData());
}

void Demo::openVerilog() 			// menu File/Open
{
    static QString lastNetListName(QDir::currentPath());
    QStringList choice =
	QFileDialog::getOpenFileNames( this,
				      "Open Verilog files",
				      lastNetListName/*bug 84783 fixed 4.1.0*/,
				      "Verilog (*.v);;all (*)" );
    if( choice.count() ) {
	lastNetListName = QFileInfo(choice[0]).path();
	detach();
#ifdef VVDI
	/* Call Verilog Parser */
	int  i;
	int  ok;
	theLogWindow = logWindow;
	using namespace Verific;
	Message::SetConsoleOutput(0);	/* disable console output */
	Message::RegisterCallBackMsg(verificMsgCB);
	Message::SetAllMessageType(VERIFIC_INFO, VERIFIC_IGNORE);
	for(i=0; i<choice.count(); i++) {
	    ok = veri_file::Analyze(choice[i].toAscii().constData());
	    if (!ok) return;	/* error */
	}
	ok = veri_file::ElaborateAll();
	if (!ok) return; 	/* error */
	attach_moreless();
#endif
    }
}

void Demo::openNetList() 			// menu File/Open
{
    static QString lastNetListName(QDir::currentPath());
    QString choice =
	QFileDialog::getOpenFileName( this,
				      "Open Nlview Netlist",
				      lastNetListName/*bug 84783 fixed 4.1.0*/,
				      "Nlview Net List (*.nlv);;all (*)" );
    if( !choice.isNull() ) {
	lastNetListName = choice;
	detach();

	NlviewArgs args;
	args << "source" << choice;
	NlviewList cmdline(args);	// properly "merge" into a commandline
	executeCommandLine((const char*)cmdline);
    }
}

void Demo::saveImage() 				// menu File/Save image
{
    static QString lastImageName(QDir::currentPath());

    // get a list of supported image writer formats
    QList<QByteArray> formats = QImageWriter::supportedImageFormats();
    QString fmtstr = "all (*)";
    while (!formats.isEmpty()) {
	QString fmt = formats.takeFirst();
	fmtstr += ";; " + fmt.toUpper() + " (*." + fmt.toLower() + ")";
    }

    QString choice =
	QFileDialog::getSaveFileName( this,
				      "Save schematic as image",
				      lastImageName/*bug 84783 fixed 4.1.0*/,
				      fmtstr);
    if( !choice.isNull() ) {
	lastImageName = choice;
	QPixmap* pm = new QPixmap(nlview->size());
	Q_ASSERT(pm);
	QPainter p(pm);
	nlview->Print(	&p,
			nlview->rect(),	// window area to capture
			0,		// current page in high resolution
			NlvQWidget::COLOR,
			NlvQWidget::VIEW,
			NlvQWidget::NOTROTATE,
			true);		// include highlight and selection
	p.end();
#if QT_VERSION >= 0x040200
	bool ok = pm->save(choice); // guess image format from file extension
#else
	QFileInfo fi(choice);
	bool ok = pm->save(choice, fi.suffix().toAscii().constData());
#endif
	delete pm;
	if (!ok) {
	    QString msg = tr("Failed to save image to file: ")+choice;
	    failure(msg.toAscii().constData());
	} else {
	    QString msg = tr("Successfully saved image to file: ")+choice;
	    statusBar()->showMessage(msg);
	}
    }
}

void Demo::toggleTooltips()			// menu Options/Show Tooltips
{
    withTooltips = !withTooltips;
    nlview->enableTooltips(withTooltips);
}

class MyNlviewMinimap : public NlviewMinimap {
 public:
    MyNlviewMinimap(QWidget* parent = NULL) : NlviewMinimap(parent) {}
    bool drawBackground(QPainter* p, const QRect& r)
    {
	NlvQWidget* nlv = getLinkedNlview();
	if (nlv) {			// a Nlview is linked to this minimap
	    QLinearGradient gradient(r.topLeft(), r.bottomRight());
	    gradient.setColorAt(0, Qt::white);
	    gradient.setColorAt(1, nlv->getBackgroundcolor());
	    p->fillRect(r, gradient);
	} else {
	    p->setPen(Qt::red);
	    p->setFont(QFont("Arial", (int)(0.7*qMin(r.height(),r.width()))));
	    p->drawText(r, Qt::AlignCenter, "?");
	}
	return true;
    }
};

void Demo::addMinimapDock()			// menu Options/Add Minimap
{
    static int cnt = 0;
    QString title = tr("MiniMap");
    if (++cnt>1) title.append(QString(" #%1").arg(cnt));
    QDockWidget* minimapdock = new QDockWidget(title, this);
#if QT_VERSION >= 0x040300
    minimapdock->setFeatures(	QDockWidget::DockWidgetVerticalTitleBar |
				QDockWidget::DockWidgetClosable         |
				QDockWidget::DockWidgetMovable          |
				QDockWidget::DockWidgetFloatable );
#endif
    minimapdock->setFloating(false);
    minimapdock->setMinimumSize(QSize(100,100));
    minimapdock->setContentsMargins(0,0,5,0);

    MyNlviewMinimap* minimap = new MyNlviewMinimap(minimapdock);
    nlview->linkMinimap(minimap);
    minimapdock->setWidget(minimap);
    addDockWidget(Qt::BottomDockWidgetArea, minimapdock);

    QAction* minimapAct = minimapdock->toggleViewAction();
    minimapAct->setStatusTip(tr("Hide or show ")+title);
    optionsMenu->addAction(minimapAct);
}

void Demo::about()				// menu Help/About
{
    bool 	ok;
    const char* vers = nlview->command(&ok, "version");		OK(vers)

    QString txt;
    txt.append("Demo application for NlviewQT widget\n");
    txt.append("Copyright(c) 2000-2018 Concept Engineering GmbH\n");
    txt.append("http://www.concept.de\n\n");

#ifdef VVDI
    int vver = Vvdi::Verific_version();
    txt.append("VVDI available (Verific version "+QString::number(vver)+")\n");
#endif

#ifdef NLVIEW_DLLIMPORT
#ifdef TDB
    txt.append("TDB library available\n");
#endif
#ifdef GEX
    txt.append("GEX library available\n");
#endif
#else /* no NLVIEW_DLLIMPORT */
#ifdef TDB
    txt.append("TDB available\n");
#endif
#ifdef GEX
    txt.append("GEX available\n");
#endif
#endif /* NLVIEW_DLLIMPORT */


#ifdef CKEY
    txt.append("LICENSE via cookie (CKEY)\n");
#elif defined(NLVLIC)
    txt.append("LICENSE via FLEXnet library\n");
    char notice[1024];
    if (NlviewGetNotice(notice))
	txt.append(QString(notice).prepend("\tfor ").append("\n"));
    const char* LMenv = getenv("LM_LICENSE_FILE");
    if (LMenv) {
	txt.append("LM_LICENSE_FILE=");
	txt.append(LMenv);
	txt.append("\n");
    }
#endif
    txt.append("\n");

    txt.append("Screen renderer: Qt");
    if (perfectfs) txt.append("/-perfectfs");
    txt.append("\n");

    txt.append("Nlview Core Version: ");
    txt.append(vers);
    txt.append("\n");

    txt.append(appName);
    txt.append(" built: " __DATE__ "/" __TIME__ " with Qt/");
    txt.append(QT_VERSION_STR);
    txt.append("\n");

    QMessageBox::about( this, "About", txt);
}

void Demo::helpStrokes()			// menu Help/Mouse Strokes
{
    QString txt = "<b>The current mouse bindings are</b>:\n";

    txt.append("<ul>\n");
    txt.append("<li>Leftclick to select single object\n");
    txt.append("<li>Left doubleclick on offpage connector to navigate\n");
    txt.append("<li>Shift+Leftclick to append selection\n");
    txt.append("<li>Shift+drag left mouse button: Nlview drag'n'drop\n");
#if QT_VERSION >= 0x040300
    txt.append("<li>Ctrl+drag left mouse button: Qt drag'n'drop\n");
#endif // QT_VERSION
    txt.append("<li>Drag middle mouse button for panning\n");
    txt.append("<li>Rightclick for popup-menu\n");
    txt.append("<li>wheel for vertical scrolling\n");
    txt.append("<li>Shift+wheel for horizontal scrolling\n");
    txt.append("<li>Ctrl+wheel for zoom in/out\n");
    txt.append("</ul>\n");

    if (sane()) {
      if (ictrlMoreLess) {
	txt.append("More bindings while Ictrl is in more/less mode:\n");
	txt.append("<ul>\n");
	txt.append("<li>Left doubleclick on net/pin/port - increment more\n");
	txt.append("<li>Left doubleclick on HIERBOX inst - autohide pins\n");
	txt.append("<li>Left doubleclick on other HIER inst - unfold/fold\n");
	txt.append("<li>Left Shift+doubleclick on inst - incr addmodule\n");
	txt.append("<li>Left Shift+doubleclick on background - incr add top\n");
	txt.append("</ul>\n");
      } else {
	txt.append("More bindings while Ictrl is in loadmodule mode:\n");
	txt.append("<ul>\n");
	txt.append("<li>Left stroke to nw: go up the hierarchy\n");
	txt.append("<li>Left doubleclick on instances: dive down\n");
	txt.append("</ul>\n");
      }
    }
    QMessageBox::information(this, "Mouse Strokes", txt, QMessageBox::Ok);
}

// **********************************************************************
// (6) Implementations for PRINT functions called from GUI
//
int Demo::getAllPages()
{
    bool ok;
    const char* nPagesStr = nlview->command(&ok, "npages");
    if (!ok) { failure(nPagesStr); return 0; }
    return NlviewList::toInt(nPagesStr);
}

int Demo::getCurrentPage()
{
    bool ok;
    const char* currentPageStr = nlview->command(&ok, "page");
    if (!ok) { failure(currentPageStr); return 0; }
    return NlviewList::toInt(currentPageStr);
}

// This function is called both from slot Demo::paintRequested as
// well as directly by the normal Demo::Print routine for the
// actual printing process.
// The printing and print preview features share common code.
//
void Demo::doPrint(
	QPrinter* pr,      // the "output" device
	int fromPg,	   // first schematic page to print, starts with 1
	int toPg)	   // last  schematic page to print (max. is npages)
{
    bool reverse = (pr->pageOrder() == QPrinter::LastPageFirst);
    int copies, curPg, sheetCnt;

    if (fromPg > toPg) {
	int tmp = fromPg;	// swap page numbers and change page order
	fromPg = toPg;
	toPg = tmp;
	reverse = !reverse;
    }

    enum NlvQWidget::PrintMode colormode =
	pr->colorMode() == QPrinter::GrayScale ?
	    NlvQWidget::MONO : NlvQWidget::COLOR;

    QPainter p(pr);

    /**
     * p.viewport() -->
     *   full sheet-size rectangle (no borders) in device coordinates.
     *   E.g. QRect(l=0,t=0,w=744,h=1053).
     *
     * pr->pageRect() -->
     *   the printable area (including borders) in device coordinates.
     *   E.g. QRect(l=30,t=30,w=684,h=993).
     *   This rectangle can be used as the clipping rectangle.
     *
     * QPainter's origin is translated to the upper left corner of the
     * printable area (because calling pr->setFullPage(false)
     * automatically respects the sheet's margins), i.e. we can start
     * painting at (0,0).
     */

    // print to file cancelled ?
    if (pr->printerState() == QPrinter::Aborted) return;
    QRect printarea(0,0, pr->pageRect().width(), pr->pageRect().height());

#if QT_VERSION >= 0x040700
    int numCopies = pr->copyCount();
#else
    int numCopies = pr->numCopies();
#endif

    for (copies=1; copies <= numCopies; copies++) {
	for (curPg=reverse ? toPg          : fromPg, sheetCnt=0;
		   reverse ? curPg>=fromPg : curPg<=toPg;
		   reverse ? curPg--       : curPg++)
	{
	    statusBar()->showMessage(
		QString("Printing page %1: sheet %2 of %3, copy %4 of %5...")
		.arg(curPg)
		.arg(++sheetCnt).arg(1 + toPg - fromPg)
		.arg(copies).arg(numCopies));
	    nlview->Print(&p, printarea, curPg, colormode,
			  NlvQWidget::FULL, NlvQWidget::NOTROTATE);
	    if (reverse ? curPg>fromPg : curPg<toPg) { pr->newPage(); }
	    if (pr->printerState() == QPrinter::Aborted) break;
	}  // pages
	if (pr->printerState() == QPrinter::Aborted) break;
	if (copies < numCopies) { pr->newPage(); }
    } // copies
    statusBar()->clearMessage();
}

void Demo::print()				// menu File/Print
{
    int curPg = getCurrentPage();
    if (curPg==0) return;

    int maxPg = getAllPages();
    if (maxPg==0) return;

    printer.setFullPage(false);
    printer.setColorMode(QPrinter::GrayScale);
    printer.setDocName("Schematic");
    printer.setCreator("NlviewQT");
    printer.setCopyCount(1);
    printer.setPageOrder(QPrinter::FirstPageFirst);

    /**
     * There are a lot of bugs in QPrintDialog (Qt-4.0.1/Unix), e.g.
     * 1. Cannot set/change the page range
     * 2. The status of 'print to file' isn't shown/evaluated correctly.
     * 3. QPrintDialog::setEnabledOptions() is completely ignored
     */
    QPrintDialog dlg(&printer, this);
    dlg.setEnabledOptions(
	QPrintDialog::PrintDialogOptions(
	    QPrintDialog::PrintToFile    |
	    QPrintDialog::PrintSelection |
	    QPrintDialog::PrintPageRange
	)
    );
    dlg.setMinMax(1, maxPg);			// boundaries for page range
    dlg.setFromTo(1, maxPg);			// pre-select a   page range
    dlg.setPrintRange(QPrintDialog::AllPages);

    if (dlg.exec() != QDialog::Accepted) return;

    if (printer.outputFileName().endsWith(".pdf"))
    	printer.setOutputFormat(QPrinter::PdfFormat);

    int fromPg=0, toPg=0;

    switch (dlg.printRange()) {
    case QPrintDialog::AllPages:  fromPg = 1;           toPg = maxPg;     break;
    case QPrintDialog::PageRange: fromPg=dlg.fromPage();toPg=dlg.toPage();break;
    case QPrintDialog::Selection: fromPg = curPg;       toPg = curPg;     break;
    default: qWarning("Unknown QPrintDialog::printRange() enumerator");   break;
    }

    fromPg = qBound(1, fromPg, maxPg);		// [1..fromPg..maxPg]
    toPg   = qBound(1, toPg,   maxPg);		// [1..toPg..maxPg]

    doPrint(&printer, fromPg, toPg);
}

// *****************************************************************
// Qt 4.4.0 is the first Qt version to include a ready-made print
// preview dialog. Show how to use it ...
// *****************************************************************
void Demo::printPreview()			// menu File/Print Preview
{
    int curPg = getCurrentPage();
    if (curPg==0) return;

    int maxPg = getAllPages();
    if (maxPg==0) return;

    printer.setFullPage(false);
    printer.setColorMode(QPrinter::GrayScale);
    printer.setDocName("Schematic");
    printer.setCreator("NlviewQT");
    printer.setCopyCount(1);
    printer.setPageOrder(QPrinter::FirstPageFirst);
    printer.setFromTo(1, maxPg);
    printer.setPrintRange(QPrinter::AllPages);

    QPrintPreviewDialog ppd(&printer, this);
    connect(&ppd, &QPrintPreviewDialog::paintRequested,
	    this, &Demo::paintRequested);
    ppd.exec();
}

void Demo::paintRequested(QPrinter* pr)
{
    int fromPg = 1;
    int toPg   = getAllPages();
    if (toPg==0) return;

    doPrint(pr, fromPg, toPg);
}

void Demo::pageSetup() {
    QPageSetupDialog dlg(&printer, this);
    dlg.exec();
}

// **********************************************************************
// (7) deleteSelection - unload selected objects from Nlview
//     usually for Incremental Schematic navigation with the VDI-attached
//     TDB - but works well in general.
//
void Demo::deleteSelection()
{
    /**
     * In Ictrl more/less mode, we use smart "ictrl less <oid>" in a loop over
     * the selection ('less' accepts only 1 oid at a time).
     * For all other cases we use the generic "unload" command to remove
     * components/nets.
     */
    bool use_less = ictrlMoreLess && sane();
    bool ok;
    NlviewList selList = nlview->command(&ok, "selection");	OK(selList)
    const char* res    = nlview->command(&ok, "increment");	OK(res)
    int selList_cnt    = selList.split();

    for (int i=0; i<selList_cnt; i++) {
	if (use_less) {
	    res = nlview->command(&ok, "ictrl", "less", "-ignore", selList[i]);
	    if (!ok) { failure(res); break; /* error */ }
	} else {
	    NlviewList oid = selList[i];
	    if (oid.objType() == NlviewList::O_Error) continue;
	    int oid_cnt = oid.split();
	    if (oid_cnt >= 3) {
		nlview->args << "unload" << oid[0] << oid[1] << oid[2];
	    } else if (oid_cnt >= 2) {
		nlview->args << "unload" << oid[0] << oid[1];
	    } else {
		continue;	// ?!
	    }
	    if (oid.objType() == NlviewList::O_Inst) {
		nlview->args << "-rmhier";
	    }
	    res = nlview->command(&ok);
	    if (!ok) failure(res);
	}
    }
    res = nlview->command(&ok, "show");				OK(res)
}

bool Demo::sane()
{
    // (A) Check, if Ictrl is initialized and attached to a DataBase
    bool        ok;
    const char* r = nlview->command(&ok, "ictrl", "isinit_and_attached"); OKF(r)
    if (NlviewList::toInt(r) != 1) return false;

    // (B) Check, if Nlview's current module == ictrl's top-module
    NlviewList curmod, top, topv;
    curmod = nlview->command(&ok, "module");		      if (!ok) goto bad;
    top    = nlview->command(&ok, "ictrl","info","-top");     if (!ok) goto bad;
    topv   = nlview->command(&ok, "ictrl","info","-topview"); if (!ok) goto bad;

    if (curmod.split() != 2) return false;
    return  (qstrcmp(curmod[0], top)  == 0) &&
            (qstrcmp(curmod[1], topv) == 0);

  bad:
    return false;
}

#if defined(TDB) || defined(VVDI)
//***********************************************************************
//
// (8) The following code shows, how Nlview uses a VDI-attached DataBase
//     (actually the "test database" TDB) to load netlist data from the TDB
//     and how to do Incremental Schematic navigation.
//
/************************************************************************
 *
 *  Idea: We "attach" the incremental control machine (Ictrl) by providing
 *        a pointer to the DataBase root and to the VDI implementation
 *        (pointer to a the VDI access functions). --> see attach()
 *
 *        Then we initialize Ictrl.
 *        Depending on Ictrl usage we do:
 *          1.) more / less:
 *              Add one or more known instances from that DataBase into Nlview
 *              as "start-logic".
 *
 *          2.) loadmodule:
 *              Load a complete Module into Nlview (only one level of
 *              hierarchy). --> No incremental navigation in this mode.
 *
 *        In Mode 1.) the GUI could listen to interactive user-requests
 *        (typically a Mouse-Double-Click event) to expand the schematic
 *        on demand).
 *
 * For more information about Nlview's "ictrl"-subcommands, see
 * Nlview's API documentation file doc/nlviewIctrl.html
 *
 ************************************************************************
 */
bool Demo::attach(NlviewArgs* modview)
{
    char pointers[50];
    bool  	ok;
    const char* r;		// result of nlview command

    struct VdiAccessFunc* vdiimpl;
    struct VdiDB*	  vdiDB = TdbInit(&vdiimpl, withTransistors);
    *modview << "ALULOGIC" << "";

    Q_ASSERT(sizeof(uintPointer) == sizeof(void*));
    sprintf(pointers, PRINTF_PTR_FMT " " PRINTF_PTR_FMT,
	    (uintPointer)vdiimpl, (uintPointer)vdiDB);

    r = nlview->command(&ok, "ictrl", "attach", pointers);		OKF(r)
    std::cout << "Ictrl attach" << pointers << std::endl;

    // initialize ictrl and load modView
    //
    r = nlview->command(&ok, "module", "deleteall");			OKF(r)
    r = nlview->command(&ok, "bind", "-reset");				OKF(r)

    if (!attachLabel) {
		attachLabel = new QLabel(tr("attached: <b>") +  (*modview)[0] + "</b>");
		attachLabel->setContentsMargins(0,0,0,0);
		attachLabel->setFrameShape(QFrame::Panel);
		statusBar()->addPermanentWidget(attachLabel);
    }
    return true;
}

void Demo::attach_loadmodule()
{
    NlviewArgs modview;
    ictrlMoreLess = false;
    bool ok = attach(&modview);		if (!ok) return;

    //
    // ictrl loadmodule
    //
    // Pre-load our hierarchy stack with parent module names to be
    // able to navigate "up" from here...
    hierStack.clear();
    nlview->args << "ictrl" << "info" << "-upmods" << modview;
    NlviewList upmods = nlview->command(&ok);		      OK(upmods)
    int upmods_cnt = upmods.split();
    int i;
    for (i=upmods_cnt-1; i>=0; i--) {
	NlviewList parents = upmods[i];
	int        parents_cnt = parents.split();
	Q_ASSERT(parents_cnt == 4); Q_UNUSED(parents_cnt);
	// make "parents" a pair of {modname modview}
	NlviewArgs tmp; tmp << parents[0] << parents[1];
	parents = tmp;
	const char* par = parents;
	hierStack << par;
    }
    loadmodule(modview[0], modview[1]);
}

void Demo::attach_moreless()
{
    NlviewArgs modview;
    const char* r;		// result of nlview command
    ictrlMoreLess = true;
    bool ok = attach(&modview);		if (!ok) return;

    //
    // ictrl more / less
    //
    r = nlview->command(&ok, "bind", "stroke-nw-1", "-action", "");	   OK(r)

    // compute inst path-to-top from return value of: ictrl info -upmods
    nlview->args << "ictrl" << "info" << "-upmods" << modview;
    NlviewList upmods = nlview->command(&ok);			      OK(upmods)
    int upmods_cnt = upmods.split();

    QStringList iList;
    int   	i;
    for (i=upmods_cnt-1; i>=0; i--) {
	NlviewList parent = upmods[i];
	int        parent_cnt = parent.split();
	Q_ASSERT(parent_cnt == 4); Q_UNUSED(parent_cnt);
	iList << parent[2];
    }
    QByteArray ipath = iList.join(".").toAscii();

    r = nlview->command(&ok, "module", "new", modview[0], modview[1]);	   OK(r)
    r = nlview->command(&ok, "ictrl", "init", ipath.constData(), ".");	   OK(r)
    r = nlview->command(&ok, "ictrl", "addports");			   OK(r)

    if (qstrcmp(r, "0") == 0) {	// there are no ports at all...
	r = nlview->command(&ok, "ictrl", "addmodule", "-nonets");	   OK(r)
    }
    r = nlview->command(&ok, "ictrl", "addDone");			   OK(r)
    r = nlview->command(&ok, "show");				  	   OK(r)
    r = nlview->command(&ok, "zoom", "1");				   OK(r)
}

void Demo::detach()
{
    const char* r;		// result of nlview command
    bool 	ok;
    r = nlview->command(&ok, "ictrl", "detach");			OK(r)
    r = nlview->command(&ok, "ictrl", "uninit");			OK(r)
    r = nlview->command(&ok, "bind", "-reset");				OK(r)
    r = nlview->command(&ok, "property", "-reset");			OK(r)
    r = nlview->command(&ok, "ictrl", "property", "-reset");		OK(r)

    if (attachLabel) {
	statusBar()->removeWidget(attachLabel);
	attachLabel = NULL;
    }
}

void Demo::loadmodule(const char* mod, const char* view)
{
    // before loading from scratch we search Nlview's module cache...
    int i,mcnt;
    bool ok;
    bool loaded = false;
    int stacksize;
    NlviewList mlist = nlview->command(&ok, "module", "list");	      OK(mlist)

    mcnt = mlist.split();
    for (i=0; i<mcnt && !loaded; i++) {
	NlviewList modID = mlist[i];
	int argc = modID.split();
	Q_ASSERT(argc == 2); Q_UNUSED(argc);
	loaded = qstrcmp(mod,modID[0])==0 && qstrcmp(view,modID[1])==0;
    }

    const char* r;		// result of nlview command
    if (loaded) {
	r = nlview->command(&ok, "module", "open", mod, view);		OK(r)
	r = nlview->command(&ok, "ictrl", "init", "");			OK(r)
    } else {
	r = nlview->command(&ok, "module", "new", mod, view);		OK(r)
	r = nlview->command(&ok, "ictrl", "init", "");			OK(r)
	r = nlview->command(&ok, "ictrl", "loadmodule");		OK(r)
	r = nlview->command(&ok, "show");				OK(r)
    }

    // Nice: make the go-up mouse binding context sensitive
    stacksize = hierStack.count();
    if (stacksize) {
	NlviewList modv = hierStack.at(stacksize-1).toAscii().constData();
	modv.split();
	QString str = QString("up to %1").arg(modv[0]);
	nlview->args	<< "bind"       << "stroke-nw-1"
			<< "-action"    << "goup"
			<< "-echo_type" << "line"
			<< "-echo_text" << str.toAscii().constData();
	r = nlview->command(&ok);					OK(r)
    }  else {
	r = nlview->command(&ok, "bind", "stroke-nw-1", "-action", "");	OK(r)
    }
}

void Demo::hierarchyDown(const char* instname)
{
    if (!ictrlMoreLess && sane()) {
	// Query the VDI DataBase for the module instantiated by instname
	// (its downModule)
	bool ok;
	NlviewArgs instID;
	instID << "inst" << instname;
	nlview->args << "ictrl" << "info" << "-downmod" << instID;
	NlviewList modID = nlview->command(&ok);		      OK(modID)
	if (modID.split() < 2) return;	// we cannot descent into a primitive

	const char* r = nlview->command(&ok, "module");		      OK(r)
	hierStack << r;
	loadmodule(modID[0]/*modname*/, modID[1]/*viewname*/);
    }
}

void Demo::hierarchyUp()
{
    if (!ictrlMoreLess && sane()) {
	int count = hierStack.count();
	if (count == 0) return;	// reached the TOP

	// take the parent module (a pair) from stack
	QByteArray elem = hierStack.takeLast().toAscii();	// pop

	NlviewList modview(elem.constData());
	if (modview.split() != 2) return;  // found unexpected garbage in stack
	loadmodule(modview[0], modview[1]);
    }
}

void Demo::incrMore(Qt::KeyboardModifiers modifiers, int x, int y)
{
    if(!ictrlMoreLess) return;

    bool ok;
    bool CtrlPressed  = !!(modifiers & Qt::ControlModifier);
    bool ShiftPressed = !!(modifiers & Qt::ShiftModifier);
    NlviewList r;		// result of nlview command
    NlviewList obj;

    if (!sane()) return;

    // Ok, we're operating NlviewQT with Ictrl support...
    // Check what is beneath the mouse cursor in Nlview...
    nlview->args << "pick" << "-hier" << "-conn" << "*" << x << y;
    r = nlview->command(&ok);					OK(r)

    obj = r;
    if (obj.isEmpty()) {			// clicked on "nothing"...
	if (ShiftPressed) {			// ... Shift? load top module
	    nlview->args << "increment" << "-x" << x << "-y" << y;
	    r = nlview->command(&ok);				OK(r)
	    r = nlview->command(&ok, "ictrl", "addmodule");		OK(r)
	    r = nlview->command(&ok, "ictrl", "addDone");		OK(r)
	    goto show;
	}
	return;				// ...else: ignore
    }

    // ignore unknown/bad object ids as well as unsupported conn object_ids
    // (off-page connectors):
    if (obj.split() < 2 || obj.objType() == NlviewList::O_Error)
	return;

    nlview->args << "increment" << "-x" << x << "-y" << y;
    r = nlview->command(&ok);					OK(r)

    switch (obj.objType()) {

	case NlviewList::O_PortBus:
	case NlviewList::O_PinBus:
	case NlviewList::O_HierPinBus:
	{
	    r = nlview->command(&ok, "ictrl", "more", "-auto", obj);	OK(r)
	    break;
	}

	case NlviewList::O_Inst:
	{
	    if (ShiftPressed) {
		r = nlview->command(&ok, "ictrl", "addmodule", obj); 	OK(r)
		r = nlview->command(&ok, "ictrl", "addDone");		OK(r)
	    } else if (!CtrlPressed) {
		toggleInst(obj);
	    }
	    break;
	}

	default:
	{
	    r = nlview->command(&ok, "ictrl", "more", obj);		OK(r)
	    break;
	}
    }

    show:
	r = nlview->command(&ok, "show");				OK(r)
}

void Demo::toggleTrans() {
    withTransistors = !withTransistors;
}
void Demo::toggleOper() {
    stopAtOperators = !stopAtOperators;
}


/***********************************************************************
 * toggleInst
 *
 * Either toggle autohide or unfold/fold the given instance (depending on
 * the symbol shape).
 *
 * HIERBOX: toggle the autohide flag at the given instance.
 *	    Also set or clear the autohide property of Ictrl for
 *	    future loading of instances via "ictrl more" or "ictrl add".
 *
 * HIER*:   unfold or fold the given instance.
 */
void Demo::toggleInst(const NlviewList& hinst)
{
    const char* r;		// result of nlview command
    bool	ok;

    Q_ASSERT(qstrcmp(hinst[0],"inst") == 0);

    NlviewList flags = nlview->command(&ok, "flag", hinst);	OK(flags)

    bool hasAutohide = false;
    bool hasFold     = false;
    bool hasUnfold   = false;
    int  flags_cnt = flags.split();
    int  i;
    for(i=0; i<flags_cnt; i++) {
	if (qstrcmp(flags[i], "-autohide") == 0) hasAutohide = true; else
	if (qstrcmp(flags[i], "-fold")     == 0) hasFold     = true; else
	if (qstrcmp(flags[i], "-unfold")   == 0) hasUnfold   = true;
    }
    if (hasFold) {
	r = nlview->command(&ok,"flag", hinst, "-unfold");		OK(r)
    } else if (hasUnfold) {
	r = nlview->command(&ok,"flag", hinst, "-fold");		OK(r)
    } else if (hasAutohide) {
	r = nlview->command(&ok,"flag", hinst, "-unautohide");		OK(r)
	r = nlview->command(&ok,"ictrl","property","autohide","0");	OK(r)
    } else if (hinst.length() > 3) {
	/* No instance flags given: we check the symbol shape to see what's
	 * the best for the user: for HIERBOX, we switch to -autohide,
	 * for HIER* we switch to -unfold, for anything else, we return.
	 */
	NlviewList sym = nlview->command(&ok,
		"search", "-exact", "symbol", hinst[2], hinst[3]);	OK(sym)
	sym.split();
	sym = sym[0];
	sym.split();
	if (qstrcmp(sym[3], "HIERBOX") == 0) {
	    r = nlview->command(&ok,"flag", hinst, "-autohide");	OK(r)
	    r = nlview->command(&ok,"ictrl","property","autohide","1");	OK(r)
	} else if (qstrncmp(sym[3], "HIER", 4) == 0) {
	    r = nlview->command(&ok,"flag", hinst, "-unfold");		OK(r)
	}
    }
}

#else /* TDB || VVDI */
    // because of MOC, we cannot easily add "#ifdef TDB" to the tapp4.h
    // header file - and MOC also creates references to all SLOTS, so we
    // must add dummy implementations for all of them:
    //
    void Demo::attach_loadmodule()			{}
    void Demo::attach_moreless()			{}
    void Demo::detach()					{}
    void Demo::hierarchyDown(const char*)		{}
    void Demo::hierarchyUp()				{}
    void Demo::incrMore(Qt::KeyboardModifiers,int,int)	{}
    void Demo::toggleTrans()				{}
    void Demo::toggleOper()				{}
#endif /* TDB || VVDI */
//***********************************************************************
//  END of (8) - code that shows, how Nlview uses a VDI-attached DataBase
//***********************************************************************


//***********************************************************************
//
// (9) The following code shows, how to export Nlview's graphics by calling
//     functions from gdump.h, gskill.h, and gscene.h (they will use Nlview's
//     GEI (graphic export interface) to access Nlview's graphic data.
//
// *****************************************************************
//
bool Demo::getCurModuleName(QByteArray* result)
{
    bool ok;
    NlviewList curmod = nlview->command(&ok, "module");		OKF(curmod)

    if (curmod.split() != 2) return failure("unexp. module name");
    *result = curmod[0];
    return true;
}

bool Demo::gdump()				// menu File/Gdump...
{
    QByteArray curmod;
    if (!getCurModuleName(&curmod)) return false;

    // choose output file name with dialog
    //
    static QString lastDumpfileName(QDir(".").absoluteFilePath("file.out"));
    QString choice =
	QFileDialog::getSaveFileName( this,
				      "Save Nlview GEI dump",
				      lastDumpfileName/*bug 84783 fixed 4.1.0*/,
				      "Dump file (*.out);;all (*)" );

    // The save-file dialog is configured (by default) to ask for confirmation
    // if an existing file should be overwritten.

    if( choice.isNull() ) return false;	// cancel
    lastDumpfileName = choice;		// update last file name

    QByteArray filename = lastDumpfileName.toAscii();

    // initialize Nlview graphic export
    //
    if(!GexDumpStart(filename.constData(), 0)) return failure(GexDumpLastErr());

    bool	ok;
    uintPointer gei;
    const char* r = nlview->command(&ok, "begGraphicExp");		OKF(r)
    if (sscanf(r, PRINTF_PTR_FMT, &gei) != 1) {
	return failure("Could not parse pointer string from begGraphicExp");
    }
    const struct GeiAccessFunc* geiPtr = (const struct GeiAccessFunc*)gei;

    // dump the current Nlview module
    //
    if (!GexDumpModule(curmod.constData(), geiPtr, NULL)) {
	nlview->command(&ok, "endGraphicExp");
	return failure(GexDumpLastErr());
    }

    // finalize Nlview graphic export
    //
    r = nlview->command(&ok, "endGraphicExp");				OKF(r)
    if (!GexDumpEnd()) return failure(GexDumpLastErr());
    return true;
}

bool Demo::gskill()				// menu File/Gskill...
{
    QByteArray curmod;
    if (!getCurModuleName(&curmod)) return false;

    // choose output file name with dialog
    //
    static QString lastSkillfileName(QDir(".").absoluteFilePath("file.il"));
    QString choice =
	QFileDialog::getSaveFileName( this,
				      "Save a SKILL script using Nlview GEI",
				      lastSkillfileName/*bug84783 fixed 4.1.0*/,
				      "SKILL file (*.il);;all (*)" );

    if( choice.isNull() ) return false;	// cancel
    lastSkillfileName = choice;		// update last file name

    QByteArray filename = lastSkillfileName.toAscii();

    // initialize Nlview graphic export
    //
    // hard-wire some options here ...
    //
    const char*  libName    = "nlv_lib";
    const char*  libDir     = ".";
    double       scale      = 0.015625;
    int          metric     = 0/*false*/;
    int          overwrite  = 0/*false*/;
    int          spiceUnits = 0/*false*/;
    int          noSym      = 0/*false*/;
    int          multiFile  = 0/*false*/;
    int          netLabel   = 0/*false*/;
    int          noRangeLabel= 0/*false*/;
    const char*  prolog     = NULL;
    const char*  epilog     = NULL;
    const char*  instSuffix = NULL;

    if (!GexSkillStart(
		filename.constData(),
		libName,
		libDir,
		prolog,
		epilog,
		scale,
		metric,
		overwrite,
		spiceUnits,
		noSym,
		multiFile,
		netLabel,
		noRangeLabel,
		instSuffix))
    {
	return failure(GexSkillLastErr());
    }
    bool	ok;
    const char* r = nlview->command(&ok, "begGraphicExp");		OKF(r)
    uintPointer gei;
    if (sscanf(r, PRINTF_PTR_FMT, &gei) != 1) {
	return failure("Could not parse pointer string from begGraphicExp");
    }
    const struct GeiAccessFunc* geiPtr = (const struct GeiAccessFunc*)gei;

    // dump the current Nlview module as SKILL script
    //
    if (!GexSkillModule(curmod.constData(), geiPtr, NULL)) {
	nlview->command(&ok, "endGraphicExp");
	return failure(GexSkillLastErr());
    }

    // finalize Nlview graphic export
    //
    r = nlview->command(&ok, "endGraphicExp");				OKF(r)
    if (!GexSkillEnd()) return failure(GexSkillLastErr());
    return true;
}


class MyGraphicsView : public QGraphicsView {
  public:
    MyGraphicsView(QGraphicsScene* scene) : QGraphicsView(scene) {
	setTransformationAnchor(AnchorUnderMouse);
	setDragMode(ScrollHandDrag);
    };

  protected:
	virtual void wheelEvent(QWheelEvent* e) {
		int numSteps = e->delta() / (8*15);
		if (e->modifiers() & Qt::AltModifier) {
			rotate(numSteps > 0 ? 3:-3);
		} else if (e->modifiers() & Qt::ControlModifier) {
			const qreal z = numSteps > 0 ? 1.1 : 0.9;
			scale(z,z);
		} else {
			QScrollBar* sb = (e->modifiers() & Qt::ShiftModifier)
					 ? horizontalScrollBar() : verticalScrollBar();
			if (sb) sb->setValue(sb->value() - sb->singleStep()*numSteps);
		}
		e->accept();
    }
    virtual void keyPressEvent(QKeyEvent* e) {
		if (e->text() == "1")
			resetTransform();
		else if (e->text() == "f")
			fitInView(sceneRect(),Qt::KeepAspectRatio);
		else {
			QGraphicsView::keyPressEvent(e); return;
		}
		e->accept();
    }
};

bool Demo::gscene()				// menu File/Gscene...
{
    QByteArray curmod;
    if (!getCurModuleName(&curmod)) return false;

    int curpg = getCurrentPage();
    if (curpg == 0) return false;

    bool	ok;
    uintPointer gei;
    const char* r = nlview->command(&ok, "begGraphicExp");		OKF(r)
    if (sscanf(r, PRINTF_PTR_FMT, &gei) != 1) {
	return failure("Could not parse pointer string from begGraphicExp");
    }
    const struct GeiAccessFunc* geiPtr = (const struct GeiAccessFunc*)gei;

    // dump the current page of the current Nlview module
    //
    QGraphicsScene* scene = new QGraphicsScene(nlview);
    bool sceneOk = GexScene(geiPtr, scene, curpg);

    // finalize Nlview graphic export
    //
    r = nlview->command(&ok, "endGraphicExp");				OKF(r)
    if (!sceneOk) return failure(GexSceneLastErr().toAscii().constData());

    QMainWindow* gscene_mw = new QMainWindow(this);
    gscene_mw->resize(500,500);
    QString title("Gscene: QGraphicsScene - ");
    title.append(QString("page %1 of Nlview Module ").arg(curpg));
    title.append(curmod.constData());
    gscene_mw->setWindowTitle(title);
    gscene_mw->setAttribute(Qt::WA_DeleteOnClose);

    MyGraphicsView* view = new MyGraphicsView(scene);
    gscene_mw->setCentralWidget(view);
    gscene_mw->show();
    return true;
}
//***********************************************************************
//  END of (9) - code that shows, how to export graphics from Nlview
//***********************************************************************


bool Demo::doCustomAction()
{
	/*
	NLVhandler nlv(nlview);
	char* commandlist[] = {
			"module new module",
			"load symbol ABOX v BOX pin input1 IN pin input2 IN pin output1 OUT",
			//load symbol ABOX vn BOX pin A IN pin B IN pin C OUT
			"load inst ABOX1 ABOX v",
			"show",
			"load cgraphic sparta1 linkto {inst BOX2} text \"TX\n0xFF 0xAB\nRX\n0x00 0x00\" -ll 0 0 5 place left 20 20"
	};

	std::list<nlv::NLElement*> elements;

	nlv::Port in1("IN1", nlv::Direction::IN), in2("IN2", nlv::Direction::IN);
	nlv::Port out1("OUT1", nlv::Direction::OUT);
	elements.push_back(&in1);
	elements.push_back(&in2);
	elements.push_back(&out1);

	nlv::Pin a("A", nlv::Direction::IN), b("B", nlv::Direction::IN), c("C", nlv::Direction::OUT);
	nlv::Symbol abox("ABOX", "vn", nlv::SType::BOX, {a,b,c});
	elements.push_back(&abox);

	nlv::Instance box1 = abox.instantiate("BOX1");
	nlv::Instance box2 = abox.instantiate("BOX2");
	elements.push_back(&box1);
	elements.push_back(&box2);

	nlv::Connection l1("l1");
	l1.add(&in1);
	l1.add(box1.getPin(a));
	l1.add(box2.getPin(a));
	elements.push_back(&l1);

	nlv::Connection l2("l2", {box1.getPin(c), box2.getPin(c), &out1});
	elements.push_back(&l2);

	nlv::Connection l3("l3", {&in2, box1.getPin(b), box2.getPin(b)});
	elements.push_back(&l3);

	nlv.init();

	for (nlv::NLElement* element : elements)
	{
		nlv.add(*element);
	}

	nlv.show();
	*/
	if (!ncs.setupConnection("1333")) {
		std::cerr << "cant set up server" << std::endl;
		exit(-1);
	}
    //TODO: listening mit failhandler, dann cli-client senden lassen
	ncs.registerInput(std::bind(&Demo::consumeExternCommand, this, std::placeholders::_1));
	ncs.startListening();
	return true;
}

void Demo::consumeExternCommand(const char* cmd)
{
	//Debug
	std::cout << "received Command " << cmd << std::endl;
	bool r;
	const char* err = nlview->commandLine(&r, cmd);
	if(!r)
	{
		std::cerr << err << std::endl;
	}
}

static QPicture* createDemoQPicture()
{
    // for demo imagedsp.nlv: register (scalable) image "sinewave"
    QPainterPath path;
    path.moveTo(0, 0);
    path.cubicTo(20, -50, 30, -50, 50, 0);
    path.cubicTo(70,  50, 80,  50, 100, 0);

    QPicture* sinewave = new QPicture();
    QPainter p(sinewave);
    p.setPen(Qt::red);
    p.drawRect(0,0,100,100);

    p.setPen(Qt::darkBlue);
    p.setFont(QFont("Arial", 8));
    p.drawText(QRect(0,0,100,15), Qt::AlignCenter, "I'm a QPicture!");

    p.setPen(QPen(Qt::blue, 2, Qt::DashLine));
    p.translate(0,50);
    p.drawPath(path);
    p.scale(1,-1);	// mirror at X-axis and draw path again...
    p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    p.drawPath(path);

    p.end();
    return sinewave;
}


//***********************************************************************
// (10) processOption processes command line options from main's argc/argv
//
const char* Demo::processOption(int argc, char* argv[], int* extraArgCnt)
{
    const char* para = argv[0];
    const char* value;
    const char* result;
    bool  ok;

    if (qstrcmp(para,"-n") == 0 && argc > 1) {
	value = argv[1];
	*extraArgCnt = 1;
	result = nlview->command(&ok, "source", value);
	return ok ? NULL : result;
    }
    if (qstrcmp(para,"-c") == 0 && argc > 1) {
	value = argv[1];
	*extraArgCnt = 1;
	result = nlview->commandLine(&ok, value);
	if (!ok) return result;		/* error */
	if (result && result[0])
	    fprintf(stdout, "%s\n", result);   /* for autotests */
	return NULL;	/* ok */
    }
    if ((qstrcmp(para,"-image") == 0 ||
	 qstrcmp(para,"-image_ls") == 0) && argc > 2)
    {
	bool limitscale = qstrcmp(para,"-image_ls") == 0;
	*extraArgCnt = 2;
	if (qstrcmp(argv[2],"builtin") == 0) {
	    QPicture* pic = createDemoQPicture();
	    // this special image file name creates an internal QPicture
	    nlview->registerImage(QString(argv[1]), pic, limitscale);
	    return NULL;	/* ok */
	}
	QPixmap* pm = new QPixmap(argv[2]);
	if (!pm || pm->isNull()) {
	    fprintf(stderr, "Error: image %s: bad file / format: %s\n",
		argv[1], argv[2]);		/* for autotests */
	    return "error loading image file";
	}
	nlview->registerImage(QString(argv[1]), pm, limitscale);
	return NULL;	/* ok */
    }

#ifdef TDB
    if (qstrcmp(para,"-vdi") == 0) {
	attach_moreless();
	result = nlview->command(&ok, "ictrl", "check");
	return ok ? NULL : result;
    }
#endif
#ifdef VVDI
    if (qstrcmp(para,"-top") == 0 && argc > 1) {
	verilogTopModule = argv[1];
	*extraArgCnt = 1;
	return NULL;	/* ok */
    }
    if (qstrcmp(para,"-verilog") == 0 && argc > 1) {
	int  i;
	int  ok;
	*extraArgCnt = argc;
	using namespace Verific;
	Message::SetAllMessageType(VERIFIC_INFO, VERIFIC_IGNORE);
	for(i=1; i<argc; i++) {
	    ok = veri_file::Analyze(argv[i]);
	    if (!ok) return "Verific Analyze error";
	}
	if (verilogTopModule) {
	    ok = veri_file::Elaborate(verilogTopModule);
	} else {
	    ok = veri_file::ElaborateAll();
	}
	if (!ok) return "Verific Elaborate error";
	attach_moreless();
	return NULL;
    }
#endif
#ifdef GEX
    if (qstrcmp(para,"-gex") == 0) {
	bool ok = gscene();
	return ok ? NULL : "error in -gex";
    }
#endif
    static QString errmsg;
    QTextStream ts(&errmsg);
    ts << "unknown option '" << argv[0] << "', try -help";
    return errmsg.toAscii().constData();
}


// **********************************************************************
// **********************************************************************
// *****************    main function    ********************************
// **********************************************************************
// **********************************************************************

#if QT_VERSION >= 0x050000	// Qt5+
  #if defined(QT_STATIC)
    // If we compile/link against a static Qt library, we additionally have to:
    // (a) Create & compile the Qt code (below) to import the static
    //     QPA plugin 'xcb' (unix) or 'windows' (Windows)
    // (b) Link the main program against the static QPA plugin library.
    //     This may pull in a lot additional dependent libraries, too.
    // Note: (a)+(b) will happen automatically when using the 'qmake' tool.
    // See also: http://qt-project.org/doc/qt-5.1/qtcore/plugins-howto.html
    //
    #include <QtPlugin>
    #ifdef _WIN32
      Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
    #else
      Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
    #endif
  #endif // static Qt library
#endif // Qt5+

int main( int argc, char* argv[] )
{
    QApplication::setDesktopSettingsAware(false); // ignore ~/.qt/qtrc
    QApplication a( argc, argv );
    Q_ASSERT(&a == qApp);

    // scan command line arguments for -noqtrend option for Nlview constructor
    int  i;
    bool perfectfs = false;
    for (i=1; i<argc; i++) {
	if (qstrcmp(argv[i],"-perfectfs") == 0) perfectfs = true;
    }

    Demo demo(argv[0], perfectfs);
    // demo.resize( 600, 600 );
    demo.show();
    bool lic_ok = demo.license();	// enable Nlview by setting license key

    // examine further command line arguments (if any)
    for (i=1; i<argc; i++)
    {
	if (qstrcmp(argv[i],"-noqtrend") == 0) continue;// handled in first loop
	if (qstrcmp(argv[i],"-perfectfs") == 0)continue;// handled in first loop

	if (qstrcmp(argv[i],"-q") == 0) {
	    qApp->processEvents();
	    return lic_ok ? EXIT_SUCCESS : EXIT_FAILURE;
	}
	if (qstrcmp(argv[i],"-help") == 0) {
	    qWarning("Usage: %s <options>", argv[0]);
	    qWarning("options:");
#ifndef _WIN32
	    qWarning("  -noqtrend: use native X11 renderer");
#endif
	    qWarning("  -perfectfs:use Qt renderer with perfect font scaling");
	    qWarning("  -n <file>: load Nlview netlist from given file");
	    qWarning("  -c <cmd> : execute given Nlview API command");
	    qWarning("  -image <iname> <file>: register named image from file");
#ifdef TDB
	    qWarning("  -vdi     : perform consistency check for tdb");
#endif
#ifdef GEX
	    qWarning("  -gex     : export schematic to Qt's QGraphicsScene");
#endif
	    qWarning("  -help    : print this help text and quit");
	    qWarning("  -q       : quit immediately after having initialized");

	    return EXIT_SUCCESS;
	}

	int acnt = 0;
	const char* err = demo.processOption(argc-i, argv+i, &acnt);
	if (err) {
	    qWarning("%s", err);
	    return EXIT_FAILURE;
	}
	if (acnt) i += acnt;
    }

    demo.doCustomAction();


    a.setQuitOnLastWindowClosed (true);
    return a.exec();
}
