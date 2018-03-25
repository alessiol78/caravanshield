/***************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.5
import QtQuick.Layouts 1.3

GamePage {
    id: measurePage

    errorMessage: deviceHandler.error
    infoMessage: deviceHandler.info

    property real __timeCounter: 0;
    property real __maxTimeCount: 60

    function close()
    {
        deviceHandler.stopMeasurement();
        deviceHandler.disconnectService();
        app.prevPage();
    }

    function start()
    {
        if (!deviceHandler.measuring) {
            __timeCounter = 0;
            deviceHandler.startMeasurement()
        }
    }

    function stop()
    {
        if (deviceHandler.measuring) {
            deviceHandler.stopMeasurement()
        }

        app.showPage("Stats.qml")
    }

    function init() {
        start()
    }

    Timer {
        id: measureTimer
        interval: 1000
        running: deviceHandler.measuring
        repeat: true
        onTriggered: {
            __timeCounter++;
            if (__timeCounter >= __maxTimeCount)
                measurePage.stop()
        }
    }

    GridLayout {
        id: grid
        columns: 2
        anchors.centerIn: parent

        Rectangle {
            id: rect_vbat
            width: Math.min(measurePage.width, measurePage.height-GameSettings.fieldHeight*4)/2 - GameSettings.fieldMargin
            height: width
            radius: width*0.05
            color: GameSettings.viewColor

            Text {
                id: text
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -parent.height*0.15
                font.pixelSize: parent.width * 0.45
                text: deviceHandler.vbat.toPrecision(3)
                color: GameSettings.textColor
                visible: deviceHandler.measuring
            }

            Item {
                id: descriptionContainer1
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width*0.7
                height: parent.height * 0.15
                anchors.bottom: parent.bottom
                anchors.bottomMargin: parent.height*0.16

                Text {
                    anchors.centerIn: parent
                    anchors.verticalCenter: parent.verticalCenter
                    text: "V bat"
                    color: GameSettings.textColor
                    font.pixelSize: GameSettings.hugeFontSize * 0.8
                }
            }
        }

        Rectangle {
            id: rect_vsol
            width: Math.min(measurePage.width, measurePage.height-GameSettings.fieldHeight*4)/2 - GameSettings.fieldMargin
            height: width
            radius: width*0.05
            color: GameSettings.viewColor

            Text {
                id: text2
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -parent.height*0.15
                font.pixelSize: parent.width * 0.45
                text: deviceHandler.vsol.toPrecision(3)
                color: GameSettings.textColor
                visible: deviceHandler.measuring
            }

            Item {
                id: descriptionContainer2
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width*0.7
                height: parent.height * 0.15
                anchors.bottom: parent.bottom
                anchors.bottomMargin: parent.height*0.16

                Text {
                    anchors.centerIn: parent
                    anchors.verticalCenter: parent.verticalCenter
                    text: "V sol"
                    color: GameSettings.textColor
                    font.pixelSize: GameSettings.hugeFontSize * 0.8
                }
            }
        }

        Rectangle {
            id: rect_valim
            //anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(measurePage.width, measurePage.height-GameSettings.fieldHeight*4)/2 - GameSettings.fieldMargin
            height: width
            radius: width*0.05
            color: GameSettings.viewColor

            Text {
                id: text3
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -parent.height*0.15
                font.pixelSize: parent.width * 0.45
                text: deviceHandler.valim.toPrecision(3)
                color: GameSettings.textColor
                visible: deviceHandler.measuring
            }

            Item {
                id: descriptionContainer3
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width*0.7
                height: parent.height * 0.15
                anchors.bottom: parent.bottom
                anchors.bottomMargin: parent.height*0.16

                Text {
                    anchors.centerIn: parent
                    anchors.verticalCenter: parent.verticalCenter
                    text: "V alim"
                    color: GameSettings.textColor
                    font.pixelSize: GameSettings.hugeFontSize * 0.8
                }
            }
        }

        Rectangle {
            id: rect_ialim
            width: Math.min(measurePage.width, measurePage.height-GameSettings.fieldHeight*4)/2 - GameSettings.fieldMargin
            height: width
            radius: width*0.05
            color: GameSettings.viewColor

            Text {
                id: text4
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -parent.height*0.15
                font.pixelSize: parent.width * 0.45
                text: deviceHandler.ialim.toPrecision(2)
                color: GameSettings.textColor
                visible: deviceHandler.measuring
            }

            Item {
                id: descriptionContainer4
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width*0.7
                height: parent.height * 0.15
                anchors.bottom: parent.bottom
                anchors.bottomMargin: parent.height*0.16

                Text {
                    anchors.centerIn: parent
                    anchors.verticalCenter: parent.verticalCenter
                    text: "I alim"
                    color: GameSettings.textColor
                    font.pixelSize: GameSettings.hugeFontSize * 0.8
                }
            }
        }
    }

    Rectangle {
        id: timeSlider
        color: GameSettings.viewColor
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: GameSettings.fieldMargin
        width: measurePage.width - 2*GameSettings.fieldMargin
        height: GameSettings.fieldHeight
        radius: GameSettings.buttonRadius

        Rectangle {
            height: parent.height
            radius: parent.radius
            color: deviceHandler.relayOn ? GameSettings.sliderColor : GameSettings.viewColor
            //width: Math.min(1.0,__timeCounter / __maxTimeCount) * parent.width
            width: parent.width
        }

        Text {
            anchors.centerIn: parent
            color: "white"
            text: "solar powered"
            font.pixelSize: GameSettings.bigFontSize
        }
    }
}
