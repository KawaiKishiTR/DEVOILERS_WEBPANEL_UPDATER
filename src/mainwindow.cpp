#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , updateManager(new Update::UpdateManager)
{
    setup();
    updateManager->set_ui(ui);

    /*
    config load ile yüklemeyi dene
    olmazsa uyarı versin devam etsin
    olursa o configle devam
    ne olur ne olmaz en sonra dump yap
    ve string yerine byte cinsinden dosya tut
    */
    QTimer::singleShot(50, [this](){
        updateManager->check_update();
        ui->progressBar->hide();
        ui->lblUpdateStatus->setText("Uygulama Başlatılıyor.");

    #if defined (Q_OS_WIN)
        QString appPath = QDir(Config::get_running_dir()).filePath("KishiPanel.exe");
    #else
        QString appPath = QDir(Config::get_running_dir()).filePath("KishiPanel");
    #endif

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    env.insert(Config::ENV_KEY, Config::SECRET_KEY);

    process.setProcessEnvironment(env);
    process.setProgram(appPath);

    qint64 pid = 0;
    bool success = process.startDetached(&pid);

    if (success) {
        qDebug() << "Uygulama basariyla baslatildi. PID: " << pid;
    } else {
        qDebug() << "Uygulama baslatilamadi:" << process.errorString();
        ui->lblUpdateStatus->setText("Hata: Uygulama başlatılamadı!");
    }});
}

void MainWindow::setup() {
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(QSize(256, 384));

    setWindowIcon(QIcon(":/dock_logo.png"));
    QPixmap orijinalResim(":/app_logo.png");

    QPixmap kucultulmusResim = orijinalResim.scaled(
        orijinalResim.width() / 2,
        orijinalResim.height() / 2,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    ui->lblLogo->setPixmap(kucultulmusResim);
}



MainWindow::~MainWindow()
{
    delete ui;
}
