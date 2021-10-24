#pragma comment(lib, "urlmon.lib")
#pragma comment (lib, "User32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <filesystem>
#include <Shlwapi.h>

#include <iostream>
#include <string>
#include <tchar.h>
#include "progress_bar.hpp"

#include <curl/curl.h>
#include <openssl/ssl.h>
using namespace std;

std::filesystem::path getOperatingPath() {
    const HMODULE h_module = GetModuleHandleW(nullptr);
    WCHAR pth[MAX_PATH];
    GetModuleFileNameW(h_module, pth, MAX_PATH);
    wstring ws(pth);
    const string path(ws.begin(), ws.end());
    return path.substr(0, path.find_last_of('\\') + 1);
}

void insert_empty_line()
{
    CONSOLE_SCREEN_BUFFER_INFO buffer_info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffer_info);

    std::string s(static_cast<double>(buffer_info.srWindow.Right) - static_cast<double>(buffer_info.srWindow.Left), ' ');
    std::cout << s << '\r';
}

double round_off(const double n) {
    double d = n * 100.0;
    const int i = d + 0.5;
    d = static_cast<float>(i) / 100.0;
    return d;
}

size_t write_data(void* ptr, const size_t size, size_t n_mem_b, FILE* stream) {
    const size_t written = fwrite(ptr, size, n_mem_b, stream);
    return written;
}

string convert_size(size_t size) {
    static const char* sizes[] = { "B", "KB", "MB", "GB" };
    int div = 0;
    size_t rem = 0;

    while (size >= 1024 && div < (sizeof sizes / sizeof * sizes)) {
        rem = (size % 1024);
        div++;
        size /= 1024;
    }

    const double size_d = static_cast<float>(size) + static_cast<float>(rem) / 1024.0;
    string result = to_string(round_off(size_d)) + " " + sizes[div];
    return result;
}

string current_download;
chrono::time_point<chrono::system_clock> last_time = std::chrono::system_clock::now();
int progress_bar(
    void* client_progress_data,
    const double dl_total,
    const double dl_now,
    double ul_total,
    double ul_now)
{
    if (const double n = dl_total; n > 0) {
        const string dls = "Downloading " + current_download + " (" + convert_size(static_cast<size_t>(dl_total)) + ")";
        auto bar1 = new ProgressBar(static_cast<unsigned long>(n), dls.c_str());
        if (const std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - last_time; elapsed_seconds.count() >= 0.2)
        {
            last_time = std::chrono::system_clock::now();
            bar1->Progressed(static_cast<unsigned long>(dl_now));
        }
    }
    return 0;
}

bool download_file(const char* url, const char* dest) {
    if (CURL* curl = curl_easy_init(); curl) {
        FILE* fp = fopen(dest, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_bar);
        curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, true);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true); // Follows 301 redirects, like in the WebView2 link
        const CURLcode res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);

        insert_empty_line();

        last_time = std::chrono::system_clock::from_time_t(0);
        cout << " Finished downloading " << current_download << endl;
        return res == CURLE_OK;
    }
    return false;
}