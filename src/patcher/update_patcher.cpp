#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#endif

namespace fs = std::filesystem;

// Updater sürecinin kapandığından emin olmak için PID kontrolü
bool isProcessRunning(int pid) {
#if defined(_WIN32)
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (process == NULL) return false;
    DWORD ret = WaitForSingleObject(process, 0);
    CloseHandle(process);
    return ret == WAIT_TIMEOUT;
#else
    return (kill(pid, 0) == 0);
#endif
}

fs::path getExecutableDir() {
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    fs::path exePath(buffer);
#else
    char buffer[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
    if (count == -1) {
        return fs::current_path(); // Fallback
    }
    buffer[count] = '\0';
    fs::path exePath(buffer);
#endif

    // fs::canonical tüm symlink ve kısayol yönlendirmelerini diskin asıl fiziksel yoluna çözer
    return fs::canonical(exePath).parent_path();
}

int main(int argc, char* argv[]) {
    const char* envVal = std::getenv("LAUNCHED_BY");

    // Değişken yoksa (nullptr) veya beklenen gizli anahtar değilse:
    if (!envVal || std::string_view(envVal) != "917f74eee56d2e89d67066768bef577de011586d33e563ad3cb59864374a1406") {
        std::cerr << "Yetkisiz doğrudan çalıştırma engellendi." << std::endl;
        return 1;
    }

    int updaterPid = 0;
    if (argc >= 2) {
        updaterPid = std::stoi(argv[1]);
    }

    // 1. Updater'ın işletim sisteminden tamamen silinmesini bekle (Kilit kalksın)
    if (updaterPid > 0) {
        int retry = 0;
        while (isProcessRunning(updaterPid) && retry < 200) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            retry++;
        }
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    try {
        fs::path rootDir = getExecutableDir();
        fs::path updateDir = rootDir / "update";

        // 2. Eğer updater/ klasörü varsa, içindekileri ana dizine taşı
        if (fs::exists(updateDir)) {
            for (const auto& entry : fs::directory_iterator(updateDir)) {
                fs::path dest = rootDir / entry.path().filename();
                
                // Dosyayı veya klasörü ana dizinin üzerine yaz
                fs::copy(entry.path(), dest, 
                         fs::copy_options::overwrite_existing | fs::copy_options::recursive);
            }

            // İçi boşalan geçici indirme klasörünü temizle
            fs::remove_all(updateDir);
        }

        // 3. Güncellenen Updater'ı ana dizinden tekrar başlat
#if defined(_WIN32)
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        std::string cmd = "QT_WEBPANEL_UPDATER.exe";
        CreateProcess(NULL, cmd.data(), NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &pi);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
#else
        if (fork() == 0) {
            setsid();
            execl("./QT_WEBPANEL_UPDATER", "./QT_WEBPANEL_UPDATER", (char*)NULL);
            exit(0);
        }
#endif

    } catch (const std::exception& e) {
        std::cerr << "[Patcher Hatası]: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}