/*  gei.h 1.61 2018/09/11

    Copyright 2003-2013 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
  	Graphic Export Interface
    ===========================================================================
*/

#ifndef gei_h
#define gei_h

enum { GeiVersion = 35 };	/* must be incremented for each modification */
enum { GeiMagic = 0x250bc853 };

/* ===========================================================================
 * The "Graphic Export Interface" is ...
 *
 * ===================================================================
 * Iterator Nesting Tree
 * =====================
 *
 *  ITERATOR		OBJ		INFO		INFOSTRUCT
 *
 *  symIter	 	Iobj-->sym	symInfo		GeiSymInfo
 *    spinIter		Iobj-->sympin	IspinInfo	GeiSymPinInfo
 *      spattrIter 			IsattrInfo	GeiSymAttrInfo
 *    sattrIter 			IsattrInfo	GeiSymAttrInfo
 *    spropIter 			IspropInfo	GeiAttrInfo
 *    shapeIter				IshapeInfo	GeiPointInfo
 *
 *  pageIter		Iobj-->page	pageInfo	GeiPageInfo
 *
 *    instIter		Iobj-->inst	instInfo	GeiInstInfo
 *      pinIter		Iobj-->pin	IpinInfo	GeiPinInfo
 *      				IjoinInfo	GeiJoinInfo
 *        pattrIter    			IattrInfo       GeiAttrInfo
 *        pattrDspIter                  IattrDspInfo  GeiSymAttrInfo+GeiAttrInfo
 *      attrIter    			IattrInfo	GeiAttrInfo
 *      attrDspIter                     IattrDspInfo  GeiSymAttrInfo+GeiAttrInfo
 *
 *    netIter		Iobj-->net	netInfo		GeiNetInfo
 *      nattrIter    			IattrInfo	GeiAttrInfo
 *      nattrDspIter                    IattrDspInfo  GeiSymAttrInfo+GeiAttrInfo
 *      connIter  			IconnInfo	GeiConnInfo
 *      wireIter  			IwireInfo	GeiPointInfo
 *
 *    nbunIter		Iobj-->nbun	netInfo		GeiNetInfo
 *      nattrIter    			IattrInfo	GeiAttrInfo
 *      nattrDspIter                    IattrDspInfo  GeiSymAttrInfo+GeiAttrInfo
 *      bconnIter  			IconnInfo	GeiConnInfo
 *      wireIter  			IwireInfo	GeiPointInfo
 *      ripIter				IripInfo	GeiRipInfo
 *        subWIter			IsubWInfo	GeiPointInfo
 *
 *    overIter				IoverInfo	GeiOverInfo
 *      ovWIter  			IovWInfo	GeiPointInfo
 *      ovRIter				IovRInfo	GeiRipInfo
 *
 * The symIter loops over all available symbol shapes, but if
 * symIter's flags & 1, then the loop includes stub symbols (for
 * INPORT/OUTPUT/PAGEIN/POWER/etc).  It loops only over "master"
 * symbols, that means GeiSymInfo.master is always NULL).
 * The "master" symbols are those symbols that have been loaded to
 * Nlview. Rotated, mirrored or "privatized" symbols are no "master"
 * symbols (see instIter below).
 *
 * The instIter loops over all primitive-level instances and over
 * all HIER frames (the HIER frames have GeiSymInfo.type==GeiSymTHier).
 * The loop includes "stub" instances (INPORT/OUTPORT/PAGEIN/POWER/etc).
 * The GeiInstInfo.sym may point to a rotated/mirrored symbol or to a
 * "privatized" symbol (a symbol with a modified shape but the same
 * interface and function). In those cases, GeiSymInfo.master points
 * to the original symbol shape (that can be visited with symIter).
 * and GeiSymInfo.xmove and GeiSymInfo.ymove provide a placement offset
 * to the master; but those xmove/ymove are not needed to
 * place the already rotated/privatized symbol (actually, the GEI user
 * has the option (A) to place GeiInstInfo.sym or (B) to place and
 * rotate GeiInstInfo.sym.master by applying xmove/ymove additionally).
 *
 *   Examples, how symbols' bboxLeft and bboxTop translate to the
 *   page coordinates, assuming "iinfo" is a GeiInstInfo filled by instInfo().
 *   Option (A):
 *  	X = iinfo.sym.bboxLeft + iinfo.x;
 *  	Y = iinfo.sym.bboxTop  + iinfo.y;
 *   Option (B):
 *  	X = rotate(iinfo.sym.master.bboxLeft) + iinfo.sym.xmove + iinfo.x;
 *  	Y = rotate(iinfo.sym.master.bboxTop)  + iinfo.sym.ymove + iinfo.y;
 *  	
 *   The "master" and its family members have identical GeiSymInfo.type,
 *   GeiSymInfo.name and vname (identical pointers).
 *
 * The pinIter and spinIter loop over all pins (pinBus if busWidth > 0); for
 * HIER frames, it loops over both the outside pin/pinBuses and then
 * the inside pin/pinBuses (with same names).  At connector instances
 * (GeiSymTINPORT, GeiSymTPAGEIN, etc) the pinIter returns one pin or
 * pinBus of the appropriate width, each with name==NULL; but the spinIter
 * always returns one pin with busWidth==0 (never a pinBus) and with the
 * given name (at symbol definition, built-in connector symbols use "" here).
 * The GeiSymTPOWER, GeiSymTGROUND and GeiSymTNEGPOWER are always single-bit
 * connectors.
 *
 * There is support for polygon shaped pins (with up to 20 polygon points):
 * If spinIter stops at a symbol pin having GeiSymPinInfo.polygon_len > 0
 * then GeiSymPinInfo.polygon is valid, otherwise GeiSymPinInfo.xStub
 * and GeiSymPinInfo.yStub is valid (no polygon shaped pin).
 *
 * PinIter's corresponding IpinInfo returns the pin name and width and
 * the attributes @name and @attr in GeiPinInfo.
 * PinIter's corresponding IjoinInfo returns the connected net or sub-net
 * in GeiJoinInfo.  The "subpinIdx" argument must be -1 for a single-bit
 * pin and must be within 0...width-1 for a pinBus.
 *
 * The pattrIter loops over all attributes of a pin. As for other attributes,
 * the IattrInfo returns the attribute information.
 * The pattrDspIter loops over all display locations of a pin. The details 
 * about the display (GeiSymAttrInfo) can be requested with IattrDspInfo,
 * which also return information about the attribute (GeiAttrInfo) shown
 * at this display.
 *
 * The same is available for instances/port instances using the attrIter 
 * and attrDspIter, and for nets/netBundles using the nattrIter and 
 * nattrDspIter.
 *
 * The connIter and bconnIter provide the reverse link to IjoinInfo.
 * The connIter  loops over all connected pins of a single-bit net.
 * The bconnIter loops over all connected pins of the given subnet of
 * a netBundle (the subnet name is returned in the argument "subnetname").
 * If those iterators stop at a connector instance, then GeiConnInfo.name 
 * is NULL (note: the connector's name (or busname) is the instance name).
 *
 * The nbunIter-->wireIter only returns the points on the common bus
 * (connecting Triangle/TriangleBus/Slash/SlashBus rippers
 * and pinBus/hierPinBus/portBus objects for the full connections).
 * The nbunIter-->ripIter-->subWIter only returns the points on the
 * visible subnet parts (excluding rippers).
 * The nbunIter-->ripIter iterates over Triangle and TriangleBus;
 * and it loops over Slash and SlashBus rippers if they are NOT merging into
 * an overlay bus (for example: top/bottom connected sub-nets).
 * the RipInfo.compass' 4 bits represent the "four winds" bringing wires
 * to the RipInfo.xBus/yBus (setting the configure property buswireexcess
 * to 0 may improve the usability of this data).
 * The nbunIter-->ripIter stops at TriangleBus/SlashBus rippers multiple
 * times (one time for each sub-net connection by decreasing RipInfo.countDown).
 * Example: Assuming a
 * TriangleBus ripper at a 32-bit netBundle represents the rip-out of
 * 3 sub-nets, then ripIter stops at this TriangleBus ripper with countDown==2,
 * then again with countDown==1 and again with countDown==0. The
 * RipInfo.name changes every time, but all other RipInfo members stay
 * identical.
 *
 *	netBundle
 *	=========+
 *		|| Triangle
 *		||>------------- sub-net
 *		||
 *		|| TriangleBus
 *		||>============= set of sub-nets
 *		||
 * 
 * The overIter stops at each "overlay bus" (an artificial bus, created
 * by Nlview to display certain situations nicely).  Each "overlay bus"
 * connects to a pinBus/hierPinBus/portBus (returned by IoverInfo)
 * and to one or more Slash or SlashBus rippers (a Slash ripper connects
 * an unbundled single-bit net and a SlashBus ripper connects a netBundle).
 * The "overlay bus" is returned by overIter->ovWIter (returning all
 * points that form the bus graphics) and by overIter->ovRIter
 * (returning all Slash and SlashBus rippers).
 * You can use overIter->ovRIter to get the grouped net names (unbundled nets)
 * and sub-net names).
 *
 *                    net
 *                    ----------+
 *	netBundle               |
 *	=========+              |
 *		||              |
 *		||              |
 *		||              |
 *		 \ SlashBus      \ Slash
 *		  \               \
 *		   ======================== "overlay bus" ==== pinBus/portBus
 *
 * ===================================================================
 */



