CONFIG = qt
QT       += network
QT       -= gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#SOURCES += \
#    ssdp_message.cpp

HEADERS += \
    ssdp_message.hpp \
    ssdp_qt.hpp \
    string_find.h \
    string_format.h \
    string_trim.h

# Default rules for deployment.
INSTALLS += target

INCLUDEPATH += $$PWD/../dep/optional/include

SOURCES += \
    ssdp_qt.cpp

DISTFILES += \
    serverdiscovery.pri
