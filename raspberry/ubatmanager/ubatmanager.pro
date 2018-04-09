QT += core bluetooth
QT -= gui

CONFIG += c++11

TARGET = ubatmanager
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    mcp3008/mcp3008Spi.cpp \
    ssd1306/Adafruit_GFX.cpp \
    bluetooth/bluetoothserver.cpp \
    bluetooth/btmanager.cpp \
    mainthread.cpp \
    ntc_library.cpp \
    displaythread.cpp \
    beepbuzzer.cpp \
    confmeasures.cpp \
    lm75a/lm75temp.cpp

HEADERS += \
    ssd1306/Adafruit_GFX.h \
    mcp3008/mcp3008Spi.h \
    bluetooth/bluetoothserver.h \
    bluetooth/btmanager.h \
    mainthread.h \
    ntc_library.h \
    beepbuzzer.h \
    displaythread.h \
    project.h \
    lm75a/lm75temp.h

contains(QT_ARCH, arm) {
message( ARM version )

SOURCES += \
    bcm2835/bcm2835.c \
    ssd1306/ArduiPi_OLED.cpp \
    ssd1306/glcdfont.c

HEADERS += \
    bcm2835/bcm2835.h \
    ssd1306/ArduiPi_OLED.h \
    ssd1306/ArduiPi_OLED_lib.h
}

contains(QT_ARCH, x86_64) {
message( PC version )
QT += network

SOURCES += \
    remoteframebuff.cpp

HEADERS += \
    remoteframebuff.h
}
