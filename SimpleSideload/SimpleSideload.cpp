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
#include <strsafe.h>

#include "TcNo.hpp"
#include "unzip.h"

using namespace std;

#ifndef _unzip_H
DECLARE_HANDLE(HZIP);
#endif

void install_self();
void connect_to_devices();
void connect_and_retry(string command);
void finish_install(int argc, char** argv);
bool associate_with_apk();
bool add_context_option();
bool unlink_associations();
bool RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey);
bool RegDelnode(HKEY hKeyRoot, LPCTSTR lpSubKey);

string adb_location = "\%AppData\%\\platform-tools\\adb.exe ";

std::filesystem::path getAppData()
{
    PWSTR path_appdata;
    auto get_folder_path_ret = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path_appdata);
    if (get_folder_path_ret != S_OK) {
        CoTaskMemFree(path_appdata);
        return "";
    }
    filesystem::path appdata = path_appdata;
    CoTaskMemFree(path_appdata);
    return path_appdata;
}

filesystem::path path, appdata;

int main(int argc, char** argv)
{
    SetConsoleTitle(_T("TcNo WSA Simple Sideload"));
    string command, lowerCommand;

    path = getOperatingPath();
    appdata = getAppData();
    if (argc > 1)
    {
		lowerCommand = argv[1];
        std::for_each(lowerCommand.begin(), lowerCommand.end(), [](char& c) {
            c = ::tolower(c);
            });
        if (lowerCommand == "finish") {
            finish_install(argc, argv);
            return 0;
        }
        if (lowerCommand == "unlink" || lowerCommand == "uninstall")
        {
            unlink_associations();
            return 0;
        }
    }

    // Handle install (If not running from AppData)
    if (path.string().rfind(appdata.string(), 0) != 0)
    {
        // Not running in AppData already
        // OR if being run with 'reinstall':
        if (!filesystem::exists(appdata / "platform-tools\\TcNo-WSA-SimpleSideload.exe") || lowerCommand == "reinstall")
        {
            // Doesn't exist > Install
            install_self();
            return 0;
        }

        // Else:
        string args;
		for (size_t i = 1; i < argc; ++i)
            args += '"' + argv[i] + '"';

        auto run = appdata / "TcNo-WSA-SimpleSideload.exe";
        ShellExecuteA(NULL,
            "runas",
            run.string().c_str(),
            args.c_str(),
            NULL,                        // default dir
            SW_SHOWNORMAL
        );
    }

    // Handle arguments
	if (argc == 1)
    {
        cout << "This utility requires an argument (APK location)" << endl;
        return 1;
    }

    if (lowerCommand == "settings") // Open settings window:
        command = "shell monkey -p com.android.settings -c android.intent.category.LAUNCHER 1";
    else if (lowerCommand == "push" && argc >= 3) {// Push file to downloads folder:
        string androidPath = "./storage/emulated/0/Download";
        if (argc >= 4) androidPath = argv[3];

        command = "push \"" + std::string(argv[3]) + "\" " + androidPath;
    }
    else { // Install application:
		std::string app_name(argv[1]);
		if (app_name.find("\\") != std::string::npos) app_name = app_name.substr(app_name.find_last_of('\\'), app_name.size() - app_name.find_last_of('\\') - 1);
        cout << "Installing: " << app_name << endl;
        command = "install \"" + std::string(argv[1]) + "\"";
    }

    auto result = exec(adb_location + command);
    if (result.find("no devices/emulators found") != std::string::npos)
    {
        connect_and_retry(adb_location + command);
    };

    cout << "Closing in 3 seconds..." << endl;
    Sleep(3000);
}

void connect_to_devices()
{
    string ip;
    cout << "Please input your WSA IP: ";
    cin >> ip;
    cout << endl;

    cout << "Connecting to ADB device: adb connect " << ip << endl;
    string connect = adb_location + "connect " + ip;
	exec(connect);
}

void connect_and_retry(string command)
{
    connect_to_devices();
    cout << "Retrying: " << command << endl;
    if (exec(command).find("no devices/emulators found") != std::string::npos)
    {
        system("pause");
        exit(0);
    };
}

void restart_as_admin_from_install(string arg)
{
    cout << "Restarting as Admin to finish install..." << endl;

    auto run  = appdata / "TcNo-WSA-SimpleSideload.exe";
    ShellExecuteA(NULL,
        "runas",
        run.string().c_str(),
        arg.c_str(),
        NULL,                        // default dir
        SW_SHOWNORMAL
    );
}

void finish_install(int argc, char** argv)
{
    // Finish install >> Add to context menu.
    cout << "Finishing install..." << endl;
    appdata = getAppData();


	if (argc == 3) // likely asked to associate with .apks as well
    {
        associate_with_apk();
        cout << "Associated with .apks." << endl;
    }

    add_context_option();
    cout << "Added option to context menu." << endl;

    cout << "Closing in 3 seconds..." << endl;
    Sleep(3000);
}

