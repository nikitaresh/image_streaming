#ifndef CLIENT_HENDLER_H
#define CLIENT_HENDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <common.h>
#include <server_storage.h>


/**
 * @brief Interaction object with a client via TCP
 */
class ISClientHendler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief The only constructor of the image streaming client
     * @param socketDescriptor
     * @param serverStorage
     * @param parent
     */
    explicit ISClientHendler( qintptr socketDescriptor, ServerStorage& serverStorage, QObject* parent = 0 );
    virtual ~ISClientHendler();

public slots:
    /**
     * @brief Slot for initializing and starting the client
     */
    void execClient();

    /**
     * @brief Returns client type
     * @return a client type
     */
    ClientType getClientType() const;

signals:
    /**
     * @brief This signal is emitted after tcp disconnection
     */
    void signalDisconnected();

    /**
     * @brief This signal is emitted to add a new message to log
     * @param message - a log message
     */
    void signalLogMessage( const QString& message );

    /**
     * @brief This signal is emitted for starting a camera, this signal of viewer client
     * connects to slot slotStartCamera() of a camera client to start the camera
     * @param fps - a camera operating frequency
     */
    void signalStartCamera( const qreal& fps ) const;

    /**
     * @brief This signal is emitted for stopping a camera, this signal of viewer client
     * connects to slot slotStopCamera() of a camera client to stop the camera
     */
    void signalStopCamera() const;

public slots:
    /**
     * @brief Slot for starting a camera, available ONLY for a camera client
     * @param fps - a camera operating frequency
     */
    void slotStartCamera( const qreal& fps );

    /**
     * @brief Slot for stopping a camera, available ONLY for a camera client
     */
    void slotStopCamera();

    /**
     * @brief Slot for sending an image, available ONLY for a viewer client
     */
    void sendImageToViewer();

/**
 * @brief These private slots are used to execute command after corresponding pClientSocket signals
 */
private slots:
    void readyRead();
    void disconnected();

private:
    /**
     * @brief Processing an input message from a client
     * @param in - input tcp stream
     */
    void processMessage( QDataStream& in );

    /**
     * @brief Processing a manage type message from a client
     * @param in - input tcp stream
     */
    void processManageMessage( QDataStream& in );

    /**
     * @brief Processing an image type message from a client
     * @param in - input tcp stream
     */
    void processImageMessage( QDataStream& in );

    /**
     * @brief Answering to client for an MT_AUTHORIZATION request
     * @param authResult - an authorization result
     */
    void answerAuthorization( bool authResult );

    /**
     * @brief Answering to client for an CCT_GET_LIST_OF_CAMS request
     */
    void answerListOfCameras();

    /**
     * @brief Starts camera with specified ip address and frequency
     * @param camIP - a camera ip address
     * @param fps - a camera operating frequency
     */
    void startCamera( quint32 camIP, const qreal& fps );

    /**
     * @brief Stops camera with specified ip address
     * @param camIP - a camera ip address
     */
    void stopCamera( quint32 camIP );

    /**
     * @brief Starts streaming, from server to the client, images from
     * camera with specified ip address
     * @param camIP - a ip address of the camera which images will be streamed
     * @param fps - a streaming operating frequency
     */
    void startImagesStreaming( quint32 camIP, const qreal& fps );

    /**
     * @brief Stops streaming from server to the client
     */
    void stopImagesStreaming();


    /**
     * @brief Adds a message to log
     * @param message - a log message
     */
    void addMessageToLog( const QString& message );

    /**
     * @brief Adds an error message to log
     * @param message - a error message
     */
    void addErrorToLog( const QString& message );

private:
    ISClientHendler();      // forbidden default constructor

private:
    ServerStorage& serverStorage;   // a storage of clients and images from cameras
    QTcpSocket* pClientSocket;      // TCP socket for communication with a client
    qintptr socketDescriptor;       // TCP socket descriptor
    ClientType clientType;          // type of client
    quint32 ip;                     // ip address of client
    qint64 nBitMessageSize;         // a bit size of a current message from the server, the
                                    // first 64 bits of each message

    QTimer* streamingTimer;         // a timer for image streaming from the server to
                                    // a viewer, used ONLY for a viewer client
    quint32 cameraIP;               // a camera ip adress for image streaming from the server to
                                    // a viewer, used ONLY for a viewer client
};

#endif // CLIENT_HENDLER_H
