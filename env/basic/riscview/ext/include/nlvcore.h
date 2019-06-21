/*  nlvcore.h 1.102 2018/09/11
  
    Copyright 2002-2017 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
  
    Title:
  	Nlview Core C-Interface
  
    Description:
  	This is the Interface between the Widget code and the
  	Nlview core algorithms.

	Wrapper (GUI)		Interface		Nlview (Core)

		-----------------NlvCore------------------->
		<----------------NlvWidget------------------
		<----------------NlvRend--------------------

	The struct definintions are:
  
  	NlvCore	-	represents the nlview core; this data struct is
  		 	private to the Nlview core.
  
  	NlvWidget -	defines a function vector to be implemented by
  			the Widget.  The Widget should extend this data
  			structure with private data like in this example:
  
  			    struct MyWidget {
  				struct NlvWidget func;
  				...my widget data...
  			    };
  
  		   	The NlvWidget pointer is used for two purposes,
  			(1) it defines the Widget callback functions to
  			the Nlview core and (2) passes the Widget's private
  			data (as first argument) to the callback functions.
  			The NlvCore can only handle one NlvWidget, the
  			NlvWidget pointer is passed to NlvCore_construct.
  			
  	NlvRend -	defines a function vector to be implemented by
  			the Renderer.  The Renderer should extend this
  			data structure with private data like in this example:
  
  			    struct MyRend {
  				struct NlvRend func;
  				... my renderer data ...
  			    };
  
  			Works analog to NlvWidget.
  
    ===========================================================================
*/

#ifndef nlvcore_h
#define nlvcore_h

#ifdef __cplusplus
extern "C" {
#endif

struct NlvCore;		/* private to the nlview core */
struct NlvRend;
struct NlvImageList;

/* ===========================================================================
 * NlvColor Service Functions
 * ===========================================================================
 */
struct NlvColor {		/* Renderer Color */
    unsigned char red;		/* value 0...255, 0=dark */
    unsigned char green;
    unsigned char blue;
    unsigned char transp;	/* transparency: 0=opaque, 255=transparent */
};
typedef struct NlvColor NlvColor;

int/*bool*/  NlvColor_init (NlvColor*, const char* rrggbb);
int/*bool*/  NlvColor_init2(NlvColor*, const char*, unsigned*, const char**);
void 	     NlvColor_print(NlvColor, char buf[12]);
int          NlvColor_cmp  (NlvColor, NlvColor);
unsigned int NlvColor_hash (NlvColor);

/* The conversion from RGB to grayscale is using the "luminosity" method, a
 * linear combination of R,G,B values that take human perception into account:
 */
#define NlvGrayValueOf(c) ((unsigned char)\
			   (0.2126*(c)->red+0.7152*(c)->green+0.0722*(c)->blue))

/* ===========================================================================
 * The WBox represents a rectangular area in WIDGET (int) coordinates.
 * ===========================================================================
 */
struct WBox {	/* WIDGET Bounding Box */
    int left;		/* X coordinate of the left   side */
    int top;		/* Y coordinate of the top    side */
    int right;		/* X coordinate of the right  side */
    int bot;		/* Y coordinate of the bottom side */
};
typedef struct WBox WBox;

