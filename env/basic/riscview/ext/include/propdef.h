/*  propdef.h 1.129 2018/03/13

    Copyright 2000-2015 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    Description:
	Nlview configure properties - defined as cpp macros of the form:

	PROPDEF_Type(name,member,def,tko,tkc,ja)

    1. arg - property name;  "-" name is identical to tk configure opt.
    2. arg - nlvprop variable
    3. arg - default value string
    4. arg - tk configure db name
    5. arg - tk configure db class name
    6. arg - ja+mfc+win+qt set/get method name

    Examples:
	PROPDEF_Bool( "allpageconn",   allpageconn,  "true",
		     "allPageconn",    "AllPageconn", Allpageconn)
	PROPDEF_Color( "attrcolor",    attrcolor,    "#ffffff",
		      "attrColor",     "AttrColor",   Attrcolor)
	PROPDEF_Short( "boxfontsize",  boxfontsize,  "12",
		      "boxFontsize",   "BoxFontsize", Boxfontsize)

    To mark a configure property as deprecated, the 2nd arg (nlvprop variable)
    can be prefixed by obsolete_, e.g.

	PROPDEF_Float( "maxbuttonzoom", obsolete_maxbuttonzoom,   "6.4",
			"maxButtonZoom","MaxButtonZoom",Maxbuttonzoom)

    --> This will issue a warning (W35) every time it is accessed.

    Author:
	Lothar Linhard
    ===========================================================================
*/



PROPDEF_Short ( "actionpick", actionpick,	      "2",
    "actionPick", "ActionPick", Actionpick)

PROPDEF_Bool ( "allpageconn",       allpageconn,     "true", 
    "allPageconn",  "AllPageconn",  Allpageconn)

PROPDEF_Short ( "analoglayout",      analoglayout,   "0", 
    "analogLayout", "AnalogLayout", Analoglayout)

PROPDEF_Color( "attrcolor",         attrcolor,       "#ffffff", 
    "attrColor",    "AttrColor",    Attrcolor)

PROPDEF_Short( "attrfontsize",      attrfontsize,    "12", 
    "attrFontsize", "AttrFontsize", Attrfontsize)

PROPDEF_Short( "autobundle",        autobundle,      "0",
    "autoBundle",   "AutoBundle",   Autobundle)

PROPDEF_Short( "autohidestyle",     autohidestyle,   "0",
    "autohideStyle","AutohideStyle",Autohidestyle)

PROPDEF_Color( "backgroundcolor",   backgroundcolor, "#000000",
    "backgroundColor","BackgroundColor",Backgroundcolor)

PROPDEF_ColAr( "boxcolor0",         boxcolor[0],     "#00d1d1",
    "boxColor0",    "BoxColor",    Boxcolor0)

PROPDEF_ColAr( "boxcolor1",         boxcolor[1],     "#00d1d1",
    "boxColor1",    "BoxColor",    Boxcolor1)

PROPDEF_ColAr( "boxcolor2",         boxcolor[2],     "#00d1d1",
    "boxColor2",    "BoxColor",    Boxcolor2)

PROPDEF_ColAr( "boxcolor3",         boxcolor[3],     "#00d1d1",
    "boxColor3",    "BoxColor",    Boxcolor3)

#ifdef ALLPROPS		/* core property command wants them all */
PROPDEF_ColAr( "boxcolor4",         boxcolor[4],     "#00e6e6",
    "boxColor4",    "BoxColor",    Boxcolor4)
PROPDEF_ColAr( "boxcolor5",         boxcolor[5],     "#00fbfb",
    "boxColor5",    "BoxColor",    Boxcolor5)
PROPDEF_ColAr( "boxcolor6",         boxcolor[6],     "#11ffff",
    "boxColor6",    "BoxColor",    Boxcolor6)
PROPDEF_ColAr( "boxcolor7",         boxcolor[7],     "#26ffff",
    "boxColor7",    "BoxColor",    Boxcolor7)
PROPDEF_ColAr( "boxcolor8",         boxcolor[8],     "#3cffff",
    "boxColor8",    "BoxColor",    Boxcolor8)
PROPDEF_ColAr( "boxcolor9",         boxcolor[9],     "#51ffff",
    "boxColor9",    "BoxColor",    Boxcolor9)
PROPDEF_ColAr( "boxcolor10",        boxcolor[10],     "#66ffff",
    "boxColor10",    "BoxColor",   Boxcolor10)
PROPDEF_ColAr( "boxcolor11",        boxcolor[11],     "#7bffff",
    "boxColor11",    "BoxColor",   Boxcolor11)
PROPDEF_ColAr( "boxcolor12",        boxcolor[12],     "#91ffff",
    "boxColor12",    "BoxColor",   Boxcolor12)
PROPDEF_ColAr( "boxcolor13",        boxcolor[13],     "#a6ffff",
    "boxColor13",    "BoxColor",   Boxcolor13)
PROPDEF_ColAr( "boxcolor14",        boxcolor[14],     "#bbffff",
    "boxColor14",    "BoxColor",   Boxcolor14)
PROPDEF_ColAr( "boxcolor15",        boxcolor[15],     "#d0ffff",
    "boxColor15",    "BoxColor",   Boxcolor15)
