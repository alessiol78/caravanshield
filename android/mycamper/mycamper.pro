TEMPLATE = app
TARGET = mycamper

QT += qml quick bluetooth
CONFIG += c++11

HEADERS += \
    connectionhandler.h \
    deviceinfo.h \
    devicefinder.h \
    devicehandler.h \
    bluetoothbaseclass.h \
    mycamper-global.h

SOURCES += main.cpp \
    connectionhandler.cpp \
    deviceinfo.cpp \
    devicefinder.cpp \
    devicehandler.cpp \
    bluetoothbaseclass.cpp

RESOURCES += qml.qrc \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

