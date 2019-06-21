# nlviewqt.pro
#   (c) 2003-2018 Concept Engineering GmbH
#
# This is a qmake project file for generating
# machine dependent Makefiles, considered as
# an alternative to shipped Makefile templates.
# Note: qmake will overwrite the existing Makefile
# by default (use option -o to prevent this).
#
# It will generate a shared library called libnlviewqt
# from NlviewQT wrapper C++/Qt source files.
# The precompiled Nlview core library will also be
# linked in statically.
#
# => This is all you need for embedding NlviewQT into
#    your own Qt application.
#
# Try:
#   qmake nlviewqt.pro -o Makefile-nlviewqt.qmake
#   make -f Makefile-nlviewqt.qmake clean
#   make -f Makefile-nlviewqt.qmake
#
TEMPLATE    = lib
CONFIG      += qt dll warn_off link_prl release
TARGET      = nlviewqt
MOC_DIR     = .
VERSION     = 7.0.16
OBJECTS_DIR = .
win32 {
    CONFIG   *= windows embed_manifest_dll
    CONFIG   -= debug_and_release
    DEFINES  *= WIN WIN32 NLVIEW_DLLEXPORT
    ROOTPATH = ..
    TARGET   = libnlviewqt

    QMAKE_TARGET_PRODUCT     = Schematic Generator
    QMAKE_TARGET_DESCRIPTION = NlviewQT wrapper library
    QMAKE_TARGET_COPYRIGHT   = Concept Engineering GmbH
    QMAKE_TARGET_COMPANY     = Concept Engineering GmbH

    win32-g++* {
        PRE_TARGETDEPS =  ./nlvcore.a
        LIBS       += ./nlvcore.a
    } else {
        PRE_TARGETDEPS =  nlvcore.lib
        LIBS       += nlvcore.lib
    }
} else {
    CONFIG     *= x11
    PRE_TARGETDEPS =  ../nlvcore.a
    LIBS       += ../nlvcore.a
    ROOTPATH   = ../..

    # create a symbolic link for backward compatibility
    QMAKE_POST_LINK += ln -s -f \
        lib$(QMAKE_TARGET).so.$$VERSION \
        lib$(QMAKE_TARGET).so.1.0.0
    QMAKE_CLEAN += lib$(QMAKE_TARGET).so.1.0.0
}

equals(QT_MAJOR_VERSION, 5) {
    QT *= widgets
}

HEADERS += $$ROOTPATH/src/wrapper.h

SOURCES += $$ROOTPATH/src/wrapper.cpp
SOURCES += $$ROOTPATH/src/nlvrqt.cpp

INCLUDEPATH += $$ROOTPATH/include

DEPENDPATH += $$ROOTPATH/include
DEPENDPATH += $$ROOTPATH/src

