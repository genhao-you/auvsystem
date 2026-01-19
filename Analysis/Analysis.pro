greaterThan(QT_MAJOR_VERSION,4): QT += widgets

TEMPLATE = lib
TARGET = Analysis
DESTDIR = ../x64/Release
CONFIG += staticlib release
DEFINES += ANALYSIS_LIB BUILD_STATIC
LIBS += -L"."
DEPENDPATH += .
MOC_DIR += GeneratedFiles/$(ConfigurationName)
OBJECTS_DIR += release
UI_DIR += GeneratedFiles
RCC_DIR += GeneratedFiles
include(Analysis.pri)

INCLUDEPATH += ../Deps/x64/include