QT -= gui
QT += network sql

CONFIG += c++17 console object_parallel_to_source no_batch
CONFIG -= app_bundle

TARGET = easyviewer-client-without-gui

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ../../../

SOURCES += \
        ../../../../Global/Component/EasyIO/EasyIOByteBuffer.cpp \
        ../../../../Global/Component/EasyIO/EasyIOContext_linux.cpp \
        ../../../../Global/Component/EasyIO/EasyIOContext_win.cpp \
        ../../../../Global/Component/EasyIO/EasyIOError.cpp \
        ../../../../Global/Component/EasyIO/EasyIOEventLoopGroup.cpp \
        ../../../../Global/Component/EasyIO/EasyIOEventLoop_linux.cpp \
        ../../../../Global/Component/EasyIO/EasyIOEventLoop_win.cpp \
        ../../../../Global/Component/EasyIO/EasyIOTCPAcceptor.cpp \
        ../../../../Global/Component/EasyIO/EasyIOTCPClient_linux.cpp \
        ../../../../Global/Component/EasyIO/EasyIOTCPClient_win.cpp \
        ../../../../Global/Component/EasyIO/EasyIOTCPConnection_linux.cpp \
        ../../../../Global/Component/EasyIO/EasyIOTCPConnection_win.cpp \
        ../../../../Global/Component/EasyIO/EasyIOTCPServer_linux.cpp \
        ../../../../Global/Component/EasyIO/EasyIOTCPServer_win.cpp \
        ../../../../Global/Component/EasyIO/EasyIOUDPClient.cpp \
        ../../../../Global/Component/EasyIO/EasyIOUDPServer.cpp \
        ../../../../Global/Component/EasyIO/EasyIOUDPSocket_linux.cpp \
        ../../../../Global/Component/EasyIO/EasyIOUDPSocket_win.cpp \
        ../../../../Global/Component/Logger/ConsoleLogger.cpp \
        ../../../../Global/Component/Logger/FileLogger.cpp \
        ../../../../Global/Component/Logger/Logger.cpp \
        ../../../../Global/Component/ReadWriteLock/ReadWriteLock.cpp \
        ../../../../Global/Component/SpeedCalculation/SpeedCalculation.cpp \
        ../../../../Global/Protocol/Error.cpp \
        ../../../../Global/Protocol/File/File.pb.cc \
        ../../../../Global/Protocol/Terminal/Terminal.pb.cc \
        ../../../../Global/Protocol/User/User.pb.cc  \
        ../../../AuthChecker.cpp \
        ../../../Command/TerminalGuardAbility.cpp \
        ../../../Command/TerminalManager.cpp \
        ../../../File/FileTransmission.cpp \
        ../../../File/FileTransmissionGuardAbility.cpp \
        ../../../Guard.cpp \
        Application.cpp \
        main.cpp \
        ../../../ViewerBase.cpp

HEADERS += \
    ../../../../Global/Component/EasyIO/EasyIO.h \
    ../../../../Global/Component/EasyIO/EasyIOByteBuffer.h \
    ../../../../Global/Component/EasyIO/EasyIOContext_linux.h \
    ../../../../Global/Component/EasyIO/EasyIOContext_win.h \
    ../../../../Global/Component/EasyIO/EasyIODef.h \
    ../../../../Global/Component/EasyIO/EasyIOError.h \
    ../../../../Global/Component/EasyIO/EasyIOEventLoopGroup.h \
    ../../../../Global/Component/EasyIO/EasyIOEventLoop_linux.h \
    ../../../../Global/Component/EasyIO/EasyIOEventLoop_win.h \
    ../../../../Global/Component/EasyIO/EasyIOIEventLoop.h \
    ../../../../Global/Component/EasyIO/EasyIOTCPAcceptor.h \
    ../../../../Global/Component/EasyIO/EasyIOTCPClient_linux.h \
    ../../../../Global/Component/EasyIO/EasyIOTCPClient_win.h \
    ../../../../Global/Component/EasyIO/EasyIOTCPConnection_linux.h \
    ../../../../Global/Component/EasyIO/EasyIOTCPConnection_win.h \
    ../../../../Global/Component/EasyIO/EasyIOTCPIClient.h \
    ../../../../Global/Component/EasyIO/EasyIOTCPIConnection.h \
    ../../../../Global/Component/EasyIO/EasyIOTCPIServer.h \
    ../../../../Global/Component/EasyIO/EasyIOTCPServer_linux.h \
    ../../../../Global/Component/EasyIO/EasyIOTCPServer_win.h \
    ../../../../Global/Component/EasyIO/EasyIOUDPClient.h \
    ../../../../Global/Component/EasyIO/EasyIOUDPIClient.h \
    ../../../../Global/Component/EasyIO/EasyIOUDPIServer.h \
    ../../../../Global/Component/EasyIO/EasyIOUDPISocket.h \
    ../../../../Global/Component/EasyIO/EasyIOUDPServer.h \
    ../../../../Global/Component/EasyIO/EasyIOUDPSocket_linux.h \
    ../../../../Global/Component/EasyIO/EasyIOUDPSocket_win.h \
    ../../../../Global/Component/Logger/ConsoleLogger.h \
    ../../../../Global/Component/Logger/FileLogger.h \
    ../../../../Global/Component/Logger/ILogger.h \
    ../../../../Global/Component/Logger/Logger.h \
    ../../../../Global/Component/ReadWriteLock/ReadWriteLock.h \
    ../../../../Global/Component/SpeedCalculation/SpeedCalculation.h \
    ../../../../Global/Define.h \
    ../../../../Global/Protocol/Error.h \
    ../../../../Global/Protocol/File/File.pb.h \
    ../../../../Global/Protocol/Protocol.h \
    ../../../../Global/Protocol/Terminal/Terminal.pb.h \
    ../../../../Global/Protocol/User/User.pb.h  \
    ../../../AuthChecker.h \
    ../../../Command/TerminalGuardAbility.h \
    ../../../Command/TerminalManager.h \
    ../../../File/FileTransmission.h \
    ../../../File/FileTransmissionGuardAbility.h \
    ../../../Guard.h \
    Application.h \
    ../../../ViewerBase.h \
    Version.h