#endif

PROPDEF_Short( "boxfontsize",       boxfontsize,     "12",
     "boxFontsize","BoxFontsize",   Boxfontsize)

PROPDEF_Short ( "boxhierpins",       boxhierpins,      "1",
    "boxHierpins",  "BoxHierpins",  Boxhierpins)

PROPDEF_Color( "boxinstcolor",      boxinstcolor,    "#8fffff",
    "boxinstColor", "BoxinstColor", Boxinstcolor)

PROPDEF_Short( "boxinstfontsize",   boxinstfontsize, "12",
    "boxinstFontsize","BoxinstFontsize",Boxinstfontsize)

PROPDEF_Short( "boxmaxwidth",       boxmaxwidth,     "1000",
    "boxmaxWidth",  "BoxmaxWidth",  Boxmaxwidth)

PROPDEF_Short( "boxmingap",         boxmingap,       "10",
    "boxminGap",    "BoxminGap",    Boxmingap)

PROPDEF_Short( "boxminheight",      boxminheight,    "20",
    "boxminHeight", "BoxminHeight", Boxminheight)

PROPDEF_Short( "boxminwidth",       boxminwidth,     "60",
    "boxminWidth",  "BoxminWidth",  Boxminwidth) 

PROPDEF_Color( "boxpincolor",       boxpincolor,     "#00d1eb",
    "boxpinColor",  "BoxpinColor",  Boxpincolor)

PROPDEF_Short( "boxpinfontsize",    boxpinfontsize,  "10",
    "boxpinFontsize","BoxpinFontsize",Boxpinfontsize)

PROPDEF_Short( "boxpingrid",        boxpingrid,     "20",
    "boxpinGrid",   "BoxpinGrid",   Boxpingrid)

PROPDEF_Short( "boxpinsquare",      boxpinsquare,    "0",
    "boxpinSquare",  "BoxpinSquare",Boxpinsquare)

PROPDEF_Color( "buscolor",          buscolor,        "#ad8f00",
    "busColor", "BusColor",         Buscolor)

PROPDEF_Int(   "buswidthlimit",     buswidthlimit,    "30000",
    "busWidthlimit", "BusWidthlimit",Buswidthlimit)

PROPDEF_Short( "buswireexcess",     buswireexcess,     "3",
    "busWireexcess", "BusWireexcess",Buswireexcess)

PROPDEF_Short( "closeenough",       closeenough,      "10",
    "closeEnough",  "CloseEnough",  Closeenough)

PROPDEF_Short( "createnetattrdsp",  createnetattrdsp,  "0",
    "createNetattrdsp", "CreateNetattrdsp", Createnetattrdsp)

PROPDEF_Short( "createvconn",       createvconn,       "0",
    "createVconn",  "CreateVconn",  Createvconn)

PROPDEF_Short( "createvconn2",      createvconn2,      "0",
    "createVconn2", "CreateVconn2", Createvconn2)

PROPDEF_Short( "decorate",          decorate,         "0",
    "decorate",     "Decorate",     Decorate)

PROPDEF_Short( "elidetext",	    elidetext,        "0", 
    "elidetext",     "Elidetext",   Elidetext)

PROPDEF_Bool(  "fastpanning",        fastpanning,     "true",
    "fastPanning",    "FastPanning", Fastpanning)

PROPDEF_ColAr( "fillcolor1",         fillcolor[0],     "#441133",
    "fillColor1",    "FillColor",    Fillcolor1)

PROPDEF_ColAr( "fillcolor2",         fillcolor[1],     "#441133",
    "fillColor2",    "FillColor",    Fillcolor2)

PROPDEF_ColAr( "fillcolor3",         fillcolor[2],     "#441133",
    "fillColor3",    "FillColor",    Fillcolor3)

#ifdef ALLPROPS		/* core property command wants them all */
PROPDEF_ColAr( "fillcolor4",         fillcolor[3],     "#541640",
    "fillColor4",    "FillColor",    Fillcolor4)
PROPDEF_ColAr( "fillcolor5",         fillcolor[4],     "#651b4d",
    "fillColor5",    "FillColor",    Fillcolor5)
PROPDEF_ColAr( "fillcolor6",         fillcolor[5],     "#761f59",
    "fillColor6",    "FillColor",    Fillcolor6)
PROPDEF_ColAr( "fillcolor7",         fillcolor[6],     "#872366",
    "fillColor7",    "FillColor",    Fillcolor7)
PROPDEF_ColAr( "fillcolor8",         fillcolor[7],     "#982774",
    "fillColor8",    "FillColor",    Fillcolor8)
PROPDEF_ColAr( "fillcolor9",         fillcolor[8],     "#a82d80",
    "fillColor9",    "FillColor",    Fillcolor9)
PROPDEF_ColAr( "fillcolor10",        fillcolor[9],     "#b9318c",
    "fillColor10",    "FillColor",   Fillcolor10)
PROPDEF_ColAr( "fillcolor11",        fillcolor[10],     "#ca359a",
    "fillColor11",    "FillColor",   Fillcolor11)
PROPDEF_ColAr( "fillcolor12",        fillcolor[11],     "#ce46a2",
    "fillColor12",    "FillColor",   Fillcolor12)
