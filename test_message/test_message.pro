QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++17
TEMPLATE = app

SOURCES +=  tst_parser.cpp
HEADERS += ../serverdiscovery/ssdp_message.hpp
INCLUDEPATH += $$PWD/../dep/optional/include
