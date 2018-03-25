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

#include "heartrate-global.h"
#include "devicehandler.h"
#include "deviceinfo.h"
#include <QtEndian>
#include <QDataStream>

DeviceHandler::DeviceHandler(QObject *parent) :
    BluetoothBaseClass(parent),
    m_control(0),
    m_service(0),
    m_currentDevice(0),
    m_foundService(false),
    m_measuring(false),
    m_out_relay(false)
{
}

void DeviceHandler::setAddressType(AddressType type)
{
    switch (type) {
    case DeviceHandler::AddressType::PublicAddress:
        m_addressType = QLowEnergyController::PublicAddress;
        break;
    case DeviceHandler::AddressType::RandomAddress:
        m_addressType = QLowEnergyController::RandomAddress;
        break;
    }
}

DeviceHandler::AddressType DeviceHandler::addressType() const
{
    if (m_addressType == QLowEnergyController::RandomAddress)
        return DeviceHandler::AddressType::RandomAddress;

    return DeviceHandler::AddressType::PublicAddress;
}

void DeviceHandler::setDevice(DeviceInfo *device)
{
    clearMessages();
    m_currentDevice = device;

#ifdef SIMULATOR
    setInfo(tr("Demo device connected."));
    return;
#endif

    // Disconnect and delete old connection
    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = 0;
    }

    // Create new controller and connect it if device available
    if (m_currentDevice) {

        // Make connections
        //! [Connect-Signals-1]
        m_control = new QLowEnergyController(m_currentDevice->getDevice(), this);
        //! [Connect-Signals-1]
        m_control->setRemoteAddressType(m_addressType);
        //! [Connect-Signals-2]
        connect(m_control, &QLowEnergyController::serviceDiscovered,
                this, &DeviceHandler::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished,
                this, &DeviceHandler::serviceScanDone);

        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
            Q_UNUSED(error);
            setError("Cannot connect to remote device.");
        });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            setInfo("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            setError("LowEnergy controller disconnected");
        });

        // Connect
        m_control->connectToDevice();
        //! [Connect-Signals-2]
    }
}

void DeviceHandler::startMeasurement()
{
    if (alive()) {
        m_start = QDateTime::currentDateTime();
        m_measuring = true;
        emit measuringChanged();
    }
}

void DeviceHandler::stopMeasurement()
{
    m_measuring = false;
    emit measuringChanged();
}

//! [Filter HeartRate service 1]
void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt == QBluetoothUuid(QBluetoothUuid::GenericAttribute)) {
        setInfo("Heart Rate service discovered. Waiting for service scan to be done...");
        m_foundService = true;
    }
}
//! [Filter HeartRate service 1]

void DeviceHandler::serviceScanDone()
{
    setInfo("Service scan done.");

    // Delete old service if available
    if (m_service) {
        delete m_service;
        m_service = 0;
    }

//! [Filter HeartRate service 2]
    // If heartRateService found, create new service
    if (m_foundService)
        m_service = m_control->createServiceObject(QBluetoothUuid(QBluetoothUuid::GenericAttribute), this);

    if (m_service) {
        connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
        connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::updateCharacteristicValue);
        //connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);
        m_service->discoverDetails();
    } else {
        setError("Heart Rate Service not found.");
    }
//! [Filter HeartRate service 2]
}

