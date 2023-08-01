# EasyViewer 是什么？
EasyViewer 是一款基于Qt开发，开源的跨平台远程控制软件，它提供了远程桌面、文件传输、远程终端三个核心功能。 <br/>
EasyViewer 当前包含以下主要程序：<br/>
- easyviewerd - 服务端程序, 对应工程目录： ./Server
- easyviewerd-ctrl - 服务端控制程序，对应工程目录： ./ServerCtrl
- easyviewer-client-gui - 带图形界面的客户端程序，对应工程目录： ./Client/Client-UI
- easyviewer-client-without-gui - 不带图形界面且仅作为受控端的客户端程序，它只能响应文件传输和远程终端请求， 对应工程目录： ./Client/Client-Console/Guard/Guard-Console
- easyviewer-terminal-viewer - 远程终端客户端，对应工程目录： ./Client/Client-Console/Viewer/TerminalViewer
# EasyViewer 有什么不同？
市面上大多数远程控制软件都是针对普通用户，EasyViewer和它们不同的是，EasyViewer特地为IT从业人员提供了远程终端功能。
# 支持的平台
当前我们仅在 Windows 和 Ubuntu 20.04 进行了验证，其它平台下目前我们不保证可用。<br/>
easyviewer-client-gui 在 Windows 上使用 DXGI 进行屏幕数据采集，你需要确保你的操作系统支持它。[关于DXGI](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/d3d10-graphics-programming-guide-dxgi)<br/>
easyviewer-client-gui 在 Linux 上使用 xcb 部分功能，请确保它被正确安装（在ubuntu上你可以执行命令 `apt install libxcb-xinerama0 xcb` 安装它）。
# 如何编译？
## 准备工作
EasyViewer 基于Qt6.2.4开发，你需要确保你的机器包含兼容Qt6.2.4的开发环境。[下载Qt](https://www.qt.io/download)
### Windows
EasyViewer 在 Windows 上使用 MSVC 构建，请确保它被正确安装。<br/>
easyviewer-client-gui 在 Windows 上使用了 Direct3D 的部分功能，请确保它被正确安装。<br/>

### Linux
EasyViewer 在 Linux 上使用 gcc、g++ 构建，请确保它们被正确安装。<br/>
easyviewer-client-gui 在 Linux 上使用了 X11 的部分功能，请确保它被正确安装（在ubuntu上你可以执行命令 `apt install libxtst-dev` 安装它）。<br/>
easyviewer-client-gui 在 Linux 上使用了 xcb 的部分功能，请确保它被正确安装（在ubuntu上你可以执行命令 `apt install libxcb-xinerama0 xcb` 安装它）。<br/>
easyviewerd-ctrl 在 Linux 上使用了 readline 的部分功能，请确保它被正确安装（在ubuntu上你可以执行命令 `apt install libreadline-dev` 安装它）。<br/>
<br/>
## 开始编译
当所有准备工作都完成以后，你只需要使用 Qt Creator 打开工程根目录下的 EasyViewer.pro， 在作好配置后点击 Build -> Build All Projects 就可以完成所有的编译工作。</br>
注：<br/>
1. 在 Windows 上，我们仅对 MSVC 进行了构建适配，在 Windows 上编译时请选择 MSVC x64 构建套
2. 我们当前仅提供了对 amd64 架构的支持，如果你需要对对其它架构的支持，你需要自行对 EasyViewer 依赖的第三方库进行编译，它们的源代码都放在 ./Global/Component 目录下， 包括：libyuv、openh264、openssl、protobuf、zlib
# 快速开始
1. 生成公私钥对，并命名为 key.public，key.private（你也可以按你的喜好自行命名）
2. 部署 easyviewerd
   1. 配置私钥<br/>
        修改 easyviewerd 目录下的 config.ini 文件，将 [Server/PrivateKeyPath] 指向 key.private 文件路径，或者将key.private 复制到  easyviewerd 目录下<br/>
        注：当 [Server/PrivateKeyPath] 未指定时，easyviewerd 将默认加载文件 appDir/key.private 作为私钥
   2. 配置管理员密码<br/>
        修改 easyviewerd 目录下的 config.ini 文件，在 [Management/Password] 处填入你的管理员密码
   3. 运行 easyviewerd <br/>
      注：easyviewerd 会监听2个端口，一个用于用户服务，一个用于后台管理，默认情况下，它们分别是 9748 和 19748
3. 添加用户账户
   1. 在 easyviewerd-ctrl 目录下运行 easyviewerd-ctrl 并附带参数 --host=服务器IP地址或域名 --port=服务器后台管理端口 --public_key_file=key.public文件路径
   2. 在输入正确的密码后，输入 `user add 用户名 密码` 即可添加用户
4. 开始使用
   1. 使用 easyviewer-client-gui 访问伙伴和无人值守
      1. 运行 easyviewer-client-gui <br/>
            注：在 Windows 上，第一次启动 easyviewer-client-gui 时，它会自动配置运行的图形处理器，点击确定退出程序后，需要重新启动 easyviewer-client-gui
      2. 添加服务器地址</br>
            在主界面左侧的 Server Book 部件鼠标右键单击后弹出菜单，点击菜单中的 Add 项，在弹出的添加服务器地址界面中输入正确的配置参数后点击确定
      3. 选择 Server<br/>
            鼠标左键双击主界面  Server Book 部件内的条目就可以选中并激活</br>
            到目前为止，无人值守已准备就绪，如果你需要访问你的伙伴，请在主界面右侧输入伙伴的ID和正确的验证码，点击 Connect 按钮即可

   2. 使用 easyviewer-client-without-gui 无人值守
      1. 修改 easyviewer-client-without-gui 目录下的 config.ini 文件填入正确的配置
      2. 运行 easyviewer-client-without-gui
   3. 使用 easyviewer-terminal-viewer 访问伙伴<br/>
        运行 easyviewer-terminal-viewer 并带上参数 --host=服务器IP地址或域名 --port=服务器用户服务端口 --public_key_file=key.public文件路径 --account=你的账号 --password=你的密码 --partner_id=伙伴ID --auth_string=伙伴验证码
# 许可
EasyViewer 基于 MIT 许可。[查看详情](https://github.com/Tancen/EasyViewer/blob/dev/LICENSE)
