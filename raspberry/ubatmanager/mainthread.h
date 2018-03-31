#ifndef MAINTHREAD_H
#define MAINTHREAD_H

#include "project.h"
#include "bluetooth/btmanager.h"

#include <QObject>
#include <QThread>
#include <QMutex>

QT_USE_NAMESPACE

class NTC_library;

class MainThread : public QThread
{
    Q_OBJECT

public:
    explicit MainThread(uint8_t rotation, QObject *parent = nullptr);
    ~MainThread();
    void esci();
    float v_bat_serv();
    float v_out_sol();
    float v_alim();
    float i_alim();
    float i_motor();
    float temp_1();
    float temp_2();
    int relay();

protected:
    void run();

signals:

public slots:
    void receiveNewConf(const ConfMeasures &newconf);

private:
    BTmanager *bluetooth;
    QThread *dthrd; // Display thread
    int out_relay;
    int out_solar;
    int bat_recharge;
    int engine_run;
    NTC_library *ntc1;
    NTC_library *ntc2;
    ConfMeasures conf;
    gmeasures gm;
    QMutex mtx;
    bool exitloop;
};

#endif // MAINTHREAD_H