// Service functions
//! [Find HRM characteristic]
void DeviceHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::DiscoveringServices:
        setInfo(tr("Discovering services..."));
        break;
    case QLowEnergyService::ServiceDiscovered:
    {
        setInfo(tr("Service discovered."));

        m_measuring = true;
        emit measuringChanged();

        const QLowEnergyCharacteristic hrChar1 = m_service->characteristic(
                    QBluetoothUuid((QBluetoothUuid::CharacteristicType)0x4001));
        if (!hrChar1.isValid()) {
            setError("Vbat Data not found.");
            break;
        }

        m_notificationDescs << hrChar1.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDescs.last().isValid())
            m_service->writeDescriptor(m_notificationDescs.last(), QByteArray::fromHex("0100"));
        //---
        const QLowEnergyCharacteristic hrChar2 = m_service->characteristic(
                    QBluetoothUuid((QBluetoothUuid::CharacteristicType)0x4002));
        if (!hrChar2.isValid()) {
            setError("Vsol Data not found.");
            break;
        }

        m_notificationDescs << hrChar2.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDescs.last().isValid())
            m_service->writeDescriptor(m_notificationDescs.last(), QByteArray::fromHex("0100"));
        //---
        const QLowEnergyCharacteristic hrChar3 = m_service->characteristic(
                    QBluetoothUuid((QBluetoothUuid::CharacteristicType)0x4003));
        if (!hrChar3.isValid()) {
            setError("Valim Data not found.");
            break;
        }

        m_notificationDescs << hrChar3.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDescs.last().isValid())
            m_service->writeDescriptor(m_notificationDescs.last(), QByteArray::fromHex("0100"));
        //---
        const QLowEnergyCharacteristic hrChar4 = m_service->characteristic(
                    QBluetoothUuid((QBluetoothUuid::CharacteristicType)0x4004));
        if (!hrChar4.isValid()) {
            setError("Ialim Data not found.");
            break;
        }

        m_notificationDescs << hrChar4.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDescs.last().isValid())
            m_service->writeDescriptor(m_notificationDescs.last(), QByteArray::fromHex("0100"));
        break;
    }
    default:
        //nothing for now
        break;
    }

    emit aliveChanged();
}
//! [Find HRM characteristic]

//! [Reading value]
void DeviceHandler::updateCharacteristicValue(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    QByteArray qb = value;
    // ignore any other characteristic change -> shouldn't really happen though
    //setInfo(c.uuid().toString());
    if (c.uuid() == QBluetoothUuid((QBluetoothUuid::CharacteristicType)0x4001) ) // vbat
    {
        QDataStream v(&qb,QIODevice::ReadOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v >> gm.v_bat_serv;
    }
    else if (c.uuid() == QBluetoothUuid((QBluetoothUuid::CharacteristicType)0x4002) ) //vsol
    {
        QDataStream v(&qb,QIODevice::ReadOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v >> gm.v_out_sol;
    }
    else if (c.uuid() == QBluetoothUuid((QBluetoothUuid::CharacteristicType)0x4003) ) //valim
    {
        QDataStream v(&qb,QIODevice::ReadOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v >> gm.v_alim;
    }
    else if (c.uuid() == QBluetoothUuid((QBluetoothUuid::CharacteristicType)0x4004) ) //ialim
    {
        QDataStream v(&qb,QIODevice::ReadOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v >> gm.i_alim;
    }
    emit statsChanged();
}
//! [Reading value]

void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    if (d.isValid() && m_notificationDescs.indexOf(d)!=-1 && value == QByteArray::fromHex("0000") && m_service) {
        //disabled notifications -> assume disconnect intent
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = 0;
    }
}

void DeviceHandler::disconnectService()
{
    m_foundService = false;

    //disable notifications
    /*if (m_notificationDesc.isValid() && m_service
            && m_notificationDesc.value() == QByteArray::fromHex("0100")) {
        m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0000"));
    } else*/ {
        if (m_control)
            m_control->disconnectFromDevice();

        delete m_service;
        m_service = 0;
    }
}

bool DeviceHandler::measuring() const
{
    return m_measuring;
}

bool DeviceHandler::alive() const
{
#ifdef SIMULATOR
    return true;
#endif

    if (m_service)
        return m_service->state() == QLowEnergyService::ServiceDiscovered;

    return false;
}

float DeviceHandler::vbat() const
{
    return gm.v_bat_serv;
}

float DeviceHandler::vsol() const
{
    return gm.v_out_sol;
}

float DeviceHandler::valim() const
{
    return gm.v_alim;
}

float DeviceHandler::ialim() const
{
    return gm.i_alim;
}

bool DeviceHandler::relayOn() const
{
    return m_out_relay;
}

int DeviceHandler::time() const
{
    return m_start.secsTo(m_stop);
}

