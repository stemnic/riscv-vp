/*  gskill.h 1.24 2018/09/18

    Copyright 2007-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	SKILL export

    Author:
        Heinz Bruederlin

    Description:
	Gskill exports Nlview schematics into skill ready for import into CDS.
	This is an example for the usage of the Graphics Export Interface (GEI).
	The current implementation is in *EXPERIMENTAL* state and may
	not always produce correct connectivity or graphics.

	There are four API functions defined:
	    GexSkillStart()
		initializes skill export.
		The meanings of the arguments are:
		libName	    - all cells are generated in a library
			      named by this argument.
		libDir	    - the library is generated in this directory.
		userProlog  
		userEpilog  - if not NULL the contents of the named files is
			      copied verbatim into the generated skill output.
		scale	    - scales the whole graphics. 
		metric	    - set userUnits and DbPerUU for all generated
			      symbols and schematics.
		overwrite   - if true, existing cells get overwritten.
			      For best results an empty library should be used
			      anyway.
		spiceUnits  - if true, spice units like (k,M etc.) in
			      expressions are converted.
		noSym	    - if true, no symbols are generated.  
		multiFile   - if true, multiple files, containing symbols,
			      modules, utility code and one 'main' file, are
			      created. The 'main' file contains 'load' commands
			      to load all generated files.
		netLabel    - if true, visible net labels at each segment are
			      created.
		noRangeLabel- if true, port labels are create without a bus
			      range.
		instSuffix  - if not NULL, the given string gets prepended
			      to all instances' names.
	
	    GexSkillModule()
		exports one module from given GEI pointer into the
		output file. The additional params parameter is an array
		of parameter names which are not global and need 'pPar()'
		quoting in expressions.

	    GexSkillEnd()
		closes output file and cleans up used data.

	    GexSkillLastErr()
		if one of the three functions above returns false to indicate
		an error, this functions returns a detailed error message.
		
        The output file can be loaded directly into CDS with the 'load()'
        command:
            load("outfile.il")
        If there were errors the global variable "errorMsg" can be checked
        for a message string after completion of the 'load()' command.
        An additional 'schCheckHier()' will create the complete connectivity.

        The generated skill contains five sections:
            - an initialization of constants, which may be changed manually
            - a prolog with utility functions and globals
	    - an optional user defined prolog
            - the symbol and schematic generation
	    - an optional user defined epilog

        The initialization section sets the name and the directory of the
        local cadence library where all generated symbols and schematics
        go to. The scaling and units are defined here. If GeVerbose is set
        to 1 or 2 verbose output is generated on the console during the
        import.

        The export creates the required index and msymbols if there are multiple
        pages. Then each page is created.
        During the creation of the instances of a page the required symbols are
        created on the fly. For the special GeiSymTHier symbols only the
        graphics are generated as sheet comment graphics and the pin pairs are
        used to create patchCord instances which alias the connected nets.
        The name of connected net or the concatenated name of the connected bus
        is stored in the 'connTable' hash table keyed by the pin coordinates.

        For nets and buses all segments are created with their name attached
        as a label. The name of buses is created by concatenating the single
        bit names with ","  and 'compressing' bits which are in sequence.
        For segments connected to a pinBus the sequence of the single bits
        is determined by looking at the names stored in the 'connTable'
        hash table. If there is a power/ground symbol connected to a net,
        the @netName is used as a name for the net.

        For each port symbol connected to net a sheetTerm is created.
        If a power/ground symbol is connected to a interface net (indicated by
        a connected port symbol) the power/ground symbol is converted to
        a sheetTerm to avoid check errors later.

        To map symbols to already existing cadence symbols the special
        attributes "@libName" and "@cellName" can be attached to a symbol
	or its instance. If present no symbol in the local library will
	be created. Instead the attribute value will be used as the name
	of a library containing the wanted symbol.
	Because it is used as a direct replacement the 'footprint' of
        the symbol must match the (scaled) symbol used in Nlview during the
        page generation. Because of the special handling of power/ground 
	symbols, the "@libName" attribute for these instances is searched at
	the connected net.

        The mapping and quoting of the created instance property values can
        be controlled by attaching additional '@type_<name>' attributes. Where
        '<name>' is the name of the attribute which should be mapped.
        They can have one of the following values:
            "string"       : the generated value is quoted with '"'
            "float", "int" : the generated value is treated as a
                             numeric value without quotes.
                             If the spiceUnit option is set, spice unit strings
                             get converted to cadence conform suffixes.
            "bulk"         : the value is treated as a net name and gets mapped
                             like normal net names.
	If there are any parameters in attribute expressions, which need to 
	be 'quoted' in 'pPar()' calls, their names need to be added to the
	params array of function GexSkillModule().

    Limitations:
        The following Nlview settings and limitations are needed for a
	successful skill export:
            - Flag hidestub must not be used at instances of HIER symbols.
            - autoalign at symbols should be turned off.
            - Flag autohide should not be used on @libName symbols.
            - Property autobundle should be switched off (0).
            - Property buswireexcess should be set to 0.
            - Property geitarget should be set to 3.
            - Direct connection of two port instances are not allowed in CDS.

    ===========================================================================
*/

#ifndef gskill_h
#define gskill_h

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

DLL int/*bool*/ GexSkillStart(const char* skillFname,
			  const char* libName,
			  const char* libDir,
			  const char* prolog,
			  const char* epilog,
			  double scale,
			  int/*bool*/ metric,
			  int/*bool*/ overwrite,
			  int/*bool*/ spiceUnits,
			  int/*bool*/ noSym,
			  int/*bool*/ multiFile,
			  int/*bool*/ netLabel,
			  int/*bool*/ noRangeLabel,
			  const char* instSuffix);
DLL int/*bool*/ GexSkillModule(const char* module,
		   const struct GeiAccessFunc* gei, const char* params[]);
DLL int/*bool*/ GexSkillEnd  (void);
DLL const char* GexSkillLastErr(void);

#ifdef __cplusplus
}
#endif

#endif	/* gskill_h */