enum NlvCoreObj {
	NlvCoreObj_Error      =  0,
	NlvCoreObj_Port       =  1,
	NlvCoreObj_PortBus    =  2,
	NlvCoreObj_Pin        =  3,
	NlvCoreObj_PinBus     =  4,
	NlvCoreObj_Segm       =  5,
	NlvCoreObj_Segm1io    =  6,
	NlvCoreObj_Segm0io    =  7,
	NlvCoreObj_Inst       =  8,
	NlvCoreObj_Net        =  9,
	NlvCoreObj_NetVector  = 10,
	NlvCoreObj_NetBundle  = 11,
	NlvCoreObj_Symbol     = 12,
	NlvCoreObj_Attr       = 13,
	NlvCoreObj_Text       = 14,
	NlvCoreObj_Image      = 15,
	NlvCoreObj_Button     = 16,
	NlvCoreObj_HierPin    = 17,
	NlvCoreObj_HierPinBus = 18,
	NlvCoreObj_PageSubNet = 19,
	NlvCoreObj_CGraphic   = 20,
	NlvCoreObj_NetWire    = 21,
	NlvCoreObj_Last       = 22
};
/* ===========================================================================
 * NlvWidget Callback Definition - functions to be implemented by the Widget.
 * =============================
 *
 * The struct NlvWidget defines the functions to be implemented by the
 * Widget.  The NlvWidget data struct is also passed as a first argument to
 * all functions - the assumption is that private data follows the NlvWidget
 * struct - i.e. the NlvWidget struct is extended by the widget implementation.
 *
 * The NlvWidget pointer is specified to the Nlview core as argument to
 * the NlvCore_construct function.
 *
 * All coordinates passed in these "member" functions are WIDGET coordinates
 * Exception: start_pan, pan_to, stop_pan and bindcallback take WINDOW coords.
 *
 * The Widget implementation can map WIDGET coords to WINDOW coords
 * by subtracting the current xOrigin/yOrigin values (see NlvRend.set_origin).
 * ===========================================================================
 */
#define THS struct NlvWidget
struct NlvWidget {
    void (*guiVersion)         (THS*, char buf[64]);

    /* gui messages to Widget, all coords in WIDGET space (zoomed db coords) */
    void (*define_scrollregion)(THS*, WBox);
    void (*undefine_scrollregion)(THS*);
    void (*damageAll)          (THS*);
    void (*damage)             (THS*, WBox);
    void (*resetAll)           (THS*);        /*damageAll+update scrollbars*/
    void (*centerAt)           (THS*, int x, int y);     /*damageAll+scroll*/
    void (*moveCursorTo)       (THS*, int x, int y);
    void (*own_selection)      (THS*);

    /* notify callback functions (synchronously) */
    void (*page_notify)        (THS*, unsigned details,int,int);
    void (*selection_notify)   (THS*, unsigned len);
    void (*hierarchy_godown)   (THS*, const char* instname);
    void (*hierarchy_goup)     (THS*);
    void (*bindcallback)       (THS*, const char*, int dX,int dY,int uX,int uY,
				      int wheelDelta, const char* oid);

    /* notify callback functions (asynchronously) */
    void (*message_output)     (THS*, const char* id, const char* txt);
    void (*progress_notify)    (THS*, int cnt, float perc, const char* msg);
    void (*property_notify)    (THS*, const char* name, const char* value);
    void (*wait_in_mainloop)   (THS*, int ms);		/* may be NULL */
    void (*get_elided_text)    (THS*, char* result, const char* text,
				      int len, int maxlen,
				      int/*enum NlvCoreObj*/otype,
				      const char* aname,
				      int/*bool*/);    /* may be NULL */

    void (*busy_cursor)        (THS*);
    void (*std_cursor)         (THS*);

    /* called from mouse handler functions (X,Y) are WINDOW coords,
     *					 but rect is WIDGET coords.
     */
    void (*start_pan)          (THS*, int X, int Y);
    void (*pan_to)             (THS*, int X, int Y);
    void (*stop_pan)           (THS*, int X, int Y);
    void (*invalidate)         (THS*, WBox rect);

    /* printer handling if called from Core (Windows only) - all may be NULL */
    struct NlvRend* (*printer_construct)(
	    void* hdc,
	    char errmsg[256],
	    char colormode,		/* 'M', 'C', 'I', '2' */
	    const char* docname,
	    const char* sheetname,
	    const char* binname,
	    const char* font,
	    const char** prnfile,
	    const char** devicename,
	    int/*bool*/ dialog,
	    int* width, int* height,
	    const struct NlvImageList* imageList,
	    const struct NlvRend*      imageFactory);
    int/*bool*/ (*printer_startPage)(struct NlvRend*);
    int/*bool*/ (*printer_endPage)  (struct NlvRend*);
    int/*bool*/ (*printer_destruct) (struct NlvRend*);

