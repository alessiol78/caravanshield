#include "mainthread.h"
#include "mcp3008/mcp3008Spi.h"
#include "bluetooth/btmanager.h"
#include "ntc_library.h"
#include "beepbuzzer.h"
#ifdef __arm__
#include "bcm2835/bcm2835.h"
#endif
#include "displaythread.h"

#include <iostream>
#include <QSettings>

#ifdef __arm__
static const int pin_relay = RPI_V3_GPIO_P1_29; // 5
static const int pin_aux1 = RPI_V3_GPIO_P1_36;  // 16
#endif

MainThread::MainThread(QObject *parent)
    : QThread(parent),
      bluetooth(new BTmanager),
//      #ifdef __arm__
      dthrd(new DisplayThread(2)),
//      #else
//      dthrd(new QThread),
//      #endif
      out_relay(0), ntc1(new NTC_library),
      ntc2(new NTC_library), exitloop(false)
{
    const float def_r27ksu10k = 0.018066406f;
    const float def_acs712 = 0.07398201f; // 66mV/A
    QSettings sett("calibrate.conf",QSettings::IniFormat);
    // partitori tensione
    conf.a_vbatsrv = sett.value("a_vbatsrv",def_r27ksu10k).toFloat();
    conf.a_voutsol = sett.value("a_voutsol",def_r27ksu10k).toFloat();
    conf.a_valim = sett.value("a_valim",def_r27ksu10k).toFloat();
    // sonda corrente acs712-30A
    conf.a_ialim = sett.value("a_ialim",def_acs712).toFloat();
    conf.a_imotor = sett.value("a_imotor",def_acs712).toFloat();
    // ntc(#1) 10k
    conf.ntc1_a0 = sett.value("ntc1_a0",0.0f).toFloat();
    conf.ntc1_a1 = sett.value("ntc1_a1",0.0f).toFloat();
    conf.ntc1_a2 = sett.value("ntc1_a2",0.0f).toFloat();
    conf.ntc1_a3 = sett.value("ntc1_a3",0.0f).toFloat();
    if(conf.ntc1_a0!=0.0f && conf.ntc1_a1!=0.0f &&
       conf.ntc1_a2!=0.0f && conf.ntc1_a3!=0.0f)
    { ntc1->calibrate(conf.ntc1_a0,conf.ntc1_a1,
                      conf.ntc1_a2,conf.ntc1_a3); }
    // ntc(#2) 10k
    conf.ntc2_a0 = sett.value("ntc2_a0",0.0f).toFloat();
    conf.ntc2_a1 = sett.value("ntc2_a1",0.0f).toFloat();
    conf.ntc2_a2 = sett.value("ntc2_a2",0.0f).toFloat();
    conf.ntc2_a3 = sett.value("ntc2_a3",0.0f).toFloat();
    if(conf.ntc2_a0!=0.0f && conf.ntc2_a1!=0.0f &&
       conf.ntc2_a2!=0.0f && conf.ntc2_a3!=0.0f)
    { ntc2->calibrate(conf.ntc2_a0,conf.ntc2_a1,
                      conf.ntc2_a2,conf.ntc2_a3); }

    conf.thr_vbat_l = sett.value("thr_vbat_l",11.0f).toFloat();
    conf.thr_vbat_h = sett.value("thr_vbat_h",14.0f).toFloat();
    conf.thr_vsol_l = sett.value("thr_vsol_l",12.4f).toFloat();
    conf.thr_vsol_h = sett.value("thr_vsol_h",13.0f).toFloat();
    conf.thr_valim_l = sett.value("thr_valim_l",12.4f).toFloat();
    conf.thr_valim_h = sett.value("thr_valim_h",13.0f).toFloat();

    dthrd->start();
}

MainThread::~MainThread()
{
    delete dthrd;
    delete bluetooth;
    delete ntc1;
    delete ntc2;
}