PROPDEF_ColAr( "fillcolor13",        fillcolor[12],     "#d257aa",
    "fillColor13",    "FillColor",   Fillcolor13)
PROPDEF_ColAr( "fillcolor14",        fillcolor[13],     "#d768b3",
    "fillColor14",    "FillColor",   Fillcolor14)
PROPDEF_ColAr( "fillcolor15",        fillcolor[14],     "#dc78bb",
    "fillColor15",    "FillColor",   Fillcolor15)
#endif

PROPDEF_Bool ( "fillednegpins",     fillednegpins,   "false",
    "fillednegpins","Fillednegpins",Fillednegpins)

PROPDEF_Short( "fitpage",           fitpage,         "0",
    "fitPage",  "FitPage",          Fitpage)

PROPDEF_Bool ( "flyingsegments",    flyingsegments,   "false",
    "flyingSegments","FlyingSegments",Flyingsegments)

PROPDEF_Color( "framecolor",        framecolor,      "#a1a1a1",
    "frameColor",   "FrameColor",   Framecolor)

PROPDEF_Short( "framefontsize",     framefontsize,   "12",
    "frameFontsize","FrameFontsize",Framefontsize)

PROPDEF_Short( "gatecellname",      gatecellname,     "0",
    "gateCellname", "GateCellname", Gatecellname)

PROPDEF_Short( "gatepinname",       gatepinname,      "2",
    "gatePinname",  "GatePinname",  Gatepinname)

PROPDEF_Short( "geitarget",         geitarget,        "0",
    "geiTarget",  "GeiTarget",      Geitarget)

PROPDEF_Short ( "greymode",         greymode,         "0",
    "greymode",     "Greymode",     Greymode)

PROPDEF_Short( "grid",              grid,            "10",
    "grid",     "Grid",             Grid)

PROPDEF_Color( "gridcolor",        gridcolor,        "#7f7f7f",
    "gridColor",    "GridColor",   Gridcolor)

PROPDEF_Bool( "groupregionpins",   groupregionpins,   "false", 
    "groupregionpins", "groupRegionPins", Groupregionpins)

PROPDEF_Bool ( "hiattrvalue",       hiattrvalue,      "false",
    "hiAttrValue",  "HiAttrValue",  Hiattrvalue)

PROPDEF_Bool ( "horizontallabels",  horizontallabels, "false",
    "horizontalLabels", "HorizontalLabels", Horizontallabels)

PROPDEF_Short( "instattrmax",       instattrmax,     "20",
    "instAttrMax",  "InstAttrMax",  Instattrmax)

PROPDEF_Short( "instdrag",          instdrag,         "3",
    "instDrag", "InstDrag",         Instdrag)

PROPDEF_Short( "instorder",         instorder,        "0",
    "instOrder", "InstOrder",       Instorder)

PROPDEF_Short( "ioorder",           ioorder,          "0",
    "ioOrder", "IoOrder",           Ioorder)

#ifdef ALLPROPS
PROPDEF_Int  ( "largenet",          obsolete_largenet,         "0",
    "largeNet", "LargeNet",         Largenet)
#endif

PROPDEF_Short( "latchfblevel",      latchfblevel,   "100",
    "latchFbLevel", "LatchFbLevel", Latchfblevel)

PROPDEF_Bool ( "mapbool2pla",       mapbool2pla,     	"false", 
    "mapBool2pla", "MapBool2pla",   Mapbool2pla)

PROPDEF_Short( "markgap",           markgap,          "2",
    "markGap", "MarkGap",           Markgap)

PROPDEF_Short( "marksize",          marksize,         "8",
    "markSize", "MarkSize",         Marksize)

PROPDEF_Short( "matrixalignment",   matrixalignment,  "0",
    "matrixAlignment","MatrixAlignment",Matrixalignment)

#ifdef ALLPROPS
PROPDEF_Float( "maxbuttonzoom",     obsolete_maxbuttonzoom,   "6.4",
    "maxButtonZoom","MaxButtonZoom",Maxbuttonzoom)
#endif

PROPDEF_Float( "maxfontsize",       maxfontsize,     "0",
    "maxFontSize", "MaxFontSize",   Maxfontsize)

PROPDEF_Float( "maxfontsizecg",     maxfontsizecg,   "0",
    "maxFontSizeCG", "MaxFontSize", Maxfontsizecg)

#ifdef ALLPROPS
PROPDEF_Float( "maxfontzoom",       obsolete_maxfontzoom,     "0",
    "maxFontZoom", "MaxFontZoom",   Maxfontzoom)
#endif

PROPDEF_Float( "maxzoom",           maxzoom,         "6.4",
    "maxZoom",  "MaxZoom",          Maxzoom)

PROPDEF_Bool ( "mergepgnets",       mergepgnets,      "true",
    "mergePgnets",  "MergePgnets",  Mergepgnets)

PROPDEF_Int  ( "mergetracks",       mergetracks,     "0",
    "mergeTracks", "MergeTracks",   Mergetracks)

PROPDEF_Short( "minchannelwidth",     minchannelwidth, "0",
    "minchannelWidth","MinchannelWidth",Minchannelwidth) 

