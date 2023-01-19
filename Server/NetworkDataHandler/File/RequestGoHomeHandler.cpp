#include "RequestGoHomeHandler.h"

NetworkDataHandler::RequestGoHome::RequestGoHome()
    :   UsualRequest(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISITING_PATH_GO_HOME,
                     GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISITING_PATH_GO_HOME)
{

}
