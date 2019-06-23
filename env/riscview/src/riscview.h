/*  tapp4.h 1.37 2018/01/02
  
    Copyright 2008-2018 by Concept Engineering GmbH.
    ===========================================================================
    This example source code can be freely used and modified completely or
    in parts by customers of Concept Engineering GmbH.
    It comes with no warranty of any kind and is provided "as-is".
    ===========================================================================
    Title:
        Qt Test Application for Nlview - for Qt4+
    ===========================================================================
*/
#pragma once

#include "wrapper.h"
#include "../../vp/src/platform/riscview/nlv/connector-server.hpp"

#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QtPrintSupport>


/*******************************************************************************
 * Extend the NlviewQT main class "NlvQWidget" by some QMouseEvent methods
 * to implement:
 *	(a) drag&drop		- by dragEnterEvent(),dropEvent(),mouse*Event()
 *	(b) tooltips 		- by event()
 *	(c) popup context menu	- by contextMenuEvent()
 *	(d) listen to hotkeys	- by keyPressEvent()
 *	(e) incremental schem	- by mouseDoubleClickEvent()
 *******************************************************************************
 */
class Nlview : public NlvQWidget
{
	Q_OBJECT
    public:
	Nlview(bool perfectfs, QWidget* parent = 0);
	~Nlview();

	void enableTooltips(bool);		/* switch tooltips on/off */

	virtual void dragEnterEvent(  QDragEnterEvent* );
	virtual void dropEvent(       QDropEvent* );
	virtual void mousePressEvent( QMouseEvent* );
	virtual void mouseMoveEvent(  QMouseEvent* );
	virtual bool event(		    QEvent* );
	virtual void contextMenuEvent(      QContextMenuEvent* );
	virtual void mouseDoubleClickEvent( QMouseEvent* );

	// define my default natural size 600x300
	virtual QSize sizeHint() const { return QSize(600, 300); };

    signals:
	void deleteSelection();
	void incrMore(Qt::KeyboardModifiers, int x, int y);

    public slots:
	void prevPage();
	void nextPage();

    private slots:
	void copy();
	void magnify();
	void fullfit();
	void fullscreen();
	void zoomSel();
	void zoom1();
	void zoomIn();
	void zoomOut();
	void regenerate();
	void optimize();
	void bindCancel();
	void selectionNotify(unsigned);
	void pageNotify(unsigned,int,int);
	void generateCode();

#ifdef CUSTOM_ELIDE
    protected:
	void getCustomElidedText(char*,const char*,int,int,
				 enum NlviewList::OType, const char*, bool);
#endif
    private:
	void createActions();
	bool		withTooltips;
	QPoint		dragStartPosition;
	QAction*	deleteAct;
	QAction*	magnifyAct;
	QAction*	copyAct;
	QAction*	fullfitAct;
	QAction*	fullscreenAct;
	QAction*	zoomSelAct;
	QAction*	zoom1Act;
	QAction*	zoomInAct;
	QAction*	zoomOutAct;
	QAction*	regenerateAct;
	QAction*	optimizeAct;
	QAction*	prevPageAct;
	QAction*	nextPageAct;
	QAction*	generateCodeAct;

};


/*******************************************************************************
 * Define a customized combobox that implements a input line with
 * history & navigation
 *******************************************************************************
 */
class MyComboBox : public QComboBox
{
	Q_OBJECT

    public:
	MyComboBox(QWidget* parent = 0);
	virtual void keyPressEvent(QKeyEvent*);
	
    signals:
	void ReturnPressed(const QString&);
};


/*******************************************************************************
 * Demo - The test application's MainWindow
 *******************************************************************************
 */
class Demo : public QMainWindow
{
	Q_OBJECT

    public:
	Demo(const char* appName, bool perfectfs);
	~Demo();

	const char* processOption(int argc, char* argv[], int*);
	bool license();			// check-out a license for Nlview
	bool unlicense();		// check-in  a license for Nlview
	bool doCustomAction();
	void consumeExternCommand(const char* cmd);

#if QT_VERSION >= 0x040400
	typedef QPlainTextEdit LogWindow;
#else
	typedef QTextEdit      LogWindow;
#endif

    private slots:
	void progressNotify( int cnt, double percentage, const char* c );
	void selectionNotify(unsigned);
	void pageNotify(unsigned,int,int);
	void messageOutput( const char* id, const char* txt );
	void bindCallback( const char*, int, int, int, int, int, const char*);

	bool evalCommand( const QString& );
	void openVerilog();
	void openNetList();
	void saveImage();
	void toggleTooltips();
	void addMinimapDock();
	void about();
	void helpStrokes();

	void print();
	void printPreview();
	void paintRequested(QPrinter*);	    // from QPrintPreviewDialog
	void pageSetup();

	void deleteSelection();

    private:
	void createMenus();
	QWidget* createCentral() const;

	bool executeCommandLine(const char* cmdline);
	void logthis( char severity, const char* ) const;
	bool failure(const char*);

	int  getAllPages();
	int  getCurrentPage();
	void doPrint(QPrinter*, int, int);
	bool sane();
	bool getCurModuleName(QByteArray* result);


    private:
	Nlview*		nlview;
	QString		appName;
	QComboBox*	inputLine;		/* input line for Nlview API */
	bool		perfectfs;		/* use perfect font scaling? */
	LogWindow*	logWindow;
	QPrinter	printer;
	bool		withTooltips;
	QMenu*		optionsMenu;
	QPushButton*	cancelButton;
	QAction*	prevPageAct;
	QAction*	nextPageAct;
	QProgressBar*	progressBar;
	QLabel*		attachLabel;
	NLVConnectorServer ncs;


    // **********************************************************
    // methods below deal with GEI-based graphics export
    // **********************************************************
    private slots:
	bool gdump();
	bool gskill();
	bool gscene();

    // **********************************************************
    // methods and data below deal with VDI-attached TDB
    // **********************************************************
    private slots:
	void attach_loadmodule();
	void attach_moreless();
	void detach();
	void hierarchyDown(const char*);
	void hierarchyUp();
	void incrMore(Qt::KeyboardModifiers, int x, int y);
	void toggleTrans();
	void toggleOper();
    private:
	bool attach(NlviewArgs*);
	void loadmodule(const char*, const char*);
	void toggleInst(const NlviewList&);

	bool withTransistors;
	bool stopAtOperators;
	bool ictrlMoreLess;
	QStringList hierStack;

	const char* verilogTopModule;	// only for VVDI command line -top
};