    /* stub'ed version of native image registering if called from Core
     * ("print" command, Windows+SVG only) - all may be NULL
     */
    int/*bool*/ (*imageAddFromFile)(
	    struct NlvImageList**,
	    const char* iname,
	    const char* fname,
	    int/*bool*/ limitscale,
	    char errmsg[256]);
    void        (*imageListFree)(struct NlvImageList**);
};
#undef THS

/* or-ed flags for the details argument of callback NlvWidget.page_notify.
 * If the flags change --> update user doc!
 */
enum NlvPageNotifyDetails {
    NlvPageNotifyDPage		= 0x0001,	/* Current Page switched */
    NlvPageNotifyDPageCnt	= 0x0002,	/* Page Count changed */
    NlvPageNotifyDModule	= 0x0004,	/* Current Module switched */
    NlvPageNotifyDCleared	= 0x0008,	/* Schematic cleared */
    NlvPageNotifyDLayout	= 0x0010,	/* Schematic Layout modified */
    NlvPageNotifyDMouse		= 0x0020,	/* Source is a Mouse Event */
    NlvPageNotifyDMagnifyClose	= 0x0040,	/* Magnify Window closed */
    NlvPageNotifyDMagnifyOpen	= 0x0080	/* Magnify Window (re)opened */
};




/* ===========================================================================
 * NlvRend Callback Definition - functions to be implemented by the Renderer.
 * ===========================
 *
 * The struct NlvRend defines the functions to be implemented by the
 * Renderer.  The NlvRend data struct is also passed as a first argument to
 * all functions - the assumption is that private data follows the NlvRend
 * struct - i.e. the NlvRend struct is extended by the Renderer implementation.
 *
 * The NlvRend pointer is specified to the Nlview core as an argument to
 * both NlvCore_construct and NlvCore_print functions.
 *
 * The "set_origin" is expected to be called from the Widget just
 * before the Widget calls NlvCore_draw; all the other NlvRend functions
 * are expected to be called from inside NlvCore_draw (except get_text_width
 * and get_text_extend that may additionally be called from NlvCore_command
 * e.g. by "show" or "regenerate" commands).
 *
 * obj_push and obj_pop are optional (may be NULL). They provide details
 * about the type of object (enum NlvCoreObj) that is currently drawn.
 * The name and pname's memory is persistent until the end of the draw cycle.
 * Their acnt argument specifies the length of the attrs string-list.
 * Their attrs argument specifies an attribute list in the format:
 *    "name(fmt)=value" -- whereas the sub-string (fmt) is optional.
 * ===========================================================================
 */
enum NlvRendLineStyle {
    NlvRendLS_SOLID       = 1,	/* ___________________ */
    NlvRendLS_DASHED      = 2,	/* _______   _______   7:3 */
    NlvRendLS_DASHED2     = 3,	/* _______  _  _______ 7:2:1:2 */
    NlvRendLS_DASHED50    = 4,	/* _____     _____     5:5 */
    NlvRendLS_DOTTED      = 5	/* _     _     _     _ 1:5 */
};
enum NlvRendFillStyle {
    NlvRendFS_TRANSPARENT = 0,
    NlvRendFS_SOLID       = 1,
    NlvRendFS_DIAGCROSS   = 2,	/*  XX  */
    NlvRendFS_CROSS       = 3,	/*  ++  */
    NlvRendFS_BDIAGONAL   = 4,	/*  //  */
    NlvRendFS_FDIAGONAL   = 5,	/*  \\  */
    NlvRendFS_VERTICAL    = 6,	/*  ||  */
    NlvRendFS_HORIZONTAL  = 7,	/*  ==  */
    NlvRendFS_SOLID2      = 8,	/* fill with current (last) FOREGROUND color */

