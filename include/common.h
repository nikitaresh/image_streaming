#ifndef COMMON_H
#define COMMON_H


/**
 * @brief The MessageType enum describes type of messages for communication
 * between the server and clients, each message contains MessageType
 */
enum MessageType { MT_AUTHORIZATION, MT_MANAGING, MT_IMAGE };   // serialization type is qint32


/**
 * @brief The ClientType enum describes type of client, each client has one of these type.
 * Initially, a client has CT_UNKNOWN type, than it change to another after successful authorization
 */
enum ClientType { CT_UNKNOWN, CT_CAMERA, CT_VIEWER };   // serialization type is qint32


/**
 * @brief The CommandControlType enum describes type of command for a message with MT_MANAGING type
 */
enum CommandControlType { CCT_GET_LIST_OF_CAMS,     // serialization type is qint32
                          CCT_START_CAM,
                          CCT_STOP_CAM,
                          CCT_START_STREAM,
                          CCT_STOP_STREAM };

#endif // COMMON_H
