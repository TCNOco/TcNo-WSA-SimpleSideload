<p align="center">
  <a href="https://tcno.co/">
    <img src="/other/img/Banner.png"></a>
</p>
<p align="center">
  <img alt="GitHub All Releases" src="https://img.shields.io/github/downloads/TcNobo/TcNo-WSA-SimpleSideload/total?logo=GitHub&style=flat-square">
  <a href="https://tcno.co/">
    <img alt="Website" src="/other/img/web.svg" height=20"></a>
  <a href="https://s.tcno.co/AccSwitcherDiscord">
    <img alt="Discord server" src="https://img.shields.io/discord/217649733915770880?label=Discord&logo=discord&style=flat-square"></a>
  <a href="https://twitter.com/TcNobo">
    <img alt="Twitter" src="https://img.shields.io/twitter/follow/TcNobo?label=Follow%20%40TcNobo&logo=Twitter&style=flat-square"></a>
  <img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/TcNobo/TcNo-WSA-SimpleSideload?logo=GitHub&style=flat-square">
  <img alt="GitHub repo size" src="https://img.shields.io/github/repo-size/TcNobo/TcNo-WSA-SimpleSideload?logo=GitHub&style=flat-square">
</p>
                                                                                                                                  
<p align="center"><a target="_blank" href="https://github.com/TcNobo/TcNo-WSA-SimpleSideload/releases/latest">
  <img alt="Download latest" src="/other/img/Download.png" height=70"></a>
</p>
 
**A simplified way to install apks, and interact with the new Windows Subsystem for Android**
**adb install xyz NO LONGER!** Running this software installs it. You can choose whether to make it the default action for .apks too!
Either double-click an apk (with it set to the default action - in the install), or Right-Click an apk and select `Install in WSA-SimpleSideload`.

# Other commands:
-  If you uninstall, or want to update, run "SimpleSideload.exe unlink". Either with the EXE in `%AppData%\platform-tools\`, or a downladed copy.
-  Run "SimpleSideload.exe reinstall" to have the opportunity to add file associations & the Context Menu option.
-  "SimpleSideload.exe settings" - Opens WSA's settings window
-  "SimpleSideload.exe push <File>" - Move a file to the WSA's Downloads folder. Add <Optional:Destination on Android> to push to another folder. Downloads (default) is: -  `./storage/emulated/0/Download`
-  "SimpleSideload.exe install <apkname/fullpath>" - Install an APK manually from the command line

## To update:
  Run "SimpleSideload.exe reinstall" in the directory you downloaded the updated program to.
  
# It runs adb commands for you
On install, if there isn't a platform-tools folder in the same directory, it automagically downloads the latest version, and extracts to `%AppData%\platform-tools`. It also copies itself there.
 The context menu interaction and arguments are all redirected into there, to work with ADB.
 
# Note
Make sure to have adb & this program closed to install, or reinstall. Having adb open could cause it to crash. Check your Task Manager.

## Screenshots

<p>
  <img alt="Main screenshot" src="/other/img/Image.png" width=773">
</p>

#### Disclaimer

```
All trademarks and materials are property of their respective owners and their licensors. This project is not affiliated
with Microsoft or Windows, or any other companies or groups that this software may have reference to. This project should
not be considered "Official" or related to platforms mentioned in any way. All it does it let you install things in WSA a lot easier.

I am not responsible for the contents of external links.
For the rest of the disclaimer, refer to the License (GNU General Public License v3.0) file:
https://github.com/TcNobo/TcNo-WSA-SimpleSideload/blob/master/LICENSE - See sections like 15, 16 and 17, as well as GitHub's
'simplification' at the top of the above website.
```
