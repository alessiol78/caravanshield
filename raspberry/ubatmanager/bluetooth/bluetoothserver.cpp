#include "bluetoothserver.h"
#include <qbluetoothserver.h>
#include <qbluetoothsocket.h>
#include <qbluetoothlocaldevice.h>

const QLatin1String serviceUuid("fa87c0d0-afac-11de-8a39-0800200c9a66");

BluetoothServer::BluetoothServer(QObject *parent)
    : QObject(parent), rfcommServer(0), running(false)
{
}

BluetoothServer::~BluetoothServer()
{
    stopServer();
}

bool BluetoothServer::startServer(const QBluetoothAddress& localAdapter)
{
    running = false;
    if (rfcommServer)
        return false;

    rfcommServer = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(rfcommServer, SIGNAL(newConnection()), this, SLOT(clientConnected()));
    bool result = rfcommServer->listen(localAdapter);
    if (!result) {
        qWarning() << "Cannot bind bluetooth server to" << localAdapter.toString();
        return false;
    }

    //serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceRecordHandle, (uint)0x00010010);

    //-- create service uuid
    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                             classId);
    classId.prepend(QVariant::fromValue(QBluetoothUuid(serviceUuid)));
    //-----

    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, tr("BT Caravan Battery Server"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription,
                             tr("Caravan bluetooth battery server"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, tr("qt-project.org"));
    serviceInfo.setServiceUuid(QBluetoothUuid(serviceUuid));

    // service Discoverability
    QBluetoothServiceInfo::Sequence publicBrowse;
    publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList,
                             publicBrowse);

    // protocol descriptor list
    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
             << QVariant::fromValue(quint8(rfcommServer->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                             protocolDescriptorList);

    // register service
    serviceInfo.registerService(localAdapter);

    running = true;
    return true;
}

void BluetoothServer::stopServer()
{
    // Unregister service
    serviceInfo.unregisterService();

    // Close sockets
    qDeleteAll(clientSockets);

    // Close server
    delete rfcommServer;
    rfcommServer = 0;
    running = false;
}

bool BluetoothServer::isRunning()
{
    return running;
}

void BluetoothServer::sendMessage(const QString &message)
{
    QByteArray text = message.toUtf8() + '\n';

    foreach (QBluetoothSocket *socket, clientSockets)
        socket->write(text);
}

void BluetoothServer::clientConnected()
{
    QBluetoothSocket *socket = rfcommServer->nextPendingConnection();
    if (!socket)
        return;

    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    clientSockets.append(socket);
    emit clientConnected(socket->peerName());
}

void BluetoothServer::clientDisconnected()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket)
        return;

    emit clientDisconnected(socket->peerName());

    clientSockets.removeOne(socket);

    socket->deleteLater();
}

void BluetoothServer::readSocket()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket)
        return;

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        emit messageReceived(socket->peerName(),
                             QString::fromUtf8(line.constData(), line.length()));
    }
}
