#ifndef BTMANAGER_H
#define BTMANAGER_H

#include "project.h"

#include <QObject>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothsocket.h>
#include <qbluetoothhostinfo.h>
#include <QMutex>

QT_USE_NAMESPACE

class BluetoothServer;

class BTmanager : public QObject
{
    Q_OBJECT
public:
    explicit BTmanager(QObject *parent = nullptr);
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
    void clientConnected(const QString &name);
    void clientDisconnected(const QString &name);
    void receiveMessage(const QString &sender, const QString &message);

private:
    QList<QBluetoothHostInfo> localAdapters;
    BluetoothServer *server;
    QString localName;
    gmeasures gm;
    ConfMeasures conf;
    QMutex mtx;
};

#endif // BTMANAGER_H