/* ===========================================================================
 * GeiObject	- is one of Symbol, Page, Instance, Net, NetBundle.
 * GeiIter	- pointer to an iterator instance.
 * ===========================================================================
 */
struct GeiObject;
struct GeiIter;



/* ===========================================================================
 * Define Types
 * ===========================================================================
 */
enum GeiSymType {
    GeiSymTInst,	/* Normal instance of a symbol */
    GeiSymTHier,	/* HIER frame */

    GeiSymTINPORT,	/* Stub: Input Port */
    GeiSymTOUTPORT,	/* Stub: Output Port */
    GeiSymTINOUTPORT,	/* Stub: Bidir Port */
    GeiSymTPAGEIN,	/* Stub: Offpage Input Connector */
    GeiSymTPAGEOUT,	/* Stub: Offpage Output Connector */
    GeiSymTPAGEINOUT,	/* Stub: Offpage InOutput Connector */
    GeiSymTPOWER,	/* Stub: Onpage Power Connector */
    GeiSymTGROUND,	/* Stub: Onpage Ground Connector */
    GeiSymTNEGPOWER 	/* Stub: Onpage Negative Power Connector */
};
enum GeiSymOrient {
    GeiSymOrientR0      = 0,
    GeiSymOrientR90     = 5,
    GeiSymOrientMXR90   = 7,
    GeiSymOrientMY      = 1,
    GeiSymOrientMX      = 2,
    GeiSymOrientMYR90   = 4,
    GeiSymOrientR270    = 6,
    GeiSymOrientR180    = 3
};
enum GeiPinDir {
    GeiPinDirLeft        = 1,	/* To left   or left side */
    GeiPinDirRight       = 2,	/* To right  or right side */
    GeiPinDirTop         = 4,	/* To top    or top side */
    GeiPinDirBot         = 8,	/* To bottom or bottom side */