    NlvRendFS_PATTERNS    = 0xf,/* mask all of the above patterns    */
    NlvRendFS_LGRADIENT   = 0x10/* a linear gradient fill "pattern"  */
};
enum NlvRendJustify {
    NlvJ_UPPERLEFT = 0, NlvJ_UPPERCENTER = 1, NlvJ_UPPERRIGHT = 2,
    NlvJ_CENTERLEFT= 3, NlvJ_CENTERCENTER= 4, NlvJ_CENTERRIGHT= 5,
    NlvJ_LOWERLEFT = 6, NlvJ_LOWERCENTER = 7, NlvJ_LOWERRIGHT = 8,
    NlvJ_JUSTMASK = 0xf, NlvJ_VERTICAL = 0x80
};
enum NlvRendOrient {
    NlvRendO_FnegX   = 0x01,      /* Negate X (-x,y) */
    NlvRendO_FnegY   = 0x02,      /* Negate Y (x,-y) */
    NlvRendO_FswapXY = 0x04,      /* Swap x/y after mirroring (y,x) */

    NlvRendO_R0    = 0,
    NlvRendO_MY    = 1,
    NlvRendO_MX    = 2,
    NlvRendO_R180  = 3,
    NlvRendO_MYR90 = 4,
    NlvRendO_R90   = 5,
    NlvRendO_R270  = 6,
    NlvRendO_MXR90 = 7
};
/* ===========================================================================
 * NlvRendLineStyle Service Functions
 * ===========================================================================
 */
int/*bool*/ NlvRendLineStyle_init (enum NlvRendLineStyle*, const char* name);
const char* NlvRendLineStyle_print(enum NlvRendLineStyle);


typedef struct NlvPaint NlvPaint;

#define THS struct NlvRend
struct NlvRend {
    /**
     * Misc functions
     */
    void (*set_origin)(THS*, int xOrigin, int yOrigin, int rotate);
    void (*set_path)  (THS*,int width,  NlvColor, enum NlvRendLineStyle, float);
    void (*set_font)  (THS*, float size, NlvColor);
    void (*obj_push)  (THS*, int/*bool*/ isMagnify, enum NlvCoreObj,
			     const char* name, const char*,
			     int acnt, const char** attrs);
    void (*obj_pop)   (THS*);

    /**
     * Draw functions for unfilled outline graphics w/settings from set_path
     * (draw_arc is optional and may be NULL).
     */
    void (*draw_line)     (THS*, int x1, int y1, int x2, int y2);
    void (*draw_rectangle)(THS*, WBox);
    void (*draw_poly)     (THS*, int n, const int* x, const int* y);
    void (*draw_circle)   (THS*, int x, int y, int diam);
    void (*draw_arc)      (THS*, double, double, double, double, double);

    /**
     * Draw functions for filled shapes in given color & style, w/o outline
     * (if NlvPaint.color==NULL: use foreground color from last set_path call).
     */
    void (*draw_filled_rectangle)(THS*, WBox, const NlvPaint*);
    void (*draw_filled_poly)(THS*, int n,const int*,const int*,const NlvPaint*);
    void (*draw_filled_circle)(THS*, int x, int y, int diam,   const NlvPaint*);

    /**
     * Draw function for images, identified by name.  Return false if the
     * given image name is not registerted to the GUI.  The WBox defines
     * the final bbox after orientation is applied.
     */
    int/*bool*/ (*draw_image)(THS*, const char*, enum NlvRendOrient, WBox);

