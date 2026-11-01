#pragma once
#include <QString>
#include <QCoreApplication>
#include <QFileInfo>

#if defined(Q_OS_WIN)
    #define VERSION_TYPE "windows"
#elif defined(Q_OS_LINUX)
    #define VERSION_TYPE "linux"
#else
    #define VERSION_TYPE "unknown"
#endif

namespace Config {
inline const QString OS_TYPE = VERSION_TYPE;

// UPDATER and APPLICATION TRUST KEY
inline const QString ENV_KEY      = "LAUNCHED_BY";
inline const QString SECRET_KEY   = "917f74eee56d2e89d67066768bef577de011586d33e563ad3cb59864374a1406";



// SERVER VARIABLES
inline QString SERVER_HOST  = "80.208.221.192";
inline QString SERVER_PORT  = "8000";

// SERVER ACCESS KEY
inline QString API_KEY = "ceb3a322577beb44c450f6a21e75612956d4f8720cd143fcff014b16345a0867";


inline QString get_url() {return "http://" + SERVER_HOST + ":" + SERVER_PORT;}

QString get_running_dir();
}