    GeiPinOutput      = 0x10,	/* The pin is an output */
    GeiPinInput       = 0x20,	/* The pin is an input */
    GeiPinInOut       = 0x30 	/* The pin is bi-directional */
};
enum GeiRipType {
    GeiRipTTriangle,	/* RipInfo returned by nbunIter->ripIter */
    GeiRipTTriangleBus,	/* RipInfo returned by nbunIter->ripIter */
    GeiRipTSlash,	/* " returned by nbunIter->ripIter & overIter->ovRIter*/
    GeiRipTSlashBus	/* " returned by nbunIter->ripIter & overIter->ovRIter*/
};
enum GeiLineStyle {
    GeiLineSDefault   = 0,	/* default line style */
    GeiLineSSolid     = 1,	/* solid   line style */
    GeiLineSDashed    = 2,	/* dashed  line style */
    GeiLineSDashed2   = 3	/* dashed2 line style */
};
enum GeiPointType {
    GeiPointTBegPin   = 1,      /* Begin wire at a pin */
    GeiPointTEndPin   = 2,      /* End a wire at a pin */
    GeiPointTMidPin   = 3,	/* Pin on wire mid-point (EndPin+BegPin) */
    GeiPointTCorner   = 4,      /* Corner at 2 intersecting segments */
    GeiPointTMidT     = 5,      /* T junction in middle of a segment (I) */
    GeiPointTBegT     = 6,      /* Begin wire at a T junction (I) */
    GeiPointTEndT     = 7,      /* End wire at a T junction (I) */
    GeiPointTNoCorner = 8,      /* Additional vertex for changing point flags */

