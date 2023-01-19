#include "RequestWriteFileBlockHandler.h"

NetworkDataHandler::RequestWriteFileBlock::RequestWriteFileBlock()
    :   UsualRequest(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_WRITE_FILE_BLOCK,
                     GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_WRITE_FILE_BLOCK, false)
{

}
