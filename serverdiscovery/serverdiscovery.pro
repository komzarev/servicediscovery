CONFIG = qt
QT       += network
QT       -= gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17
!win32:QMAKE_CXXFLAGS += -fPIC
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#SOURCES += \
#    ssdp_message.cpp

HEADERS += \
    ssdp_asio.hpp \
    ssdp_message.hpp \
    ssdp_qt.hpp \
    string_find.h \
    string_format.h \
    string_trim.h

# Default rules for deployment.
INSTALLS += target

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
