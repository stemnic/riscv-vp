/*  vdi.h 1.106 2018/11/02

    Copyright 2002-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
  	Virtual DataBase Interface
    ===========================================================================
*/

#ifndef vdi_h
#define vdi_h

enum { VdiVersion = 41 };	/* must be incremented for each modification */

/* ===========================================================================
 * The "Virtual DataBase Interface" is optionally used by Nlview to load
 * netlist data from an "unknown" DataBase.  The DataBase is assumed to
 * be hierarchical with the following containment model:
 *
 * The DataBase contains a set of Modules.  Each Module contains Ports,
 * PortBusses, Nets, NetBundles and Instances.  Each of these objects is
 * represented by a unique Object ID "Obj*" (unique at least within modules).
 * The Nets and Ports are single-bit objects. NULL is a reserved Object ID.
 * 
 * An Instance is either a primitive ("downModule" returns NULL) or a
 * hierarchical block ("primName" returns NULL).  All Instances that
 * return the same primName must have the same pin-footprint (same
 * number of pins and same pin names).  Each hierarchical block must
 * have pins/pinBusses identically to the downModule's Ports/PortBusses.
 * The modNames and primNames must share the same namespace.
 *
 * Each PortBus contains subports and each pinBus contains subpins.
 * Subports/subpins are hidden behind their PortBus/pinBus, that means
 * subports are neither visible to "portFind" nor to "portIter" and
 * subpins are not visible to "conIterInst".
 *
 * A NetBundle does not "contain" its subnets (they are not hidden),
 * that means, subnets are visible to "netFind" and to "netIter".
 * The iterator "subIter" loops over the subnets of a NetBundle,
 * but "subIter" is not a true containment iterator (because subnets
 * are "contained" by the Module).
 * 
 * ---------------------------------------------------------------------------
 * The CONNECTIVITY is local to each Module.  It is defined by
 * single-bit Nets connecting single-bit Ports, subports, Instance-pins
 * and Instance-subpins (this is why Ports and subports - as well as
 * pins and subpins - must share the same namespace, i.e. they must
 * have different names).
 *
 * OOM Support: In addition to the local connectivity, there is support
 *		for OOM (Out of Module) connections:
 *		if Cnet returns NULL, CnetOOM must either return NULL
 *		(unconnected) or an OOM connection (hierarchical net name, i.e.
 *		including instance path); the HObj* argument to CnetOOM
 *		refers to the same inst as given to conIterInst, but includes
 *		the instance path to it. If Cnet returns not NULL, then CnetOOM
 *		must return NULL.
 *
 *		The conIterNetOOM is called after conIterNet is executed
 *		to get the extra, unique OOM connections.
 *
 * NetVector Support (planned for future use):
 * 		In addition to the single-bit connectivity by Nets,
 *		NetVectors (Nets with VdiNetFVector flag set) define
 *		connectivity on bus level (NetVectors connect to
 *		Instance-PinBuses and PortBuses).
 *
 * Arc Support: arcs define logical relations/dependencies between pins of the
 *		same primitive (instance).
 *		This information helps command "ictrl addcone" to traverse
 *		paths thru primitives more accurately and faster.
 *		Arcs can be returned by the conIterInstArcs iterator
 *		which is identical to conIterInst, except that
 *		it must only loop over a single pin's or subpin's arcs.
 *		The pin or subpin for which the arcs are queried is given
 *		indirectly by passing a conIterNet iterator object stopping
 *		at that primitive pin/subpin.
 *		==> The implementation of conIterInstArcs is optional:
 *		if not implemented, its entry in VdiAccessFunc must be NULL.
 *
 * The Connectivity-Iterator "Citer" is used to traverse the
 * connectivity:
 *
 *  (a)	Loop over Net pins: "conIterNet" creates a Citer that loops over
 *	connected Instance-pins, Ports and subports.  The iterator
 *	must support:
 *	* Ccomp	     returns the connected Obj* (Inst/Port/PortBus) and
 *		     sets the "type" to 0/1/2 respectively).
 *	* Cpinnumber return the order number of the connected pin (Device only)
 *                   (0 and 1: current flow pins, 2 gate, 3 bulk)
 *	* Cpinname   return the connected pinname/portname/subportname
 *		     respectively.
 *	* CportFlags returns the direction of the pin
 *	* Cnet       *undefined*
 *	* CnetOOM    *undefined*
 *	* Cpbusname  *undefined*
 *	* Cmembers   *undefined*
 *	* CcompOOM   *undefined*
 *	* CpinAttr   *undefined*
 *
 *  (a2) Loop over NetVector pins (planned for future use):
 *  	"conIterNet" creates a Citer that
 *	loops over connected Instance-pinBuses and PortBuses.  The iterator
 *	must support:
 *	* Ccomp	     returns the connected Obj* (Inst/PortBus) and
 *		     sets the "type" to 0/2 respectively).
 *	* Cpinnumber *undefined*
 *	* Cpinname   *undefined*
 *	* CportFlags returns the direction of the pinBus/portBus
 *	* Cnet       *undefined*
 *	* CnetOOM    *undefined*
 *	* Cpbusname  return the connected pinBus/portBus name respectively.
 *	* Cmembers   *undefined*
 *	* CcompOOM   *undefined*
 *	* CpinAttr   *undefined*
 *
 *  (b) Loop over Net's OOM pins: "conIterNetOOM" creates a Citer that loops
 *	over connected OOM Instance-pins. The iterator
 *	must support:
 *	* Ccomp      *undefined*
 *	* CcompOOM   returns the path to the connected object (Inst).
 *	* Cpinnumber return the order number - identically to (a)
 *	* Cpinname   return the connected pinname.
 *	* CportFlags returns the direction of the pin
 *	* Cnet       *undefined*
 *	* CnetOOM    *undefined*
 *	* Cpbusname  *undefined*
 *	* Cmembers   *undefined*
 *	* CpinAttr   *undefined*
 *
 *  (c)	Loop over Instance pins and pinBusses: "conIterInst" creates
 *      a Citer that must support:
 *      If the iterator stops at a pin:
 *	* Cnet       return connected Net
 *	* CnetOOM    return connected out-of-module Net
 *	* Cpinnumber *undefined*
 *	* Cpinname   return the pinname
 *      * Cpbusname  return NULL
 *      * CportFlags return the pin direction
 *      * Cmembers   return NULL
 *	* Ccomp	     *undefined*
 *	* CcompOOM   *undefined*
 *	* CpinAttr   return attributes to be added/displayed at the pin
 *      Important: conIterInst must return the device pins in ascending
 *                 "Cpinnumber"-order starting with pin-number 0
 *
 *      If the iterator stops at a pinBus:
 *	* Cnet       *undefined*
 *	* CnetVector return connected NetVector (planned for future use)
 *	* CnetOOM    *undefined*
 *	* Cpinnumber *undefined*
 *	* Cpinname   return NULL
 *      * Cpbusname  return the pinBus name
 *      * CportFlags return the pinBus direction
 *      * Cmembers   return new nested Citer to loop over the subpins
 *	* Ccomp	     *undefined*
 *	* CcompOOM   *undefined*
 *	* CpinAttr   return attributes to be added/displayed at the pinBus
 *
 *     	Loop over Instance-pinBus subpins: "Cmembers" creates a nested
 *      Citer that must support:
 *	* Cnet       return connected Net
 *	* CnetOOM    return connected out-of-module Net
 *	* Cpinnumber *undefined*
 *	* Cpinname   return the subpin name
 *      * Cpbusname  return NULL
 *      * CportFlags return the subpin flags (for details see documentation)
 *      * Cmembers   return NULL
 *	* Ccomp	     *undefined*
 *	* CcompOOM   *undefined*
 *	* CpinAttr   return the subpin attributes (for details see doc)
 *
 *  (d)	Loop over PortBus subports: "conIterPBus" creates a
 *      Citer that must support:
 *	* Cnet       return connected Net
 *	* CnetOOM    *undefined*
 *	* Cpinnumber *undefined*
 *	* Cpinname   return the subport name
 *      * Cpbusname  *undefined*
 *      * CportFlags return the subport flags (for details see documentation)
 *      * Cmembers   *undefined*
 *	* Ccomp	     *undefined*
 *	* CcompOOM   *undefined*
 *	* CpinAttr   return the subport attributes (for details see doc)
 *
 * The relation between PortBus and the subports is only available by
 * the "conIterPBus" iterator.  The relation between Instance-pinBus
 * and the Instance-subpins is only available by the nested Citer
 * (created by "Cmember").
 *
 * ---------------------------------------------------------------------------
 * The *Attr() functions return attributes (name value pairs) to be added
 * to the corresponding object (inst, port, pbus, net, nbun and pin).
 * The returned attributes must be stored as NULL-terminated array
 * of "struct VdiHAttr" (the terminating element has VdiHAttr.name == NULL).
 * The returned VdiHAttr array may e.g. be stored in static memory and can
 * be re-used for each call, because Nlview will pick up all data immediately.
 *
 * In addition to attributes, the *Attr() functions can return highlight
 * information by storing that information into the members of the given
 * "struct VdiHHi".  Only netAttr() may set VdiHHi.segm to point to a single-
 * linked list of "VdiHSegmHi" representing net-segment highlight information
 * (the VdiHSegmHi may reside in static memory, Nlview will pick up all
 * information immediately).  Before each *Attr() call, Nlview will
 * clear the VdiHHi.hi flag, so the *Attr() functions may just not touch
 * the given VdiHHi if there is no highlight information. If there is
 * highlight information, the *Attr() functions must fully initialize the
 * VdiHHi struct.
 *
 * The instAttr() function in addition should set the flags "fl" to indicate
 * if the given instance's pins/pinBuses/subpins have
 * attributes and/or highlight information.
 * The flags "fl" are an OR-combination of 5 bits,
 * 4 performance bits and 1 feature bit.
 * In doubt, the implementation of instAttr may set the flags to 0xf to make
 * Nlview traverse everything.  But, if instAttr() does not modify the
 * flags, then they stay at zero and Nlview will skip traversing the
 * instance pins (with conIterInst/Cmembers calling CpinAttr for each
 * pin/pinBus/subpin) -
 * this results in some speed-up.  The flags in "fl" advice Nlview to:
 *
 *	0x1 - traverse all instance pins and check for attributes
 *	0x2 - traverse all instance pins and check for highlight data
 *	0x4 - traverse all ports of instance's down-module for attributes
 *	0x8 - traverse all ports of instance's down-module for highlight data
 *  sub-bit support:
 *     0x10 - traverse all instance subpins and check for attributes
 *
 * 
 * Display rules:
 *	instAttr's "@name" and "@cell" will be displayed at built-in gate-level
 *	symbol shapes ("@cell" depend on the Nlview configure properties
 *	gatecellname and showcellname).  But "@name" and "@value" will be
 *	displayed at built-in transistor-level symbol shapes.
 *	If instAttr does not return "@name", then (a) if displayed inside
 *	a HIER-box, the last name segment will be automatically stored into
 *	"@name" (to show short names only), else (b) if displayed in
 *	one-block-at-a-time mode, then "@name" stays undefined and Nlview
 *	will display the full instance name (default behavior).
 *
 *	CpinAttr's "@name", "@attr", "@val" and "@marks" will be displayed
 *	at the pins/pinBuses of the built-in symbol shapes, if the Nlview
 *	configure property showpinname, showattribute, showval and showmarks
 *	is > 1, respectively.
 *	If CpinAttr does not define @name, then
 *	the pin name is displayed instead (default behavior).
 *	Please note: some built-in shapes, e.g. transistor-level symbols,
 *	don't display the pin names at all, so the "@name" attribute is
 *	ignored; other built-in shapes' behavior depend on Nlview configure
 *	property gatepinname.
 *
 *	netAttr's "@name", "@attr" and "@marks" will be displayed,
 *	if the Nlview configure property shownetname, showattribute and
 *	showmarks is > 1, respectively, and createnetattrdsp is configured
 *	accordingly. At PG nets, the "@pgtype"
 *	will be displayed at the power/ground stubs if showpgtype is > 1.
 *
 *	portAttr's and pbusAttr's "@name" will be displayed at the built-in
 *	port symbols if property showportname is set to true.
 *      "@attr", "@val" and "@marks" are handled as in CpinAttr explained
 *      above.
 *
 *	At user-defined symbol shapes (see -symlib file), all kind of
 *	attributes can be displayed, depending on the symbol's attrdsp
 *	entities and the setting of configure property showattribute.
 *
 * Example:
 * (I1)	For PMOS/NMOS devices, instAttr might define "@value" as "W=7u\nL=3u";
 *	for Rs and Cs it might be just the resistance or capacitance,
 *	like defining "@value" as ".57pF".
 * (I2)	For gate-level instances, instAttr might define the boolean equation,
 *	e.g. define "@cell" as "Y=A+B+(C*D)"
 * (N1)	For power nets, netAttr might define "@pgvalue"
 *	as "VDD" or "+5V" (those value attributes should be short, because
 *	Nlview does not reserve horizontal space for them).
 * (P1)	For any pin, CpinAttr might define a delay value as "@attr" = "2.7ns"
 *	or a logical value as "@val" = "1" (or whatever makes sense in the
 *	desired application).
 *
 * All *Attr functions may return tree-based values (e.g. for back-annotated
 * capacitors, or delay values).  For this purpose, all *Attr functions get
 * an HObj* instead of an Obj*.  The HObj* additionally define the instance
 * path to the Obj*.  The CpinAttr function's HObj* only defines the
 * instance path context, the HObj's Obj* is NULL.
 *
 * ---------------------------------------------------------------------------
 * The instValue function works on Rs, Cs and Ls. It is supposed to return
 * their "value" in Ohm, Farad and Henry, respectively. It is only called by
 * Nlview for "devices" (instances which have the VdiInstFDevice flag set).
 * This function is only used by Nlview to perform some computing
 * (those values are never displayed).
 * ===========================================================================
 */



