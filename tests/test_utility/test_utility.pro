include($$PWD/../../serverdiscovery/qt_serverdiscovery.pri)

QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++17
TEMPLATE = app
QT       += network


INCLUDEPATH += $$PWD/../dep/optional/include

SOURCES += \
    tst_clientutility.cpp