PROPDEF_Short( "minlevelwidth",     minlevelwidth,     "0",
    "minlevelWidth","MinlevelWidth",Minlevelwidth) 

PROPDEF_Short( "netattrmax",        netattrmax,       "-1",
    "netAttrMax",   "NetAttrMax",   Netattrmax)

PROPDEF_Int  ( "netautohide",       netautohide,       "0",
    "netAutoHide",  "NetAutoHide",  Netautohide)

PROPDEF_Color( "netcolor",          netcolor,        "#ffff00",
    "netColor", "NetColor",         Netcolor)

PROPDEF_Short( "netfontsize",       netfontsize,     "12",
    "netFontsize",  "NetFontsize",  Netfontsize)

PROPDEF_Short( "nethidedetails",    nethidedetails,   "0",
    "netHideDetails", "NetHideDetails", Nethidedetails)

PROPDEF_Short( "netstubminlen",     netstubminlen,    "0",
    "netStubMinlen","NetStubMinlen",Netstubminlen)

PROPDEF_Bool ( "nohierattrs",       nohierattrs,      "false",
    "noHierattrs",  "NoHierattrs",  Nohierattrs)

PROPDEF_Bool ( "nohiernegpins",     nohiernegpins,    "true",
    "noHiernegpins","NoHiernegpins",Nohiernegpins)

#ifdef ALLPROPS		/* core property command wants them all */
PROPDEF_Bool ( "nooverlap",	    nooverlap,	      "false",
    "noOverlap", "NoOverlap",	    Nooverlap)
#endif

PROPDEF_Color( "objectgrey",        objectgrey,         "#7f7f7f",
    "objectgrey", "Objectgrey", Objectgrey)

PROPDEF_ColAr( "objecthighlight0",  objecthighlight[0], "#ff0000",
    "objectHighlight0","ObjectHighlight",Objecthighlight0)

PROPDEF_ColAr( "objecthighlight1",  objecthighlight[1], "#ff0000",
    "objectHighlight1","ObjectHighlight",Objecthighlight1)

PROPDEF_ColAr( "objecthighlight2",  objecthighlight[2], "#ff0000",
    "objectHighlight2","ObjectHighlight",Objecthighlight2)

PROPDEF_ColAr( "objecthighlight3",  objecthighlight[3], "#ff0000",
    "objectHighlight3","ObjectHighlight",Objecthighlight3)

PROPDEF_ColAr( "objecthighlight4",  objecthighlight[4], "#ff0000",
    "objectHighlight4","ObjectHighlight",Objecthighlight4)

PROPDEF_ColAr( "objecthighlight5",  objecthighlight[5], "#ff0000",
    "objectHighlight5","ObjectHighlight",Objecthighlight5)

PROPDEF_ColAr( "objecthighlight6",  objecthighlight[6], "#ff0000",
    "objectHighlight6","ObjectHighlight",Objecthighlight6)

PROPDEF_ColAr( "objecthighlight7",  objecthighlight[7], "#ff0000",
    "objectHighlight7","ObjectHighlight",Objecthighlight7)

PROPDEF_ColAr( "objecthighlight8",  objecthighlight[8], "#ff0000",
    "objectHighlight8","ObjectHighlight",Objecthighlight8)

PROPDEF_ColAr( "objecthighlight9",  objecthighlight[9], "#ff0000",
    "objectHighlight9","ObjectHighlight",Objecthighlight9)

PROPDEF_ColAr( "objecthighlight10", objecthighlight[10], "#ff0000",
    "objectHighlight10","ObjectHighlight",Objecthighlight10)

PROPDEF_ColAr( "objecthighlight11", objecthighlight[11], "#ff0000",
    "objectHighlight11","ObjectHighlight",Objecthighlight11)

PROPDEF_ColAr( "objecthighlight12", objecthighlight[12], "#ff0000",
    "objectHighlight12","ObjectHighlight",Objecthighlight12)

PROPDEF_ColAr( "objecthighlight13", objecthighlight[13], "#ff0000",
    "objectHighlight13","ObjectHighlight",Objecthighlight13)

PROPDEF_ColAr( "objecthighlight14", objecthighlight[14], "#ff0000",
    "objectHighlight14","ObjectHighlight",Objecthighlight14)

PROPDEF_ColAr( "objecthighlight15", objecthighlight[15], "#ff0000",
    "objectHighlight15","ObjectHighlight",Objecthighlight15)

PROPDEF_ColAr( "objecthighlight16", objecthighlight[16], "#ff0000",
    "objectHighlight16","ObjectHighlight",Objecthighlight16)

PROPDEF_ColAr( "objecthighlight17", objecthighlight[17], "#ff0000",
    "objectHighlight17","ObjectHighlight",Objecthighlight17)

PROPDEF_ColAr( "objecthighlight18", objecthighlight[18], "#ff0000",
    "objectHighlight18","ObjectHighlight",Objecthighlight18)

PROPDEF_ColAr( "objecthighlight19", objecthighlight[19], "#ff0000",
    "objectHighlight19","ObjectHighlight",Objecthighlight19)

#ifdef ALLPROPS		/* core property command wants them all */

PROPDEF_ColAr( "objecthighlight20", objecthighlight[20], "#ff0000",
    "objectHighlight20","ObjectHighlight",Objecthighlight20)
