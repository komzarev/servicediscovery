
INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/ssdp_message.hpp \
    $$PWD/ssdp_asio.hpp \
    $$PWD/string_find.h \
    $$PWD/string_format.h \
    $$PWD/string_trim.h

SOURCES += \
    $$PWD/asio_cache.cpp \
    $$PWD/ssdp_asio.cpp \

DESTDIR      = $$PROJECT_ROOT_DIR/bin/
OBJECTS_DIR  = $$PROJECT_ROOT_DIR/obj/$$TARGET
MOC_DIR      = $$PROJECT_ROOT_DIR/moc/$$TARGET
UI_DIR       = $$PROJECT_ROOT_DIR/ui/$$TARGET

DEFINES+=_WIN32_WINNT=0x0A00