/* ===========================================================================
 * Obj* refers to a Module, Instance, Net, NetBundle, Port or PortBus object
 * or is NULL.
 * DB* is an arbitrary pointer to the foreign DataBase root.
 * Iter/Citer are Iterator/Connectivity-Iterator completely managed by the
 * foreign DataBase.
 * ===========================================================================
 */
struct VdiObject;
struct VdiDB;
struct VdiIter;
struct VdiCiter;

/* VdiHObject defines a tree-based context to "obj" - that is: topinfo.path.obj
 * The "path" is a list of instance names, the elements are separated with
 * "pathSep".  The "topinfo" is not interpreted by Nlview, it is identically
 * as specified to "ictrl init".
 */
struct VdiHObject {		/* defines a tree-based context to "obj" */
    const char* topinfo;	/* as given to "ictrl init" */
    const char* path;		/* instance path from top to obj */
    char	pathSep;	/* separator character for elements in "path" */
    struct VdiObject* obj;	/* the object, NULL in CpinAttr */
    struct VdiObject* mod;	/* module containing object, NULL in CpinAttr */
    struct VdiObject* top;	/* top module as given to "ictrl init" */
};


/* VdiHAttr is a name-value pair defining an attribute.  The optional format
 * string is used to define the appearance of visible attributes.
 * A pointer to a NULL-terminated array of VdiHAttr is returned
 * by the *Attr() functions.
 */