    /**
     * Text handling functions
     */
    void (*draw_text)      (THS*, const char* txt, int len, float x, float y,
			    enum NlvRendJustify);
    /**
     * Return font metrics:
     * get_text_width() returns string width
     *			(like get_text_extend's r-l or b-t)
     *
     * get_text_extend() returns string's WBox rounded to pixels
     *
     * get_text_linefeed() returns linefeed factor common for all
     *			   font sizes: the caller will multiply with fontsize
     *			   to get the line-to-line distance.
     */
    int  (*get_text_width) (const THS*, const char* txt, float size);
    void (*get_text_extend)(const THS*, float size, const char* txt, int len,
			    enum NlvRendJustify, WBox*, int/*bool*/ linear);
    float (*get_text_linefeed)(const THS*);

    /**
     * Optional: function to get the sampled data (pixels) of named images.
     * Return the pixels (samples) in a Malloc'd array of NlvColor
     * entries. The ownership of this memory block will be transferred to the
     * caller and will be Free'd at any time. The entries in this array are
     * strictly organized representing the RGBA-values of pixels visited
     * from left to right and from top to bottom. The size (length) of the array
     * must be exactly width x height. Returns NULL if no image info available.
     *
     * Warning: this function will be called outside of _init / _finit cycle.
     *
     * Arguments:
     *	name       IN:	image name (from imagedsp)
     *	width      OUT:	must be set to the native image width  in pixels
     *	height     OUT:	must be set to the native image height in pixels
     *	limitscale OUT:	corresponds to the option set when registering
     *			the image in the GUI and is a hint for the caller
     *			to not stretch/scale this image beyond its native
     *			resolution when rendering it on its output device.
     */
    NlvColor* (*get_image)(const THS*, const char* name, unsigned* width,
				  unsigned* height, int/*bool*/* limitscale);

    /**
     * The max font-size supported by this Renderer, initialized in the
     * Rend constructor, e.g. NlvRX11_construct() or any printer_construct()
     * 0 means: no limit.
     */
    float maxfontsize;
};
#undef THS



/* ===========================================================================
 * NlvPaint
 *
 *				fillstyle	fg		bg	coords
 *
 * 1. solid background:		SOLID		color		-	-
 * 2. solid background:		SOLID2		-		-	-
 * 3. pattern background:	<pattern>	fgcolor		bgcolor	-
 * 4. gradient background:	LGRADIENT	start		stop	location
 *
 * for 2: use the color set by last call to NlvRend.set_path()
 * for 3: <pattern> is one of
 *		DIAGCROSS, CROSS, BDIAGONAL, FDIAGONAL, VERTICAL, HORIZONTAL
 * 	  if bgcolor.transp == 0xff, then draw a pattern on transparent
 *	  background (even on machines that don't support alpha blending).
 * for 4: draw a linear gradient from renderer coords: startx/y to stopx/y
 *	  (fillstyle & PATTERNS) is ignored
 * ===========================================================================
 */
struct NlvPaint {               /* fill-styles: solid, pattern or gradient */
    enum NlvRendFillStyle fillstyle;
    NlvColor		  fg, bg;
    char/*bool*/          nomono;	/* don't paint this area on monochrome
					 * printer.
					 */
    char/*bool*/          noalpha;	/* don't paint this area on screens
					 * without support for alpha-blending.
					 */
    /* These values are only valid if fillstyle==NlvRendFS_LGRADIENT
     * (linear gradient) - initialized by ncore.h:NlvPaint_set_lgradient
     */
    int                   startx, starty;	/* location of start color */
    int                   stopx,  stopy;	/* location of stop  color */
};

void  NlvPaint_init_solid(NlvPaint*, const NlvColor* fill);


/* ============================================================================
 * Service functions for NlvRend implementations:
 *   NlvClipLine - move, clip     and rotate a line
 *   NlvTrimPoly - move, cut/trim and rotate a polygon/polyline
 *   NlvRend_initNoops - initialize NlvRend with noop functions
 * ============================================================================
 */
int/*bool*/ NlvClipLine(int* x0, int* y0, int* x1, int* y1, 
			int minCoord, int maxCoord,
			int xOrigin, int yOrigin, int/*bool*/ rotate);

