#include "Factory.h"
#include "Global/Define.h"
#include "Global/Protocol/Protocol.h"
#include "NetworkDataHandler/Common/RequestEchoHandler.h"
#include "NetworkDataHandler/User/RequestLoginHandler.h"
#include "NetworkDataHandler/Screen/PublishScreenFrameHandler.h"
#include "NetworkDataHandler/Screen/PublishCursorPositionHandler.h"
#include "NetworkDataHandler/Screen/RequestSubscribeScreenHandler.h"
#include "NetworkDataHandler/Screen/ResponseSubscribeScreen2Handler.h"
#include "NetworkDataHandler/Screen/ScreenVisitorTransferingOnlyHandler.h"
#include "NetworkDataHandler/Screen/SetClipboardTextHandler.h"
#include "NetworkDataHandler/File/RequestDeleteEntryHandler.h"
#include "NetworkDataHandler/File/RequestGoHomeHandler.h"
#include "NetworkDataHandler/File/RequestGoUpHandler.h"
#include "NetworkDataHandler/File/RequestListEntriesHandler.h"
#include "NetworkDataHandler/File/RequestMakeDirectoryHandler.h"
#include "NetworkDataHandler/File/RequestRenameEntryHandler.h"
#include "NetworkDataHandler/File/RequestVisitFilesHandler.h"
#include "NetworkDataHandler/File/ResponseDeleteEntryHandler.h"
#include "NetworkDataHandler/File/ResponseGoHomeHandler.h"
#include "NetworkDataHandler/File/ResponseGoUpHandler.h"
#include "NetworkDataHandler/File/ResponseListEntriesHandler.h"
#include "NetworkDataHandler/File/ResponseMakeDirectoryHandler.h"
#include "NetworkDataHandler/File/ResponseRenameEntryHandler.h"
#include "NetworkDataHandler/File/ResponseVisitFiles2Handler.h"
#include "NetworkDataHandler/File/CloseFileHandler.h"
#include "NetworkDataHandler/File/RequestOpenFileHandler.h"
#include "NetworkDataHandler/File/ResponseOpenFileHandler.h"
#include "NetworkDataHandler/File/RequestReadFileBlockHandler.h"
#include "NetworkDataHandler/File/ResponseReadFileBlockHandler.h"
#include "NetworkDataHandler/File/RequestWriteFileBlockHandler.h"
#include "NetworkDataHandler/File/ResponseWriteFileBlockHandler.h"
#include "NetworkDataHandler/File/CloseFileHandler.h"
#include "NetworkDataHandler/Terminal/PublishTerminalOutputHandler.h"
#include "NetworkDataHandler/Terminal/RequestCreateTerminalHandler.h"
#include "NetworkDataHandler/Terminal/ResponseCreateTerminal2Handler.h"
#include "NetworkDataHandler/Terminal/WriteCommandHandler.h"
#include "NetworkDataHandler/Terminal/ResizeTerminalHandler.h"
#include "NetworkDataHandler/Terminal/KickoutTerminalVisitorHandler.h"

namespace NetworkDataHandler
{
    INetworkDataHandlerPtr gRequestEchoHandler(new RequestEcho());
    INetworkDataHandlerPtr gRequestLoginHandler(new RequestLogin());
    INetworkDataHandlerPtr gPublishScreenFrameHandler(new PublishScreenFrame());
    INetworkDataHandlerPtr gPublishCursorPositionHandler(new PublishCursorPosition());
    INetworkDataHandlerPtr gRequestSubscribeScreenHandler(new RequestSubscribeScreen());
    INetworkDataHandlerPtr gResponseSubscribeScreen2Handler(new ResponseSubscribeScreen2());
    INetworkDataHandlerPtr gControlMouseMoveHandler(new ScreenVisitorTransferingOnly(
                                GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_MOVE));
    INetworkDataHandlerPtr gControlKeyPressHandler(new ScreenVisitorTransferingOnly(
                                GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_KEY_PRESS));
    INetworkDataHandlerPtr gControlKeyReleaseHandler(new ScreenVisitorTransferingOnly(
                                GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_KEY_RELEASE));
    INetworkDataHandlerPtr gControlMouseButtonDoubleClickHandler(new ScreenVisitorTransferingOnly(
                                GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_DOUBLE_CLICK));
    INetworkDataHandlerPtr gControlMouseButtonPressHandler(new ScreenVisitorTransferingOnly(
                                GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_PRESS));
    INetworkDataHandlerPtr gControlMouseButtonReleaseHandler(new ScreenVisitorTransferingOnly(
                                GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_RELEASE));
    INetworkDataHandlerPtr gControlMouseWheelHandler(new ScreenVisitorTransferingOnly(
                                GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_WHEEL));
    INetworkDataHandlerPtr gSetClipboardTextHandler(new SetClipboardText());
    INetworkDataHandlerPtr gShortcutHandler(new ScreenVisitorTransferingOnly(
                                GLOBAL_PROTOCOL_NETWORK_TAG_SHORTCUT));

