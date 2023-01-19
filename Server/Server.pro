QT -= gui
QT += sql

CONFIG += c++17 console object_parallel_to_source no_batch
CONFIG -= app_bundle

TARGET = easyviewerd

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ../

SOURCES += \
        ../Global/Component/EasyIO/EasyIOByteBuffer.cpp \
        ../Global/Component/EasyIO/EasyIOContext_linux.cpp \
        ../Global/Component/EasyIO/EasyIOContext_win.cpp \
        ../Global/Component/EasyIO/EasyIOError.cpp \
        ../Global/Component/EasyIO/EasyIOEventLoopGroup.cpp \
        ../Global/Component/EasyIO/EasyIOEventLoop_linux.cpp \
        ../Global/Component/EasyIO/EasyIOEventLoop_win.cpp \
        ../Global/Component/EasyIO/EasyIOTCPAcceptor.cpp \
        ../Global/Component/EasyIO/EasyIOTCPClient_linux.cpp \
        ../Global/Component/EasyIO/EasyIOTCPClient_win.cpp \
        ../Global/Component/EasyIO/EasyIOTCPConnection_linux.cpp \
        ../Global/Component/EasyIO/EasyIOTCPConnection_win.cpp \
        ../Global/Component/EasyIO/EasyIOTCPServer_linux.cpp \
        ../Global/Component/EasyIO/EasyIOTCPServer_win.cpp \
        ../Global/Component/EasyIO/EasyIOUDPClient.cpp \
        ../Global/Component/EasyIO/EasyIOUDPServer.cpp \
        ../Global/Component/EasyIO/EasyIOUDPSocket_linux.cpp \
        ../Global/Component/EasyIO/EasyIOUDPSocket_win.cpp \
        ../Global/Component/Logger/ConsoleLogger.cpp \
        ../Global/Component/Logger/FileLogger.cpp \
        ../Global/Component/Logger/Logger.cpp \
        ../Global/Component/ReadWriteLock/ReadWriteLock.cpp \
        ../Global/Protocol/Error.cpp \
        ../Global/Protocol/File/File.pb.cc \
        ../Global/Protocol/Management/Server/Management.pb.cc \
        ../Global/Protocol/Screen/Control.pb.cc \
        ../Global/Protocol/Screen/Screen.pb.cc \
        ../Global/Protocol/Terminal/Terminal.pb.cc \
        ../Global/Protocol/User/User.pb.cc \
        ../Global/Protocol/common/Echo.pb.cc \
        Application.cpp \
        ApplicationServer.cpp \
        BlackList.cpp \
        FilesSharingRoomManager.cpp \
        ManagementServer.cpp \
        NetworkDataHandler/Common/RequestEchoHandler.cpp \
        NetworkDataHandler/Factory.cpp \
        NetworkDataHandler/File/CloseFileHandler.cpp \
        NetworkDataHandler/File/RequestDeleteEntryHandler.cpp \
        NetworkDataHandler/File/RequestGoHomeHandler.cpp \
        NetworkDataHandler/File/RequestGoUpHandler.cpp \
        NetworkDataHandler/File/RequestListEntriesHandler.cpp \
        NetworkDataHandler/File/RequestMakeDirectoryHandler.cpp \
        NetworkDataHandler/File/RequestOpenFileHandler.cpp \
        NetworkDataHandler/File/RequestReadFileBlockHandler.cpp \
        NetworkDataHandler/File/RequestRenameEntryHandler.cpp \
        NetworkDataHandler/File/RequestVisitFilesHandler.cpp \
        NetworkDataHandler/File/RequestWriteFileBlockHandler.cpp \
        NetworkDataHandler/File/ResponseDeleteEntryHandler.cpp \
        NetworkDataHandler/File/ResponseGoHomeHandler.cpp \
        NetworkDataHandler/File/ResponseGoUpHandler.cpp \
        NetworkDataHandler/File/ResponseListEntriesHandler.cpp \
        NetworkDataHandler/File/ResponseMakeDirectoryHandler.cpp \
        NetworkDataHandler/File/ResponseOpenFileHandler.cpp \
        NetworkDataHandler/File/ResponseReadFileBlockHandler.cpp \
        NetworkDataHandler/File/ResponseRenameEntryHandler.cpp \
        NetworkDataHandler/File/ResponseVisitFiles2Handler.cpp \
        NetworkDataHandler/File/ResponseWriteFileBlockHandler.cpp \
        NetworkDataHandler/Management/RequestAddAccountHandler.cpp \
        NetworkDataHandler/Management/RequestAddBlockedAccountHandler.cpp \
        NetworkDataHandler/Management/RequestAddBlockedIPAddressHandler.cpp \
        NetworkDataHandler/Management/RequestChangeAccountPasswordHandler.cpp \
        NetworkDataHandler/Management/RequestDeleteAccountHandler.cpp \
        NetworkDataHandler/Management/RequestDisableAccountHandler.cpp \
        NetworkDataHandler/Management/RequestEnableAccountHandler.cpp \
        NetworkDataHandler/Management/RequestListAccountHandler.cpp \
        NetworkDataHandler/Management/RequestListBlockedAccountsHandler.cpp \
        NetworkDataHandler/Management/RequestListBlockedIPAddressesHandler.cpp \
        NetworkDataHandler/Management/RequestRemoveBlockedAccountHandler.cpp \
        NetworkDataHandler/Management/RequestRemoveBlockedIPAddressHandler.cpp \
        NetworkDataHandler/Screen/PublishCursorPositionHandler.cpp \
        NetworkDataHandler/Screen/PublishScreenFrameHandler.cpp \
        NetworkDataHandler/Screen/RequestSubscribeScreenHandler.cpp \
        NetworkDataHandler/Screen/ResponseSubscribeScreen2Handler.cpp \
        NetworkDataHandler/Screen/ScreenVisitorTransferingOnlyHandler.cpp \
        NetworkDataHandler/Screen/SetClipboardTextHandler.cpp \
        NetworkDataHandler/Terminal/KickoutTerminalVisitorHandler.cpp \
        NetworkDataHandler/Terminal/PublishTerminalOutputHandler.cpp \
        NetworkDataHandler/Terminal/RequestCreateTerminalHandler.cpp \
        NetworkDataHandler/Terminal/ResizeTerminalHandler.cpp \
        NetworkDataHandler/Terminal/ResponseCreateTerminal2Handler.cpp \
        NetworkDataHandler/Terminal/WriteCommandHandler.cpp \
        NetworkDataHandler/User/RequestLoginHandler.cpp \
        RequestProcessor.cpp \
        ScreenSharingRoomManager.cpp \
        ServerBase.cpp \
        SharingRoom.cpp \
        Task/File/VisitFilesTask.cpp \
        Task/ITask.cpp \
        Task/Screen/SubscribeScreenTask.cpp \
        Task/TaskManager.cpp \
        Task/Terminal/CreateTerminalTask.cpp \
        TerminalSharingRoom.cpp \
        TerminalSharingRoomManager.cpp \
        AccountManager.cpp \
        UserConnection.cpp \
        main.cpp