PROPDEF_ColAr( "objecthighlight21", objecthighlight[21], "#ff0000",
    "objectHighlight21","ObjectHighlight",Objecthighlight21)
PROPDEF_ColAr( "objecthighlight22", objecthighlight[22], "#ff0000",
    "objectHighlight22","ObjectHighlight",Objecthighlight22)
PROPDEF_ColAr( "objecthighlight23", objecthighlight[23], "#ff0000",
    "objectHighlight23","ObjectHighlight",Objecthighlight23)
PROPDEF_ColAr( "objecthighlight24", objecthighlight[24], "#ff0000",
    "objectHighlight24","ObjectHighlight",Objecthighlight24)
PROPDEF_ColAr( "objecthighlight25", objecthighlight[25], "#ff0000",
    "objectHighlight25","ObjectHighlight",Objecthighlight25)
PROPDEF_ColAr( "objecthighlight26", objecthighlight[26], "#ff0000",
    "objectHighlight26","ObjectHighlight",Objecthighlight26)
PROPDEF_ColAr( "objecthighlight27", objecthighlight[27], "#ff0000",
    "objectHighlight27","ObjectHighlight",Objecthighlight27)
PROPDEF_ColAr( "objecthighlight28", objecthighlight[28], "#ff0000",
    "objectHighlight28","ObjectHighlight",Objecthighlight28)
PROPDEF_ColAr( "objecthighlight29", objecthighlight[29], "#ff0000",
    "objectHighlight29","ObjectHighlight",Objecthighlight29)
PROPDEF_ColAr( "objecthighlight30", objecthighlight[30], "#ff0000",
    "objectHighlight30","ObjectHighlight",Objecthighlight30)
PROPDEF_ColAr( "objecthighlight31", objecthighlight[31], "#ff0000",
    "objectHighlight31","ObjectHighlight",Objecthighlight31)
PROPDEF_ColAr( "objecthighlight32", objecthighlight[32], "#ff0000",
    "objectHighlight32","ObjectHighlight",Objecthighlight32)
PROPDEF_ColAr( "objecthighlight33", objecthighlight[33], "#ff0000",
    "objectHighlight33","ObjectHighlight",Objecthighlight33)
PROPDEF_ColAr( "objecthighlight34", objecthighlight[34], "#ff0000",
    "objectHighlight34","ObjectHighlight",Objecthighlight34)
PROPDEF_ColAr( "objecthighlight35", objecthighlight[35], "#ff0000",
    "objectHighlight35","ObjectHighlight",Objecthighlight35)
PROPDEF_ColAr( "objecthighlight36", objecthighlight[36], "#ff0000",
    "objectHighlight36","ObjectHighlight",Objecthighlight36)
PROPDEF_ColAr( "objecthighlight37", objecthighlight[37], "#ff0000",
    "objectHighlight37","ObjectHighlight",Objecthighlight37)
PROPDEF_ColAr( "objecthighlight38", objecthighlight[38], "#ff0000",
    "objectHighlight38","ObjectHighlight",Objecthighlight38)
PROPDEF_ColAr( "objecthighlight39", objecthighlight[39], "#ff0000",
    "objectHighlight39","ObjectHighlight",Objecthighlight39)
PROPDEF_ColAr( "objecthighlight40", objecthighlight[40], "#ff0000",
    "objectHighlight40","ObjectHighlight",Objecthighlight40)
PROPDEF_ColAr( "objecthighlight41", objecthighlight[41], "#ff0000",
    "objectHighlight41","ObjectHighlight",Objecthighlight41)
PROPDEF_ColAr( "objecthighlight42", objecthighlight[42], "#ff0000",
    "objectHighlight42","ObjectHighlight",Objecthighlight42)
PROPDEF_ColAr( "objecthighlight43", objecthighlight[43], "#ff0000",
    "objectHighlight43","ObjectHighlight",Objecthighlight43)
PROPDEF_ColAr( "objecthighlight44", objecthighlight[44], "#ff0000",
    "objectHighlight44","ObjectHighlight",Objecthighlight44)
PROPDEF_ColAr( "objecthighlight45", objecthighlight[45], "#ff0000",
    "objectHighlight45","ObjectHighlight",Objecthighlight45)
PROPDEF_ColAr( "objecthighlight46", objecthighlight[46], "#ff0000",
    "objectHighlight46","ObjectHighlight",Objecthighlight46)
PROPDEF_ColAr( "objecthighlight47", objecthighlight[47], "#ff0000",
    "objectHighlight47","ObjectHighlight",Objecthighlight47)
PROPDEF_ColAr( "objecthighlight48", objecthighlight[48], "#ff0000",
    "objectHighlight48","ObjectHighlight",Objecthighlight48)
PROPDEF_ColAr( "objecthighlight49", objecthighlight[49], "#ff0000",
    "objectHighlight49","ObjectHighlight",Objecthighlight49)
PROPDEF_ColAr( "objecthighlight50", objecthighlight[50], "#ff0000",
    "objectHighlight50","ObjectHighlight",Objecthighlight50)
PROPDEF_ColAr( "objecthighlight51", objecthighlight[51], "#ff0000",
    "objectHighlight51","ObjectHighlight",Objecthighlight51)
