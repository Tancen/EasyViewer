QT       += core gui openglwidgets sql widgets network

CONFIG += c++17 object_parallel_to_source no_batch

DEFINES += QT_DEPRECATED_WARNINGS

TARGET = easyviewer-client-gui

SOURCES += \
    ../../Global/Component/EasyIO/EasyIOByteBuffer.cpp \
    ../../Global/Component/EasyIO/EasyIOContext_linux.cpp \
    ../../Global/Component/EasyIO/EasyIOContext_win.cpp \
    ../../Global/Component/EasyIO/EasyIOError.cpp \
    ../../Global/Component/EasyIO/EasyIOEventLoopGroup.cpp \
    ../../Global/Component/EasyIO/EasyIOEventLoop_linux.cpp \
    ../../Global/Component/EasyIO/EasyIOEventLoop_win.cpp \
    ../../Global/Component/EasyIO/EasyIOTCPAcceptor.cpp \
    ../../Global/Component/EasyIO/EasyIOTCPClient_linux.cpp \
    ../../Global/Component/EasyIO/EasyIOTCPClient_win.cpp \
    ../../Global/Component/EasyIO/EasyIOTCPConnection_linux.cpp \
    ../../Global/Component/EasyIO/EasyIOTCPConnection_win.cpp \
    ../../Global/Component/EasyIO/EasyIOTCPServer_linux.cpp \
    ../../Global/Component/EasyIO/EasyIOTCPServer_win.cpp \
    ../../Global/Component/EasyIO/EasyIOUDPClient.cpp \
    ../../Global/Component/EasyIO/EasyIOUDPServer.cpp \
    ../../Global/Component/EasyIO/EasyIOUDPSocket_linux.cpp \
    ../../Global/Component/EasyIO/EasyIOUDPSocket_win.cpp \
    ../../Global/Component/Logger/ConsoleLogger.cpp \
    ../../Global/Component/Logger/FileLogger.cpp \
    ../../Global/Component/Logger/Logger.cpp \
    ../../Global/Component/SpeedCalculation/SpeedCalculation.cpp \
    ../../Global/Protocol/Common/Echo.pb.cc \
    ../../Global/Protocol/Error.cpp \
    ../../Global/Protocol/File/File.pb.cc \
    ../../Global/Protocol/Screen/Control.pb.cc \
    ../../Global/Protocol/Screen/Screen.pb.cc \
    ../../Global/Protocol/Terminal/Terminal.pb.cc \
    ../../Global/Protocol/User/User.pb.cc \
    ../AuthChecker.cpp \
    ../Command/TerminalGuardAbility.cpp \
    ../Command/TerminalManager.cpp \
    ../Desktop/Clipboard.cpp \
    ../Desktop/DesktopDisplayer.cpp \
    ../Desktop/ScreenCapturerBase.cpp \
    ../Desktop/ScreenCapturer_linux.cpp \
    ../Desktop/ScreenCapturer_win.cpp \
    ../Desktop/ScreenGuardAbility.cpp \
    ../Desktop/ScreenViewer.cpp \
    ../File/FileTransmissionGuardAbility.cpp \
    ../File/FileTransmissionViewer.cpp \
    ../Guard.cpp \
    ../ViewerBase.cpp \
    ../File/FileTransmission.cpp \
    ../File/FilesBrowseWidget.cpp \
    MainWidget.cpp \
    PartnerSettingsDialog.cpp \
    ServerAddressBook.cpp \
    ServerAddressDialog.cpp \
    main.cpp

HEADERS += \
    ../../Global/Component/Cryptology/Cryptology.hpp \
    ../../Global/Component/EasyIO/EasyIO.h \
    ../../Global/Component/EasyIO/EasyIOByteBuffer.h \
    ../../Global/Component/EasyIO/EasyIOContext_linux.h \
    ../../Global/Component/EasyIO/EasyIOContext_win.h \
    ../../Global/Component/EasyIO/EasyIODef.h \
    ../../Global/Component/EasyIO/EasyIOError.h \
    ../../Global/Component/EasyIO/EasyIOEventLoopGroup.h \
    ../../Global/Component/EasyIO/EasyIOEventLoop_linux.h \
    ../../Global/Component/EasyIO/EasyIOEventLoop_win.h \
    ../../Global/Component/EasyIO/EasyIOIEventLoop.h \
    ../../Global/Component/EasyIO/EasyIOTCPAcceptor.h \
    ../../Global/Component/EasyIO/EasyIOTCPClient_linux.h \
    ../../Global/Component/EasyIO/EasyIOTCPClient_win.h \
    ../../Global/Component/EasyIO/EasyIOTCPConnection_linux.h \
    ../../Global/Component/EasyIO/EasyIOTCPConnection_win.h \
    ../../Global/Component/EasyIO/EasyIOTCPIClient.h \
    ../../Global/Component/EasyIO/EasyIOTCPIConnection.h \
    ../../Global/Component/EasyIO/EasyIOTCPIServer.h \
    ../../Global/Component/EasyIO/EasyIOTCPServer_linux.h \
    ../../Global/Component/EasyIO/EasyIOTCPServer_win.h \
    ../../Global/Component/EasyIO/EasyIOUDPClient.h \
    ../../Global/Component/EasyIO/EasyIOUDPIClient.h \
    ../../Global/Component/EasyIO/EasyIOUDPIServer.h \
    ../../Global/Component/EasyIO/EasyIOUDPISocket.h \
    ../../Global/Component/EasyIO/EasyIOUDPServer.h \
    ../../Global/Component/EasyIO/EasyIOUDPSocket_linux.h \
    ../../Global/Component/EasyIO/EasyIOUDPSocket_win.h \
    ../../Global/Component/Logger/ConsoleLogger.h \
    ../../Global/Component/Logger/FileLogger.h \
    ../../Global/Component/Logger/ILogger.h \
    ../../Global/Component/Logger/Logger.h \
    ../../Global/Component/SpeedCalculation/SpeedCalculation.h \
    ../../Global/Define.h \
    ../../Global/Protocol/Common/Echo.pb.h \
    ../../Global/Protocol/Error.h \
    ../../Global/Protocol/File/File.pb.h \
    ../../Global/Protocol/Protocol.h \
    ../../Global/Protocol/Screen/Control.pb.h \
    ../../Global/Protocol/Screen/Screen.pb.h \
    ../../Global/Protocol/Terminal/Terminal.pb.h \
    ../../Global/Protocol/User/User.pb.h \
    ../AuthChecker.h \
    ../Command/TerminalArguments.h \
    ../Command/TerminalGuardAbility.h \
    ../Command/TerminalManager.h \
    ../Define.h \
    ../Desktop/Buttons_linux.h \
    ../Desktop/Clipboard.h \
    ../Desktop/DesktopDisplayer.h \
    ../Desktop/ScreenCapturer_linux.h \
    ../Desktop/ScreenCapturer_win.h \
    ../Desktop/ScreenGuardAbility.h \
    ../Desktop/ScreenViewer.h \
    ../Desktop/Shortcuts.h \
    ../File/FileTransmissionGuardAbility.h \
    ../File/FileTransmissionViewer.h \
    ../Guard.h \
    ../UsualParseProtobufDataMacro.h \
    ../ViewerBase.h \
    ../File/FileTransmission.h \
    ../File/FilesBrowseWidget.h \
    MainWidget.h \
    ../Desktop/IScreenCapturer.h \
    ../Desktop/ScreenCapturerBase.h \
    PartnerSettingsDialog.h \
    ServerAddressBook.h \
    ServerAddressDialog.h



