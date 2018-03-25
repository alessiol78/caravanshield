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

#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "bluetoothbaseclass.h"

#include <QDateTime>
#include <QVector>
#include <QTimer>
#include <QLowEnergyController>
#include <QLowEnergyService>

class DeviceInfo;

typedef struct {
    float v_bat_serv;
    float v_out_sol;
    float i_alim;
    float i_motor;
    float ntc_1;
    float ntc_2;
    float v_alim;
} gmeasures;

class ConfMeasures {
public:
    //-- params for adc conversion
    float a_valim;
    float a_vbatsrv;
    float a_voutsol;
    float a_ialim;
    float a_imotor;
    float ntc1_a0;
    float ntc1_a1;
    float ntc1_a2;
    float ntc1_a3;
    float ntc2_a0;
    float ntc2_a1;
    float ntc2_a2;
    float ntc2_a3;
    //---- threashold for switch strategy
    float thr_vsol_h;   // min V out solar controller good
    float thr_vsol_l;   // under voltage level out solar protection
    float thr_vbat_h;   // V service battery
    float thr_vbat_l;   // V service battery
    float thr_valim_h;  // V motor recharger
    float thr_valim_l;  // V motor recharger

    ConfMeasures& operator=(const ConfMeasures &o) {
        a_valim = o.a_valim;
        a_vbatsrv = o.a_vbatsrv;
        a_voutsol = o.a_voutsol;
        a_ialim = o.a_ialim;
        a_imotor = o.a_imotor;
        ntc1_a0 = o.ntc1_a0;
        ntc1_a1 = o.ntc1_a1;
        ntc1_a2 = o.ntc1_a2;
        ntc1_a3 = o.ntc1_a3;
        ntc2_a0 = o.ntc2_a0;
        ntc2_a1 = o.ntc2_a1;
        ntc2_a2 = o.ntc2_a2;
        ntc2_a3 = o.ntc2_a3;
        thr_vsol_h = o.thr_vsol_h;
        thr_vsol_l = o.thr_vsol_l;
        thr_vbat_h = o.thr_vbat_h;
        thr_vbat_l = o.thr_vbat_l;
        thr_valim_h = o.thr_valim_h;
        thr_valim_l = o.thr_valim_l;
        return *this;
    }
};
Q_DECLARE_METATYPE(ConfMeasures)

class DeviceHandler : public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool measuring READ measuring NOTIFY measuringChanged)
    Q_PROPERTY(bool alive READ alive NOTIFY aliveChanged)
    Q_PROPERTY(float vbat READ vbat NOTIFY statsChanged)
    Q_PROPERTY(float vsol READ vsol NOTIFY statsChanged)
    Q_PROPERTY(float valim READ valim NOTIFY statsChanged)
    Q_PROPERTY(float ialim READ ialim NOTIFY statsChanged)
    Q_PROPERTY(bool relayOn READ relayOn NOTIFY statsChanged)
    Q_PROPERTY(int time READ time NOTIFY statsChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)
    Q_PROPERTY(ConfMeasures config READ config NOTIFY configChanged)

public:
    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    DeviceHandler(QObject *parent = 0);

    void setDevice(DeviceInfo *device);
    void setAddressType(AddressType type);
    void setConfig(ConfMeasures new_conf);
    AddressType addressType() const;
    ConfMeasures config() const;

    bool measuring() const;
    bool alive() const;

    // Statistics
    float vbat() const;
    float vsol() const;
    float valim() const;
    float ialim() const;
    bool relayOn() const;
    int time() const;

signals:
    void measuringChanged();
    void aliveChanged();
    void statsChanged();
    void configChanged();

public slots:
    void startMeasurement();
    void stopMeasurement();
    void disconnectService();

private:
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void updateCharacteristicValue(const QLowEnergyCharacteristic &c,
                              const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d,
                              const QByteArray &value);

private:
    QLowEnergyController *m_control;
    QLowEnergyService *m_service;
    QList<QLowEnergyDescriptor> m_notificationDescs;
    DeviceInfo *m_currentDevice;

    bool m_foundService;
    bool m_measuring;

    // Statistics
    QDateTime m_start;
    QDateTime m_stop;

    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;

#ifdef SIMULATOR
    QTimer m_demoTimer;
#endif
    bool m_out_relay;
    gmeasures gm;
    ConfMeasures conf;
};

#endif // DEVICEHANDLER_H
