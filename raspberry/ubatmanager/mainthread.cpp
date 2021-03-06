#include "mainthread.h"
#include "mcp3008/mcp3008Spi.h"
#include "bluetooth/btmanager.h"
#include "ntc_library.h"
#include "beepbuzzer.h"
#ifdef __arm__
#include "bcm2835/bcm2835.h"
#endif
#include "displaythread.h"
#include "lm75a/lm75temp.h"
#ifdef __x86_64__
#include <curses.h>
#endif
#include <iostream>
#include <QSettings>

#define POINT_FORMAT 1
#define LINE_FORMAT  2

#define GRAPH_FORMAT  LINE_FORMAT

MainThread::MainThread(uint8_t rotation, QObject *parent)
    : QThread(parent),
      bluetooth(new BTmanager(true)),
//      #ifdef __arm__
      dthrd(new DisplayThread(4, rotation)),
//      #else
//      dthrd(new QThread),
//      #endif
      out_relay(0), out_solar(0), bat_recharge(0),
      engine_run(0), ntc1(new NTC_library),
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

    // threasholds for service battery recharge
    conf.thr_vbat_l = sett.value("thr_vbat_l",13.5f).toFloat();
    conf.thr_vbat_h = sett.value("thr_vbat_h",14.0f).toFloat();
    // threasholds for out solar good (with sun)
    conf.thr_vsol_l = sett.value("thr_vsol_l",12.4f).toFloat();
    conf.thr_vsol_h = sett.value("thr_vsol_h",13.0f).toFloat();
    // threasholds for engine run
    conf.thr_valim_l = sett.value("thr_valim_l",13.8f).toFloat();
    conf.thr_valim_h = sett.value("thr_valim_h",14.0f).toFloat();

    bluetooth->start();
    bluetooth->updateConf(conf);
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
    LM75temp temp_int;

    std::cout << "MainThread - RUN" << std::endl;

    BeepBuzzer buzz;
    buzz.playBeep();

#ifdef __arm__
    bcm2835_gpio_fsel(pin_relay,BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pin_aux1,BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pin_aux2,BCM2835_GPIO_FSEL_OUTP);

    bcm2835_gpio_clr(pin_relay);
    bcm2835_gpio_clr(pin_aux1);
    bcm2835_gpio_clr(pin_aux2);
#endif

    reinterpret_cast<DisplayThread*>(dthrd)->waitForInit();

    QList<int> graph_i, graph_v;
    QSize sizefb = reinterpret_cast<DisplayThread*>(dthrd)->displaySize();
    if(sizefb.isValid()) {
        qDebug("size: %dx%d",sizefb.width(),sizefb.height());
        for(int i=0;i<sizefb.width()-8;i++)
            { graph_i.append(30); graph_v.append(60); }
    }

    std::cout << "t_os_alarm: " << temp_int.getTos() << "°C" << std::endl;
    std::cout << "t_os_hyst : " << temp_int.getThyst() << "°C" << std::endl;

#ifdef __x86_64__
    WINDOW *mainwin = initscr();
    if(mainwin) {
        start_color();
        if ( has_colors() && COLOR_PAIRS >= 13 ) {
            init_pair(1,  COLOR_RED,     COLOR_BLACK);
            init_pair(2,  COLOR_GREEN,   COLOR_BLACK);
            init_pair(3,  COLOR_YELLOW,  COLOR_BLACK);
            init_pair(4,  COLOR_BLUE,    COLOR_BLACK);
            init_pair(5,  COLOR_MAGENTA, COLOR_BLACK);
            init_pair(6,  COLOR_CYAN,    COLOR_BLACK);
            init_pair(7,  COLOR_BLUE,    COLOR_WHITE);
            init_pair(8,  COLOR_WHITE,   COLOR_RED);
            init_pair(9,  COLOR_BLACK,   COLOR_GREEN);
            init_pair(10, COLOR_BLUE,    COLOR_YELLOW);
            init_pair(11, COLOR_WHITE,   COLOR_BLUE);
            init_pair(12, COLOR_WHITE,   COLOR_MAGENTA);
            init_pair(13, COLOR_BLACK,   COLOR_CYAN);
        }
    }
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
                usleep(20000);
            }
            mean = mean / N;
#ifdef DEBUG
            std::cout << i << ": " << mean << std::endl;
#endif
            //---- i-channel
            vadc[i] = mean;
            //-------
        }


        mtx.lock();
        gm.v_bat_serv = (float)vadc[0]*conf.a_vbatsrv;
        gm.v_out_sol = (float)vadc[1]*conf.a_voutsol;
        gm.i_alim = (float)(vadc[2]-512)*conf.a_ialim;
        gm.i_motor = (float)(vadc[3]-512)*conf.a_imotor;
        gm.t_board = temp_int.getTemperature();
        gm.ntc_1 = (float)ntc1->fromAdc(vadc[4]);
        gm.ntc_2 = (float)ntc2->fromAdc(vadc[5]);
        gm.v_alim = (float)vadc[7]*conf.a_valim;

