QT -= gui core

TARGET = STLINK-V3-BRIDGE

TEMPLATE = lib
DEFINES += STLINKV3BRIDGE_LIBRARY

win32
{
    DEFINES += \
        WIN32 \
        USING_ERRORLOG
}

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    src/bridge \
    src/common \
    src/error

SOURCES += \
    src/bridge/bridge.cpp \
    src/common/stlink_interface.cpp \
    src/common/stlink_device.cpp \
    src/common/criticalsectionlock.cpp \
    src/error/ErrLog.cpp

HEADERS += \
    src/bridge/bridge.h \
    src/bridge/stlink_fw_const_bridge.h \
    src/bridge/stlink_fw_api_bridge.h \
    src/common/STLinkUSBDriver.h \
    src/common/stlink_type.h \
    src/common/stlink_interface.h \
    src/common/stlink_if_common.h \
    src/common/stlink_fw_api_common.h \
    src/common/stlink_device.h \
    src/common/criticalsectionlock.h \
    src/error/ErrLog.h

# Default rules for deployment.
unix
{
    target.path = /usr/lib
}
win32
{
    target.path = $$PWD
}
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -lShLwApi
