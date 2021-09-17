CONFIG += c++14
INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD\ssdp_message.hpp \
    $$PWD\ssdp_qt.hpp \
    $$PWD\string_find.h \
    $$PWD\string_format.h \
    $$PWD\string_trim.h

SOURCES += \
    $$PWD\ssdp_qt.cpp

DEFINES+=_WIN32_WINNT=0x0A00