    GeiPointTBegWire  =11,      /* Begin wire anywhere, e.g. for Symbol shape */
    GeiPointTMidArc   =12,      /* Middle of an Arc,    e.g. for Symbol shape */
    GeiPointTEndWire  =13,      /* End wire anywhere,   e.g. for Symbol shape */
    GeiPointTDot      =14,	/* Dot,                 e.g. for Symbol shape */
    GeiPointTRipL     =15,	/* Ripper to left  side */
    GeiPointTRipR     =16	/* Ripper to right side */
};

enum GeiAttrJust {
    GeiAttrFJustUL  = 1,	/* The justification is topleft      -ul */
    GeiAttrFJustUC  = 2,    	/* The justification is topcenter    -uc */
    GeiAttrFJustUR  = 3,    	/* The justification is topright     -ur */
    GeiAttrFJustCL  = 4,    	/* The justification is centerleft   -cl */
    GeiAttrFJustCC  = 5,    	/* The justification is centercenter -cc */
    GeiAttrFJustCR  = 6,    	/* The justification is centerright  -cr */
    GeiAttrFJustLL  = 7,    	/* The justification is lowerleft    -ll */
    GeiAttrFJustLC  = 8,    	/* The justification is lowercenter  -lc */
    GeiAttrFJustLR  = 9    	/* The justification is lowerright   -lr */
};

