#-------------------------------------------------
#
# Project created by QtCreator 2018-02-27T22:28:58
#
# Copyright SME GmbH
# Open source license to be defined. GPL2?
#
#-------------------------------------------------

QT       -= core
QT       += serialport

TARGET = ebmbus
TEMPLATE = lib

DEFINES += EBMBUS_LIBRARY

SOURCES += ebmbus.cpp

HEADERS += ebmbus.h \
    ebmbus_global.h

linux-g++: QMAKE_TARGET.arch = $$QMAKE_HOST.arch
linux-g++-32: QMAKE_TARGET.arch = x86
linux-g++-64: QMAKE_TARGET.arch = x86_64
linux-cross: QMAKE_TARGET.arch = x86
win32-cross-32: QMAKE_TARGET.arch = x86
win32-cross: QMAKE_TARGET.arch = x86_64
win32-g++: QMAKE_TARGET.arch = $$QMAKE_HOST.arch
win32-msvc*: QMAKE_TARGET.arch = $$QMAKE_HOST.arch
linux-raspi: QMAKE_TARGET.arch = armv6l
linux-armv6l: QMAKE_TARGET.arch = armv6l
linux-armv7l: QMAKE_TARGET.arch = armv7l
linux-arm*: QMAKE_TARGET.arch = armv6l
linux-aarch64*: QMAKE_TARGET.arch = aarch64

unix {
    equals(QMAKE_TARGET.arch , x86_64): {
        message("Configured for x86_64")
        target.path = /usr/lib64
    }

    equals(QMAKE_TARGET.arch , x86): {
        message("Configured for x86")
        target.path = /usr/lib
    }

    equals(QMAKE_TARGET.arch , armv6l): {
        message("Configured for armv6l")
        target.path = /usr/lib
    }

    equals(QMAKE_TARGET.arch , armv7l): {
        message("Configured for armv7l")
        target.path = /usr/lib
    }

    headers.path = "/usr/include/libebmbus"
    headers.files = $$PWD/*.h

    INSTALLS += target headers
}
