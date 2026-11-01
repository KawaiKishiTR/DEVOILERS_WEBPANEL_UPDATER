#pragma once
#include "config.h"
#include "ui_mainwindow.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <QQueue>
#include <QPair>
#include <QDebug>
#include <QMainWindow>


namespace Update {

class UpdateManager {
public:
    UpdateManager();
    void check_update();

    inline void set_ui(Ui::MainWindow* ui) {_ui = ui;}

private slots:
    void processNextDownload();

private:
    QJsonObject jsonParse(const QByteArray &InputData);
    void _downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void startUpdateProcess(const QJsonObject &networkJson, const QJsonObject &localJson);
    void onManifestReceived(QNetworkReply *reply);
    QNetworkReply* getRequest(QNetworkRequest &request);

    inline QJsonObject get_local_manifest() {
        return jsonParse(QFile(get_manifest_file()).readAll());}
    inline QString get_update_endpoint() {
        return Config::get_url() + "/version/" + Config::OS_TYPE;}
    inline QFile get_manifest_file() {
        return QFile(Config::get_running_dir() + "/manifest.json");}
    inline void connectReply2ProcessBar(QNetworkReply *reply) {
        QObject::connect(reply, &QNetworkReply::downloadProgress, 
        [this](qint64 rec, qint64 tot){
            this->_downloadProgress(rec, tot);
        });}

    Ui::MainWindow *_ui;
    QNetworkAccessManager *networkManager;
    QQueue<QPair<QString, QString>> _downloadQueue;
    QJsonObject _pendingManifest;
};

}