include($$PWD/../../serverdiscovery/asio_serverdiscovery.pri)
INCLUDEPATH += $$PWD/../../dep/optional/include

CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

windows {
    *-g++* {
        # MinGW
        QMAKE_CXXFLAGS += -Wno-unused-local-typedefs #-Wno-unused-parameter
        DEFINES+=WINVER=0x0A00 #Win10
        DEFINES+=_WIN32_WINNT=0x0A00 #Win10
        LIBS += -lws2_32  -lwsock32
    }
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DEFINES += BOOST_ASIO_STANDALONE BOOST_ASIO_SEPARATE_COMPILATION
INCLUDEPATH += $$PWD/../../dep/boost-ho/include
qnx7: INCLUDEPATH += -isystem $$PWD/../../dep/boost-ho/qnx7
qnx: LIBS += -lsocket

