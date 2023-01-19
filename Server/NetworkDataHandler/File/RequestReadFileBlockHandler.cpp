#include "RequestReadFileBlockHandler.h"

NetworkDataHandler::RequestReadFileBlock::RequestReadFileBlock()
    :   UsualRequest(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_READ_FILE_BLOCK,
                     GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_READ_FILE_BLOCK, false)
{

}