PROPDEF_ColAr( "objecthighlight52", objecthighlight[52], "#ff0000",
    "objectHighlight52","ObjectHighlight",Objecthighlight52)
PROPDEF_ColAr( "objecthighlight53", objecthighlight[53], "#ff0000",
    "objectHighlight53","ObjectHighlight",Objecthighlight53)
PROPDEF_ColAr( "objecthighlight54", objecthighlight[54], "#ff0000",
    "objectHighlight54","ObjectHighlight",Objecthighlight54)
PROPDEF_ColAr( "objecthighlight55", objecthighlight[55], "#ff0000",
    "objectHighlight55","ObjectHighlight",Objecthighlight55)
PROPDEF_ColAr( "objecthighlight56", objecthighlight[56], "#ff0000",
    "objectHighlight56","ObjectHighlight",Objecthighlight56)
PROPDEF_ColAr( "objecthighlight57", objecthighlight[57], "#ff0000",
    "objectHighlight57","ObjectHighlight",Objecthighlight57)
PROPDEF_ColAr( "objecthighlight58", objecthighlight[58], "#ff0000",
    "objectHighlight58","ObjectHighlight",Objecthighlight58)
PROPDEF_ColAr( "objecthighlight59", objecthighlight[59], "#ff0000",
    "objectHighlight59","ObjectHighlight",Objecthighlight59)
PROPDEF_ColAr( "objecthighlight60", objecthighlight[60], "#ff0000",
    "objectHighlight60","ObjectHighlight",Objecthighlight60)
PROPDEF_ColAr( "objecthighlight61", objecthighlight[61], "#ff0000",
    "objectHighlight61","ObjectHighlight",Objecthighlight61)
PROPDEF_ColAr( "objecthighlight62", objecthighlight[62], "#ff0000",
    "objectHighlight62","ObjectHighlight",Objecthighlight62)
PROPDEF_ColAr( "objecthighlight63", objecthighlight[63], "#ff0000",
    "objectHighlight63","ObjectHighlight",Objecthighlight63)
PROPDEF_ColAr( "objecthighlight64", objecthighlight[64], "#ff0000",
    "objectHighlight64","ObjectHighlight",Objecthighlight64)
PROPDEF_ColAr( "objecthighlight65", objecthighlight[65], "#ff0000",
    "objectHighlight65","ObjectHighlight",Objecthighlight65)
PROPDEF_ColAr( "objecthighlight66", objecthighlight[66], "#ff0000",
    "objectHighlight66","ObjectHighlight",Objecthighlight66)
PROPDEF_ColAr( "objecthighlight67", objecthighlight[67], "#ff0000",
    "objectHighlight67","ObjectHighlight",Objecthighlight67)
PROPDEF_ColAr( "objecthighlight68", objecthighlight[68], "#ff0000",
    "objectHighlight68","ObjectHighlight",Objecthighlight68)
PROPDEF_ColAr( "objecthighlight69", objecthighlight[69], "#ff0000",
    "objectHighlight69","ObjectHighlight",Objecthighlight69)
PROPDEF_ColAr( "objecthighlight70", objecthighlight[70], "#ff0000",
    "objectHighlight70","ObjectHighlight",Objecthighlight70)
PROPDEF_ColAr( "objecthighlight71", objecthighlight[71], "#ff0000",
    "objectHighlight71","ObjectHighlight",Objecthighlight71)
PROPDEF_ColAr( "objecthighlight72", objecthighlight[72], "#ff0000",
    "objectHighlight72","ObjectHighlight",Objecthighlight72)
PROPDEF_ColAr( "objecthighlight73", objecthighlight[73], "#ff0000",
    "objectHighlight73","ObjectHighlight",Objecthighlight73)
PROPDEF_ColAr( "objecthighlight74", objecthighlight[74], "#ff0000",
    "objectHighlight74","ObjectHighlight",Objecthighlight74)
PROPDEF_ColAr( "objecthighlight75", objecthighlight[75], "#ff0000",
    "objectHighlight75","ObjectHighlight",Objecthighlight75)
PROPDEF_ColAr( "objecthighlight76", objecthighlight[76], "#ff0000",
    "objectHighlight76","ObjectHighlight",Objecthighlight76)
PROPDEF_ColAr( "objecthighlight77", objecthighlight[77], "#ff0000",
    "objectHighlight77","ObjectHighlight",Objecthighlight77)
PROPDEF_ColAr( "objecthighlight78", objecthighlight[78], "#ff0000",
    "objectHighlight78","ObjectHighlight",Objecthighlight78)
PROPDEF_ColAr( "objecthighlight79", objecthighlight[79], "#ff0000",
    "objectHighlight79","ObjectHighlight",Objecthighlight79)
PROPDEF_ColAr( "objecthighlight80", objecthighlight[80], "#ff0000",
    "objectHighlight80","ObjectHighlight",Objecthighlight80)
PROPDEF_ColAr( "objecthighlight81", objecthighlight[81], "#ff0000",
    "objectHighlight81","ObjectHighlight",Objecthighlight81)
PROPDEF_ColAr( "objecthighlight82", objecthighlight[82], "#ff0000",
    "objectHighlight82","ObjectHighlight",Objecthighlight82)
