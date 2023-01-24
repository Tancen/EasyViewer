# What is EasyViewer?
[中文文档](https://github.com/Tancen/EasyViewer/blob/dev/README_zh.md)<br/>
EasyViewer is an open source cross-platform remote control software developed based on Qt. It provides three core functions of remote desktop, file transfer and remote terminal. <br/>
EasyViewer currently includes the following programs：<br/>
- easyviewerd - Server program, under the project directory: ./Server
- easyviewerd-ctrl - Server control program, under the project directory: ./ServerCtrl
- easyviewer-client-gui - Client program with gui，under the project directory: ./Client/Client-UI
- easyviewer-client-without-gui - As a controlled client program without gui, it can only respond to file transfers and remote terminal requests， under the project directory: ./Client/Client-Console/Guard/Guard-Console
- easyviewer-terminal-viewer - Remote terminal client，under the project directory： ./Client/Client-Console/Viewer/TerminalViewer
# How is EasyViewer different?
Most of the remote control software on the market is aimed at ordinary users. The difference between EasyViewer and them is that EasyViewer provides remote terminal functions for IT practitioners.
# Supported platforms
Currently we have only verified it on Windows and Ubuntu 20.04, we do not guarantee availability on other platforms.<br/>
easyviewer-client-gui uses DXGI for screen data sampling on Windows, you need to make sure your OS supports it. [About DXGI](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/d3d10-graphics-programming-guide-dxgi)<br/>
easyviewer-client-gui uses some functions of xcb on Linux, please make sure it is installed correctly (on ubuntu you can execute the command `apt install libxcb-xinerama0 xcb` to install it)
# How to compile?
## Preparation
EasyViewer is developed based on Qt6.2.4, you need to ensure that your machine contains a development environment compatible with Qt6.2.4. [Download Qt](https://www.qt.io/download)
### Windows
EasyViewer is built with MSVC on Windows, please make sure it is installed correctly.<br/>
easyviewer-client-gui uses some functions of Direct3D on Windows, please make sure it is installed correctly.<br/>

### Linux
EasyViewer is built with gcc, g++ on Linux, please make sure they are installed correctly.<br/>
easyviewer-client-gui uses some functions of X11 on Linux, please make sure it is installed correctly(on ubuntu you can execute the command `apt install libxtst-dev` to install it).<br/>
easyviewer-client-gui uses some functions of xcb on Linux, please make sure it is installed correctly(on ubuntu you can execute the command `apt install libxcb-xinerama0 xcb` to install it).<br/>
easyviewerd-ctrl uses some functions of readline on Linux, please make sure it is installed correctly(on ubuntu you can execute the command `apt install libreadline-dev` to install it).<br/>
<br/>
## Start compiling
When all the preparatory work is completed, you only need to use Qt Creator to open EasyViewer.pro in the project root directory, and after configuration, click Build -> Build All Projects to complete all compilation work.</br>
Note:<br/>
1. On Windows, we only build for MSVC, please select MSVC x64 kits when compiling on Windows
2. We currently only provide support for the amd64 architecture. If you need support for other architectures, you need to compile the third-party libraries that EasyViewer depends on yourself. Their source codes are placed in the ./Global/Component directory, including : libyuv, openh264, openssl, protobuf, zlib
# Quick start
1. Generate a public-private key pair and name it key.public, key.private (you can also name it as you like)
2. Deploy easyviewerd
   1. Configure private key<br/>
        Modify the config.ini file in the easyviewerd directory, point [Server/PrivateKeyPath] to the key.private file path, or copy key.private to the easyviewerd directory<br/>
        Note: When [Server/PrivateKeyPath] is not specified, easyviewerd will load the file appDir/key.private as the private key by default
   2. Configure administrator password<br/>
        Modify the config.ini file in the easyviewerd directory, and fill in your administrator password at [Management/Password]
   3. Run easyviewerd <br/>
      Note: easyviewerd will listen to 2 ports, one for user service and one for background management. By default, they are 9748 and 19748 respectively
3. Add user
   1. Run easyviewerd-ctrl in the easyviewerd-ctrl directory with parameters --host=server_IP address_or_domain_name --port=server_background_management_port --public_key_file=key.public_file_path
   2. After entering the correct password, enter `user add username password` to add users
4. Start using
   1. Use easyviewer-client-gui to access partner and unattended
      1. Run easyviewer-client-gui <br/>
            Note: On Windows, when you start easyviewer-client-gui for the first time, it will automatically configure the running graphics processor. After clicking OK to exit the program, you need to restart easyviewer-client-gui
      2. Add server address</br>
            Right-click the Server Book component on the left side of the main window to pop up a menu, click the Add item in the menu, enter the correct configuration in the pop-up Add Server Address dialog, and click OK
      3. Select Server<br/>
            Double-click the item in the Server Book part of the main window with the left mouse button to select and activate it</br>
            So far, unattended is ready. If you need to access your partner, please enter the partner ID and the correct verification code on the right side of the main window, and click the Connect button.

   2. Use easyviewer-client-without-gui unattended
      1. Modify the config.ini file in the easyviewer-client-without-gui directory to fill in the correct configuration
      2. Run easyviewer-client-without-gui
   3. Access partner with easyviewer-terminal-viewer<br/>
        Run easyviewer-terminal-viewer with parameters --host=server_IP_address_or_domain_name --port=server_user_service_port --public_key_file=key.public_file_path --account=your_account --password=your_password --partner_id=Partner_ID --auth_string=Partner_verification_code
# License
EasyViewer is under the MIT license. See the [LICENSE](https://github.com/Tancen/EasyViewer/blob/dev/LICENSE) file for details.
# Other
EasyViewer is continuously improving, you can also use commercial software to get a better experience, here we list some commercial software that most recognizes:<br/>
- teamviewer：https://www.teamviewer.com/
- sunlogin：https://sunlogin.oray.com/
