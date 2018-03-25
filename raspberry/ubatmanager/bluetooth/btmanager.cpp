#include "btmanager.h"
#include "bluetoothserver.h"

#include <qbluetoothuuid.h>
#include <qbluetoothserver.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothhostinfo.h>
#include <qlowenergyadvertisingparameters.h>
#include <qlowenergycharacteristic.h>
#include <qlowenergydescriptordata.h>
#include <qlowenergyservice.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <iostream>
#include <QDataStream>

enum CharacteristicPresentationFormat {
CharFormat_Boolean = 1,
CharFormat_unsigned_2_bit_integer,
CharFormat_unsigned_4_bit_integer,
CharFormat_unsigned_8_bit_integer,
CharFormat_unsigned_12_bit_integer,
CharFormat_unsigned_16_bit_integer,
CharFormat_unsigned_24_bit_integer,
CharFormat_unsigned_32_bit_integer,
CharFormat_unsigned_48_bit_integer,
CharFormat_unsigned_64_bit_integer,
CharFormat_unsigned_128_bit_integer,
CharFormat_signed_8_bit_integer,
CharFormat_signed_12_bit_integer,
CharFormat_signed_16_bit_integer,
CharFormat_signed_24_bit_integer,
CharFormat_signed_32_bit_integer,
CharFormat_signed_48_bit_integer,
CharFormat_signed_64_bit_integer,
CharFormat_signed_128_bit_integer,
CharFormat_IEEE_754_32_bit_floating_point,
CharFormat_IEEE_754_64_bit_floating_point,
CharFormat_IEEE_11073_16_bit_SFLOAT,
CharFormat_IEEE_11073_32_bit_FLOAT,
CharFormat_IEEE_20601_format,
CharFormat_UTF_8_string,
CharFormat_UTF_16_string,
CharFormat_Opaque_Structure
};

BTmanager::BTmanager(bool ble, QObject *parent)
  : QObject(parent), localName(""), hasble(ble)
{

    localAdapters = QBluetoothLocalDevice::allDevices();

    if(localAdapters.isEmpty()) return;

    QBluetoothDeviceInfo di;
    qDebug() << "coreConfigurations:" << di.coreConfigurations();

    QStringList l;
    foreach (QBluetoothHostInfo adbt, localAdapters) {
        l << adbt.address().toString();
    }
    qDebug() << "BT adapters:" << l;

    if(ble)
    {
        qDebug("use BLE device framework");

        // Advertising Data
        advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
        advertisingData.setIncludePowerLevel(true);
        advertisingData.setLocalName("uBTserver");
        advertisingData.setServices(QList<QBluetoothUuid>() << QBluetoothUuid::GenericAttribute);

        // Service Data
        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid::GenericAttribute);

        short uuid = 0x4001;
        serviceData.addCharacteristic(createCharacteristic("Vbat",uuid++,CharFloat,0.0f));
        serviceData.addCharacteristic(createCharacteristic("Vsol",uuid++,CharFloat,0.0f));
        serviceData.addCharacteristic(createCharacteristic("Valim",uuid++,CharFloat,0.0f));
        serviceData.addCharacteristic(createCharacteristic("Ialim",uuid++,CharFloat,0.0f));
        serviceData.addCharacteristic(createCharacteristic("Imotor",uuid++,CharFloat,0.0f));
        serviceData.addCharacteristic(createCharacteristic("ntc1",uuid++,CharFloat,-273.0f));
        serviceData.addCharacteristic(createCharacteristic("ntc2",uuid++,CharFloat,-273.0f));

        serviceData.addCharacteristic(createCharacteristic("conf_adc",uuid++,CharConfAdc,QVariant::Invalid));
        serviceData.addCharacteristic(createCharacteristic("conf_ntc1",uuid++,CharConfNtc,QVariant::Invalid));
        serviceData.addCharacteristic(createCharacteristic("conf_ntc2",uuid++,CharConfNtc,QVariant::Invalid));
        serviceData.addCharacteristic(createCharacteristic("conf_thr",uuid++,CharConfThr,QVariant::Invalid));
    }
    else
    {
        qWarning("NO BLE support, use standard bluetooth rfcomm service");

        server = new BluetoothServer(this);
        connect(server, SIGNAL(clientConnected(QString)), this, SLOT(clientConnected(QString)));
        connect(server, SIGNAL(clientDisconnected(QString)), this, SLOT(clientDisconnected(QString)));
        connect(server, SIGNAL(messageReceived(QString,QString)),
                this, SLOT(receiveMessage(QString,QString)));
        connect(this, SIGNAL(sendMessage(QString)), server, SLOT(sendMessage(QString)));
    }

    localName = QBluetoothLocalDevice().name();
}

