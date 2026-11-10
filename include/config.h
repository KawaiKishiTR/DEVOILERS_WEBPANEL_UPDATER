#pragma once
#include <QString>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>


namespace Config {
inline QString get_running_dir() {
    // 1. Çalışan binary'nin tam dosya yolu (Örn: /usr/bin/app veya C:/Program Files/App/app.exe)
    QString rawPath = QCoreApplication::applicationFilePath();

    // 2. Symlink veya kısayolları çözüp gerçek fiziksel dosya yolunu al
    QFileInfo fileInfo(rawPath);
    QString realBinaryPath = fileInfo.canonicalFilePath();

    // 3. Binary'nin bulunduğu klasörün gerçek yolu (Updater için dosya indirip yazacağınız yer)
    return fileInfo.canonicalPath();
}
inline QString get_appPath() {
#if defined (Q_OS_WIN)
    QString appPath = QDir(Config::get_running_dir()).filePath("app/QT_WEBPANEL.exe");
#else
    QString appPath = QDir(Config::get_running_dir()).filePath("app/QT_WEBPANEL");
#endif
    return appPath;
}
inline QString get_patcherPath() {
#if defined (Q_OS_WIN)
    QString patcherPath = QDir(Config::get_running_dir()).filePath("updater_patcher.exe");
#else
    QString patcherPath = QDir(Config::get_running_dir()).filePath("updater_patcher");
#endif
    return patcherPath;
}


// UPDATER and APPLICATION TRUST KEY
inline const QString ENV_KEY      = "LAUNCHED_BY";
inline const QString SECRET_KEY   = "917f74eee56d2e89d67066768bef577de011586d33e563ad3cb59864374a1406";

#if defined(Q_OS_WIN)
    inline const QString OS_TYPE = "windows";
#elif defined(Q_OS_LINUX)
    inline const QString OS_TYPE = "linux";
#else
    inline const QString OS_TYPE = "unknown";
#endif




// SERVER VARIABLES
inline QString SERVER_HOST  = "80.208.221.192";
inline QString SERVER_PORT  = "8000";

// SERVER ACCESS KEY
inline QString API_KEY = "ceb3a322577beb44c450f6a21e75612956d4f8720cd143fcff014b16345a0867";


inline QString get_url() {return "http://" + SERVER_HOST + ":" + SERVER_PORT;}

}