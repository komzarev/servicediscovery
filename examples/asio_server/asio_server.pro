include($$PWD/../../serverdiscovery/asio_serverdiscovery.pri)
INCLUDEPATH += $$PWD/../../dep/optional/include

CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG -= qt
unix:QMAKE_CXXFLAGS += -fPIC

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DEFINES += BOOST_ASIO_STANDALONE BOOST_ASIO_SEPARATE_COMPILATION
INCLUDEPATH += $$PWD/../../dep/boost-ho/include
qnx7: INCLUDEPATH += -isystem $$PWD/../../dep/boost-ho/qnx7
