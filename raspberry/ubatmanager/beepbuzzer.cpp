#include "beepbuzzer.h"
#include "project.h"
#ifdef __arm__
#include "bcm2835/bcm2835.h"
#endif
#include <unistd.h>

#ifdef __arm__
static const int pin_out = pin_aux2;
#endif

BeepBuzzer::BeepBuzzer(QObject *parent) : QObject(parent)
{
#ifdef __arm__
    bcm2835_gpio_fsel(pin_out, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_clr(pin_out);
#endif
}

void BeepBuzzer::playBeep()
{
#ifdef __arm__
    bcm2835_gpio_set(pin_out);
    usleep(100000);
    bcm2835_gpio_clr(pin_out);
    usleep(50000);
    bcm2835_gpio_set(pin_out);
    usleep(100000);
    bcm2835_gpio_clr(pin_out);
#endif
}