bool associate_with_apk()
{
    HKEY hkey;
    string desc = "WSA Application";
    auto run = appdata / "platform-tools\\TcNo-WSA-SimpleSideload.exe";
    string app = run.string() + " \"%1\"";
    string extension = ".apk";

    string path = "SOFTWARE\\Classes\\" + extension + "\\shell\\open\\command\\";
    //string path = "SOFTWARE\\Classes\\.apk\\shell\\Install in WSA-SimpleSideload\\command\\";

    // Sub-key creation -- HKEY_CLASSES_ROOT\.apk
    if (RegCreateKeyExA(HKEY_CURRENT_USER, extension.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, 0) != ERROR_SUCCESS)
    {
        return false;
    }
    RegSetValueExA(hkey, "", 0, REG_SZ, (BYTE*)desc.c_str(), sizeof(desc)); // default vlaue is description of file extension
    RegCloseKey(hkey);

    // Action sub-keys -- HKEY_CLASSES_ROOT\.apk\\Shell\\X\\command
    if (RegCreateKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, 0) != ERROR_SUCCESS)
    {
        return false;
    }
    RegSetValueExA(hkey, "", 0, REG_SZ, (BYTE*)app.c_str(), app.length());

    RegCloseKey(hkey);
}

bool add_context_option()
{
    HKEY hkey;
    string desc = "WSA Application";
    auto run = appdata / "platform-tools\\TcNo-WSA-SimpleSideload.exe";
    string app = run.string() + " \"%1\"";
    string extension = ".apk";

    string path = extension + "\\shell\\Install in WSA-SimpleSideload\\command\\";

    // Sub-key creation -- HKEY_CLASSES_ROOT\.apk
    if (RegCreateKeyExA(HKEY_CLASSES_ROOT, extension.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, 0) != ERROR_SUCCESS)
    {
        return false;
    }
    RegSetValueExA(hkey, "", 0, REG_SZ, (BYTE*)desc.c_str(), sizeof(desc)); // default vlaue is description of file extension
    RegCloseKey(hkey);

    // Action sub-keys -- HKEY_CLASSES_ROOT\.apk\\Shell\\X\\command
    if (RegCreateKeyExA(HKEY_CLASSES_ROOT, path.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, 0) != ERROR_SUCCESS)
    {
        return false;
    }
    RegSetValueExA(hkey, "", 0, REG_SZ, (BYTE*)app.c_str(), app.length());

    RegCloseKey(hkey);
}

bool is_elevated() {
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return fRet;
}

bool unlink_associations()
{
    if (!is_elevated())
    {
        cout << "Restarting as Admin to finish unlink..." << endl;

        auto run = appdata / "platform-tools\\TcNo-WSA-SimpleSideload.exe";
        ShellExecuteA(NULL,
            "runas",
            run.string().c_str(),
            "unlink",
            NULL,                        // default dir
            SW_SHOWNORMAL
        );

        return false;
    }


    if (RegDelnode(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Classes\\.apk\\shell\\open")))
    {
        cout << "Removed Install in WSA-SimpleSideload option from context menu" << endl;
    }
    if (RegDelnode(HKEY_CLASSES_ROOT, TEXT(".apk\\shell\\Install in WSA-SimpleSideload")))
    {
        cout << "Removed as default option for .apk filetype" << endl;
    }

    system("pause");
}

void install_self()
{
	filesystem::current_path(path);
    cout << "Welcome to the TcNO WSA Simple Sideload!" << endl <<
        "This software will let you double-click .apks on Windows 11!" << endl <<
        "------------------------------------------------------------------------" << endl;

    // 1. Download/Locate Platform Tools (for adb)
    // Check if platform-tools available:
    if (!(filesystem::exists(path / "platform-tools\\") && filesystem::exists(path / "platform-tools\\adb.exe")))
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
    }
    else
    {
        cout << "adb found in platform-tools!" << endl;
    }


    // 2. Move platform-tools to a more permanent home
    appdata = appdata / "platform-tools";

    cout << "Now copying platform-tools to " << appdata << endl;
    filesystem::copy(path / "platform-tools\\", appdata, filesystem::copy_options::overwrite_existing);
    cout << "Copying self to \%AppData\%\\platform-tools" << endl;
    filesystem::copy(getExe(), appdata / "TcNo-WSA-SimpleSideload.exe", filesystem::copy_options::overwrite_existing);


    cout << "------------------------------------------------------------------------" << endl <<
        "Do you want to associate .apks with this auto-installer? (in AppData/platform-tools)" << endl <<
        "Y/N: ";
    string yn;
    cin >> yn;
    cout << endl;

    if (yn == "y" || yn == "Y")
        restart_as_admin_from_install("finish associate");
    else
        restart_as_admin_from_install("finish");
}

// https://docs.microsoft.com/en-us/windows/win32/sysinfo/deleting-a-key-with-subkeys
bool RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
        return TRUE;

    lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS)
    {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            printf("Key not found.\n");
            return TRUE;
        }
        else {
            printf("Error opening key.\n");
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\'))
    {
        *lpEnd = TEXT('\\');
        lpEnd++;
        *lpEnd = TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
        NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS)
    {
        do {

            *lpEnd = TEXT('\0');
            StringCchCat(lpSubKey, MAX_PATH * 2, szName);

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey(hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
        return TRUE;

    return FALSE;
}

// https://docs.microsoft.com/en-us/windows/win32/sysinfo/deleting-a-key-with-subkeys
bool RegDelnode(HKEY hKeyRoot, LPCTSTR lpSubKey)
{
    TCHAR szDelKey[MAX_PATH * 2];

    StringCchCopy(szDelKey, MAX_PATH * 2, lpSubKey);
    return RegDelnodeRecurse(hKeyRoot, szDelKey);

}