struct VdiHAttr {
    const char* name;
    const char* value;
    const char* format;		/* NULL or e.g. "(+#3a3a3a,#eecc99,1.5)" */
};

/* VdiHHi stores highlight data, if "hi" is 0, then all the members
 * except "segm" are ignored (no highlight data).
 * The pointer "segm" is NULL or - only for net objects -
 * point to the head of a single-linked list of VdiHSegmHi.  Each
 * VdiHSegmHi stores net-segment highlight data - the next-pointer is
 * "h.segm" - the last VdiHSegmHi has h.segm == NULL.
 */
struct VdiHHi {
    unsigned char hi;		/* flag 0/1: 0 means no highlight */
    unsigned char fill;		/* flag 0/1: 0 means no bg highlight filling */
    unsigned char color;	/* highlight color   0..99 */
    unsigned char width;	/* highlight width   0...n */
    unsigned char style;	/* style: 0(default),
				 *	  1(solid):	___________________
				 *	  2(dashed):	_______   _______
				 *	  3(dashed2):	_______  _  _______
				 *	  4(dashed50):	_____     _____
				 *	  5(dotted):	_     _     _     _
				 */
    unsigned short prio;	/* highlight priority 0..65535, default=0 */
    short         sublist;	/* highlight sublist 0..99, global list: -1 */

