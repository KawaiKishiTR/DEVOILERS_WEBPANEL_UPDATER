#include "config.h"
#include <QFile>
#include <QTextStream>

QString Config::get_running_dir() {
    // 1. Çalışan binary'nin tam dosya yolu (Örn: /usr/bin/app veya C:/Program Files/App/app.exe)
    QString rawPath = QCoreApplication::applicationFilePath();

    // 2. Symlink veya kısayolları çözüp gerçek fiziksel dosya yolunu al
    QFileInfo fileInfo(rawPath);
    QString realBinaryPath = fileInfo.canonicalFilePath();

    // 3. Binary'nin bulunduğu klasörün gerçek yolu (Updater için dosya indirip yazacağınız yer)
    return fileInfo.canonicalPath();
}