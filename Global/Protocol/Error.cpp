#include "Error.h"

const char *Global::Protocol::formatError(int err)
{
    switch (err)
    {
    case GLOBAL_PROTOCOL_ERR_NO_ERROR :
        return "";

    case GLOBAL_PROTOCOL_ERR_INVALID_PARAMS :
        return "invalid params";

    case GLOBAL_PROTOCOL_ERR_ACCOUNT_NOT_EXISTS :
        return "account not exists";

    case GLOBAL_PROTOCOL_ERR_PASSWORD_INCORRECT :
        return "password incorrect";

    case GLOBAL_PROTOCOL_ERR_REPEAT_LOGIN :
        return "repeat login";

    case GLOBAL_PROTOCOL_ERR_CONNECT_FAILED :
        return "connect failed";

    case GLOBAL_PROTOCOL_ERR_DISCONNECTED :
        return "disconnected";

    case GLOBAL_PROTOCOL_ERR_UNSUPPORTED :
        return "unsupported";

    case GLOBAL_PROTOCOL_ERR_TARGET_UNREACHABLE:
        return "target unreachable";

    case GLOBAL_PROTOCOL_ERR_TIMEOUT:
        return "timeout";

    case GLOBAL_PROTOCOL_ERR_PATH_UNREACHABLE:
        return "path unreachable";

    case GLOBAL_PROTOCOL_ERR_OPERATE_FAILED:
        return "operate failed";

    case GLOBAL_PROTOCOL_ERR_UNKNOW :
        return "unknow";

    case GLOBAL_PROTOCOL_ERR_ILLEGAL_DATA:
        return "illegal data";

    case GLOBAL_PROTOCOL_ERR_ACCOUNT_LOCKED:
        return "account locked";

    case GLOBAL_PROTOCOL_ERR_ACCOUNT_DISABLED:
        return "account disabled";

    case GLOBAL_PROTOCOL_ERR_IP_ADDRESS_BLOCKED:
        return "ip address blocked";

    case GLOBAL_PROTOCOL_ERR_ACCOUNT_BLOCKED:
        return "account blocked";

    case GLOBAL_PROTOCOL_ERR_FILE_ALREADY_EXISTS:
        return "file already exists";

    case GLOBAL_PROTOCOL_ERR_FILE_NOT_EXISTS:
        return "file not exists";

    case GLOBAL_PROTOCOL_ERR_END_OF_FILE:
        return "end of file";

    case GLOBAL_PROTOCOL_ERR_FILE_CLOSED:
        return "file closed";
    }

    return "";
}
