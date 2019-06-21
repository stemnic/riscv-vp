/*  data.h 1.28 2018/09/18
    Copyright 2003-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	Test DataBase objects
    ===========================================================================
*/


#ifndef data_h
#define data_h


#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================================================
 * Definition of the Cell interface
 * ===========================================================================
 */
typedef unsigned short	PortFlags;	/* Copy of vdi.h */
#define PortFUnknown	0x0	/* Port direction: Unknown */
#define PortFOut	0x01	/* Port direction: Output */
#define PortFIn		0x02	/* Port direction: Input */
#define PortFInOut	0x03	/* Port direction: Bidir */

#define PortFNeg	0x100	/* primitive pin function: pin is inverted */
#define PortFClock	0x200	/* primitive pin function: pin is clocked  */

#define PortFTop	0x1000	/* display port at top */
#define PortFBot	0x2000	/* display port at bottom */
#define PortFLeft	0x4000	/* display port at left side */
#define PortFRight	0x8000	/* display port at right side*/
#define PortFDIR	(PortFTop|PortFBot|PortFLeft|PortFRight)  /* mask */

#define PortFHide	0x010	/* instance pin is hidden (not routed to) */
#define PortFHideStub	0x020	/* instance pin has 0-length pin-stub */
#define PortFDontHide	0x040	/* instance pin excluded from pin-autohide */
#define PortFHideMask	(PortFHide|PortFHideStub|PortFDontHide)	  /* mask */

typedef unsigned short CellFlags;
#define CellFModule	0x001	/* This is a Module (not a Primitive) */
#define CellFWeakFlow	0x080 	/* Set for Weakflow transistor-level devices */
#define CellFFeedthru	0x100 	/* This cell is suitable to trace "thru" */

typedef struct Port {		/* Single Bit Interface Port */
    const char*	name;		/* Port name */
    PortFlags	flags;          /* Port direction */
    int		busMember;	/* Member of PortBus or -1 */
    struct Net*	net;		/* Inside net or NULL (only for Module Ports) */
    const char** attrList;	/* Dynamic list of name=value attributes */
} Port;

typedef struct PortBus {	/* Interface PortBus - a group of Ports */
    const char*	name;		/* PortBus name */
    PortFlags	flags;          /* Port direction */
    int		first;		/* First Member: Cell.portList[first] or -1 */
    int		last;		/* Last Member. Width = abs(last-first)+1 */
    const char** attrList;	/* Dynamic list of name=value attributes */
} PortBus;

