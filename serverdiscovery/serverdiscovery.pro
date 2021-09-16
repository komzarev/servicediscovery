CONFIG = qt
QT       += network
QT       -= gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    finally.hpp \
    ssdp_asio.hpp \
    ssdp_message.hpp \
    ssdp_qt.hpp \
    string_find.h \
    string_format.h \
    string_trim.h

DEFINES+=_WIN32_WINNT=0x0A00

INCLUDEPATH += $$PWD/../dep/optional/include

DEFINES += BOOST_ASIO_STANDALONE BOOST_ASIO_SEPARATE_COMPILATION
INCLUDEPATH += $$PWD/../dep/boost-ho/include
qnx7: INCLUDEPATH += -isystem $$PWD/../dep/boost-ho/qnx7

SOURCES += \
    asio_cache.cpp \
    ssdp_asio.cpp \
    ssdp_qt.cpp

DISTFILES += \
    qt_serverdiscovery.pri \
    asio_serverdiscovery.pri
