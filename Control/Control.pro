greaterThan(QT_MAJOR_VERSION,4): QT += widgets

QT += printsupport xml
TEMPLATE = lib
TARGET = Control
DESTDIR = ../x64/Release
CONFIG += staticlib release
DEFINES += CONTROL_LIB BUILD_STATIC
LIBS += -L"."
DEPENDPATH += .
MOC_DIR += .
OBJECTS_DIR += release
UI_DIR += .
RCC_DIR += GeneratedFiles
include(Control.pri)

INCLUDEPATH += ../Deps/x64/include