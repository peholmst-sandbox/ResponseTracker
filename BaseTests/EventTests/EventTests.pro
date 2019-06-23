QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase c++17
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_eventtest.cpp

INCLUDEPATH += $$PWD/../../Base
DEPENDPATH += $$PWD/../../Base