INCLUDEPATH += ../
INCLUDEPATH += ../../
INCLUDEPATH += ../../Global/Component/protobuf/include
INCLUDEPATH += ../../Global/Component/zlib/include
INCLUDEPATH += ../../Global/Component/openssl/include
INCLUDEPATH += ../../Global/Component/libyuv/include
INCLUDEPATH += ../../Global/Component/openh264/include

windows:{
    msvc {
        QMAKE_CFLAGS += /utf-8
        QMAKE_CXXFLAGS += /utf-8

        QMAKE_LFLAGS += /MANIFESTUAC:"level='requireAdministrator'"
    }

    DEFINES += WIN32_LEAN_AND_MEAN
    DEFINES += PROTOBUF_USE_DLLS
    LIBS += -lWs2_32 -lD3D11 -lUser32 -lopengl32 -lglu32 -ldxgi -ldxguid -lAdvapi32

    CONFIG(debug, debug|release){
        QMAKE_CFLAGS_RELEASE += -g
        QMAKE_CXXFLAGS_RELEASE += -g
        QMAKE_CXXFLAGS_RELEASE -= -O2
        QMAKE_CFLAGS_RELEASE -= -O2

        # protobuf
        LIBS += $$PWD/../../Global/Component/protobuf/lib/win/debug/libprotobufd.lib
        LIBS += $$PWD/../../Global/Component/protobuf/lib/win/debug/libprotobuf-lited.lib
        LIBS += $$PWD/../../Global/Component/protobuf/lib/win/debug/libprotocd.lib
    } else {
        # protobuf
        LIBS += $$PWD/../../Global/Component/protobuf/lib/win/release/libprotobuf.lib
        LIBS += $$PWD/../../Global/Component/protobuf/lib/win/release/libprotobuf-lite.lib
        LIBS += $$PWD/../../Global/Component/protobuf/lib/win/release/libprotoc.lib
    }

    # zlib
    LIBS += $$PWD/../../Global/Component/zlib/lib/win/zlibwapi.lib

    # openssl
    LIBS += $$PWD/../../Global/Component/openssl/lib/win/libcrypto.lib
    LIBS += $$PWD/../../Global/Component/openssl/lib/win/libssl.lib

    # libyuv
    LIBS += $$PWD/../../Global/Component/libyuv/lib/win/yuv.lib

    # openh264
    LIBS += $$PWD/../../Global/Component/openh264/lib/win/openh264.lib
}


linux:{
    QMAKE_LFLAGS += -Wl,-rpath=\$$ORIGIN

    # protobuf
    LIBS += $$PWD/../../Global/Component/protobuf/lib/linux/libprotobuf.a
    LIBS += $$PWD/../../Global/Component/protobuf/lib/linux/libprotobuf-lite.a
    LIBS += $$PWD/../../Global/Component/protobuf/lib/linux/libprotoc.a

    # zlib
    LIBS += $$PWD/../../Global/Component/zlib/lib/linux/libz.a

    # openssl
    LIBS += $$PWD/../../Global/Component/openssl/lib/linux/libcrypto.a
    LIBS += $$PWD/../../Global/Component/openssl/lib/linux/libssl.a

    # libyuv
    LIBS += $$PWD/../../Global/Component/libyuv/lib/linux/libyuv.a

    # openh264
    LIBS += $$PWD/../../Global/Component/openh264/lib/linux/libopenh264.so.7

    LIBS += -lutil -lX11 -lXtst -lXi -lGLU -ldl
}

RESOURCES += \
    res.qrc

FORMS += \
    Forms/Desktop/ScreenViewer.ui \
    Forms/File/FileTransmissionViewer.ui \
    Forms/File/FilesBrowseWidget.ui \
    Forms/MainWidget.ui \
    Forms/PartnerSettingsDialog.ui \
    Forms/ServerAddressDialog.ui
