#include "RequestListEntriesHandler.h"

NetworkDataHandler::RequestListEntries::RequestListEntries()
    :   UsualRequest(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_ENTRIES,
                     GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_ENTRIES)
{

}
