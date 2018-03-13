#ifndef BLUETOOTHSERVER_H
#define BLUETOOTHSERVER_H

#include <qbluetoothserviceinfo.h>
#include <qbluetoothaddress.h>

#include <QtCore/QObject>
#include <QtCore/QList>

QT_FORWARD_DECLARE_CLASS(QBluetoothServer)
QT_FORWARD_DECLARE_CLASS(QBluetoothSocket)

class BluetoothServer : public QObject
{
    Q_OBJECT

public:
    BluetoothServer(QObject *parent = 0);
    ~BluetoothServer();

    bool startServer(const QBluetoothAddress &localAdapter = QBluetoothAddress());
    void stopServer();
    bool isRunning();

public slots:
    void sendMessage(const QString &message);

signals:
    void messageReceived(const QString &sender, const QString &message);
    void clientConnected(const QString &name);
    void clientDisconnected(const QString &name);

private slots:
    void clientConnected();
    void clientDisconnected();
    void readSocket();

private:
    QBluetoothServer *rfcommServer;
    QBluetoothServiceInfo serviceInfo;
    QList<QBluetoothSocket *> clientSockets;
    bool running;
};

extern const QLatin1String serviceUuid;

#endif // BLUETOOTHSERVER_H