HEADERS += \
    ../Global/Component/Cryptology/Cryptology.hpp \
    ../Global/Component/EasyIO/EasyIO.h \
    ../Global/Component/EasyIO/EasyIOByteBuffer.h \
    ../Global/Component/EasyIO/EasyIOContext_linux.h \
    ../Global/Component/EasyIO/EasyIOContext_win.h \
    ../Global/Component/EasyIO/EasyIODef.h \
    ../Global/Component/EasyIO/EasyIOError.h \
    ../Global/Component/EasyIO/EasyIOEventLoopGroup.h \
    ../Global/Component/EasyIO/EasyIOEventLoop_linux.h \
    ../Global/Component/EasyIO/EasyIOEventLoop_win.h \
    ../Global/Component/EasyIO/EasyIOIEventLoop.h \
    ../Global/Component/EasyIO/EasyIOTCPAcceptor.h \
    ../Global/Component/EasyIO/EasyIOTCPClient_linux.h \
    ../Global/Component/EasyIO/EasyIOTCPClient_win.h \
    ../Global/Component/EasyIO/EasyIOTCPConnection_linux.h \
    ../Global/Component/EasyIO/EasyIOTCPConnection_win.h \
    ../Global/Component/EasyIO/EasyIOTCPIClient.h \
    ../Global/Component/EasyIO/EasyIOTCPIConnection.h \
    ../Global/Component/EasyIO/EasyIOTCPIServer.h \
    ../Global/Component/EasyIO/EasyIOTCPServer_linux.h \
    ../Global/Component/EasyIO/EasyIOTCPServer_win.h \
    ../Global/Component/EasyIO/EasyIOUDPClient.h \
    ../Global/Component/EasyIO/EasyIOUDPIClient.h \
    ../Global/Component/EasyIO/EasyIOUDPIServer.h \
    ../Global/Component/EasyIO/EasyIOUDPISocket.h \
    ../Global/Component/EasyIO/EasyIOUDPServer.h \
    ../Global/Component/EasyIO/EasyIOUDPSocket_linux.h \
    ../Global/Component/EasyIO/EasyIOUDPSocket_win.h \
    ../Global/Component/Logger/ConsoleLogger.h \
    ../Global/Component/Logger/FileLogger.h \
    ../Global/Component/Logger/ILogger.h \
    ../Global/Component/Logger/Logger.h \
    ../Global/Component/ReadWriteLock/ReadWriteLock.h \
    ../Global/Define.h \
    ../Global/Protocol/Error.h \
    ../Global/Protocol/File/File.pb.h \
    ../Global/Protocol/Management/Server/Management.pb.h \
    ../Global/Protocol/Protocol.h \
    ../Global/Protocol/Screen/Control.pb.h \
    ../Global/Protocol/Screen/Screen.pb.h \
    ../Global/Protocol/Terminal/Terminal.pb.h \
    ../Global/Protocol/User/User.pb.h \
    ../Global/Protocol/common/Echo.pb.h \
    Application.h \
    ApplicationServer.h \
    BlackList.h \
    FilesSharingRoomManager.h \
    IConnection.h \
    ManagementServer.h \
    NetworkDataHandler/Common/RequestEchoHandler.h \
    NetworkDataHandler/Factory.h \
    NetworkDataHandler/File/CloseFileHandler.h \
    NetworkDataHandler/File/RequestDeleteEntryHandler.h \
    NetworkDataHandler/File/RequestGoHomeHandler.h \
    NetworkDataHandler/File/RequestGoUpHandler.h \
    NetworkDataHandler/File/RequestListEntriesHandler.h \
    NetworkDataHandler/File/RequestMakeDirectoryHandler.h \
    NetworkDataHandler/File/RequestOpenFileHandler.h \
    NetworkDataHandler/File/RequestReadFileBlockHandler.h \
    NetworkDataHandler/File/RequestRenameEntryHandler.h \
    NetworkDataHandler/File/RequestVisitFilesHandler.h \
    NetworkDataHandler/File/RequestWriteFileBlockHandler.h \
    NetworkDataHandler/File/ResponseDeleteEntryHandler.h \
    NetworkDataHandler/File/ResponseGoHomeHandler.h \
    NetworkDataHandler/File/ResponseGoUpHandler.h \
    NetworkDataHandler/File/ResponseListEntriesHandler.h \
    NetworkDataHandler/File/ResponseMakeDirectoryHandler.h \
    NetworkDataHandler/File/ResponseOpenFileHandler.h \
    NetworkDataHandler/File/ResponseReadFileBlockHandler.h \
    NetworkDataHandler/File/ResponseRenameEntryHandler.h \
    NetworkDataHandler/File/ResponseVisitFiles2Handler.h \
    NetworkDataHandler/File/ResponseWriteFileBlockHandler.h \
    NetworkDataHandler/File/UsualRequestHandler.h \
    NetworkDataHandler/File/UsualResponseHandler.h \
    NetworkDataHandler/INetworkDataHandler.h \
    NetworkDataHandler/Management/RequestAddAccountHandler.h \
    NetworkDataHandler/Management/RequestAddBlockedAccountHandler.h \
    NetworkDataHandler/Management/RequestAddBlockedIPAddressHandler.h \
    NetworkDataHandler/Management/RequestChangeAccountPasswordHandler.h \
    NetworkDataHandler/Management/RequestDeleteAccountHandler.h \
    NetworkDataHandler/Management/RequestDisableAccountHandler.h \
    NetworkDataHandler/Management/RequestEnableAccountHandler.h \
    NetworkDataHandler/Management/RequestListAccountHandler.h \
    NetworkDataHandler/Management/RequestListBlockedAccountsHandler.h \
    NetworkDataHandler/Management/RequestListBlockedIPAddressesHandler.h \
    NetworkDataHandler/Management/RequestRemoveBlockedAccountHandler.h \
    NetworkDataHandler/Management/RequestRemoveBlockedIPAddressHandler.h \
    NetworkDataHandler/Screen/PublishCursorPositionHandler.h \
    NetworkDataHandler/Screen/PublishScreenFrameHandler.h \
    NetworkDataHandler/Screen/RequestSubscribeScreenHandler.h \
    NetworkDataHandler/Screen/ResponseSubscribeScreen2Handler.h \
    NetworkDataHandler/Screen/ScreenVisitorTransferingOnlyHandler.h \
    NetworkDataHandler/Screen/SetClipboardTextHandler.h \
    NetworkDataHandler/Terminal/KickoutTerminalVisitorHandler.h \
    NetworkDataHandler/Terminal/PublishTerminalOutputHandler.h \
    NetworkDataHandler/Terminal/RequestCreateTerminalHandler.h \
    NetworkDataHandler/Terminal/ResizeTerminalHandler.h \
    NetworkDataHandler/Terminal/ResponseCreateTerminal2Handler.h \
    NetworkDataHandler/Terminal/WriteCommandHandler.h \
    NetworkDataHandler/User/RequestLoginHandler.h \
    RequestProcessor.h \
    ScreenSharingRoomManager.h \
    ServerBase.h \
    SharingRoom.h \
    SharingRoomManager.h \
    Task/File/VisitFilesTask.h \
    Task/ITask.h \
    Task/Screen/SubscribeScreenTask.h \
    Task/TaskManager.h \
    Task/Terminal/CreateTerminalTask.h \
    TerminalSharingRoom.h \
    TerminalSharingRoomManager.h \
    AccountManager.h \
    UserConnection.h \
    Account.h \
    UsualParseProtobufDataMacro.h \
    Version.h

