# riscview.pro
#   (c) 2003-2018 Concept Engineering GmbH
#
# This is a qmake project file for generating
# machine dependent Makefiles, considered as
# an alternative to shipped Makefile templates.
# Note: qmake will overwrite the existing Makefile
# by default (use option -o to prevent this).
#
# It will generate a (mostly) statically linked test
# application called riscview:
#
# - if qmake can find our shared run-time library
#   called libnlvlics it will compile with FLEXnet
#   licensing support (-DNLVLIC).
#
# - if qmake can find the path to your local Verific
#   software installation (see below) it will compile
#   with VVDI support (-DVVDI).
#
# Try:
#   qmake riscview.pro -o Makefile-riscview.qmake
#   make -f Makefile-riscview.qmake clean
#   make -f Makefile-riscview.qmake
#
TEMPLATE         = app
CONFIG           += qt warn_off link_prl release
TARGET           = riscview
VERSION          = 7.0.16
MOC_DIR          = ./moc
OBJECTS_DIR      = .
QMAKE_RPATHDIR   += . ./ext
EXT_DIR          = ./ext
SERVERPATH       = ../../vp/src/platform/riscview/nlv
CONFIG   *= x11
LIBS     += ./nlvcore.a
ROOTPATH = .
exists( ./libnlvlics.so ) {
    DEFINES += NLVLIC
    LIBS    += -L. -lnlvlics
}

QT       *= widgets printsupport
static {
    CONFIG   -= import_plugins
    win32 {
        QTPLUGIN *= qwindows
    } else {
        QTPLUGIN *= qxcb
    }
}
HEADERS  += $$ROOTPATH/src/riscview.h
SOURCES  += $$ROOTPATH/src/riscview.cpp

HEADERS  += $$ROOTPATH/src/nlvhandler.hpp
SOURCES  += $$ROOTPATH/src/nlvhandler.cpp
SOURCES  += $$ROOTPATH/src/elegantEnums.cpp

HEADERS += $$ROOTPATH/src/wrapper.h
SOURCES += $$ROOTPATH/src/wrapper.cpp
SOURCES += $$ROOTPATH/src/nlvrqt.cpp

HEADERS += $$SERVERPATH/connector-server.hpp
SOURCES += $$SERVERPATH/connector-server.cpp

INCLUDEPATH += $$ROOTPATH/ext/include

DEPENDPATH += $$ROOTPATH/ext/include
DEPENDPATH += $$ROOTPATH/src


# Add TDB demo code (a static/dummy DataBase)
# to show Nlview's incremental control features.
#
DEFINES     += TDB CONCEPT_DEBUGGING
SOURCES     += $$EXT_DIR/tdb/vdiimpl.cpp
SOURCES     += $$EXT_DIR/tdb/data.c
SOURCES     += $$EXT_DIR/tdb/datatrans.c
SOURCES     += $$EXT_DIR/tdb/tdb.c
HEADERS     += $$EXT_DIR/tdb/tdb.h
INCLUDEPATH += $$EXT_DIR/tdb
DEPENDPATH  += $$EXT_DIR/tdb

# Add GEX demo code that exports Nlview schematics
# to text, SKILL files or a Qt QGraphicsScene
# using Nlview's GEI (Graphics Export Interface).
#
DEFINES     += GEX
HEADERS     += $$EXT_DIR/include/gei.h
HEADERS     += $$EXT_DIR/gex/gdump.h
HEADERS     += $$EXT_DIR/gex/gskill.h
HEADERS     += $$EXT_DIR/gex/gscene.h
HEADERS     += $$EXT_DIR/gex/gassert.h
HEADERS     += $$EXT_DIR/gex/ghash.h
SOURCES     += $$EXT_DIR/gex/gdump.c
SOURCES     += $$EXT_DIR/gex/gskill.c
SOURCES     += $$EXT_DIR/gex/gscene.cpp
SOURCES     += $$EXT_DIR/gex/ghash.c
INCLUDEPATH += $$EXT_DIR/gex
INCLUDEPATH *= $$EXT_DIR/include
DEPENDPATH  += $$EXT_DIR/gex

