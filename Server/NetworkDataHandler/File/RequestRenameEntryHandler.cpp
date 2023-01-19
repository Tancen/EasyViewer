#include "RequestRenameEntryHandler.h"

NetworkDataHandler::RequestRenameEntry::RequestRenameEntry()
    :   UsualRequest(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_RENAME_ENTRY,
                     GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_RENAME_ENTRY)
{

}
