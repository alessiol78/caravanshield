#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QTcpServer>
#include <QPixmap>

namespace Ui {
class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();

protected slots:
    void receiveFB();
    void readyReadFB();
    void endReceive();

private:
    void testLCD();
    QByteArray buff;
    Ui::MainDialog *ui;
    QTcpServer* server;
    QTcpSocket *sock;
    QPixmap pix;
};

#endif // MAINDIALOG_H
