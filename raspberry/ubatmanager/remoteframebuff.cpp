#include "remoteframebuff.h"

#include <QTcpSocket>

RemoteFrameBuff::RemoteFrameBuff(const QString &remote_ip, int remote_port)
{
    rem_ip = remote_ip;
    rem_port = remote_port;
}

bool RemoteFrameBuff::init()
{
    QTcpSocket sock;
    sock.connectToHost(rem_ip,rem_port);
    if(!sock.waitForConnected(3000)) return false;
    sock.disconnectFromHost();
    return true;
}

void RemoteFrameBuff::sendFB(const uint8_t *data, const int size)
{
    QTcpSocket sock;
    sock.connectToHost(rem_ip,rem_port);
    if(!sock.waitForConnected(3000)) return;
    QByteArray qb;
    qb.append( QByteArray::fromRawData((const char*)data, size).toBase64() );
    uint8_t bsize[2];
    bsize[0] = (qb.size() >> 8) &0xff;
    bsize[1] = qb.size() & 0xff;
    qb.prepend((const char*)bsize,2);
    qDebug( "%d from %d", qb.size(), size );
    sock.write(qb);
    if(!sock.waitForBytesWritten(3000)) return;
    sock.disconnectFromHost();
    sock.waitForDisconnected(3000);
}
