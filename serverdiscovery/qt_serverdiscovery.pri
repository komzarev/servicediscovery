
INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD\ssdp_message.hpp \
    $$PWD\ssdp_qt.hpp \
    $$PWD\string_find.h \
    $$PWD\string_format.h \
    $$PWD\string_trim.h

DESTDIR      = $$PROJECT_ROOT_DIR/bin/
OBJECTS_DIR  = $$PROJECT_ROOT_DIR/obj/$$TARGET
MOC_DIR      = $$PROJECT_ROOT_DIR/moc/$$TARGET
UI_DIR       = $$PROJECT_ROOT_DIR/ui/$$TARGET

SOURCES += \
    $$PWD\ssdp_qt.cpp

DEFINES+=_WIN32_WINNT=0x0A00