    INetworkDataHandlerPtr gRequestDeleteEntryHandler(new RequestDeleteEntry());
    INetworkDataHandlerPtr gRequestGoHomeHandler(new RequestGoHome());
    INetworkDataHandlerPtr gRequestGoUpHandler(new RequestGoUp());
    INetworkDataHandlerPtr gRequestListEntriesHandler(new RequestListEntries());
    INetworkDataHandlerPtr gRequestMakeDirectoryHandler(new RequestMakeDirectory());
    INetworkDataHandlerPtr gRequestRenameEntryHandler(new RequestRenameEntry());
    INetworkDataHandlerPtr gRequestVisitFilesHandler(new RequestVisitFiles());
    INetworkDataHandlerPtr gResponseDeleteEntryHandler(new ResponseDeleteEntry());
    INetworkDataHandlerPtr gResponseGoHomeHandler(new ResponseGoHome());
    INetworkDataHandlerPtr gResponseGoUpHandler(new ResponseGoUp());
    INetworkDataHandlerPtr gResponseListEntriesHandler(new ResponseListEntries());
    INetworkDataHandlerPtr gResponseMakeDirectoryHandler(new ResponseMakeDirectory());
    INetworkDataHandlerPtr gResponseRenameEntryHandler(new ResponseRenameEntry());
    INetworkDataHandlerPtr gResponseVisitFiles2Handler(new ResponseVisitFiles2());

    INetworkDataHandlerPtr gCloseFileHandler(new CloseFile());
    INetworkDataHandlerPtr gRequestOpenFileHandler(new RequestOpenFile());
    INetworkDataHandlerPtr gResponseOpenFileHandler(new ResponseOpenFileTask());
    INetworkDataHandlerPtr gRequestReadFileBlockHandler(new RequestReadFileBlock());
    INetworkDataHandlerPtr gResponseReadFileBlockHandler(new ResponseReadFileBlock());
    INetworkDataHandlerPtr gRequestWriteFileBlockHandler(new RequestWriteFileBlock());
    INetworkDataHandlerPtr gResponseWriteFileBlockHandler(new ResponseWriteFileBlock());

    INetworkDataHandlerPtr gPublishTerminalOutputHandler(new PublishTerminalOutput());
    INetworkDataHandlerPtr gRequestCreateTerminalHandler(new RequestCreateTerminal());
    INetworkDataHandlerPtr gResponseCreateTerminal2Handler(new ResponseCreateTerminal2());
    INetworkDataHandlerPtr gWriteCommandHandler(new WriteCommand());
    INetworkDataHandlerPtr gResizeTerminalHandler(new ResizeTerminal());
    INetworkDataHandlerPtr gKickoutTerminalVisitorHandler(new KickoutTerminalVisitor());
}

NetworkDataHandler::INetworkDataHandlerPtr NetworkDataHandler::Factory::create(int tag)
{
    INetworkDataHandlerPtr ret;

    switch (tag)
    {
    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ECHO:
        ret = gRequestEchoHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LOGIN:
        ret = gRequestLoginHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_SCREEN_FRAME:
        ret = gPublishScreenFrameHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_CURSOR_POSITION:
        ret = gPublishCursorPositionHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_SUBSCRIBE_SCREEN:
        ret = gRequestSubscribeScreenHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_SUBSCRIBE_SCREEN2:
        ret = gResponseSubscribeScreen2Handler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_MOVE:
        ret = gControlMouseMoveHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_KEY_PRESS:
        ret = gControlKeyPressHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_KEY_RELEASE:
        ret = gControlKeyReleaseHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_PRESS:
        ret = gControlMouseButtonPressHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_RELEASE:
        ret = gControlMouseButtonReleaseHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_DOUBLE_CLICK:
        ret = gControlMouseButtonDoubleClickHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_WHEEL:
        ret = gControlMouseWheelHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_CLIPBOARD_TEXT:
        ret = gSetClipboardTextHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_SHORTCUT:
        ret = gShortcutHandler;
        break;


    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISIT_FILES :
        ret = gRequestVisitFilesHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISIT_FILES2 :
        ret = gResponseVisitFiles2Handler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_OPEN_FILE :
        ret = gRequestOpenFileHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_OPEN_FILE :
        ret = gResponseOpenFileHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_READ_FILE_BLOCK :
        ret = gRequestReadFileBlockHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_READ_FILE_BLOCK :
        ret = gResponseReadFileBlockHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_WRITE_FILE_BLOCK :
        ret = gRequestWriteFileBlockHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_WRITE_FILE_BLOCK :
        ret = gResponseWriteFileBlockHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_FILE:
        ret = gCloseFileHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_MAKE_DIRECTORY :
        ret = gRequestMakeDirectoryHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_MAKE_DIRECTORY :
        ret = gResponseMakeDirectoryHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DELETE_ENTRY :
        ret = gRequestDeleteEntryHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_DELETE_ENTRY :
        ret = gResponseDeleteEntryHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_RENAME_ENTRY :
        ret = gRequestRenameEntryHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_RENAME_ENTRY :
        ret = gResponseRenameEntryHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISITING_PATH_GO_UP :
        ret = gRequestGoUpHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISITING_PATH_GO_UP :
        ret = gResponseGoUpHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISITING_PATH_GO_HOME :
        ret = gRequestGoHomeHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISITING_PATH_GO_HOME :
        ret = gResponseGoHomeHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_ENTRIES :
        ret = gRequestListEntriesHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_ENTRIES :
        ret = gResponseListEntriesHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_CREATE_TERMINAL :
        ret = gRequestCreateTerminalHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_TERMINAL_OUTPUT :
        ret = gPublishTerminalOutputHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_CREATE_TERMINAL2 :
        ret = gResponseCreateTerminal2Handler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_WRITE_COMMAND_TO_TERMINAL :
        ret = gWriteCommandHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESIZE_TERMINAL :
        ret = gResizeTerminalHandler;
        break;

    case GLOBAL_PROTOCOL_NETWORK_TAG_KICKOUT_VISITOR_TERMINAL :
        ret = gKickoutTerminalVisitorHandler;
        break;


    }

    return ret;
}
