#-------------------------------------------------
#
# Project created by QtCreator 2018-02-27T22:28:58
#
#-------------------------------------------------

QT       -= core serialport

TARGET = ebmbus
TEMPLATE = lib

DEFINES += EBMBUS_LIBRARY

SOURCES += ebmbus.cpp

HEADERS += ebmbus.h \
    ebmbus_global.h

unix {
    target.path = /usr/lib

    headers.path = "/usr/include/libebmbus"
#    headers.files = $$PWD/src/*.h
    headers.files = $$PWD/*.h

    INSTALLS += target headers
}
