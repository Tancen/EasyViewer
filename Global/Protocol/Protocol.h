#ifndef GLOBAL_PROTOCOL_H
#define GLOBAL_PROTOCOL_H

#define GLOBAL_PROTOCOL_REMOTE_CONTROL_MODE_SCREEN      1
#define GLOBAL_PROTOCOL_REMOTE_CONTROL_MODE_TERMINAL    2
#define GLOBAL_PROTOCOL_REMOTE_CONTROL_MODE_FILE        3

#define GLOBAL_PROTOCOL_DATETIME_FORMAT     "yyyy-MM-dd HH:mm:ss"

#define GLOBAL_PROTOCOL_MAX_TRANSMITS_FILE_BLOCK_SIZE     (64*1024)

#define GLOBAL_PROTOCOL_FILE_TRANSMISSION_TASK_CLOSE_REASON_FINISHED            1
#define GLOBAL_PROTOCOL_FILE_TRANSMISSION_TASK_CLOSE_REASON_ERROR_OCCURRED      2
#define GLOBAL_PROTOCOL_FILE_TRANSMISSION_TASK_CLOSE_REASON_REMOVED             3
#define GLOBAL_PROTOCOL_FILE_TRANSMISSION_TASK_CLOSE_REASON_CANCELLED           4
#define GLOBAL_PROTOCOL_FILE_TRANSMISSION_TASK_CLOSE_REASON_STOPED              5

//------------ network tags ------------------
//common
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ECHO       1
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ECHO      2

// user
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LOGIN       101
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LOGIN      102

// screen
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_SUBSCRIBE_SCREEN        201
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_SUBSCRIBE_SCREEN       202
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_SUBSCRIBE_SCREEN2       203
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_SUBSCRIBE_SCREEN2      204
#define GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_SCREEN_FRAME            205
#define GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_CURSOR_POSITION         206
#define GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_KEY_PRESS               207
#define GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_KEY_RELEASE             208
#define GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_MOVE              209
#define GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_PRESS      210
#define GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_RELEASE        211
#define GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_DOUBLE_CLICK   212
#define GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_WHEEL                 213
#define GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_CLIPBOARD_TEXT          214
#define GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_AUTO_PUBLISH_DESKTOP    215
#define GLOBAL_PROTOCOL_NETWORK_TAG_SHORTCUT                            216

// file
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISIT_FILES         301
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISIT_FILES        302
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISIT_FILES2        303
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISIT_FILES2       304
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_MAKE_DIRECTORY                      305
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_MAKE_DIRECTORY                     306
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DELETE_ENTRY                        307
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_DELETE_ENTRY                       308
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_RENAME_ENTRY                        309
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_RENAME_ENTRY                       310
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISITING_PATH_GO_UP                 311
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISITING_PATH_GO_UP                312
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISITING_PATH_GO_HOME               313
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISITING_PATH_GO_HOME              314
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_ENTRIES                        315
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_ENTRIES                       316
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_OPEN_FILE                           317
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_OPEN_FILE                          318
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_WRITE_FILE_BLOCK                    319
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_WRITE_FILE_BLOCK                   320
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_READ_FILE_BLOCK                     321
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_READ_FILE_BLOCK                    322
#define GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_FILE                                  323

// terminal
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_CREATE_TERMINAL                     401
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_CREATE_TERMINAL                    402
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_CREATE_TERMINAL2                    403
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_CREATE_TERMINAL2                   404
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESIZE_TERMINAL                             405
#define GLOBAL_PROTOCOL_NETWORK_TAG_WRITE_COMMAND_TO_TERMINAL                   406
#define GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_TERMINAL_OUTPUT                     407
#define GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_TERMINAL                              408
#define GLOBAL_PROTOCOL_NETWORK_TAG_KICKOUT_VISITOR_TERMINAL                    409

// server ctrl
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_ACCOUNT                        501
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_ACCOUNT                       502
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ADD_ACCOUNT                         503
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ADD_ACCOUNT                        504
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ENABLE_ACCOUNT                      505
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ENABLE_ACCOUNT                     506
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DISABLE_ACCOUNT                     507
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_DISABLE_ACCOUNT                    508
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DELETE_ACCOUNT                      509
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_DELETE_ACCOUNT                     510
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_CHANGE_ACCOUNT_PASSWORD             511
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_CHANGE_ACCOUNT_PASSWORD            512
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_BLOCKED_IP_ADDRESSES           513
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_BLOCKED_IP_ADDRESSES          514
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_REMOVE_BLOCKED_IP_ADDRESS           515
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_REMOVE_BLOCKED_IP_ADDRESS          516
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ADD_BLOCKED_IP_ADDRESS              517
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ADD_BLOCKED_IP_ADDRESS             518
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_BLOCKED_ACCOUNTS               519
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_BLOCKED_ACCOUNTS              520
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_REMOVE_BLOCKED_ACCOUNT              521
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_REMOVE_BLOCKED_ACCOUNT             522
#define GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ADD_BLOCKED_ACCOUNT                 523
#define GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ADD_BLOCKED_ACCOUNT                524

#define GLOBAL_PROTO_WRAPPER_DESC(_type, _protobuf, _dst) \
{   \
    _dst.write((uint32_t)_type);\
    _dst.write((uint32_t)_protobuf.ByteSizeLong()); \
    _dst.fill(0, _protobuf.ByteSizeLong());   \
    _protobuf.SerializeToArray(_dst.data() + 8, _protobuf.ByteSizeLong());   \
}

#define GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(_type, _protobuf, _dst, __pENCRYPTOR) \
{   \
    decltype(_dst) __tmp; \
    __tmp.fill(0, _protobuf.ByteSizeLong());   \
    _protobuf.SerializeToArray(__tmp.readableBytes(), _protobuf.ByteSizeLong());   \
    _dst.write((uint32_t)_type);  \
    _dst.fill(0, 4); \
    auto __result = __pENCRYPTOR->encrypt(__tmp.readableBytes(), __tmp.numReadableBytes(), _dst); \
    assert(__result.isOk); \
    *((uint32_t*)(__result.data.readableBytes() + 4)) = __result.data.numReadableBytes() - 8;  \
}

#define GLOBAL_PROTO_WRAPPER_DESC2(_type, _pd, _l, _dst) \
{   \
    _dst.write((uint32_t)_type);\
    _dst.write((uint32_t)_l); \
    _dst.write(_pd, _l); \
}


#endif // GLOBAL_PROTOCOL_H