#ifdef __x86_64__
        if(mainwin) {
            char tmp[512]; int n = 1;
            color_set(0, NULL);
            mvaddstr(n++, 1, "......................");
            if(bat_recharge) color_set(13, NULL);
            else             color_set(0, NULL);
            sprintf(tmp,"v bat: %.2f < %.2f < %.2f  ", conf.thr_vbat_l,
                    gm.v_bat_serv, conf.thr_vbat_h);
            mvaddstr(n++, 1, tmp);
            if(out_solar) color_set(0, NULL);
            else          color_set(1, NULL);
            sprintf(tmp,"v sol: %.2f < %.2f < %.2f  ", conf.thr_vsol_l,
                    gm.v_out_sol, conf.thr_vsol_h);
            mvaddstr(n++, 1, tmp);
            if(bat_recharge) color_set(0, NULL);
            else             color_set(1, NULL);
            sprintf(tmp,"v ali: %.2f < %.2f < %.2f  ", conf.thr_valim_l,
                    gm.v_alim, conf.thr_valim_h);
            mvaddstr(n++, 1, tmp);
            color_set(0, NULL);
            sprintf(tmp,"i ali: %.2f  ", gm.i_alim);
            mvaddstr(n++, 1, tmp);
            sprintf(tmp,"t_brd: %.2f  ", gm.t_board);
            mvaddstr(n++, 1, tmp);
            sprintf(tmp,"ntc 1: %.2f  ", gm.ntc_1);
            mvaddstr(n++, 1, tmp);
            sprintf(tmp,"ntc 2: %.2f  ", gm.ntc_2);
            mvaddstr(n++, 1, tmp);
            mvaddstr(n++, 1, "-----------------");
            sprintf(tmp,"relay: %d", out_relay);
            mvaddstr(n++, 1, tmp);
            sprintf(tmp,"engin: %d", engine_run);
            mvaddstr(n++, 1, tmp);
            sprintf(tmp,"solar: %d", out_solar);
            mvaddstr(n++, 1, tmp);
            sprintf(tmp,"charg: %d", bat_recharge);
            mvaddstr(n++, 1, tmp);

            refresh();
        }
        else
#endif
        {
            system("clear");
            std::cout << "......................" << std::endl;
            std::cout << "v bat: " << conf.thr_vbat_l << " < " <<
                         gm.v_bat_serv << " < " << conf.thr_vbat_h << std::endl;
            std::cout << "v sol: " << conf.thr_vsol_l << " < " <<
                         gm.v_out_sol << " < " << conf.thr_vsol_h << std::endl;
            std::cout << "v ali: " << conf.thr_valim_l << " < " <<
                         gm.v_alim << " < " << conf.thr_valim_h << std::endl;
            std::cout << "i ali: " << gm.i_alim << std::endl;
            std::cout << "t_brd: " << gm.t_board << std::endl;
            std::cout << "ntc 1: " << gm.ntc_1 << std::endl;
            std::cout << "ntc 2: " << gm.ntc_2 << std::endl;
            std::cout << "-----------------" << std::endl;
            std::cout << "relay: " << out_relay << std::endl;
            std::cout << "engin: " << engine_run << std::endl;
            std::cout << "solar: " << out_solar << std::endl;
            std::cout << "charg: " << bat_recharge << std::endl;
            std::cout << "-----------------" << std::endl;
        }

        //****  SOLAR  ****
        if(gm.v_out_sol > conf.thr_vsol_h)
        {
            // solar power is good
            out_solar = 1;
#ifdef DEBUG
            std::cout << "solar: ok" << std::endl;
#endif
        }
        else if(gm.v_out_sol < conf.thr_vsol_l)
        {
            // solar power not usable
            out_solar = 0;
#ifdef DEBUG
            std::cout << "solar: TOO LOW" << std::endl;
#endif
        }

        //****  VALIM  ****
        if(gm.v_alim > conf.thr_valim_h)
        {
            // engine start and start recharge battery request
            engine_run = 1;
#ifdef DEBUG
            std::cout << "engine: RUN" << std::endl;
#endif
        }
        else if(gm.v_alim < conf.thr_valim_l)
        {
            // engine shutdown
            engine_run = 0;
#ifdef DEBUG
            std::cout << "engine: OFF" << std::endl;
#endif
        }

        //****  SERVICE BATTERY  ****
        if(gm.v_bat_serv > conf.thr_vbat_h)
        {
            // service battery recharge
            bat_recharge = 1;
#ifdef DEBUG
            std::cout << "battery: CHARGING" << std::endl;
#endif
        }
        else if(gm.v_bat_serv < conf.thr_vbat_l)
        {
            // service battery discharge
            bat_recharge = 0;
#ifdef DEBUG
            std::cout << "battery: DISCHARGE" << std::endl;
#endif
        }

        //--------------------------------
        // *** DRIVE RELAY STRATEGY ***
        //--------------------------------


        (bat_recharge || engine_run || !out_solar) ? out_relay = 0 : out_relay = 1;


        //------ BATTERY PROTECTION ------
        if(gm.v_alim < 10.0f)
        {
            out_relay = 1; // FORCE solar input for disconnect service battery
#ifdef __arm__
            // low voltage protection
            bcm2835_gpio_set(pin_aux1);
#endif
            buzz.playBeep();
            if(!mainwin) std::cout << "low bat protection: ON" << std::endl;
        }
        else if(gm.v_alim > 11.0f)
        {
            if(!mainwin) std::cout << "low bat protection: OFF" << std::endl;
        }

        //======== RELAY DRIVE =========