INCLUDEPATH += ../../../
INCLUDEPATH += ../../../../
INCLUDEPATH += ../../../../Global/Component/protobuf/include
INCLUDEPATH += ../../../../Global/Component/libvpx/include
INCLUDEPATH += ../../../../Global/Component/zlib/include
INCLUDEPATH += ../../../../Global/Component/openssl/include

windows:{

    msvc {
       QMAKE_CFLAGS += /utf-8
       QMAKE_CXXFLAGS += /utf-8
    }

    DEFINES += WIN32_LEAN_AND_MEAN
    DEFINES += PROTOBUF_USE_DLLS
    LIBS += -lWs2_32 -lUser32 -lAdvapi32 -lShell32

    CONFIG(debug, debug|release){
        QMAKE_CFLAGS_RELEASE += -g
        QMAKE_CXXFLAGS_RELEASE += -g
        QMAKE_CXXFLAGS_RELEASE -= -O2
        QMAKE_CFLAGS_RELEASE -= -O2

        # protobuf
        LIBS += $$PWD/../../../../Global/Component/protobuf/lib/win/debug/libprotobufd.lib
        LIBS += $$PWD/../../../../Global/Component/protobuf/lib/win/debug/libprotobuf-lited.lib
        LIBS += $$PWD/../../../../Global/Component/protobuf/lib/win/debug/libprotocd.lib
    } else {
        # protobuf
        LIBS += $$PWD/../../../../Global/Component/protobuf/lib/win/release/libprotobuf.lib
        LIBS += $$PWD/../../../../Global/Component/protobuf/lib/win/release/libprotobuf-lite.lib
        LIBS += $$PWD/../../../../Global/Component/protobuf/lib/win/release/libprotoc.lib
    }

    # zlib
    LIBS += $$PWD/../../../../Global/Component/zlib/lib/win/zlibwapi.lib

    # openssl
    LIBS += $$PWD/../../../../Global/Component/openssl/lib/win/libcrypto.lib
    LIBS += $$PWD/../../../../Global/Component/openssl/lib/win/libssl.lib
}

linux:{
    QMAKE_LFLAGS += -Wl,-rpath=\$$ORIGIN

    # protobuf
    LIBS += $$PWD/../../../../Global/Component/protobuf/lib/linux/libprotobuf.a
    LIBS += $$PWD/../../../../Global/Component/protobuf/lib/linux/libprotobuf-lite.a
    LIBS += $$PWD/../../../../Global/Component/protobuf/lib/linux/libprotoc.a

    # zlib
    LIBS += $$PWD/../../../../Global/Component/zlib/lib/linux/libz.a

    # openssl
    LIBS += $$PWD/../../../../Global/Component/openssl/lib/linux/libcrypto.a
    LIBS += $$PWD/../../../../Global/Component/openssl/lib/linux/libssl.a

    LIBS += -ldl -lutil
}

DISTFILES += \
    config.ini
