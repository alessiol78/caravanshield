#ifndef LM75TEMP_H
#define LM75TEMP_H

#include <QObject>

/* By def. the app uses I2C address 0x48.
 * Solder pin A0, A1, A2 of LM75A to ground to use the def.addr */
#define LM75_Address    0x48  // if A0 = to Vcc => 0x49 NOTE: fix constructor

class LM75temp : public QObject
{
    Q_OBJECT
public:
    enum OS_FAULT_QUEUE {
        queue_value_1 = 0b00,
        queue_value_2 = 0b01,
        queue_value_4 = 0b10,
        queue_value_6 = 0b11
    };
    enum OS_POLARITY {
        os_active_low = 0,
        os_active_high = 1
    };
    enum OS_MODE {
        os_comparator = 0,
        os_interrupt = 1
    };
    enum DEV_SHUTDOWN {
        dev_normal = 0,
        dev_shutdown = 1
    };
    Q_ENUM(OS_FAULT_QUEUE)
    Q_ENUM(OS_POLARITY)
    Q_ENUM(OS_MODE)
    Q_ENUM(DEV_SHUTDOWN)
    struct confReg
    {
        OS_FAULT_QUEUE queue;
        OS_POLARITY pol;
        OS_MODE mode;
        DEV_SHUTDOWN dev_mode;
    };
    explicit LM75temp(QObject *parent = nullptr);
    int saveSettings(const QString &filename);
    int loadSettings(const QString &filename);
    float getTemperature();
    float getTos();
    float getThyst();

signals:

public slots:

private:
    int8_t _i2c_addr;

    float readTemp();
    confReg readConf();

    quint8 readRegister8(uint8_t reg, bool *ok = 0);
    quint16 readRegister16(uint8_t reg);
};

#endif // LM75TEMP_H