BTmanager::~BTmanager()
{
    delete server;
}

QLowEnergyCharacteristicData BTmanager::createCharacteristic(const QString &name, const short uuid, CharacteristicType typeval, const QVariant &value)
{
    uuidMeasures[name] = (QBluetoothUuid::CharacteristicType)uuid;

    // Characteristic Data
    QLowEnergyCharacteristicData charData;
    QLowEnergyDescriptorData clientFormat;
    QLowEnergyDescriptorData clientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                QByteArray(2, 0));
    charData.setUuid(QBluetoothUuid( uuidMeasures[name] ));
    if(typeval==CharFloat)
    {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << (float)value.toDouble();
        charData.setProperties(QLowEnergyCharacteristic::Notify);
        charData.setValue(qbval);
        QByteArray qb(7,0);
        qb[0] = CharFormat_IEEE_11073_32_bit_FLOAT;
        qb[4] = 1; //Bluetooth SIG Assigned Numbers
        clientFormat = QLowEnergyDescriptorData(QBluetoothUuid::CharacteristicPresentationFormat, qb);
        clientFormat.setWritePermissions(false);
    }
    else if(typeval==CharTemp)
    {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << (float)value.toDouble();
        qbval.prepend((char)0x00); // Temperature Measurement Value (Celsius)
        charData.setProperties(QLowEnergyCharacteristic::Notify);
        charData.setValue(qbval);
    }
    else if(typeval==CharConfAdc)
    {
        QByteArray qbval(5*sizeof(float),0);
        charData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Write);
        charData.setValue(qbval);
    }
    else if(typeval==CharConfNtc)
    {
        QByteArray qbval(4*sizeof(float),0);
        charData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Write);
        charData.setValue(qbval);
    }
    else if(typeval==CharConfThr)
    {
        QByteArray qbval(6*sizeof(float),0);
        charData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Write);
        charData.setValue(qbval);
    }
    QLowEnergyDescriptorData clientDescript(QBluetoothUuid::CharacteristicUserDescription,
                                            qPrintable(name));
    clientDescript.setWritePermissions(false);
    clientConfig.setWritePermissions(true);
    charData.addDescriptor(clientConfig);
    charData.addDescriptor(clientDescript);
    if(clientFormat.isValid()) charData.addDescriptor(clientFormat);
    return charData;
}

bool BTmanager::start()
{
    if(hasble)
    {
        // Start Advertising
        leController.reset(QLowEnergyController::createPeripheral());
        qDebug() << "use:" << leController->localAddress().toString();
        service.reset(leController->addService(serviceData)); // primary service
        leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                       advertisingData);

        // check error
        if(leController.data()->error()) {
            qDebug() << "leController:" << leController.data()->errorString();
            return false;
        }
        connect(service.data(),SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
                this,SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));
        connect(leController.data(), SIGNAL(disconnected()), this, SLOT(reconnect()));
        QTimer *tmr = new QTimer;
        connect(tmr,SIGNAL(timeout()),this,SLOT(refreshLE()));
        tmr->start(600);
        return true;
    }
    return server->startServer();
}

void BTmanager::reconnect()
{
    leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                   advertisingData);
}

