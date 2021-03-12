QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../../../vp/src/platform/redv/ \
               ../../../vp/src/

SOURCES += \
    gpioutils.cpp \
    main.cpp \
    mainwindow.cpp \
    preferencesdialog.cpp \
    uartthread.cpp \
    ../../../vp/src/platform/redv/gpio/gpiocommon.cpp \
    ../../../vp/src/platform/redv/gpio/gpio-client.cpp \
    ../../../vp/src/util/elegantEnums.cpp \
    ../../../vp/src/platform/redv/oled/common.cpp \

HEADERS += \
    button.h \
    gpioutils.h \
    led.h \
    mainwindow.h \
    preferencesdialog.h \
    sevensegment.h \
    uartthread.h \
    ../../../vp/src/platform/redv/gpio/gpiocommon.hpp \
    ../../../vp/src/platform/redv/gpio/gpio-client.hpp \
    ../../../vp/src/util/elegantEnums.hpp

FORMS += \
    mainwindow.ui \
    preferencesdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    vp-redboard.qrc
