#ifndef REMOTEFRAMEBUFF_H
#define REMOTEFRAMEBUFF_H

#include <QString>

class RemoteFrameBuff
{
public:
    RemoteFrameBuff(const QString &remote_ip, int remote_port);
    void sendFB(const uint8_t *data, const int size);
    bool init();
    int buttonA();
    int buttonB();

private:
    QString rem_ip;
    int rem_port;
    bool btn_a_down;
    bool btn_b_down;
};

#endif // REMOTEFRAMEBUFF_H
