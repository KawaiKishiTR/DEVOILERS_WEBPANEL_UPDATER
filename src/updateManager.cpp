#include "updateManager.h"
#include <QDir>


namespace Update {

UpdateManager::UpdateManager()
    : networkManager(new QNetworkAccessManager) {}

QNetworkReply* UpdateManager::getRequest(QNetworkRequest &request) {
    QNetworkReply *reply = networkManager->get(request);
    connectReply2ProcessBar(reply);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, [&loop](){loop.quit();});

    loop.exec();

    return reply;
}

void UpdateManager::check_update() {
    _ui->lblUpdateStatus->setText("manifest dosyası alınıyor...");
    QNetworkRequest request(get_update_endpoint() + "/manifest.json");
    request.setRawHeader("x-api-key", Config::API_KEY.toUtf8());

    QNetworkReply *reply = getRequest(request);
    onManifestReceived(reply);

    while (!_downloadQueue.isEmpty()) {
        processNextDownload();
    }

    qDebug() << "Tum guncelleme dosyalari basariyla indirildi!";
    
    // Yeni manifesti diske yerel manifest olarak kaydet
    QFile mFile(get_manifest_file());
    if (mFile.open(QIODevice::WriteOnly)) {
        mFile.write(QJsonDocument(_pendingManifest).toJson());
        mFile.close();
        qDebug() << "Yerel manifest.json guncellendi.";
    }
    return;

}

void UpdateManager::onManifestReceived(QNetworkReply *reply) {
if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Manifest Indirme Hatasi: " << reply->errorString();
        reply->deleteLater();
        return;
    }

    QJsonObject networkJson = this->jsonParse(reply->readAll());
    QJsonObject localJson = this->get_local_manifest();

    QString netVersionHash = networkJson["version_hash"].toString();
    QString locVersionHash = localJson["version_hash"].toString();

    // Sürümler aynıysa güncellemeye gerek yok
    if (!netVersionHash.isEmpty() && locVersionHash == netVersionHash) {
        qDebug() << "Uygulama zaten en guncel surumde.";
        reply->deleteLater();
        return;
    }

    qDebug() << "Guncelleme tespit edildi. Dosya karsilastirmasi baslatiliyor...";
    startUpdateProcess(networkJson, localJson);

    reply->deleteLater();
}

void UpdateManager::startUpdateProcess(const QJsonObject &networkJson, const QJsonObject &localJson) {
    this->_pendingManifest = networkJson;
    _downloadQueue.clear();

    QJsonObject netFiles = networkJson["version_full_file_index"].toObject();
    QJsonObject locFiles = localJson["version_full_file_index"].toObject();

    // Sunucunuzdaki dosyaların bulunduğu temel indirme URL'i (Örn: https://api.site.com/static/files/)
    QString serverBaseUrl = get_update_endpoint(); 

    // 1. Ağdaki tüm dosyaları kontrol et
    for (auto it = netFiles.begin(); it != netFiles.end(); ++it) {
        QString relativePath = it.key();         // Örn: "bin/game.exe" veya "configs/settings.json"
        QString netHash = it.value().toString();  // Dosyanın sunucudaki hash'i

        // Dosya yerelde yoksa VEYA hash'i sunucudakinden farklıysa listeye ekle
        bool fileExistsLocally = locFiles.contains(relativePath);
        QString locHash = locFiles[relativePath].toString();

        if (!fileExistsLocally || locHash != netHash) {
            // Dosyanın sunucudaki indirme adresi:
            QString fileDownloadUrl = serverBaseUrl + "/" + relativePath;

            _downloadQueue.enqueue(qMakePair(fileDownloadUrl, relativePath));
            qDebug() << "Guncellenecek dosya bulundu:" << relativePath;
        }
    }

    // 2. İndirme kuyruğunu başlat
    if (!_downloadQueue.isEmpty()) {
        qDebug() << "Toplam indirilecek dosya adedi:" << _downloadQueue.size();
    } else {
        qDebug() << "Indirilecek yeni dosya yok.";
    }
}

void UpdateManager::processNextDownload() {
    // Sıradaki dosyayı kuyruktan çek
    auto currentTask = _downloadQueue.dequeue();
    QString downloadUrl = currentTask.first;
    QString relativePath = currentTask.second;

    QDir dir(Config::get_running_dir());
    QString targetFilePath = dir.filePath("app/" + relativePath);

    QNetworkRequest request((QUrl(downloadUrl)));
    request.setRawHeader("x-api-key", Config::API_KEY.toUtf8()); // Gerekirse API key ekleyin
    
    _ui->lblUpdateStatus->setText("D: " + relativePath);
    QNetworkReply *reply = getRequest(request);

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Dosya indirme hatasi (" << targetFilePath << "):" << reply->errorString();
        reply->deleteLater();
    }

    // 1. Dosyanın kaydedileceği alt klasörler yoksa otomatik oluştur (mkdir -p mantığı)
    QFileInfo fileInfo(targetFilePath);
    QDir targetDir = fileInfo.dir();
    if (!targetDir.exists()) {
        targetDir.mkpath("."); 
    }

    // 2. Ham veriyi diske yaz
    QFile file(targetFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reply->readAll());
        file.close();
        qDebug() << "Dosya indirildi ve yazildi:" << targetFilePath;
    } else {
        qDebug() << "Dosya yazma hatasi:" << file.errorString();
    }

    reply->deleteLater();
}

QJsonObject UpdateManager::jsonParse(const QByteArray &InputData) {
    // 1. Ham veriyi JSON dokümanına çevir (Hata kontrolü ile)
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(InputData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON Ayrıştırma Hatası:" << parseError.errorString();
        return QJsonObject();
    }

    if (doc.isObject()) {
        return doc.object();
    }

    return QJsonObject();
}

void UpdateManager::_downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        _ui->progressBar->setMaximum(bytesTotal);
        _ui->progressBar->setValue(bytesReceived);
    }
}

} // namespace Update