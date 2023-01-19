#ifndef USUALPARSEPROTOBUFDATAMACRO_H
#define USUALPARSEPROTOBUFDATAMACRO_H

#include "BlackList.h"

#define USUAL_PARSE_PROTOBUF_DATA_MACRO(__protoVarType, __protoVarName, __data, __len, __pDecryptor) \
    auto __result = __pDecryptor->decrypt((const char*)__data, __len); \
    if (!__result.isOk) \
    { \
        Logger::warning("%s:%d - !decrypt, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__, \
                        con->peerIP().c_str(), con->peerPort()); \
        BlackList::share()->increaseIPAddress(con->peerIP()); \
        con->disconnect(); \
        return; \
    } \
    \
    __protoVarType __protoVarName; \
    if(!__protoVarName.ParseFromArray(__result.data.readableBytes(), __result.data.numReadableBytes())) \
    { \
        Logger::warning("%s:%d - !parseFromArray, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__, \
                        con->peerIP().c_str(), con->peerPort()); \
        BlackList::share()->increaseIPAddress(con->peerIP()); \
        con->disconnect(); \
        return; \
    }



#endif // USUALPARSEPROTOBUFDATAMACRO_H
