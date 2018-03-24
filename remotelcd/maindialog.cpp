#include "maindialog.h"
#include "ui_maindialog.h"

#include <QTcpSocket>
#include <iostream>
#include <QImage>

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog), server(new QTcpServer), sock(NULL)
{
    ui->setupUi(this);
    if(!server->listen(QHostAddress::Any,21345))
    {
        ui->label->setText("BIND error !!!");
    }
    else
    {
        connect(server,SIGNAL(newConnection()),
                this,SLOT(receiveFB()));
    }

//    testLCD();
}

// Arduino Compatible Macro
#define _BV(bit) (1 << (bit))
void MainDialog::testLCD()
{
    QImage img(128,64,QImage::Format_MonoLSB);
    img.fill(0);
    uint8_t p;
    int nl = img.bytesPerLine();
    for(int x=0;x<128;x++) {
        img.setPixel(x,0,1);
    }
    ui->label->setPixmap( QPixmap::fromImage(img) );
}

MainDialog::~MainDialog()
{
    if(server->isListening()) server->close();
    delete ui;
    delete server;
}

void MainDialog::receiveFB()
{
    while(server->hasPendingConnections())
    {
        buff.clear();
        sock = server->nextPendingConnection();
        connect(sock,SIGNAL(readyRead()),
                this,SLOT(readyReadFB()));
        connect(sock,SIGNAL(disconnected()),
                this,SLOT(endReceive()));
        QByteArray qb;
        qb.append('A');
        qb.append(ui->toolButton_A->isDown()?'1':'0');
        qb.append('B');
        qb.append(ui->toolButton_B->isDown()?'1':'0');
        sock->write(qb);
    }
}

void MainDialog::readyReadFB()
{
//    std::cout << "receive FB..." << std::endl;
    buff += sock->readAll();
}

void MainDialog::endReceive()
{
    if(buff.isEmpty()) return;
    buff += sock->readAll();
//    std::cout << "buff size: " << buff.size() << std::endl;
    int size = buff[0]<<8 | buff[1];
    if(size!=(buff.size()-2)) {
        std::cout << "incomplete buffer, received size: " << size << std::endl;
        return;
    }
    QByteArray data = QByteArray::fromBase64(buff.mid(2));
//    std::cout << "data size: " << data.size() << std::endl;
    pix = QPixmap(128,64);
#if 0
    QImage img((uchar*)data.data(),64,128,QImage::Format_Mono);
    QMatrix matrix;
    matrix.rotate(90);
    pix = QPixmap::fromImage(img).transformed(matrix);
#else
    QImage img(128,64,QImage::Format_MonoLSB);
    img.fill(0);
    uint8_t p;
    for(int x=0;x<128;x++) {
        for(int y=0;y<64;y++) {
            // x is which column
            p = (uint8_t)data.at( x + (y/8)*128 );
            if(p & _BV(y%8))
                img.setPixel(x,y,1);
            else
                img.setPixel(x,y,0);
        }
    }
    pix = QPixmap::fromImage(img);
#endif
    ui->label->setPixmap(pix);
}
