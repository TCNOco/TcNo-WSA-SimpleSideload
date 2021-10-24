// WSA-SimpleSideload
// Copyright (C) 2021 TechNobo (Wesley Pyburn)
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Just note that this requires curl to be installed. I used "vcpkg install curl[openssl]:x64-windows-static" in Microsoft's vcpkg to accomplish that.
// This is based on my other project's code: the TcNo Account Switcher - https://github.com/TcNobo/TcNo-Acc-Switcher

#pragma comment(lib, "urlmon.lib")
#pragma comment (lib, "User32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <conio.h>
#include <filesystem>
#include <windows.h>
#include <Shlwapi.h>
#include <io.h>

#include <iostream>
#include <string>
#include <tchar.h>

#include <curl/curl.h>
#include <openssl/ssl.h>
#include <shlobj.h>

#include "TcNo.hpp"
#include "unzip.h"
using namespace std;
#ifndef _unzip_H
DECLARE_HANDLE(HZIP);
#endif


int main(int argc, wchar_t** argv)
{
    auto path = getOperatingPath();
    current_path(path);
    SetConsoleTitle(_T("TcNo WSA Simple Sideload"));
    cout << "Welcome to the TcNO WSA Simple Sideload!" << endl <<
        "This software will let you double-click .apks on Windows 11!" << endl <<
        "------------------------------------------------------------------------" << endl;

    // 1. Download/Locate Platform Tools (for adb)
    // Check if platform-tools available:
    if (!(exists(path / "platform-tools\\") && exists(path / "platform-tools\\adb.exe")))
    {
        // Download platform-tools if not next to exe.
        cout << "platform-tools not found. Downloading..." << endl;

        if (!download_file("https://dl.google.com/android/repository/platform-tools-latest-windows.zip", "platform-tools-latest-windows.zip"))
        {
            cout << "Failed to platform-tools. Please download from:" << endl <<
                "https://dl.google.com/android/repository/platform-tools-latest-windows.zip" << endl <<
                "and manually extract to place the platform-tools folder next to this .exe" << endl << endl;
        }

        cout << "Unzipping platform-tools..." << endl;
        HZIP hz = OpenZip(L"platform-tools-latest-windows.zip", 0);
        ZIPENTRY ze; GetZipItem(hz, -1, &ze); int numitems = ze.index;
        for (int i = 0; i < numitems; i++)
        {
            GetZipItem(hz, i, &ze);
            UnzipItem(hz, i, ze.name);
        }
        CloseZip(hz);
        cout << "Done!" << endl;
    } else
    {
        cout << "adb found in platform-tools!" << endl;
    }


    // 2. Move platform-tools to a more permanent home
    PWSTR path_appdata;
    auto get_folder_path_ret = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path_appdata);
    if (get_folder_path_ret != S_OK) {
        CoTaskMemFree(path_appdata);
        return 1;
    }
    std::filesystem::path appdata = path_appdata;
    appdata = appdata / "platform-tools";
    CoTaskMemFree(path_appdata);

    cout << "Now copying platform-tools to " << appdata << endl;
    filesystem::copy(path / "platform-tools\\", appdata, filesystem::copy_options::overwrite_existing);
    cout << "Copying self to \%AppData\%\\platform-tools" << endl;
    filesystem::copy(getExe(), appdata / "TcNo-WSA-SimpleSideload.exe", filesystem::copy_options::overwrite_existing);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