struct GeiPageInfo {
	int		pageNumber;
	int		pageLeft, pageRight, pageTop, pageBottom;
};
struct GeiSymInfo {
	enum GeiSymType type;
	const char*	name;
	const char*	vname;
	enum GeiSymOrient orient;
	int 		bboxLeft,bboxTop,bboxRight,bboxBot;
	int		familyRefCount;	/* if this is master: sum of refCounts*/
	unsigned	privatized:1;	/* is a "private" copy of master */
	struct GeiObject* master;	/* The not-rotated master or NULL */
	int		xmove;		/* x offset if master is used */
	int		ymove;		/* y offset if master is used */
	float		scale;		/* "private" symbols may be scaled */
	int		linewidth;	/* -1 (default) >=0 sym outline width */
	enum GeiLineStyle linestyle;
	unsigned	linecolor:4;	/* configure property boxcolor0..15 */
	unsigned	fillstyle:5;	/* 0=unset (transp.) NlvRendFillStyle */
	unsigned	fillfg:4;	/* fg-color 0=transp, prop fillcolor */
	unsigned	fillbg:4;	/* bg-color 0=transp, prop fillcolor */
	unsigned	fillregion:4;	/* 0=unset (transp.), prop fillcolor */
	unsigned	sandwich:3;	/* draw symbol in sandwich style */
	unsigned	boxpinsquare:3;	/* prop boxpinsquare at symbol */
};
struct GeiInstInfo {
	enum GeiSymType   type;		/* Identical to sym's type */
	struct GeiObject* sym;		/* The (eventually rotated) symbol */
	const char*	name;
	int		x;
	int		y;
	unsigned	pinpermute:1;	/* may have GeiPinInfo.swap_name */
};
struct GeiNetInfo {
	const char*	name;		/* net or netBundle name */
	unsigned	busWidth;	/* zero is net, >0 is netBundle */
	unsigned	isPower:1;	/* is a power net */
	unsigned	isGround:1;	/* is a ground net */
	unsigned	isIn:1;		/* is an input  (interface) net */
	unsigned	isOut:1;	/* is an output (interface) net */
	unsigned	isHidden:1;	/* is a hidden net */
};
struct GeiPointInfo {
	int		x;
	int		y;
	enum GeiPointType type;
	unsigned	filled:1;	/* filled polygon - only in symbol */
};
struct GeiSymAttrInfo {
	const char*	name;		/* name of attribute (or text label) */
	int		x;
	int		y;
	enum GeiAttrJust just;  
	int		size:16;
	unsigned	vertical:1;
	unsigned	text:1;		/* is a text label (name is displayed)*/
	unsigned	marks:1;	/* display is for graphical marks */
	unsigned	additional:1;	/* additional auto-generated display,
					 * which is not defined in the symbol */
};
struct GeiAttrInfo {
	const char*	value;		/* value of the attribute */
	const char*	name;		/* name of attribute */
	int		nameLen;	/* because name is not null-terminated*/

	char		fmtForeground[8];	/* changed fg color or "" */
	char		fmtBackground[8];	/* changed bg color or "" */
	float		fmtFontscale;		/* fontsize scale fac or 1.0 */
	char		fmtUserData[8];		/* arbitrary user data or "" */
};
struct GeiSymPinInfo {
	const char*	name;		/* pin or pinBus name */
	int		x;
	int		y;
	int		xStub;
	int		yStub;
	unsigned	    polygon_len;/* if >0 then polygon, else x/yStub */
	struct GeiPointInfo polygon[20];
	unsigned	neg:1;		/* is bubbled */
	unsigned	clk:1;		/* is clocked pin */
	unsigned	hier:1;		/* is inside pin at GeiSymTHier */
	unsigned	hidden:1;	/* is hidden pin */
	unsigned	hidestub:1;	/* is pin with hidden stub */
	unsigned	busWidth;	/* zero is pin, >0 is pinBus */
	int		linewidth;	/* -1 (default) >=0 pin outline width */
	enum GeiLineStyle linestyle;
	unsigned	linecolor:4;	/* 0 boxpincolor, n>0: boxcolor[n] */
	unsigned	fillstyle:5;	/* 0=unset (transp.) NlvRendFillStyle */
	unsigned	fillfg:4;	/* fg-color 0=transp, prop fillcolor */
	unsigned	fillbg:4;	/* bg-color 0=transp, prop fillcolor */
	enum GeiPinDir	dir;

	struct GeiSymAttrInfo nameLabel;/* obsolete, use spattrIter */
	struct GeiSymAttrInfo attrLabel;/* obsolete, use spattrIter */
};
struct GeiPinInfo {
	const char*	name;		/* pin or pinBus name */
	const char*	swap_name;	/* pin permutation: name of symbol pin
					   whose location is used - or NULL */
	unsigned	busWidth;	/* zero is pin, >0 is pinBus */
	unsigned	hier:1;		/* is inside pin at GeiSymTHier */
	unsigned	hidden:1;	/* is hidden pin */
	unsigned	hidestub:1;	/* is pin with hidden stub */

