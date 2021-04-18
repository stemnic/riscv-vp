#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    if (argc > 1)
    {
        // gpio client thing... no sigpipes
        signal(SIGPIPE, SIG_IGN);
        w.loadBoardConfig(argv[1]);
    }

    return a.exec();
}
