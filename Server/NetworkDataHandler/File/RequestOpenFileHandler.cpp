#include "RequestOpenFileHandler.h"

NetworkDataHandler::RequestOpenFile::RequestOpenFile()
    :   UsualRequest(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_OPEN_FILE,
                     GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_OPEN_FILE)
{

}

