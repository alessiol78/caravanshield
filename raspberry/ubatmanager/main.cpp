#include <QCoreApplication>
#include "mainthread.h"
#ifdef __arm__
#include "bcm2835/bcm2835.h"
#endif
#include <signal.h>
#include <iostream>

MainThread *gMainThrd = NULL;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        //fprintf(stdout, "%s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fprintf(stdout, "%s\n", localMsg.constData()); fflush(0);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}

void signal_handler(int signum)
{
    std::cout << "signum:" << signum << std::endl;
    if(signum==SIGSEGV)
    {
        std::cout << "segm.fault!" << std::endl;
        exit(EXIT_FAILURE);
    }
    qApp->quit();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QCoreApplication a(argc, argv);

#ifdef __arm__
    // Init Raspberry PI GPIO
    if (!bcm2835_init())
        return 1;
#endif

    // catch CTRL-C
    struct sigaction term;
    term.sa_handler = signal_handler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    sigaction(SIGTERM, &term, NULL);
    sigaction(SIGQUIT, &term, NULL);
    sigaction(SIGINT, &term, NULL);
    sigaction(SIGSEGV, &term, NULL);

    int n = -1;
    if(argc>1)
    {
        n = atoi(argv[1]);
    }
    qDebug("rotation: %d",n);

    MainThread thrd(n);
    thrd.start();

    gMainThrd = &thrd;

    int rv = a.exec();

    thrd.esci();
    thrd.wait(15000);

#ifdef __arm__
    bcm2835_close();
#endif
    return rv;
}
