#include "btmanager.h"
#include "bluetoothserver.h"

#include <qbluetoothuuid.h>
#include <qbluetoothserver.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothlocaldevice.h>

#include <QJsonObject>
#include <QJsonDocument>

#include <iostream>

BTmanager::BTmanager(QObject *parent)
  : QObject(parent)
{

    localAdapters = QBluetoothLocalDevice::allDevices();

    server = new BluetoothServer(this);
    connect(server, SIGNAL(clientConnected(QString)), this, SLOT(clientConnected(QString)));
    connect(server, SIGNAL(clientDisconnected(QString)), this, SLOT(clientDisconnected(QString)));
    connect(server, SIGNAL(messageReceived(QString,QString)),
            this, SLOT(receiveMessage(QString,QString)));
    connect(this, SIGNAL(sendMessage(QString)), server, SLOT(sendMessage(QString)));

    localName = QBluetoothLocalDevice().name();
}

BTmanager::~BTmanager()
{
    delete server;
}

bool BTmanager::start()
{
    return server->startServer();
}

void BTmanager::stop()
{
    server->stopServer();
}

bool BTmanager::isRunning()
{
    return server->isRunning();
}

QString BTmanager::adapterName()
{
    return localName;
}

void BTmanager::clientConnected(const QString &name)
{
    std::cout << QString::fromLatin1("%1 connected.\n").arg(name).toUtf8().constData();
}

void BTmanager::clientDisconnected(const QString &name)
{
    std::cout << QString::fromLatin1("%1 disconnected.\n").arg(name).toUtf8().constData();
}


void BTmanager::updateMeasures(const gmeasures *m)
{
    mtx.lock();
    memcpy(&gm,m,sizeof(gmeasures));
    mtx.unlock();
}

void BTmanager::updateConf(const ConfMeasures &cm)
{
    mtx.lock();
    conf = cm;
    mtx.unlock();
}

void BTmanager::receiveMessage(const QString &sender, const QString &message)
{
    Q_UNUSED(sender)
    if(message.startsWith("status"))
    {
        mtx.lock();
        QJsonObject json;
        json["vbat"] = (double)gm.v_bat_serv;
        json["vsol"] = (double)gm.v_out_sol;
        json["valm"] = (double)gm.v_alim;
        json["ialm"] = (double)gm.i_alim;
        json["imot"] = (double)gm.i_motor;
        //resp << "orel=" + QString::number(gMainThrd->relay());
        mtx.unlock();
        emit sendMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
    }
    else if(message.startsWith("conf"))
    {
        if(message.size()==4)
        {
            mtx.lock();
            QJsonObject json;
            json["a_valim"] = (double)conf.a_valim;
            json["a_vbatsrv"] = (double)conf.a_vbatsrv;
            json["a_voutsol"] = (double)conf.a_voutsol;
            json["a_ialim"] = (double)conf.a_ialim;
            json["a_imotor"] = (double)conf.a_imotor;
            json["ntc1_a0"] = (double)conf.ntc1_a0;
            json["ntc1_a1"] = (double)conf.ntc1_a1;
            json["ntc1_a2"] = (double)conf.ntc1_a2;
            json["ntc1_a3"] = (double)conf.ntc1_a3;
            json["ntc2_a0"] = (double)conf.ntc2_a0;
            json["ntc2_a1"] = (double)conf.ntc2_a1;
            json["ntc2_a2"] = (double)conf.ntc2_a2;
            json["ntc2_a3"] = (double)conf.ntc2_a3;
            json["thr_vsol_h"] = (double)conf.thr_vsol_h;
            json["thr_vsol_l"] = (double)conf.thr_vsol_l;
            json["thr_vbat_h"] = (double)conf.thr_vbat_h;
            json["thr_vbat_l"] = (double)conf.thr_vbat_l;
            json["thr_valim_h"] = (double)conf.thr_valim_h;
            json["thr_valim_l"] = (double)conf.thr_valim_l;
            mtx.unlock();
            emit sendMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
        }
        else
        {
            ConfMeasures newconf;
            QJsonObject json = QJsonDocument::fromJson(message.mid(5).toUtf8()).object();
            newconf.a_valim = (float)json["a_valim"].toDouble();
            newconf.a_vbatsrv = (float)json["a_vbatsrv"].toDouble();
            newconf.a_voutsol = (float)json["a_voutsol"].toDouble();
            newconf.a_ialim = (float)json["a_ialim"].toDouble();
            newconf.a_imotor = (float)json["a_imotor"].toDouble();
            newconf.ntc1_a0 = (float)json["ntc1_a0"].toDouble();
            newconf.ntc1_a1 = (float)json["ntc1_a1"].toDouble();
            newconf.ntc1_a2 = (float)json["ntc1_a2"].toDouble();
            newconf.ntc1_a3 = (float)json["ntc1_a3"].toDouble();
            newconf.ntc2_a0 = (float)json["ntc2_a0"].toDouble();
            newconf.ntc2_a1 = (float)json["ntc2_a1"].toDouble();
            newconf.ntc2_a2 = (float)json["ntc2_a2"].toDouble();
            newconf.ntc2_a3 = (float)json["ntc2_a3"].toDouble();
            newconf.thr_vsol_h = (float)json["thr_vsol_h"].toDouble();
            newconf.thr_vsol_l = (float)json["thr_vsol_l"].toDouble();
            newconf.thr_vbat_h = (float)json["thr_vbat_h"].toDouble();
            newconf.thr_vbat_l = (float)json["thr_vbat_l"].toDouble();
            newconf.thr_valim_h = (float)json["thr_valim_h"].toDouble();
            newconf.thr_valim_l = (float)json["thr_valim_l"].toDouble();
            emit newConf(newconf);
        }
    }
}