    struct VdiHSegmHi* segm;	/* NULL or net segment highlight info */
};

struct VdiHSegmHi {
    struct VdiHHi h;
    /* define net segment connection points - same as in "conIterNet" */
    struct {
	int               type;		/* as returned by Ccomp */
	struct VdiObject* comp;		/* as returned by Ccomp */
	const char*       pinname;	/* as returned by Cpinname
					 * or by Cpbusname for NetVector
					 * (planned for future use)
					 */
    } con[2];
};

/* ===========================================================================
 * Define Flags
 * ===========================================================================
 */
enum VdiInstFlags {
    VdiInstFAutohide = 0x00001,  /* autohide pin at this instance */
    VdiInstFNAutohide= 0x00002,  /* never autohide pins at this instance */
    VdiInstFFold     = 0x00004,  /* at HIER-instance set the fold flag */
    VdiInstFUnfold   = 0x00008,  /* at HIER-instance set the unfold flag */
    VdiInstFPinHide  = 0x00010,  /* has pins with VdiPortFHideMask */
    VdiInstFTarget   = 0x00020,  /* used by ictrl addcone -targetFlaggedInst */
    VdiInstFExclude  = 0x04000,  /* used by ictrl addcone -excludeFlaggedInst */
    VdiInstFFeedthru = 0x00040,  /* used by ictrl more: 2-pin feed thru prim */
    VdiInstFGBundled = 0x00080,  /* internal use: instance bundled by glayer */

