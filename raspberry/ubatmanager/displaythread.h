#ifndef DISPLAYTHREAD_H
#define DISPLAYTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QList>
#include <QSize>
#include "ssd1306/Adafruit_GFX.h"

class DisplayThread : public QThread
{
    Q_OBJECT

public:
    explicit DisplayThread(unsigned short num_pages, QObject *parent = 0);
    bool waitForInit(int timeout = 3000);
    bool isInitialized();
    void setBTstate(const QString &state);
    void esci();
    Adafruit_GFX *page(int index);
    QSize displaySize();

protected:
    void run();

private:
    QList<Adafruit_GFX*> pages;
    QMutex mtx;
    QString btstate;
    bool exitloop;
    bool initdone;
};

#endif // DISPLAYTHREAD_H