void MainThread::run()
{
    mcp3008Spi adc;

    std::cout << "MainThread - RUN" << std::endl;

    BeepBuzzer buzz;
    buzz.playBeep();

#ifdef __arm__
    bcm2835_gpio_fsel(pin_relay,BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pin_aux1,BCM2835_GPIO_FSEL_OUTP);
#endif
    while(!exitloop)
    {
        const int NUM_CHANNELS = 8;
        int vadc[NUM_CHANNELS];
        // acquisizione canali con filtro
        const int N = 5; // mettere un numero dispari
        int measures[N];
        int mean = 0;
        for(int i=0;i<NUM_CHANNELS;i++) {
            mean = 0;
            for(int j=0;j<N;j++) {
                measures[j] = adc.readChannel(i); // 10bit = 1024
                mean += measures[j];
                usleep(50000);
            }
            mean = mean / N;
            //std::cout << i << ": " << mean << std::endl;
            //---- i-channel
            vadc[i] = mean;
            //-------
        }

        mtx.lock();
        gm.v_bat_serv = (float)vadc[0]*conf.a_vbatsrv;
        gm.v_out_sol = (float)vadc[1]*conf.a_voutsol;
        gm.i_alim = (float)(vadc[2]-512)*conf.a_ialim;
        gm.i_motor = (float)(vadc[3]-512)*conf.a_imotor;
        gm.ntc_1 = (float)ntc1->fromAdc(vadc[4]);
        gm.ntc_2 = (float)ntc2->fromAdc(vadc[5]);
        gm.v_alim = (float)vadc[7]*conf.a_valim;

        std::cout << "......................" << std::endl;
        std::cout << "v bat: " << conf.thr_vbat_l << " < " <<
                     gm.v_bat_serv << " < " << conf.thr_vbat_h << std::endl;
        std::cout << "v sol: " << conf.thr_vsol_l << " < " <<
                     gm.v_out_sol << " < " << conf.thr_vsol_h << std::endl;
        std::cout << "v ali: " << gm.v_alim << std::endl;
        std::cout << "i ali: " << gm.i_alim << std::endl;
        std::cout << "ntc 1: " << gm.ntc_1 << std::endl;
        std::cout << "ntc 2: " << gm.ntc_2 << std::endl;

        std::cout << "-----------------" << std::endl;
        if(gm.v_out_sol > conf.thr_vsol_h)
        {
            // solar power is good
            std::cout << "solar: ok" << std::endl;
        }
        else if(gm.v_out_sol < conf.thr_vsol_l)
        {
            // solar power not usable
            std::cout << "solar: TOO LOW" << std::endl;
        }

        if(gm.v_alim > conf.thr_valim_h)
        {
#ifdef __arm__
            // engine start and start recharge battery request
            bcm2835_gpio_clr(pin_relay);
#endif
            std::cout << "engine: RUN" << std::endl;
        }
        else if(gm.v_alim < conf.thr_valim_l)
        {
#ifdef __arm__
            // low voltage protection
            bcm2835_gpio_set(pin_relay);
            bcm2835_gpio_set(pin_aux1);
#endif
            buzz.playBeep();
            std::cout << "engine: VOLT too LOW" << std::endl;
        }
        if(gm.v_bat_serv > conf.thr_vbat_h)
        {
            std::cout << "battery: CHARGING" << std::endl;
        }

        if(bluetooth->isRunning()) {
            bluetooth->updateMeasures(&gm);
        }

#if 1 //def __arm__
        DisplayThread *display = (DisplayThread*)dthrd;
        int n = 0;
        if(display->page(n)) {
            display->page(n)->clearDisplay();
            display->page(n)->setCursor(0,0);
            display->page(n)->printf( "BT adapter: %s\n%s",
                                      bluetooth->isRunning() ? "OK" : "--",
                                      bluetooth->adapterName().toUtf8().constData()
                                       );
        }
        if(display->page(++n)) {
            display->page(n)->clearDisplay();
            display->page(n)->setCursor(0,0);
            display->page(n)->printf( " V bat: %02.1f\n"
                                      " V sol: %02.1f\n"
                                      "V alim: %02.1f\n"
                                      "--------------\n"
                                      "I alim: %01.2f"
                                      ,gm.v_bat_serv
                                      ,gm.v_out_sol
                                      ,gm.v_alim
                                      ,gm.i_alim);
        }
#endif
        mtx.unlock();
    }

    bluetooth->stop();

#if 1 //def __arm__
    DisplayThread *thrd = (DisplayThread*)dthrd;
    thrd->esci();
#else
    QThread *thrd = dthrd;
    thrd->terminate();
#endif
    std::cout << "wait: " << thrd->wait(10000) << std::endl;
}

void MainThread::esci()
{
    mtx.lock();
    exitloop = true;
    mtx.unlock();
}

float MainThread::v_bat_serv()
{
    float rv;
    mtx.lock();
    rv = gm.v_bat_serv;
    mtx.unlock();
    return rv;
}

float MainThread::v_out_sol()
{
    float rv;
    mtx.lock();
    rv = gm.v_out_sol;
    mtx.unlock();
    return rv;
}

float MainThread::v_alim()
{
    float rv;
    mtx.lock();
    rv = gm.v_alim;
    mtx.unlock();
    return rv;
}

float MainThread::i_alim()
{
    float rv;
    mtx.lock();
    rv = gm.i_alim;
    mtx.unlock();
    return rv;
}

float MainThread::i_motor()
{
    float rv;
    mtx.lock();
    rv = gm.i_motor;
    mtx.unlock();
    return rv;
}

float MainThread::temp_1()
{
    float rv;
    mtx.lock();
    rv = gm.ntc_1;
    mtx.unlock();
    return rv;
}

float MainThread::temp_2()
{
    float rv;
    mtx.lock();
    rv = gm.ntc_2;
    mtx.unlock();
    return rv;
}

int MainThread::relay()
{
    int rv;
    mtx.lock();
    rv = out_relay;
    mtx.unlock();
    return rv;
}

void MainThread::receiveNewConf(const ConfMeasures &newconf)
{
    mtx.lock();
    conf = newconf;
    mtx.unlock();
}