INCLUDEPATH += ../
INCLUDEPATH += ../Global/Component/protobuf/include
INCLUDEPATH += ../Global/Component/zlib/include
INCLUDEPATH += ../Global/Component/openssl/include

windows:{
    msvc {
       QMAKE_CFLAGS += /utf-8
       QMAKE_CXXFLAGS += /utf-8
    }

    DEFINES += WIN32_LEAN_AND_MEAN
    DEFINES += PROTOBUF_USE_DLLS
    LIBS += -lWs2_32

    CONFIG(debug, debug|release){
        QMAKE_CFLAGS_RELEASE += -g
        QMAKE_CXXFLAGS_RELEASE += -g
        QMAKE_CXXFLAGS_RELEASE -= -O2
        QMAKE_CFLAGS_RELEASE -= -O2

        # protobuf
        LIBS += $$PWD/../Global/Component/protobuf/lib/win/debug/libprotobufd.lib
        LIBS += $$PWD/../Global/Component/protobuf/lib/win/debug/libprotobuf-lited.lib
        LIBS += $$PWD/../Global/Component/protobuf/lib/win/debug/libprotocd.lib
    } else {
        # protobuf
        LIBS += $$PWD/../Global/Component/protobuf/lib/win/release/libprotobuf.lib
        LIBS += $$PWD/../Global/Component/protobuf/lib/win/release/libprotobuf-lite.lib
        LIBS += $$PWD/../Global/Component/protobuf/lib/win/release/libprotoc.lib
    }


    # zlib
    LIBS += $$PWD/../Global/Component/zlib/lib/win/zlibwapi.lib

    # openssl
    LIBS += $$PWD/../Global/Component/openssl/lib/win/libcrypto.lib
    LIBS += $$PWD/../Global/Component/openssl/lib/win/libssl.lib
}


linux:{
    QMAKE_LFLAGS += -Wl,-rpath=\$$ORIGIN

    # protobuf
    LIBS += $$PWD/../Global/Component/protobuf/lib/linux/libprotobuf.a
    LIBS += $$PWD/../Global/Component/protobuf/lib/linux/libprotobuf-lite.a
    LIBS += $$PWD/../Global/Component/protobuf/lib/linux/libprotoc.a

    # zlib
    LIBS += $$PWD/../Global/Component/zlib/lib/linux/libz.a

    # openssl
    LIBS += $$PWD/../Global/Component/openssl/lib/linux/libcrypto.a
    LIBS += $$PWD/../Global/Component/openssl/lib/linux/libssl.a

    LIBS += -lutil -ldl

}

DISTFILES += \
    config.ini