typedef enum PrimitiveFunc {

    /* Gate Level Primitives */
    PrimFUNKNOWN = 1,       /* Unknown Gate Function */

    PrimFAND,               /* Is an AND gate (1 out, n inputs) */
    PrimFNAND,              /* Is a NAND gate (1 out, n inputs) */
    PrimFOR,                /* Is an  OR gate (1 out, n inputs) */
    PrimFNOR,               /* Is a  NOR gate (1 out, n inputs) */
    PrimFXOR,               /* Is a  XOR gate (1 out, n inputs) */
    PrimFXNOR,              /* Is a XNOR gate (1 out, n inputs) */

    PrimFBUF,               /* Is a Verilog buffer gate (n outs, 1 in) */
    PrimFINV,               /* Is a Verilog not    gate (n outs, 1 in) */

    PrimFMUX,               /* Is a MUX gate   (1 out, 2 in, 1 select)   */
    PrimFADD,               /* Is a full adder (1 out, 1 co, 2 in, 1 ci) */

    PrimFDFF,               /* Is a D FlipFlip (out, d, clk, reset, set) */
    PrimFLATCH,             /* Is a D Latch    (out, d, en,  reset, set) */

    /* Tri State Primitives */
    PrimFBUFIF0,            /* Is a Verilog bufif0 gate (1 out, 1 in, 1 ctrl) */
    PrimFBUFIF1,            /* Is a Verilog bufif1 gate (1 out, 1 in, 1 ctrl) */
    PrimFINVIF0,            /* Is a Verilog notif0 gate (1 out, 1 in, 1 ctrl) */
    PrimFINVIF1,            /* Is a Verilog notif1 gate (1 out, 1 in, 1 ctrl) */
    PrimFTRAN,              /* Is a Verilog tran gate    (2 pins) */
    PrimFTRANIF0,           /* Is a Verilog tranif0 gate (2 pins, 1 ctrl) */
    PrimFTRANIF1,           /* Is a Verilog tranif1 gate (2 pins, 1 ctrl) */

    /* Transistor Level Primitives */
    PrimFNMOS,              /* Is a Verilog nmos transistor (3..4 pins) */
    PrimFPMOS,              /* Is a Verilog pmos transistor (3..4 pins) */
    PrimFCMOS,              /* Is a Verilog cmos transistor pair (4 pins) */
    PrimFNPN,               /* Is a Bipolar NPN transistor (3..4 pins) */
    PrimFPNP,               /* Is a Bipolar NPN transistor (3..4 pins) */
    PrimFRES,               /* Is a Resistor (2 pins) */
    PrimFCAP,               /* Is a Capacitance (2 pins) */
    PrimFINDUCTOR,          /* Is a Inductor (2 pins) */
    PrimFDIODE,             /* Is a Diode (2 pins) */
    PrimFZDIODE,            /* Is a ZDiode (2 pins) */
    PrimFSWITCH,            /* Is a Switch (2 or 4 pins) */
    PrimFVSOURCE,           /* Is a Voltage source (2 or 4 pins) */
    PrimFISOURCE,           /* Is a Current source (2 or 4 pins) */
    PrimFTRANSLINE,         /* Is a transition line (>=4 pins) */
    PrimFUDRCLINE,          /* Is a udrc line (3 pins) */
    PrimFAMP,               /* Is a amp (5 or 7 pins) */

    /* Gate Level Primitives with single-bit output and PortBus input */
    PrimFREDUCE_AND,        /* Is an AND-gate (1 out, 1 in bus) */
    PrimFREDUCE_NAND,       /* Is a NAND-gate (1 out, 1 in bus) */
    PrimFREDUCE_OR,         /* Is an  OR-gate (1 out, 1 in bus) */
    PrimFREDUCE_NOR,        /* Is a  NOR-gate (1 out, 1 in bus) */
    PrimFREDUCE_XOR,        /* Is a  XOR-gate (1 out, 1 in bus) */
    PrimFREDUCE_XNOR,       /* Is a XNOR-gate (1 out, 1 in bus) */

    /* Gate Level Primitives with PortBuses (all buses the same width) */
    PrimFWIDE_AND,          /* Is an AND-gate (1 out bus, n in bus) */
    PrimFWIDE_NAND,         /* Is a NAND-gate (1 out bus, n in bus) */
    PrimFWIDE_OR,           /* Is an OR-gate  (1 out bus, n in bus) */
    PrimFWIDE_NOR,          /* Is a  NOR-gate (1 out bus, n in bus) */
    PrimFWIDE_XOR,          /* Is a  XOR-gate (1 out bus, n in bus) */
    PrimFWIDE_XNOR,         /* Is a XNOR-gate (1 out bus, n in bus) */
    PrimFWIDE_BUF,          /* Is a BUFFER    (1 out bus, 1 in bus) */
    PrimFWIDE_INV,          /* Is a INVERTER  (1 out bus, 1 in bus) */
    PrimFWIDE_TRI,          /* Is a TRI-STATE (1 out bus, 1 in bus, 1 ctrl) */

    /* Gate Level Primitives with a mix of PortBuses of different sizes */
    PrimFWIDE_MUX,          /* Multiplexer    (1 out bus, 2 in bus, 1 select)*/
    PrimFDECODER,           /* Is a DECODER   (1 out bus, 1 in bus) */

    PrimFLAST
} PrimitiveFunc;

/* ===========================================================================
 * Definition of a Primitive Cell
 * ===========================================================================
 */
typedef struct Cell {
    const char*	name;		/* Cell name */
    CellFlags	flags;		/* Cell Flags */
    Port*	portList;	/* Dynamic list of single bit Ports - ordered */
    PortBus*	portBusList;	/* Dynamic list of PortBus - ordered, or NULL */
    const char** attrList;	/* Dynamic list of name=value attributes */
    PrimitiveFunc func;		/* Function */
} Cell;

typedef struct Primitive {
    Cell	  c;		/* Is a Cell */
} Primitive;

/* ===========================================================================
 * Definition of a Module
 * ===========================================================================
 */
typedef struct Module {
    Cell		c;		/* Is a Cell */
    struct Inst**	instList;	/* Dynamic list of Instances */
    struct Net**	netList;	/* Dynamic list of Nets */
    struct NetBus**	netBusList;	/* Dynamic list of Buses */
} Module;


/* ===========================================================================
 * Definition of Cell Instances
 *
 *  The instance pins refer to the Cell pins in the same order.
 *  Inst.pinCount is equal to zListLength(Inst.cellRef->portList).
 * ===========================================================================
 */
typedef struct Pin {		/* Single Bit Pin */
    struct Net*	net;            /* connected Net - may be NULL if unconnected */
    PortFlags	flags;		/* Pin Direction and other flags */
    const char** attrList;	/* Dynamic list of name=value attributes */
} Pin;