    /* transistor-level device flags and masks */
    VdiInstFDevice   = 0x00100,
    VdiInstFPolar    = 0x00200,
    VdiInstFTransfer = 0x00400,
    VdiInstFNoTransfer=0x00800,
    VdiInstFTransMask= (VdiInstFTransfer|VdiInstFNoTransfer),
    VdiInstFNoFlow   = 0x01000,
    VdiInstFWeakFlow = 0x02000,
    VdiInstFFullFlow = (VdiInstFNoFlow|VdiInstFWeakFlow),
    VdiInstFFlowMask = (VdiInstFNoFlow|VdiInstFWeakFlow),

    /* orientation flags and masks */
    VdiInstMY        = 0x10000,  /* Negate X (-x,y) */
    VdiInstMX        = 0x20000,  /* Negate Y (x,-y) */
    VdiInstMYR90     = 0x40000,  /* Swap x/y after mirroring (y,x) */
    VdiInstR90       = (VdiInstMY|VdiInstMYR90),
    VdiInstR270      = (VdiInstMX|VdiInstMYR90),
    VdiInstR180      = (VdiInstMX|VdiInstMY),
    VdiInstMXR90     = (VdiInstMY|VdiInstMX|VdiInstMYR90),
    VdiInstOrientMask= (VdiInstMY|VdiInstMX|VdiInstMYR90)
};
enum VdiNetFlags {
    VdiNetFAny       = 0x00000,

    /* P/G related flags */
    VdiNetFPower     = 0x00001,  /* -power    */
    VdiNetFGround    = 0x00002,  /* -ground   */
    VdiNetFNegPower  = 0x00004,  /* -negpower */
    VdiNetFVSource   = 0x00008,  /* -vsource (a routed P/G net) */

    /* routing related flags */
    VdiNetFConst     = 0x00010,  /* connections are relevant for showvconn */
    VdiNetFHide      = 0x00020,  /* -hide */
    VdiNetFDontHide  = 0x00040,  /* -donthide (never autohide this net) */
    VdiNetFPriority  = 0x00080,  /* -priority */
    VdiNetFFly       = 0x00100,  /* -fly */

    VdiNetFHideMask  = (VdiNetFHide|VdiNetFDontHide),
    VdiNetFNoRouteMask = (VdiNetFPower|VdiNetFGround|VdiNetFNegPower|
			  VdiNetFHide|VdiNetFConst),

    VdiNetFTarget    = 0x00200,  /* used by ictrl addcone -targetFlaggedNet */
    VdiNetFExclude   = 0x00400,  /* used by ictrl addcone -excludeFlaggedNet */
    VdiNetFVector    = 0x00000   /* reserved for future support of netVector */
};
enum VdiPortFlags {
    VdiPortFUnknown  = 0x00000,                   /* unknown direction */
    VdiPortFOut      = 0x00001,                   /* output port */
    VdiPortFIn       = 0x00002,                   /* input port */
    VdiPortFInOut    = (VdiPortFOut|VdiPortFIn),  /* bi-directional port */

    VdiPortFNeg      = 0x00010,
    VdiPortFTop      = 0x00020,
    VdiPortFBot      = 0x00040,
    VdiPortFLeft     = 0x00080,
    VdiPortFRight    = 0x00100,
    VdiPortFClk      = 0x00200,
    VdiPortFFunc     = (VdiPortFNeg|VdiPortFTop|VdiPortFBot|
                        VdiPortFLeft|VdiPortFRight|VdiPortFClk),

    /* the following flags are per-instance flags, they may differ between
     * different instances of the same Primitive or Module
     */
    VdiPortFHide     = 0x01000,
    VdiPortFHideStub = 0x02000,
    VdiPortFDontHide = 0x04000,
    VdiPortFHideMask = (VdiPortFHide|VdiPortFHideStub|VdiPortFDontHide)
};

/* ===========================================================================
 * Define Instance Type (Primitive Function)
 * ===========================================================================
 */
