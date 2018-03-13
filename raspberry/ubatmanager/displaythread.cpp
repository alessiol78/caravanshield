#include "displaythread.h"
#ifdef __arm__
#include "ssd1306/ArduiPi_OLED.h"
#include "ssd1306/ArduiPi_OLED_lib.h"
#include "bcm2835/bcm2835.h"
#elif defined(__x86_64__)
#include "remoteframebuff.h"
#include <QCoreApplication>
#endif

#include <iostream>

#ifdef __arm__
static const int pin_btn1 = RPI_V3_GPIO_P1_38;
static const int pin_btn2 = RPI_V3_GPIO_P1_40;
#endif

DisplayThread::DisplayThread(unsigned short num_pages, QObject *parent)
    : QThread(parent)
{
    btstate = "--";
    exitloop = false;
    for(unsigned short i=0;i<num_pages;i++)
    {
        pages.append(new Adafruit_GFX);
    }

}

void DisplayThread::esci()
{
    mtx.lock();
    exitloop = true;
    mtx.unlock();
}

void DisplayThread::setBTstate(const QString &state)
{
    mtx.lock();
    btstate = state;
    mtx.unlock();
}

Adafruit_GFX *DisplayThread::page(int index)
{
    if((index < 0) || (index >= pages.count()) ) return nullptr;
    return pages[index];
}

void DisplayThread::run()
{
#ifdef __arm__
    ArduiPi_OLED display;
    if(!display.init(OLED_I2C_NO_RESET,
                     OLED_ADAFRUIT_I2C_128x64))
#elif defined(__x86_64__)
    RemoteFrameBuff remotefb("127.0.0.1",21345);
    if(!remotefb.init())
#endif
    {
        std::cout << "DISPLAY init error!" << std::endl;
        return;
    }

    // build frame buffer pages
    foreach(Adafruit_GFX *page, pages)
    {
#ifdef __arm__
        page->constructor(display.getOledWidth(),
                          display.getOledHeight());
#else
        page->constructor( 128, 64 );
#endif
        page->setTextColor(1);
        page->setTextSize(1);
        page->clearDisplay();
    }

    // test display
    Adafruit_GFX test_page;
#ifdef __arm__
    test_page.constructor(display.getOledWidth(),
                          display.getOledHeight());
#else
    test_page.constructor( 128, 64 );
#endif
    //test_page.drawLine(0,0,127,0,1);
    test_page.fillScreen(1);
#ifdef __arm__
    display.display(&test_page);
#elif defined(__x86_64__)
    remotefb.sendFB( test_page.frameBuffer(), test_page.frameBufferSize() );
#endif

    sleep(1);

    // clear
    test_page.clearDisplay();
#ifdef __arm__
    display.display(&test_page);
#elif defined(__x86_64__)
    remotefb.sendFB( test_page.frameBuffer(), test_page.frameBufferSize() );
#endif

    sleep(1);

#ifdef __arm__
    bcm2835_gpio_fsel(pin_btn1,BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(pin_btn2,BCM2835_GPIO_FSEL_INPT);
#endif

    int screen = 0;
    int button1 = 0;
    while(!exitloop)
    {
        mtx.lock();
#ifdef __arm__
        display.display(pages[screen++]); // refresh
#elif defined(__x86_64__)
        Adafruit_GFX *page = pages[screen++];
        //page->setTextSize(10);
        //page->print("prova");
        remotefb.sendFB( page->frameBuffer(), page->frameBufferSize() );
#endif
        if(screen>=pages.count()) screen = 0;
        mtx.unlock();

#ifdef __arm__
        // READ button 1
        int cnt = 0;
        do {
            if(bcm2835_gpio_lev(pin_btn1)==HIGH) break;
            // debounce
            cnt++;
            usleep(10000);
        } while(cnt<3);
        if(cnt>=3) {
            // pressed button 1 => roll
            if(button1==0) {
                button1 = 1;
                std::cout << "BUTTON 1 => pressed";
                continue;
            } else {
                // wait release button
                while (bcm2835_gpio_lev(pin_btn1)==LOW) {
                    usleep(50000);
                }
                button1 = 0;
                std::cout << "BUTTON 1 => released";
            }
        }
#endif
        sleep(5);
    }
}
