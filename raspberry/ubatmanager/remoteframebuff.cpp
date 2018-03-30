#include "remoteframebuff.h"

#include <QTcpSocket>

RemoteFrameBuff::RemoteFrameBuff(const QString &remote_ip, int remote_port)
{
    rem_ip = remote_ip;
    rem_port = remote_port;
    btn_a_down = false;
    btn_b_down = false;
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
#if 1
    int _width = 128/8; int _height=64;
    //qDebug("rotate!!!");
    uint8_t tmpbuf[size];
    for(int n=_height-1, i=0;n>=0;n--,i++)
    { memcpy(&tmpbuf[n*_width],&data[i*_width],_width); }
    uint8_t *p = tmpbuf;
#endif
    qb.append( QByteArray::fromRawData((const char*)p, size).toBase64() );
    uint8_t bsize[2];
    bsize[0] = (qb.size() >> 8) &0xff;
    bsize[1] = qb.size() & 0xff;
    qb.prepend((const char*)bsize,2);
//    qDebug( "%d from %d", qb.size(), size );
    sock.write(qb);
    if(!sock.waitForBytesWritten(3000)) return;
    if( sock.waitForReadyRead(500) )
    {
        QByteArray resp = sock.readAll();
//        qDebug( "rx: %s", resp.constData() );
        if(resp.size()==4) {
            btn_a_down = resp.at(1)=='1' ? true : false;
            btn_b_down = resp.at(3)=='1' ? true : false;
        }
    }
    sock.disconnectFromHost();
//    sock.waitForDisconnected(3000);
}

int RemoteFrameBuff::buttonA()
{
    return btn_a_down ? 1 : 0;
}

int RemoteFrameBuff::buttonB()
{
    return btn_b_down ? 1 : 0;
}
