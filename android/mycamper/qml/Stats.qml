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
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

GamePage {

    function init() {
        opacity = 1.0
    }

    Rectangle {
        id: viewContainer
        anchors.top: parent.top
        anchors.bottom: saveButton.top
        anchors.topMargin: GameSettings.fieldMargin + messageHeight
        anchors.bottomMargin: GameSettings.fieldMargin
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - GameSettings.fieldMargin*2
        color: GameSettings.viewColor
        radius: GameSettings.buttonRadius


        Text {
            id: title
            width: parent.width
            height: GameSettings.fieldHeight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: GameSettings.textColor
            font.pixelSize: GameSettings.mediumFontSize
            text: qsTr("CONFIGURE")

            BottomLine {
                height: 1;
                width: parent.width
                color: "#898989"
            }
        }

        ListModel {
            id: configModel

            //-- threashold
            ListElement {
                name: "Thr Vbat H"
                ivalue: 0
            }
            ListElement {
                name: "Thr Vbat L"
                ivalue: 0
            }
            ListElement {
                name: "Thr Vsol H"
                ivalue: 0
            }
            ListElement {
                name: "Thr Vsol L"
                ivalue: 0
            }
            ListElement {
                name: "Thr Valim H"
                ivalue: 0
            }
            ListElement {
                name: "Thr Valim L"
                ivalue: 0
            }
            //-- gain
            ListElement {
                name: "Gain V bat"
                ivalue: 0.2 //deviceHandler.config.a_valim.toPrecision(6)
            }
            ListElement {
                name: "Gain V sol"
                ivalue: 3.25
            }
            ListElement {
                name: "Gain V alim"
                ivalue: 1.95
            }
            ListElement {
                name: "Gain I alim"
                ivalue: 0.2 //deviceHandler.config.a_valim.toPrecision(6)
            }
            ListElement {
                name: "Gain I motor"
                ivalue: 3.25
            }
            //-- ntc
            ListElement {
                name: "Coeff A0 NTC 1"
                ivalue: 0
            }
            ListElement {
                name: "Coeff A1 NTC 1"
                ivalue: 0
            }
            ListElement {
                name: "Coeff A2 NTC 1"
                ivalue: 0
            }
            ListElement {
                name: "Coeff A3 NTC 1"
                ivalue: 0
            }
            ListElement {
                name: "Coeff A0 NTC 2"
                ivalue: 0
            }
            ListElement {
                name: "Coeff A1 NTC 2"
                ivalue: 0
            }
            ListElement {
                name: "Coeff A2 NTC 2"
                ivalue: 0
            }
            ListElement {
                name: "Coeff A3 NTC 2"
                ivalue: 0
            }
        }

        ListView {
            id: params
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: title.bottom
            model: configModel
            clip: true

            delegate: Rectangle {
                id: box
                height:GameSettings.fieldHeight * 1.2
                width: parent.width
                color: GameSettings.delegate1Color

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        /*deviceFinder.connectToService(modelData.deviceAddress);
                        app.showPage("Measure.qml")*/
                    }
                }

                StatsLabel {
                    title: name
                    value: ivalue
                }
            }
        }
    }

    GameButton {
        id: saveButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: GameSettings.fieldMargin
        width: viewContainer.width
        height: GameSettings.fieldHeight
        //enabled: !deviceFinder.scanning
        //onClicked: deviceFinder.startSearch()

        Text {
            anchors.centerIn: parent
            font.pixelSize: GameSettings.tinyFontSize
            text: qsTr("SAVE")
            color: saveButton.enabled ? GameSettings.textColor : GameSettings.disabledTextColor
        }
    }

    /*
    Column {
        anchors.centerIn: parent
        width: parent.width

        Text {
            id: titleText
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: GameSettings.hugeFontSize
            color: GameSettings.textColor
            text: qsTr("CONFIGURE")
        }

        Item {
            height: GameSettings.fieldHeight
            width: 1
        }

        Flickable {
            anchors.fill: parent
            contentWidth: viewport.width
            contentHeight: labelsColumn.height + GameSettings.fieldHeight * 1.5
            ScrollView {
                id: scrollView
                width: Math.min(content.width + 2*GameSettings.fieldMargin, appWindow.width);
                height: Math.min(content.height, appWindow.height);

                Item {
                    id: content
                    width: titleText.width
                    height: 14*GameSettings.fieldHeight

                    ColumnLayout {
                        id: labelsColumn
                        anchors.fill: parent
                        anchors.centerIn: parent
                        anchors.margins: GameSettings.fieldMargin
                        //width: parent.width

                        StatsLabel {
                            title: qsTr("Gain VBat")
                            value: deviceHandler.config.a_vbatsrv.toFixed(2)
                        }

                        StatsLabel {
                            title: qsTr("Gain VSol")
                            value: deviceHandler.config.a_voutsol.toFixed(2)
                        }

                        StatsLabel {
                            title: qsTr("Gain Valim")
                            value: deviceHandler.config.a_valim.toFixed(2)
                        }

                        StatsLabel {
                            title: qsTr("Gain Ialim")
                            value: deviceHandler.config.a_ialim.toFixed(3)
                        }

                        StatsLabel {
                            title: qsTr("Gain Imotor")
                            value: deviceHandler.config.a_imotor.toFixed(3)
                        }

                        //---
                        StatsLabel {
                            title: qsTr("Threashold (H) VSol")
                            value: deviceHandler.config.thr_vsol_h.toFixed(2)
                        }

                        StatsLabel {
                            title: qsTr("Threashold (L) VSol")
                            value: deviceHandler.config.thr_vsol_l.toFixed(2)
                        }

                        StatsLabel {
                            title: qsTr("Threashold (H) VBat")
                            value: deviceHandler.config.thr_vbat_h.toFixed(2)
                        }

                        StatsLabel {
                            title: qsTr("Threashold (L) VBat")
                            value: deviceHandler.config.thr_vbat_l.toFixed(3)
                        }

                        StatsLabel {
                            title: qsTr("Threashold (H) Valim")
                            value: deviceHandler.config.thr_valim_h.toFixed(3)
                        }

                        StatsLabel {
                            title: qsTr("Threashold (L) Valim")
                            value: deviceHandler.config.thr_valim_l.toFixed(3)
                        }
                    }
                }
            }
        }
    }*/
}
