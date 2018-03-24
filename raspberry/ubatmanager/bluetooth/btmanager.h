#ifndef BTMANAGER_H
#define BTMANAGER_H

#include "project.h"

#include <QObject>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothsocket.h>
#include <qbluetoothhostinfo.h>
#include <qlowenergycharacteristicdata.h>
#include <qlowenergyservicedata.h>
#include <qlowenergyadvertisingdata.h>
#include <qlowenergycontroller.h>
#include <QMutex>

QT_USE_NAMESPACE

class BluetoothServer;
enum CharacteristicType {
    CharFloat,
    CharTemp,
    CharConfAdc,
    CharConfNtc,
    CharConfThr
};

class BTmanager : public QObject
{
    Q_OBJECT
public:
    explicit BTmanager(bool ble = false, QObject *parent = nullptr);
    ~BTmanager();
    QString adapterName();
    bool start();
    void stop();
    bool isRunning();

    void updateMeasures(const gmeasures *m);
    void updateConf(const ConfMeasures &cm);

signals:
    void sendMessage(const QString &message);
    void newConf(const ConfMeasures &newconf);

private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void clientConnected(const QString &name);
    void clientDisconnected(const QString &name);
    void receiveMessage(const QString &sender, const QString &message);
    void reconnect();
    void refreshLE();

private:
    QLowEnergyCharacteristicData createCharacteristic(const QString &name, const short uuid, CharacteristicType typeval, const QVariant &value);
    QHash<QString,QBluetoothUuid::CharacteristicType> uuidMeasures;
    QScopedPointer<QLowEnergyController> leController;
    QScopedPointer<QLowEnergyService> service;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceData;
    QList<QBluetoothHostInfo> localAdapters;
    BluetoothServer *server;
    QString localName;
    gmeasures gm;
    ConfMeasures conf;
    QMutex mtx;
    bool hasble;
};

#endif // BTMANAGER_H