typedef void (*NlvAddPointFunc)(void**,int, int);
int NlvTrimPoly(const int* xt, const int* yt, unsigned len, 
		int/*bool*/autoclose,
		void* buf, unsigned maxlen, NlvAddPointFunc, 
		int minCoord, int maxCoord,
		int xOrigin, int yOrigin, int/*bool*/ rotate);

void NlvRend_initNoops(struct NlvRend*);




/* ===========================================================================
 * NlvCore Major Functions	(-) non-reentrant (protected by a enter/leave)
 *				    call Command{Enter,Leave}
 *
 *				(R) reentrant functions,
 *				    call Command{Enter,Leave}Recursive
 *
 *				(r) reentrant functions,
 *				    no enter/leave (no Log installed, etc)
 * ===========================================================================
 *   NlvCore_construct		(non-reentrant unprotected, Log installed)
 *   NlvCore_construct_dummy	(similar to NlvCore_construct)
 *   NlvCore_destruct		(non-reentrant unprotected)
 *   NlvCore_remove_mouse_bindings
 *   NlvCore_draw		-
 *   NlvCore_lost_selection	-
 *   NlvCore_stroke_pending	R
 *   NlvCore_draw_stroke	-
 *   NlvCore_print		-
 *   NlvCore_command		-
 *   NlvCore_command_line	-
 *   NlvCore_progress		R
 *   NlvCore_setProperty	R
 *   NlvCore_visible_area	R
 *   NlvCore_zoomlock		R
 *   NlvCore_swap		-
 *
 * ===========================================================================
 */
#define THS struct NlvCore
struct NlvCore* NlvCore_construct(struct NlvWidget*, struct NlvRend*);
struct NlvCore* NlvCore_construct_dummy(struct NlvWidget*, struct NlvRend*,
					const struct NlvCore*);
int/*bool*/     NlvCore_destruct(THS*);
void		NlvCore_remove_mouse_bindings(THS*);

int/*bool*/ NlvCore_draw          (THS*, WBox draw_window, int/*bool*/);
int/*bool*/ NlvCore_lost_selection(THS*);
int/*bool*/ NlvCore_stroke_pending(THS*);
int/*bool*/ NlvCore_draw_stroke   (THS*);

enum NlvPrintScale {
    NlvPrint_FULL, NlvPrint_FULLFIT, NlvPrint_VISIBLE, NlvPrint_VIEW,
    NlvPrint_ONE, NlvPrint_MINIMAP, NlvPrint_SGEN
};
typedef enum NlvPrintScale NlvPrintScale;

enum NlvPrintOrient {
    NlvPrint_AUTO, NlvPrint_LANDSCAPE, NlvPrint_PORTRAIT,
    NlvPrint_ROTATE, NlvPrint_NOTROTATE
};
typedef enum NlvPrintOrient NlvPrintOrient;

int/*bool*/ NlvCore_print(THS*, struct NlvRend* printer, int pageno,
		    enum NlvPrintScale, enum NlvPrintOrient,
		    int prL, int prT, int prW, int prH,	/* printer sheet */
		    int/*bool*/ highlight,int/*bool*/ fillbg,
		    int* xoff, int* yoff, double* zoom,
		    int/*bool*/* rotate, int scrollr[4]/*l,t,w,h*/,
		    const char** result);

int/*bool*/ NlvCore_command      (THS*, int ac, const char** av, const char*,
					const char** result);
int/*bool*/ NlvCore_command_line (THS*, const char* cmd, const char** result);


int	    NlvCore_progress     (THS*, int/*bool*/ interrupt, float* perc);
int/*bool*/ NlvCore_setProperty  (THS*,char errmsg[24],const char*,const char*);
int/*bool*/ NlvCore_visible_area (THS*, int,int, int,int,int,int, int coord[4]);
void        NlvCore_zoomlock 	 (THS*, int);
int/*bool*/ NlvCore_swap         (THS*, THS*, int flags, const char** result);