typedef unsigned short InstFlags;
#define InstFWeakFlow		0x0080 	/* Set for Weakflow transistor device */
#define InstFOrientR90		0x0100
#define InstFOrientR180		0x0200
#define InstFOrientR270		0x0400
#define InstFOrientMY		0x0800
#define InstFOrientMYR90	0x1000
#define InstFOrientMX		0x2000
#define InstFOrientMXR90	0x4000

typedef struct Inst {
    const char*	name;		/* Instance name */
    Cell*	cellRef;	/* Instance cell interface */
    const char** attrList;	/* Dynamic list of name=value attributes */
    InstFlags	flags;		/* Instance Flags */
    Pin*	pinList;	/* Dynamic list of Pins */
} Inst;


/* ===========================================================================
 * Definition of Net Connectivity
 *
 *  The Net's pinRefList refers to instance pins or to this module's
 *  Ports (if Pin.inst == NULL).
 * ===========================================================================
 */
typedef unsigned short	NetFlags;
#define NetFGlobal	0x0002	/* Global Net, connectivity by name */
#define NetFPower	0x0010	/* Power Net */
#define NetFGround	0x0020	/* Ground Net */
#define NetFNegPower	0x0040	/* Negative Power Net */
#define NetFConst	0x0080	/* Net contributes to a constant value */
#define NetFFly		0x0100	/* flyline routing */
#define NetFHide	0x0200	/* Prevent Net from being routed */
#define NetFDontHide	0x0400	/* Net must never be autohidden */
#define NetFTarget	0x0800	/* Net is a target for cone searching algos */
#define NetFVSource	0x1000	/* is a routed net, carrying a voltage */
#define NetFAutogen	0x2000	/* is an automatically generated net */

typedef struct PinRef {		/* Reference to an inst Pin */
    Inst*	inst;		/* Pointer to the instance or NULL */
    int		pinNo;		/* inst->pin[pinNo] or Cell.pin[pinNo] */
} PinRef;

typedef struct Net {
    const char*	name;           /* Net name */
    NetFlags	flags;		/* Net type and other flags */
    PinRef*	pinRefList;	/* Dynamic list defines Connectivity */
    const char** attrList;	/* Dynamic list of name=value attributes */
} Net;

typedef struct NetBus {
    const char*	name;           /* NetBus name */
    NetFlags	flags;		/* Net type and other flags */
    Net**	subnetList;	/* Dynamic list defines sub nets */
    const char** attrList;	/* Dynamic list of name=value attributes */
} NetBus;


/* ===========================================================================
 * Definition of the Data Base Root
 * ===========================================================================
 */
typedef struct DB {
    Module**	moduleList;	/* Dynamic list of Modules */
    Module**	topList;	/* Dynamic list of Top modules or NULL*/
    Primitive**	primList;	/* Dynamic list of Primitives */
    const char** attrList;	/* Dynamic list of name=value attributes */
} DB;


/* ===========================================================================
 * Definition Object Type 
 * ===========================================================================
 */
typedef unsigned char OidType;
enum {  OidBAD, OidModule, OidPrimitive, OidPort, OidPortBus, OidInst, OidPin,
        OidPinBus, OidNet, OidNetBus, OidSignal, OidNetSeg, OidLAST
};

/* ===========================================================================
 * Dynamic list
 *
 *  Struct members named "*List" point to a list of objects plus the
 *  list length (before the first array element). All Lists must be built up
 *  like AnyList.
 *  Eventually we should check the pointer offset with:
 *  assert( (((char*)NULL)-((char*)&(((struct AnyList*)NULL)->entry))) % 
 *          sizeof(int) == 0 )
 *  To calculate the byte offset of a struct member from the beginning of the
 *  struct, we could also use the offsetof macro from stddef.h
 *  (as part of the Standard C Library).
 * ===========================================================================
 */
struct AnyList {
    int length;
    void* entry[1];
};
#ifdef _WIN64
#define BYTEOFF ((__int64)&(((struct AnyList*)NULL)->entry))
#else
#define BYTEOFF ((long)&(((struct AnyList*)NULL)->entry))
#endif
#define zListLength(list)    (*(int*)(((char*)(list))-BYTEOFF))
#define zList2AnyList(list)  ((struct AnyList*)(((char*)(list))-BYTEOFF))


/* ===========================================================================
 * Global Data Base Root Instantiation, defined in
 *	data.c		- Define complete gate-level       TestDB.
 *	datatrans.c	- Define complete transistor-level TestDB.
 * ===========================================================================
 */


#ifdef __cplusplus
}
#endif


#endif /* data_h */