void BTmanager::refreshLE()
{
    QLowEnergyCharacteristic characteristic;
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["Vbat"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << gm.v_bat_serv; service->writeCharacteristic(characteristic,qbval);
    }
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["Valim"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << gm.v_alim; service->writeCharacteristic(characteristic,qbval);
    }
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["Vsol"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << gm.v_out_sol; service->writeCharacteristic(characteristic,qbval);
    }
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["Ialim"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << gm.i_alim; service->writeCharacteristic(characteristic,qbval);
    }
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["Imotor"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << gm.i_motor; service->writeCharacteristic(characteristic,qbval);
    }
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["ntc1"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << gm.ntc_1; service->writeCharacteristic(characteristic,qbval);
    }
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["ntc2"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << gm.ntc_2; service->writeCharacteristic(characteristic,qbval);
    }
}

void BTmanager::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    QString name = uuidMeasures.key( (QBluetoothUuid::CharacteristicType)
                                     characteristic.uuid().toUInt16() );
    if( name.isEmpty() ) return;
    QByteArray qbval = newValue;
    if(name=="conf_adc") {
        QDataStream v(&qbval,QIODevice::ReadOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        mtx.lock();
        v >> conf.a_ialim;
        v >> conf.a_imotor;
        v >> conf.a_valim;
        v >> conf.a_vbatsrv;
        v >> conf.a_voutsol;
        mtx.unlock();
    }
    else if(name=="conf_ntc1") {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::ReadOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        mtx.lock();
        v >> conf.ntc1_a0;
        v >> conf.ntc1_a1;
        v >> conf.ntc1_a2;
        v >> conf.ntc1_a3;
        mtx.unlock();
    }
    else if(name=="conf_ntc2") {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::ReadOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        mtx.lock();
        v >> conf.ntc2_a0;
        v >> conf.ntc2_a1;
        v >> conf.ntc2_a2;
        v >> conf.ntc2_a3;
        mtx.unlock();
    }
    else if(name=="conf_thr") {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::ReadOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        mtx.lock();
        v >> conf.thr_valim_h;
        v >> conf.thr_valim_l;
        v >> conf.thr_vbat_h;
        v >> conf.thr_vbat_l;
        v >> conf.thr_vsol_h;
        v >> conf.thr_vsol_l;
        mtx.unlock();
    }
}

void BTmanager::stop()
{
    if(hasble) return;
    server->stopServer();
}

bool BTmanager::isRunning()
{
    if(hasble) return true;
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
    QLowEnergyCharacteristic characteristic;
    if(service.isNull()) return;
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["conf_adc"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << cm.a_ialim;
        v << cm.a_imotor;
        v << cm.a_valim;
        v << cm.a_vbatsrv;
        v << cm.a_voutsol;
        service->writeCharacteristic(characteristic,qbval);
    }
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["conf_ntc1"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << cm.ntc1_a0;
        v << cm.ntc1_a1;
        v << cm.ntc1_a2;
        v << cm.ntc1_a3;
        service->writeCharacteristic(characteristic,qbval);
    }
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["conf_ntc2"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << cm.ntc2_a0;
        v << cm.ntc2_a1;
        v << cm.ntc2_a2;
        v << cm.ntc2_a3;
        service->writeCharacteristic(characteristic,qbval);
    }
    characteristic = service->characteristic(QBluetoothUuid( uuidMeasures["conf_thr"] ));
    if(characteristic.isValid()) {
        QByteArray qbval;
        QDataStream v(&qbval,QIODevice::WriteOnly);
        v.setFloatingPointPrecision(QDataStream::SinglePrecision);
        v << cm.thr_valim_h;
        v << cm.thr_valim_l;
        v << cm.thr_vbat_h;
        v << cm.thr_vbat_l;
        v << cm.thr_vsol_h;
        v << cm.thr_vsol_l;
        service->writeCharacteristic(characteristic,qbval);
    }
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
