greaterThan(QT_MAJOR_VERSION,4): QT += widgets

QT += xml
TEMPLATE = lib
TARGET = Core
DESTDIR = ../x64/Release
CONFIG += staticlib release
DEFINES += BUILD_STATIC
LIBS += -L"."
DEPENDPATH += .
MOC_DIR += GeneratedFiles/$(ConfigurationName)
OBJECTS_DIR += release
UI_DIR += GeneratedFiles
RCC_DIR += GeneratedFiles
include(Core.pri)

INCLUDEPATH += ../Deps/x64/include