	struct GeiAttrInfo nameLabel;	/* obsolete, use pattrIter */
	struct GeiAttrInfo attrLabel;	/* obsolete, use pattrIter */

	/* Example 1: instance single-bit pin, "pin U1 A" (not pin-swapped)
	 *	name=A		swap_name=NULL	busWidth=0
	 *
	 * Example 2: instance bus-pin: "pinBus U1 D<0:4> 5 D0 D1 D2 D3 D4"
	 *	name=D<0:4>	swap_name=NULL	busWidth=5
	 *
	 * Example 3: HIER inside bus-pin: "hierPinBus U2 A<2:4> 3 A2 A3 A4"
	 *	name=A<2:4>	swap_name=NULL	busWidth=3	hier=1
	 *
	 * Example 4: connector e.g. "port A"
	 *	name=NULL	swap_name=NULL	busWidth=0
	 *
	 * Example 5: bus-connector e.g. "portBus D<0:5> 6 D0 D1 D2 D3 D4 D5"
	 *	name=NULL	swap_name=NULL	busWidth=6
	 *
	 * Example 6: instance single-bit pin, "pin U1 A" (pin-swapped)
	 *	name=A		swap_name=B	busWidth=0
	 */
};

struct GeiJoinInfo {			/* pin's (or sub-pin's) connection */
	const char*	subpinname;	/* pinBus' subpin name (NULL for pin)*/

	struct GeiObject* net_or_nbun;	/* connected net or netBundle's bit */
	int		subnetIdx;	/* 0...width-1 in nbun (-1 for net) */
	const char*	subname;	/* subnet name in nbun (NULL for net) */

	/* Example 1: single-bit "pin U1 A" connects net CLR
	 *	subname=NULL
	 *	net-->CLR	subnetIdx=-1	subnetname=NULL
	 *
	 * Example 2: single-bit "pin U1 A" connects sub-net D5
	 *	subname=NULL
	 *	nbun-->D<9:0>	subnetIdx=4	subnetname=D5
	 *
	 * Example 3: subpin #3 of "pinBus U1 D<0:4> 5 D0 D1 D2 D3 D4"
	 *	      connects net CLR
	 *	subname=D3
	 *	net-->CLR	subnetIdx=-1	subnetname=NULL
	 *
	 * Example 4: subpin #3 of "pinBus U1 D<0:4> 5 D0 D1 D2 D3 D4"
	 * 	      connects sub-net D5
	 *	subname=D3
	 *	nbun-->D<9:0>	subnetIdx=4	subnetname=D5
	 */
};

struct GeiConnInfo {			/* net's (or sub-net's) connection */
	struct GeiObject* inst;		/* instance id */
	const char*	name;		/* pin or pinBus name */
	int		subpinIdx;	/* 0...width-1 (or -1 for pin) */
	const char*	subname;	/* pinBus's subpin name (NULL for pin)*/
	unsigned	hier:1;		/* is inside pin at GeiSymTHier */
	unsigned	hidden:1;	/* is hidden pin */
	unsigned	hidestub:1;	/* is pin with hidden stub */

	/* Example 1: connect single-bit "pin U1 A"
	 *	inst.type=GeiSymTInst 	inst.name=U1
	 *	name=A		subname=NULL	subpinIdx=-1
	 *
	 * Example 2: connect subpin #3 of "pinBus U1 D<0:4> 5 D0 D1 D2 D3 D4"
	 *	inst.type=GeiSymTInst 	inst.name=U1
	 *	name=D<0:4>	subname=D3	subpinIdx=3
	 *
	 * Example 3: connect subpin #1 of "hierPinBus U2 A<2:4> 3 A2 A3 A4"
	 *	inst.type=GeiSymTHIER 	inst.name=U2	hier=1
	 *	name=A<2:4>	subname=A3	subpinIdx=1
	 *
	 *
	 * Example 4: connect single-bit "port A"
	 *	inst.type=GeiSymTINPORT	inst.name=A
	 *	name=NULL		subname=NULL	subpinIdx=-1
	 *
	 * Example 5: connect subpin #3 of "portBus D<0:5> 6 D0 D1 D2 D3 D4 D5"
	 *	inst.type=GeiSymTINPORT	inst.name=D<0:5>
	 *	name=NULL		subname=D3	subpinIdx=3
	 *
	 * Example 6: connect subpin #3 of a pageConn at "netBundle D<0:5> 6"
	 *	inst.type=GeiSymTPAGEOUT inst.name=D<0:5>
	 *	name=NULL		subname=D3	subpinIdx=3
	 */
};