PROPDEF_ColAr( "objecthighlight83", objecthighlight[83], "#ff0000",
    "objectHighlight83","ObjectHighlight",Objecthighlight83)
PROPDEF_ColAr( "objecthighlight84", objecthighlight[84], "#ff0000",
    "objectHighlight84","ObjectHighlight",Objecthighlight84)
PROPDEF_ColAr( "objecthighlight85", objecthighlight[85], "#ff0000",
    "objectHighlight85","ObjectHighlight",Objecthighlight85)
PROPDEF_ColAr( "objecthighlight86", objecthighlight[86], "#ff0000",
    "objectHighlight86","ObjectHighlight",Objecthighlight86)
PROPDEF_ColAr( "objecthighlight87", objecthighlight[87], "#ff0000",
    "objectHighlight87","ObjectHighlight",Objecthighlight87)
PROPDEF_ColAr( "objecthighlight88", objecthighlight[88], "#ff0000",
    "objectHighlight88","ObjectHighlight",Objecthighlight88)
PROPDEF_ColAr( "objecthighlight89", objecthighlight[89], "#ff0000",
    "objectHighlight89","ObjectHighlight",Objecthighlight89)
PROPDEF_ColAr( "objecthighlight90", objecthighlight[90], "#ff0000",
    "objectHighlight90","ObjectHighlight",Objecthighlight90)
PROPDEF_ColAr( "objecthighlight91", objecthighlight[91], "#ff0000",
    "objectHighlight91","ObjectHighlight",Objecthighlight91)
PROPDEF_ColAr( "objecthighlight92", objecthighlight[92], "#ff0000",
    "objectHighlight92","ObjectHighlight",Objecthighlight92)
PROPDEF_ColAr( "objecthighlight93", objecthighlight[93], "#ff0000",
    "objectHighlight93","ObjectHighlight",Objecthighlight93)
PROPDEF_ColAr( "objecthighlight94", objecthighlight[94], "#ff0000",
    "objectHighlight94","ObjectHighlight",Objecthighlight94)
PROPDEF_ColAr( "objecthighlight95", objecthighlight[95], "#ff0000",
    "objectHighlight95","ObjectHighlight",Objecthighlight95)
PROPDEF_ColAr( "objecthighlight96", objecthighlight[96], "#ff0000",
    "objectHighlight96","ObjectHighlight",Objecthighlight96)
PROPDEF_ColAr( "objecthighlight97", objecthighlight[97], "#ff0000",
    "objectHighlight97","ObjectHighlight",Objecthighlight97)
PROPDEF_ColAr( "objecthighlight98", objecthighlight[98], "#ff0000",
    "objectHighlight98","ObjectHighlight",Objecthighlight98)
PROPDEF_ColAr( "objecthighlight99", objecthighlight[99], "#ff0000",
    "objectHighlight99","ObjectHighlight",Objecthighlight99)

#endif /* ALLPROPS */

PROPDEF_Bool ( "ongrid",      	    ongrid,     	"false", 
    "onGrid",       "OnGrid",       Ongrid)

PROPDEF_Short( "outfblevel",        outfblevel,       "2",
    "outFbLevel",   "OutFbLevel",   Outfblevel)

PROPDEF_Color( "overlapcolor",      overlapcolor,    "#8c8c8c",
    "overlapColor", "OverlapColor", Overlapcolor)

PROPDEF_Color( "pbuscolor",         pbuscolor,       "#006868",
    "pbusColor",    "PbusColor",    Pbuscolor)

PROPDEF_Color( "pbusnamecolor",     pbusnamecolor,   "#ad8f00",
    "pbusnameColor","PbusnameColor",Pbusnamecolor)

PROPDEF_Short( "pinattrmax",        pinattrmax,       "10",
    "pinAttrMax",   "PinAttrMax",   Pinattrmax)

PROPDEF_Short( "pinautohide",       pinautohide,      "1",
    "pinAutoHide",  "PinAutoHide",  Pinautohide)

PROPDEF_Short( "pinorder",	    pinorder,         "0",
    "pinOrder","PinOrder",          Pinorder)

PROPDEF_Bool ( "pinpermute",        pinpermute,      "true",
    "pinPermute",   "PinPermute",   Pinpermute)

PROPDEF_Short( "portattrmax",        portattrmax,       "-1",
    "portAttrMax", "PortAttrMax",   Portattrmax)

PROPDEF_Color( "portcolor",         portcolor,       "#00d1d1",
    "portColor",    "PortColor",    Portcolor)

PROPDEF_Color( "portnamecolor",     portnamecolor,   "#ffff00",
    "portnameColor","PortnameColor",Portnamecolor)

PROPDEF_Int ( "reducejogs",	   reducejogs,	     "0", 
    "reduceJogs", "ReduceJogs",    Reducejogs)

PROPDEF_Short( "ripattrmax",       ripattrmax,       "3",
    "ripAttrMax",  "RipAttrMax",   Ripattrmax)

PROPDEF_Short( "ripindexfontsize",  ripindexfontsize,   "10",
    "ripindexFontsize", "RipindexFontsize", Ripindexfontsize)