#ifdef __arm__
        if(out_relay)
            bcm2835_gpio_set(pin_relay);
        else
            bcm2835_gpio_clr(pin_relay);
#endif
        //==============================

        if(bluetooth->isRunning()) {
            bluetooth->updateMeasures(&gm);
        }

        if(graph_i.count()) {
            graph_i.push_back( qBound(0,30 - int(gm.i_alim),60) );
            graph_i.pop_front();
        }
        if(graph_v.count()) {
            graph_v.push_back( qBound(0,60 - int(gm.v_alim * 3.0f),60) );
            graph_v.pop_front();
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
            // riga alta
            display->page(n)->drawRoundRect(2,1,60,30,2,1);
            display->page(n)->drawRoundRect(65,1,60,30,2,1);
            display->page(n)->setTextSize(1);
            display->page(n)->setCursor(10,21);
            display->page(n)->printf( " V bat" );
            display->page(n)->setCursor(75,21);
            display->page(n)->printf( " V sol" );
            display->page(n)->setTextSize(2);
            display->page(n)->setCursor(8+(gm.v_bat_serv<10.0 ? 7 : 0),3);
            display->page(n)->printf("%3.1f",gm.v_bat_serv);
            display->page(n)->setCursor(72+(gm.v_out_sol<10.0 ? 7 : 0),3);
            display->page(n)->printf("%3.1f",gm.v_out_sol);
            // riga bassa
            display->page(n)->drawRoundRect(2,33,60,30,2,1);
            display->page(n)->drawRoundRect(65,33,60,30,2,1);
            display->page(n)->setTextSize(1);
            display->page(n)->setCursor(12,53);
            display->page(n)->printf( "V alim" );
            display->page(n)->setCursor(77,53);
            display->page(n)->printf( "I alim" );
            display->page(n)->setTextSize(2);
            display->page(n)->setCursor(8+(gm.v_alim<10.0 ? 7 : 0),35);
            display->page(n)->printf("%3.1f",gm.v_alim);
            display->page(n)->setCursor(66+(gm.i_alim>0 ? 6 : 0)
                                        +(gm.i_alim<=10.0 ? 7 : 0)
                                        +(gm.i_alim<=-10.0 ? 4 : 0),35);
            if(gm.i_alim <= -10.0) {
                display->page(n)->printf("%3.0f",gm.i_alim);
            } else {
                display->page(n)->printf("%3.1f",gm.i_alim);
            }
            display->page(n)->drawCircle(63,31,8,1);
            display->page(n)->fillCircle(63,31,7,0);
            if(out_relay) {
                display->page(n)->fillCircle(63,31,5,1);
            }
        }
        if(display->page(++n)) {
            Adafruit_GFX *fb = display->page(n);
            fb->clearDisplay();
            fb->drawLine(4,4,4,60,1);
            fb->drawLine(4,60,fb->width()-4,60,1);
            for(int j=1;j<graph_i.count();j++) {
#if GRAPH_FORMAT == POINT_FORMAT
                fb->drawPixel(4+j, 4+graph_i.at(j), 1);
#elif GRAPH_FORMAT == LINE_FORMAT
                fb->drawLine(3+j,graph_i.at(j-1),
                             4+j,graph_i.at(j), 1);
#else
#error graph format unknown
#endif
            }
            fb->fillRect(120,5,8,5,0);
            fb->setCursor(120,4);
            fb->print("I");
        }
        if(display->page(++n)) {
            Adafruit_GFX *fb = display->page(n);
            fb->clearDisplay();
            fb->drawLine(4,4,4,60,1);
            fb->drawLine(4,60,fb->width()-4,60,1);
            for(int j=1;j<graph_v.count();j++) {
#if GRAPH_FORMAT == POINT_FORMAT
                fb->drawPixel(4+j, 4+graph_v.at(j), 1);
#elif GRAPH_FORMAT == LINE_FORMAT
                fb->drawLine(3+j,graph_v.at(j-1),
                             4+j,graph_v.at(j), 1);
#else
#error graph format unknown
#endif
            }
            fb->fillRect(120,5,8,5,0);
            fb->setCursor(120,4);
            fb->print("V");
        }
#endif
        mtx.unlock();
    }

    delwin(mainwin);
    endwin();
    refresh();

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