struct GeiRipInfo {
	enum GeiRipType type;
	int		xBus;
	int		yBus;
	int		xSubnet;
	int		ySubnet;
	int		countDown;	/* 0 (or repeat count for bus rippers)*/
	int		subnetIdx;	/* subnet index or -1 */
	const char*	subname;	/* name of subnet or NULL */
	struct GeiObject* net_or_nbun;	/* for ovRIter only */
	unsigned	compass:4;	/* NSEW (== 8,4,2,1) wire direction */

	/* Example 1: ripIter (operating on netBundle D<0:5>)
	 *  stop at a Triangle ripper of bit#1 (D1)
	 *	countDown=0
	 *	subnetIdx=1
	 *	subname=D1
	 *  stop at a TriangleBus ripper of bits #1,2,5 (D1,D2,D5)
	 *	countDown=2,1,0
	 *	subnetIdx=1,2,5
	 *	subname=D1,D2,D5
	 *
	 * Example 2: ovRIter (operating on "overlay bus" CLR,A7,D2,D4)
	 *  stop at a Slash ripper CLR
	 *	countDown=0
	 *	subnetIdx=-1
	 *	net_or_nbun-->net CLR
	 *	subname=NULL
	 *  stop at a Slash ripper A7
	 *	countDown=0
	 *	subname=A7
	 *	net_or_nbun-->netBundle A
	 *	subnetIdx=7
	 *  stop at a SlashBus ripper D2,D4
	 *	countDown=1,0
	 *	subname=D2,D4
	 *	net_or_nbun-->netBundle D,-->netBundle D
	 *	subnetIdx=2,4
	 */

	struct GeiAttrInfo label;
};

struct GeiOverInfo {
	struct GeiObject* inst;		/* instance id */
	const char*	name;		/* pinBus name */
	unsigned	busWidth;	/* zero is pin, >0 is pinBus */
	unsigned	hier:1;		/* is inside pin at GeiSymTHier */

	/* Example 1: overIter for "overlay bus" at "portBus D<0:5>"
	 *	inst.type=GeiSymTINPORT
	 *	inst.name=D<0:5>
	 *	name=NULL
	 *	busWidth=6
 	 *
	 * Example 2: overIter for "overlay bus" at "pinBus U1 A<2:5>"
	 *	inst.type=GeiSymTInst
	 *	inst.name=U1
	 *	name=A<2:5>
	 *	busWidth=4
	 */
};



/* ===========================================================================
 * Nlview Graphics Access Functions
 * ===========================================================================
 */
struct GeiAccessFunc {

#define OBJ   struct GeiObject
#define Iter  struct GeiIter

	/* ===================================================================
	 * Get Version of the interface and the implementation
	 */
	int  	     (*geiVersion)(void);	/* return enum GeiVersion */
	const char*  (*implVersion)(void);	/* return e.g. "1.0 zdb" */
	long          magic;			/* is GeiMagic */

	/* ===================================================================
	 * Iterator - loop over Nlview DataBase.  Imore returns 0 or 1, Inext
	 * switches to the next element and Iobj returns the pointer to
	 * the current element (not all iterators support Iobj).
	 */
	int   (*Imore)(const Iter*);
	void  (*Inext)(Iter*);
	OBJ*  (*Iobj)(const Iter*);

