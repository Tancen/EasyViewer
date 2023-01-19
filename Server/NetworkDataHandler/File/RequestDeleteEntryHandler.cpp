#include "RequestDeleteEntryHandler.h"

NetworkDataHandler::RequestDeleteEntry::RequestDeleteEntry()
    :   UsualRequest(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DELETE_ENTRY,
                     GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_DELETE_ENTRY)
{

}