/* ===========================================================================
 * NlvCore Mouse Handler	(-) non-reentrant (protected by a enter/leave)
 *				    All coordinates passed to the
 *				    NlvCore_mouse_* functions are WINDOW coords.
 * ===========================================================================
 *
 *   NlvCore_mouse_press	-
 *   NlvCore_mouse_release	-
 *   NlvCore_mouse_dblclick	-
 *   NlvCore_mouse_move		-
 *   NlvCore_mouse_wheel	-
 *	The mouse wheel delta is the amount of degree the mouse wheel was
 *	turned measured in 1/8th of a degree. Typical low-resolution mouse
 *	wheels create wheel events of 15 degress per notch - resulting in
 *	a wheel delta value of 120 (if scrolled down/south/towards the user)
 *	and a wheel delta of  -120 (if scrolled up/north/away from the user),
 *	respectively.
 *	Higher resolution mouse wheels can create mouse wheel events more
 *	frequently with values below 120 each.
 * ===========================================================================
 */
#define NlvCoreModifier_SHIFT (1<<0)	/* using values from X.h:ShiftMask... */
#define NlvCoreModifier_CTRL  (1<<2)
#define NlvCoreModifier_ALT   (1<<3)
typedef unsigned int NlvCoreModifiers;	/* mask of above keyboard modifiers */

enum NlvCoreButton {
    NlvCoreButton_NO=0, NlvCoreButton_1, NlvCoreButton_2, NlvCoreButton_3
};
typedef enum NlvCoreButton   NlvCoreButton;

int/*bool*/ NlvCore_mouse_press(THS*, NlvCoreButton,        NlvCoreModifiers,
				      int X, int Y);
void NlvCore_mouse_release     (THS*, NlvCoreButton,        NlvCoreModifiers,
				      int X, int Y);
void NlvCore_mouse_dblclick    (THS*, NlvCoreButton,        NlvCoreModifiers,
				      int X, int Y);
void NlvCore_mouse_move2       (THS*, int/*bool*/anybutton, NlvCoreModifiers,
				      int X, int Y);
void NlvCore_mouse_wheel       (THS*, int/*bool*/anybutton, NlvCoreModifiers,
				      int X, int Y, int wdelta);
#undef THS


/* ===========================================================================
 * NlvCore Tcl-Lists Utility Functions 	(R) reentrant functions
 * ===========================================================================
 *   NlvCore_splitList		R
 *   NlvCore_splitFree		R
 *   NlvCore_mergeList		R
 *   NlvCore_mergeFree		R
 *
 *   NlvCore_lookupObjType	R	- for Tcl-lists that are Object IDs
 * ===========================================================================
 */
const char** NlvCore_splitList(const char* str, int *argc);
void         NlvCore_splitFree(const char** argv);
char*        NlvCore_mergeList(int argc, const char** argv);
void         NlvCore_mergeFree(char* str);

enum NlvCoreObj NlvCore_lookupObjType(const char* argv0);

/* ===========================================================================
 * NlvCore Utility Functions
 * ===========================================================================
 *   NlvCore_applyPrinterColormode
 *   NlvCore_initNoops	- initialize NlvWidget with noop functions
 *
 *   NlvCore_version	- return Nlview version info into given string buffer
 *   NlvCore_tclcompat	- global flag: sets tclcompat flag
 *   NlvCore_threadsafe	- global flag: make a non-TLS build thread safe
 * ===========================================================================
 */
void NlvCore_applyPrinterColormode(char colormode, NlvColor*, float rgb[3]);
void NlvCore_initNoops(struct NlvWidget* widget);

void		NlvCore_version(char buf[100]);  /* implemented in command.c */
int/*bool*/	NlvCore_tclcompat(int/*bool*/on);   /* set/clear global flag */
int/*bool*/	NlvCore_threadsafe(int/*bool*/on);  /* set/clear global flag */


#ifdef __cplusplus
}
#endif

#endif /* nlvcore_h */
