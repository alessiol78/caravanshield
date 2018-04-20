#include "lm75temp.h"
#include "project.h"
#include <QSettings>
#ifdef __arm__
extern "C" {
#include "../bcm2835/bcm2835.h"
}
#endif

//#define DEBUG_LM75

int twos_complement(unsigned input_value, uint8_t num_bits) {
    unsigned val = input_value >> (16-num_bits);
#ifdef DEBUG_LM75
    printf("val: %04x\n",val);
#endif
    // Calculates a two's complement integer from the given input value's bits
    unsigned mask = 0;
    for(int i=0;i<(num_bits - 1);i++) mask |= (1<<i);
    return (int)(val & ~mask)-(int)(val & mask);
}

LM75temp::LM75temp(QObject *parent) : QObject(parent)
{
#ifdef __arm__
    bcm2835_gpio_fsel(pin_a0,BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_clr(pin_a0); // tied to ground
#endif
    _i2c_addr = LM75_Address;
}

int LM75temp::saveSettings(const QString &filename)
{
    QSettings f(filename,QSettings::IniFormat);
    confReg conf = readConf();
    f.setValue("queue",conf.queue);
    f.setValue("pol",conf.pol);
    f.setValue("mode",conf.mode);
    f.setValue("dev_mode",conf.dev_mode);
    return 0;
}

LM75temp::confReg LM75temp::readConf()
{
    confReg c;
    memset(&c,0,sizeof(confReg));
    bool ok;
    quint8 r = readRegister8(0x01, &ok);
    if(ok)
    {
        c.queue = (OS_FAULT_QUEUE)((r >> 3) & 0x03);
        c.pol = (OS_POLARITY)((r >> 2) & 0x01);
        c.mode = (OS_MODE)((r >> 1) & 0x01);
        c.dev_mode = (DEV_SHUTDOWN)(r & 0x01);
    }
    return c;
}

float LM75temp::getTemperature()
{
    return readTemp();
}

float LM75temp::getTos()
{
    float rv = -300.0f;
    int raw = -twos_complement( readRegister16(0x03), 9);
#ifdef DEBUG_LM75
    printf("raw: %d\n", raw);
#endif
    if(raw!=0xffff)
    { rv = (float)(raw * 0.5f); }
    return rv;
}

float LM75temp::getThyst()
{
    float rv = -300.0f;
    int raw = -twos_complement( readRegister16(0x02), 9);
#ifdef DEBUG_LM75
    printf("raw: %d\n", raw);
#endif
    if(raw!=0xffff)
    { rv = (float)(raw * 0.5f); }
    return rv;
}

float LM75temp::readTemp()
{
    float rv = -300.0f;
    int raw = -twos_complement( readRegister16(0x00), 11);
#ifdef DEBUG_LM75
    printf("raw: %d\n", raw);
#endif
    if(raw!=0xffff)
    { rv = (float)(raw * 0.125f); }
    return rv;
}

quint8 LM75temp::readRegister8(uint8_t reg, bool *ok)
{
    quint8 rv = 0;
    char buff[3] ;
    memset(buff,0,sizeof(buff));
    if(ok) *ok = false;
#ifdef __arm__
    bcm2835_i2c_setSlaveAddress(_i2c_addr);
    buff[0] = (char)reg;
    // Write Data on I2C
    if(bcm2835_i2c_write(buff, 1)==BCM2835_I2C_REASON_OK)
    {
        // Read Data on I2C
        if(bcm2835_i2c_read(buff,1)==BCM2835_I2C_REASON_OK) {
            if(ok) *ok = true;
            rv = buff[0];
        }
    }
#else
    quint8 raw = 0x00;
    if(ok) *ok = true;
    rv = raw;
    rv = raw;
#endif
    qDebug("reg %02xh: %03x",reg,rv);
    return rv;
}

quint16 LM75temp::readRegister16(uint8_t reg)
{
    quint16 rv = 0xffff;
    char buff[3] ;
    memset(buff,0,sizeof(buff));
#ifdef __arm__
    bcm2835_i2c_setSlaveAddress(_i2c_addr);
    buff[0] = (char)reg;
    // Write Data on I2C
    if(bcm2835_i2c_write(buff, 1)==BCM2835_I2C_REASON_OK)
    {
        // Read Data on I2C
        if(bcm2835_i2c_read(buff,2)==BCM2835_I2C_REASON_OK) {
            short raw = (short)((buff[0]<<8) & 0xff00);
            raw += buff[1];
            rv = raw;
        }
    }
#else
    if(reg==0x00) {
        buff[0] = 0xc9; // c920h => 0649h
        buff[1] = 0x20;
    }
    else if(reg==0x03) {
        buff[0] = 0x50; // 5000h => A0h
        buff[1] = 0x00;
    }
    else if(reg==0x02) {
        buff[0] = 0x4B; // 4B00h => 96h
        buff[1] = 0x00;
    }
    quint16 raw = (quint16)((buff[0]<<8) & 0xff00);
    raw |= buff[1] & 0x00ff;
    rv = raw;
#endif
#ifdef DEBUG_LM75
    qDebug("reg %02xh: %04x",reg,rv);
#endif
    return rv;
}
