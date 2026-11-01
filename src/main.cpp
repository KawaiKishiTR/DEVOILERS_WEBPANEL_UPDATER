#include "mainwindow.h"
#include "config.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    Config::dump_config();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
