#ifndef BEEPBUZZER_H
#define BEEPBUZZER_H

#include <QObject>

class BeepBuzzer : public QObject
{
    Q_OBJECT
public:
    explicit BeepBuzzer(QObject *parent = nullptr);
    void playBeep();

signals:

public slots:
};

#endif // BEEPBUZZER_H
