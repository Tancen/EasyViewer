#ifndef USUALPARSEPROTOBUFDATAMACRO_H
#define USUALPARSEPROTOBUFDATAMACRO_H

#define USUAL_PARSE_PROTOBUF_DATA_MACRO(__con, __protoVarType, __protoVarName, __data, __len, __pDecryptor) \
    auto __result = __pDecryptor->decrypt((const char*)__data, __len); \
    if (!__result.isOk) \
    { \
        Logger::warning("%s:%d - !decrypt[%s], remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__, \
                        __result.errorString.c_str(), __con->peerIP().c_str(), __con->peerPort()); \
        __con->disconnect(); \
        return; \
    } \
    \
    __protoVarType __protoVarName; \
    if(!__protoVarName.ParseFromArray(__result.data.readableBytes(), __result.data.numReadableBytes())) \
    { \
        Logger::warning("%s:%d - !ParseFromArray, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__, \
                        __con->peerIP().c_str(), __con->peerPort()); \
        __con->disconnect(); \
        return; \
    }


#endif // USUALPARSEPROTOBUFDATAMACRO_H
