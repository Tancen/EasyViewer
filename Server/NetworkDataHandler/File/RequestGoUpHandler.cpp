#include "RequestGoUpHandler.h"

NetworkDataHandler::RequestGoUp::RequestGoUp()
    :   UsualRequest(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISITING_PATH_GO_UP,
                     GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISITING_PATH_GO_UP)
{

}