enum VdiInstType {
    /* Gate Level Primitives */
    VdiInstTUNKNOWN,       /* Unknown gate - use symlib */
    VdiInstTAND,           /* Is an AND gate (1 out, n inputs) */
    VdiInstTNAND,          /* Is a NAND gate (1 out, n inputs) */
    VdiInstTOR,            /* Is an  OR gate (1 out, n inputs) */
    VdiInstTNOR,           /* Is a  NOR gate (1 out, n inputs) */
    VdiInstTXOR,           /* Is a  XOR gate (1 out, n inputs) */
    VdiInstTXNOR,          /* Is a XNOR gate (1 out, n inputs) */
    VdiInstTMUX,           /* Is a  MUX gate (1 out, n inputs, n select) */
    VdiInstTDECODER,       /* Is a DECODER (n outs, n inputs, n top/bot) */
    VdiInstTLATCH,         /* Is a LATCH (<=2 out, <=2 in, <=1 clk, 1 top/bot)*/
    VdiInstTAO,            /* Is an And/Or combi gate (1 out, n ins, param) */
    VdiInstTOA,            /* Is an Or/And combi gate (1 out, n ins, param) */
    VdiInstTAX,            /* Is an And/Xor combi gate (1 out, n ins, param) */
    VdiInstTOX,            /* Is an Or/Xor  combi gate (1 out, n ins, param) */
    VdiInstTXA,            /* Is a  Xor/And combi gate (1 out, n ins, param) */
    VdiInstTXO,            /* Is a  Xor/Or  combi gate (1 out, n ins, param) */
    VdiInstTPSWITCH,       /* Is a circular symbol w/cross, similar to RTL */
    VdiInstTRTL,           /* Is a circular symbol   (n outs, n ins, param) */
    VdiInstTALU,           /* Is an adder-like symbol (1 out, 2 ins, param) */
    VdiInstTPLA,           /* Is a Progr. Logic Array (1 out, >=2 ins, param) */
    VdiInstTCLOUD,         /* Is a cloud shape (n left/right/top/bot) */
    VdiInstTGEN,           /* Is a "generic" (n out, n inputs, n clock) */
    VdiInstTCONCAT,        /* Is a Ripper symbol (n outs, n ins) */

    VdiInstTBUF,           /* Is a Verilog buffer gate (1 in, n outputs) */
    VdiInstTBUFIF0,        /* Is a Verilog bufif0 gate (2 in, n outputs) */
    VdiInstTBUFIF1,        /* Is a Verilog bufif1 gate (2 in, n outputs) */
    VdiInstTINV,           /* Is a Verilog not    gate (1 in, n outputs) */
    VdiInstTINVIF0,        /* Is a Verilog notif0 gate (2 in, n outputs) */
    VdiInstTINVIF1,        /* Is a Verilog notif1 gate (2 in, n outputs) */
    VdiInstTPULLUP,        /* Is a Verilog pullup   gate (1 output) */
    VdiInstTPULLDOWN,      /* Is a Verilog pulldown gate (1 output) */

    /* T-engine Primitives (Transistor Level) */
    VdiInstTNMOS,          /* Is a Verilog nmos transistor (3/4 pins) */
    VdiInstTPMOS,          /* Is a Verilog pmos transistor (3/4 pins) */
    VdiInstTTRAN,          /* Is a Verilog tran gate (2 pins) */
    VdiInstTTRANIF0,       /* Is a Verilog tranif0 gate (3 pins) */
    VdiInstTTRANIF1,       /* Is a Verilog tranif1 gate (3 pins) */
    VdiInstTNPN,           /* Is a Bipolar NPN transistor (3/4 pins) */
    VdiInstTPNP,           /* Is a Bipolar NPN transistor (3/4 pins) */
    VdiInstTRES,           /* Is a Resistor    (2 pins, +n bulks) */
    VdiInstTCAP,           /* Is a Capacitor   (2 pins, +n bulks) */
    VdiInstTDIODE,         /* Is a Diode       (2 pins, +n bulks) */
    VdiInstTZDIODE,        /* Is a Zener-Diode (2 pins, +n bulks) */
    VdiInstTINDUCTOR,      /* Is a Inductor    (2 pins, +n bulks) */
    VdiInstTSWITCH,        /* Is a Switch (2 or 4 pins) */
    VdiInstTVSOURCE,       /* Is a Voltage source (2 or 4 pins) */
    VdiInstTISOURCE,       /* Is a Current source (2 or 4 pins) */
    VdiInstTTRANSLINE,     /* Is a transmission line (>=4 pins) */
    VdiInstTUDRCLINE,      /* Is a udrc line (3 pins) */
    VdiInstTAMP,           /* Is a amp (5 or 7 pins) */
    VdiInstTJJ,            /* Is a Josephson Junction (2 pins) */
    VdiInstTCMOS,          /* Is a nmos-pmos transistor pair */
    VdiInstTNMOSM,         /* Is a multi-gate, multi-bulk nmos transistor */
    VdiInstTPMOSM,         /* Is a multi-gate, multi-bulk pmos transistor */
    VdiInstTNPNM,          /* Is a multi-base/-bulk Bipolar NPN transistor */
    VdiInstTPNPM,          /* Is a multi-base/-bulk Bipolar PNP transistor */

    /* E-engine Primitives (for E/E architecture) */
    VdiInstTECU,           /* Is an Electronic Control Unit */
    VdiInstTINLINER,       /* Is an Inliner (connector unit) */
    VdiInstTSPLICE,        /* Is a  Splice point */
    VdiInstTEYELET,        /* Is an Eyelet point */

    VdiInstTLAST    
};