	/* ===================================================================
	 * These functions create and initialize iterators;
	 * "freeIter" destroys them.
	 */
	Iter*   (*symIter)(int flags);
	Iter*  (*pageIter)(void);

	Iter*  (*spinIter)(OBJ* sym);
	Iter*(*spattrIter)(OBJ* sympin);
	Iter* (*sattrIter)(OBJ* sym);
	Iter* (*spropIter)(OBJ* sym);
	Iter* (*shapeIter)(OBJ* sym);

	Iter*  (*instIter)(OBJ* page);
	Iter*   (*netIter)(OBJ* page);
	Iter*  (*nbunIter)(OBJ* page);

	Iter*   (*pinIter)(OBJ* inst);
	Iter*  (*attrIter)(OBJ* inst);
	Iter*  (*attrDspIter)(OBJ* inst);

	Iter*  (*pattrIter)(OBJ* pin);
	Iter*  (*pattrDspIter)(OBJ* pin, OBJ* inst);

	Iter*  (*connIter)(OBJ* net);
	Iter* (*bconnIter)(OBJ* nbun, int subnetIdx, const char** subnetname);
	Iter* (*nattrIter)(OBJ* net_or_nbun);
	Iter* (*nattrDspIter)(OBJ* net_or_nbun);
	Iter*  (*wireIter)(OBJ* net_or_nbun);
	Iter*   (*ripIter)(OBJ* nbun);
	Iter*  (*subWIter)(Iter* ripIter);

	Iter*  (*overIter)(OBJ* page);
	Iter*   (*ovWIter)(Iter* overIter);
	Iter*   (*ovRIter)(Iter* overIter);

	void   (*freeIter)(Iter*);

	/* ===================================================================
	 * Get detail Infos.
	 */
	void    (*symInfo)(OBJ* sym,        struct GeiSymInfo*);
	void   (*pageInfo)(OBJ* page,       struct GeiPageInfo*);

	void  (*IspinInfo)(Iter* spinIter,  struct GeiSymPinInfo*);
	void (*IsattrInfo)(Iter* sattrIter, struct GeiSymAttrInfo*);
	void (*IspropInfo)(Iter* sattrIter, struct GeiAttrInfo*);
	void (*IshapeInfo)(Iter* shapeIter, struct GeiPointInfo*);

	void   (*instInfo)(OBJ* inst,       struct GeiInstInfo*);
	void    (*netInfo)(OBJ* net_or_nbun,struct GeiNetInfo*);

	void   (*IpinInfo)(Iter* pinIter,   struct GeiPinInfo*);
	void  (*IjoinInfo)(Iter* pinIter,   int subpinIdx, struct GeiJoinInfo*);

	void  (*IconnInfo)(Iter* connIter,  struct GeiConnInfo*);
	void  (*IattrInfo)(Iter* attrIter,  struct GeiAttrInfo*);

	void  (*IwireInfo)(Iter* wireIter,  struct GeiPointInfo*);
	void   (*IripInfo)(Iter* ripIter,   struct GeiRipInfo*);
	void  (*IsubWInfo)(Iter* subWIter,  struct GeiPointInfo*);

	void   (*IoverInfo)(Iter* overIter, struct GeiOverInfo*);
	void   (*IovWInfo)(Iter* wireIter,  struct GeiPointInfo*);
	void   (*IovRInfo)(Iter* ripIter,   struct GeiRipInfo*);

	void  (*IattrDspInfo)(Iter* attrDspIter, struct GeiSymAttrInfo*, 
						    struct GeiAttrInfo*); 

	/* ===================================================================
	 * Service function to convert Arc(p1,p2,p3) --> center,diam,start,span
	 */
	int/*bool*/ (*calculateArc)(
	    int p1x,int p1y, int p2x,int p2y, int p3x,int p3y,
	    double* cx, double* cy, double* diam, double* start, double* span);

#undef OBJ
#undef Iter
};

#endif