PROPDEF_Color( "rippercolor",       rippercolor,     "#00d1d1",
    "ripperColor",  "RipperColor",  Rippercolor)

PROPDEF_Short( "rippershape",       rippershape,      "0",
    "ripperShape",   "RipperShape", Rippershape)

PROPDEF_Color( "rubberbandcolor",   rubberbandcolor, "#ff0000",
    "rubberbandColor", "RubberbandColor", Rubberbandcolor)

PROPDEF_Short( "rubberbandfontsize",rubberbandfontsize, "14",
    "rubberbandFontsize", "RubberbandFontsize", Rubberbandfontsize)

PROPDEF_Short( "selectattr",        selectattr,       "1",
    "selectAttr",   "SelectAttr",   Selectattr)

PROPDEF_Short ( "selectionappearance", selectionappearance,  "5",
    "selectionAppearance", "SelectionAppearance", Selectionappearance)

PROPDEF_Color ( "selectioncolor",    selectioncolor,  "#ff0000",
    "selectionColor","SelectionColor",Selectioncolor)

PROPDEF_Short ( "shadowstyle",      shadowstyle,      "0",
    "shadowstyle",  "ShadowStyle",  Shadowstyle)

PROPDEF_Float( "sheetheight",       sheetheight,      "17",
    "sheetHeight",  "SheetHeight",  Sheetheight)

PROPDEF_Float( "sheetwidth",        sheetwidth,       "22",
    "sheetWidth",   "SheetWidth",   Sheetwidth)

PROPDEF_Bool ( "shortobjid",        shortobjid,       "false",
    "shortObjId",   "ShortObjId",   Shortobjid)

PROPDEF_Short ( "showattribute",     showattribute,   "1",
    "showattribute","ShowObjectName",Showattribute)

PROPDEF_Bool ( "showcellname",      showcellname,     "true",
    "showcellname","ShowObjectName",Showcellname)

PROPDEF_Short ( "showframe",        showframe,        "0",
    "showFrame",    "ShowFrame",    Showframe)

PROPDEF_Short ( "showgrid",         showgrid,         "0",
    "showGrid",     "ShowGrid",     Showgrid)

PROPDEF_Short ( "showhierpinname",  showhierpinname,  "0",
    "showhierpinname","ShowObjectName",Showhierpinname)

PROPDEF_Bool ( "showinstname",      showinstname,     "true",
    "showinstname","ShowObjectName",Showinstname)

PROPDEF_Bool ( "showinvisibles",    showinvisibles,   "false",
    "showinvisibles","ShowInvisibles",Showinvisibles)

PROPDEF_Short ( "showlevels",       showlevels,       "0",
    "showLevels",   "ShowLevels",   Showlevels)

PROPDEF_Short ( "showmarks",        showmarks,        "0",
    "showMarks",    "ShowMarks",    Showmarks)

PROPDEF_Short( "shownetattr",       shownetattr,      "0",
    "showNetattr",  "ShowNetattr",  Shownetattr)

PROPDEF_Bool ( "shownetname",       shownetname,      "true",
    "shownetname","ShowObjectName", Shownetname)

PROPDEF_Short ( "showpagenumbers",  showpagenumbers,  "0",
    "showPageNumbers","ShowPageNumbers",Showpagenumbers)

PROPDEF_Short ( "showpgtype",       showpgtype,       "0",
    "showPGtype",   "ShowPGtype",   Showpgtype)

PROPDEF_Short ( "showpinname",       showpinname,     "1",
    "showpinname","ShowObjectName", Showpinname)

PROPDEF_Bool ( "showportname",      showportname,     "true",
    "showportname","ShowObjectName", Showportname)

#ifndef NEXT_MAJOR_RELEASE
PROPDEF_Short( "showripindex",      showripindex,     "0",
    "showRipindex", "ShowRipindex", Showripindex)
#else
PROPDEF_Bool( "showripindex",      showripindex,      "true",
    "showRipindex", "ShowRipindex", Showripindex)
#endif

PROPDEF_Short ( "showval",          showval,          "0",
    "showVal",      "ShowVal",      Showval)

PROPDEF_Short ( "showvconn",        showvconn,        "0",
    "showVconn",    "ShowVconn",    Showvconn)

PROPDEF_Short ( "showvconn2",       showvconn2,       "0",
    "showVconn2",   "ShowVconn2",   Showvconn2)

PROPDEF_Short( "stubattrmax",       stubattrmax,     "7",
    "stubAttrMax",  "StubAttrMax",  Stubattrmax)

#ifdef ALLPROPS		/* only for core property command */
PROPDEF_Bool( "tclcompat",          obsolete_tclcompat,    "false",
    "tclCompat", "TclCompat", Tclcompat)
#endif

PROPDEF_Int  ( "timelimit",         timelimit,       "40",
    "timeLimit",    "TimeLimit",    Timelimit)

PROPDEF_Short ( "transistorlayout", transistorlayout, "1", 
    "transistorLayout", "TransistorLayout", Transistorlayout)

PROPDEF_Short( "valattr",           valattr,         "0",
    "valAttr",  "ValAttr",          Valattr)

PROPDEF_Bool( "viewnameoptional",   viewnameoptional, "true",
    "viewnameOptional", "ViewnameOptional", Viewnameoptional)
