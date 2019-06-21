/*  gdump.h 1.26 2012/04/03

    Copyright 2003-2012 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	Dump Data from Graphics Export Interface

    Author:
	Lothar Linhard

    Description:
	Gdump demonstrates the use of Nlview's C-level
	Graphics Export Interface (GEI), by dumping the
	schematic (graphic) data into a human readable
	ASCII file.

	It uses GEI's various iterators defined in gei.h
	to access the schematic data.  Accessing the GEI
	must be embraced by the following API commands:

	begGraphicExp
	    access Nlview' schematic data tru GEI
	    by calling iterators etc from gei.h...
	endGraphicExp

	"begGraphicExp" returns a string representation of a pointer
	to the function vector (const struct GeiAccessFunc*) which is needed
	to call GEI functions from C. The string needs to be converted
	into a pointer (we assume 'long' is enough to temporarily
	store a pointer for conversion[*]). Examples:

	Tcl/Tk:
	  long gei;
	  const struct GeiAccessFunc* g;
	  const char* objv[3] = "0x12345678";
	  if (Tcl_GetLongFromObj(interp,objv[3],&gei)!=TCL_OK) return TCL_ERROR;
	  g = (const struct GeiAccessFunc*)gei;
	  
	Qt:
	  bool ok;
	  const struct GeiAccessFunc* g;
	  QString geistr = QString("0x12345678");
	  g = (const struct GeiAccessFunc*)geistr.toULong(&ok, 0);
	  if (!ok) return false;
	  
	Generic C code:
	  const struct GeiAccessFunc* g;
	  objv[3] = "0x12345678";
	  g = (const struct GeiAccessFunc*)strtoul(objv[3], 0, 0);
	  

	[*] Exception Windows 64-bits [WIN64]: long is 32-bits wide while
	pointers are 64-bits wide. In this case, you should use the following
	WIN64 code:

	Tcl/Tk 8.4 - use Tcl_WideInt, it is guaranteed to be 64-bits wide:
	  Tcl_WideInt gei;
	  const struct GeiAccessFunc* g;
	  const char* objv[3] = "0x12345678";
	  if (Tcl_GetWideIntFromObj(interp, objv[3], &gei) != TCL_OK)
	      return TCL_ERROR;
	  g = (const struct GeiAccessFunc*)(unsigned __int64)gei;

	Qt/Generic C code - use sscanf with appropriate format specifier:
	  unsigned __int64 gei;
	  const struct GeiAccessFunc* g;
	  const char* objv[3] = "0x123456789abcdef";
	  if (sscanf(objv[3], "0x%I64x", &gei) != 1) return;
	  g = (const struct GeiAccessFunc*)gei;


    The output file format:
	The output (dump file) includes graphic data plus connectivity
	(netlist) data.  The connectivity is reported twice:
	* at each pin (pin-->net)		- except GexDumpFNoJoin is given
	* at each net (net-->list of pins)	- except GexDumpFNoConn is given

	Additionally, the connectivity as returned by IjoinInfo() is
	verified against IconnInfo().  This slow O(n*n) check is disabled
 	if GexDumpFNoVerify is given.

	The GexDumpFInstPrefix additionally prefixes each instance line
	with a "inst" - mostly to be able to "grep" the instances easily
	from the dump file.

	The connectivity data includes both the bus level (bus name plus
	subbit index starting at 0) and the subbit level (sub-name).
	Note: Nlview has no restriction or assumption about bus name
	syntax.  Examples for bus-level syntax are:
	    #0 of netBundle OP/ALU (OP/ALU(4))
	    #3 of netBundle OP/ALU (OP/ALU(5))
	    #7 of pinBus "ALU" (ALU(7)) at HIER[OPRLOGICv] OP
	    #1 of portBus (AL(1)) OUT[OutConn] AL
	The 1st means: bit 0 at net bus "OP/ALU" - also named "OP/ALU(4)"
	The 2nd means: bit 3 at net bus "OP/ALU" - also named "OP/ALU(5)"
	The 3rd means: bit 7 at pin bus "ALU"    - also named "ALU(7)"
	The 4th means: bit 1 at port bus "AL"    - also named "AL(1)"
	The 4th is an exception, because the portBus is also an instance
	of the "OutConn" symbol, called "AL".

	The instances are reported as symbol type, symbol name and instance
	name.  Examples:
	    [MUX2TO1v] OP/REG/U20
	    HIER[REG8BITSv] OP/REG
	    OUT[OutConn] AL
	The 1st means: ordinary instance of "MUX2TO1v" called "OP/REG/U20"
	The 2nd means: GeiSymTHier instance of "REG8BITSv" called "OP/REG"
	The 3rd means: GeiSymTOUTPORT instance of "OutConn" called "AL" (IOport)

    ===========================================================================
*/

#ifndef gdump_h
#define gdump_h

struct GeiAccessFunc;

#ifdef __cplusplus
extern "C" {
#endif

#if   (defined(_WIN32) || defined(__CYGWIN32__)) && defined(NLVIEW_DLLEXPORT)
#define DLL __declspec(dllexport)
#elif (defined(_WIN32) || defined(__CYGWIN32__)) && defined(NLVIEW_DLLIMPORT)
#define DLL __declspec(dllimport)
#else
#define DLL /**/
#endif

enum GexDumpFlags {	GexDumpFNoJoin   = 1,
			GexDumpFNoConn   = 2,
			GexDumpFNoShape  = 4,
			GexDumpFNoPlace  = 8,
			GexDumpFNoWire   = 16,
			GexDumpFNoVerify = 32,
			GexDumpFInstPrefix = 64,
			GexDumpFCompass    = 128 };

/* ===========================================================================
 * All functions return true on success or false on error. If they
 * return false, then GexDumpLastErr() will return the error message.
 * ===========================================================================
 */
DLL int/*bool*/ GexDumpStart(const char* dumpfile, unsigned flags);
DLL int/*bool*/ GexDumpModule(const char* module,
		      const struct GeiAccessFunc* gei, const char** params);
DLL int/*bool*/ GexDumpEnd(void);
DLL const char* GexDumpLastErr(void);

#ifdef __cplusplus
}
#endif

#endif	/* gdump_h */
