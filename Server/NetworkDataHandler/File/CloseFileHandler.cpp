#include "CloseFileHandler.h"

NetworkDataHandler::CloseFile::CloseFile()
    :   UsualResponseHandler(GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_FILE, false)
{

}
