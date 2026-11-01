#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "updateManager.h"
#include <QMainWindow>
#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>
#include <QCoreApplication>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    Update::UpdateManager *updateManager;
    void setup();
};
#endif // MAINWINDOW_H