/* ===========================================================================
 * Temp define short names
 * ===========================================================================
 */
#define Obj   struct VdiObject
#define HObj  const struct VdiHObject
#define VDB   struct VdiDB
#define ATTR  const struct VdiHAttr
#define Iter  struct VdiIter
#define Citer struct VdiCiter

#define InstFlags enum VdiInstFlags
#define NetFlags  enum VdiNetFlags
#define PortFlags enum VdiPortFlags
#define InstType  enum VdiInstType


/* ===========================================================================
 * DataBase Access Functions
 * ===========================================================================
 */
struct VdiAccessFunc {

	/* ===================================================================
	 * Get Version of the interface and the implementation
	 */
	int  	     (*vdiVersion)(VDB*);	/* return enum VdiVersion */
	const char*  (*implVersion)(VDB*);	/* return e.g. "1.0 zdb" */

	/* ===================================================================
	 * Search object by name - return Obj* or if not found, return NULL.
	 * modFind searches on top-level, all the others search on Module-level
	 * for Instances/Ports/PortBusses/Nets/NetBundles respectively.
	 */
	Obj*  (*modFind)(VDB*, const char* name, const char* vname);
	Obj* (*instFind)(VDB*, const char* name, Obj* mod);
	Obj* (*portFind)(VDB*, const char* name, Obj* mod);
	Obj* (*pbusFind)(VDB*, const char* name, Obj* mod);
	Obj*  (*netFind)(VDB*, const char* name, Obj* mod);
	Obj* (*nbunFind)(VDB*, const char* name, Obj* mod);

	/* ===================================================================
	 * Iterator - loop over containment.  Imore returns 0 or 1, Inext
	 * switches to the next element and Iobj returns the current element.
	 */
	int   (*Imore)(const Iter*);
	void  (*Inext)(Iter*);
	Obj*  (*Iobj)(const Iter*);

	/* ===================================================================
	 * These functions create and initialize containment iterators;
	 * "freeIter" destroys them.
	 */
	Iter*  (*modIter)(VDB*);
	Iter* (*instIter)(VDB*, Obj* mod);
	Iter* (*portIter)(VDB*, Obj* mod);
	Iter* (*pbusIter)(VDB*, Obj* mod);
	Iter*  (*netIter)(VDB*, Obj* mod);
	Iter* (*nbunIter)(VDB*, Obj* mod);
	Iter*  (*subIter)(VDB*, Obj* mod, Obj* netbundle); /* not containment */
	void  (*freeIter)(VDB*, Iter*);

	/* ===================================================================
	 * Return object count in module
	 * cnt[0..4] #ports, #portBuses, #insts, #nets, #netBundles
	 */
	void  (*moduleObjCount)(VDB*, Obj* mod, long cnt[5]);

	/* ===================================================================
	 * Get object name
	 */
	const char*  (*modName)(VDB*, Obj* mod, const char** vname);
	const char* (*instName)(VDB*, Obj* mod, Obj* inst);
	const char* (*portName)(VDB*, Obj* mod, Obj* port);
	const char* (*pbusName)(VDB*, Obj* mod, Obj* portbus);
	const char*  (*netName)(VDB*, Obj* mod, Obj* net);
	const char* (*nbunName)(VDB*, Obj* mod, Obj* nbun);

	/* ===================================================================
	 * Get attributes and highlight info for the object; the returned
	 * VdiHAttr* must point to a NULL-terminated array of VdiHAttr; the
	 * given VdiHHi can be filled with highlight data (Nlview will
	 * clear VdiHHi.hi before calling the *Attr() function, that means,
	 * the *Attr() can just ignore the VdiHHi* if there is no highlight
	 * data to return.  The instAttr() function should set bits in "fl"
	 * as described above in the big comment section.
	 */
	ATTR* (*instAttr)(VDB*, HObj* inst, struct VdiHHi*, int* fl);
	ATTR* (*portAttr)(VDB*, HObj* port, struct VdiHHi*);
	ATTR* (*pbusAttr)(VDB*, HObj* pbus, struct VdiHHi*);
	ATTR*  (*netAttr)(VDB*, HObj* net,  struct VdiHHi*);
	ATTR* (*nbunAttr)(VDB*, HObj* nbun, struct VdiHHi*);

	/* ===================================================================
	 * Get object information - downModule and primName are described
	 * in comments above.  primType returns one of Nlview's built-in
	 * symbol types (e.g. NAND, INV, MUX, etc); symbol types like RTL
	 * additionally need a parameter that must be stored in "param"
	 * (if param is not NULL).
	 * modType does the same for Modules (usually returning VdiInstTUNKNOWN,
	 * making Nlview to display a BOX or HIERBOX or customer shapes from
	 * the -symlib file, but modType can make Nlview to display built-in
	 * symbols like NAND or HIERNAND for some Modules).
	 * The "symdef" argument is an optional return value that may contain
	 * an in-line definition of a symbol shape to use (maximum priority)
	 * (if symdef is not NULL).
  	 * primFlags returns transistor-level device flags the are common to
	 * all instances of the same primitive.
  	 * For primitives instFlags must return a super set of flags from
	 * primFlags; unlike primFlags instFlags are on a per instance basis.
	 * Instances with same primName must return the same primType,
	 * the same primFlags and identical conIterInst (identical footprint).
	 * The "symprops" argument is an optional return value that may
	 * define symbol properties (if symprops is not NULL).
	 */
	Obj*        (*downModule)(VDB*, Obj* mod, Obj* inst);
	const char* (*primName)(  VDB*, Obj* mod, Obj* inst,const char** vname);
	InstType    (*modType)(   VDB*, Obj* mod,            char param[16],
					const char** symdef, ATTR** symprops);
	InstType    (*primType)(  VDB*, Obj* mod, Obj* inst, char param[16],
					const char** symdef, ATTR** symprops);
	InstFlags   (*primFlags)( VDB*, Obj* mod, Obj* inst);
	InstFlags   (*instFlags)( VDB*, Obj* mod, Obj* inst);
	float       (*instValue)( VDB*, Obj* mod, Obj* inst);
	PortFlags   (*portFlags)( VDB*, Obj* mod, Obj* port);
	NetFlags    (*netFlags)(  VDB*, Obj* mod, Obj* net);

	/* ===================================================================
	 * Citer - Loop over the connectivity and Instance-pins.
	 * This complex iterator is described above.  CpinAttr returns
	 * attributes or highlight information identically to the *Attr()
	 * functions above.
	 * The functions conPort/conInst return NULL or the connected
	 * Net.  The conInst and the Citer's pinnumber are only needed for
	 * transistor-level devices (inst with flag VdiInstFDevice).
	 */
	int          (*Cmore)(const Citer*);
	void         (*Cnext)(Citer*);
	Obj*         (*Cnet) (const Citer*);
	const char*  (*CnetOOM)(const Citer*, HObj* inst);
	Obj*         (*Ccomp)(const Citer*, int* type);
	const char*  (*CcompOOM)(const Citer*);
	int          (*Cpinnumber)(const Citer*);
	const char*  (*Cpinname)(const Citer*);
	const char*  (*Cpbusname)(const Citer*);
	PortFlags    (*CportFlags)(const Citer*);
	Citer*       (*Cmembers)(const Citer*);
	ATTR*        (*CpinAttr)(Citer*, HObj*, struct VdiHHi*);

	Citer* (*conIterNet)( VDB*, Obj* mod, Obj* net);
	Citer* (*conIterNetOOM)(VDB*, HObj* net);
	Citer* (*conIterInst)(VDB*, Obj* mod, Obj* inst);
	Citer* (*conIterInstArcs)(VDB*, Obj* mod, Obj* inst, const Citer* cin);
	Citer* (*conIterPBus)(VDB*, Obj* mod, Obj* portbus);
	void     (*freeCiter)(VDB*, Citer*);
	Obj*       (*conPort)(VDB*, Obj* mod, Obj* port);
	Obj*       (*conInst)(VDB*, Obj* mod, Obj* inst, int pinnumber);

	/* ===================================================================
	 * The functions portType/pbusType/pgnetType are optional. If not
	 * implemented, their entry in VdiAccessFunc must be NULL. Those
	 * functions can override the built-in port/portBus symbols as well
	 * as the built-in power/ground symbols.  They may return
	 * (1) symname and symviewname - to make Nlview looking up the
	 * external symlib file or
	 * (2) symname, symviewname and symdef to define a new symbol shape
	 * under the given name for the given port/portBus or PG Net.
	 */
	void      (*portType)(VDB*, Obj* mod, Obj* port,
			      const char** symname,
			      const char** symviewname,
			      const char** symdef);
	void      (*pbusType)(VDB*, Obj* mod, Obj* pbus,
			      const char** symname,
			      const char** symviewname,
			      const char** symdef);
	void      (*pgnetType)(VDB*, Obj* mod, Obj* net,
			       const char** symname,
			       const char** symviewname,
			       const char** symdef);
};

/* ===========================================================================
 * undefine short names
 * ===========================================================================
 */
#undef Obj
#undef HObj
#undef VDB
#undef ATTR
#undef Iter
#undef Citer

#undef InstFlags
#undef NetFlags
#undef PortFlags
#undef InstType
#endif
