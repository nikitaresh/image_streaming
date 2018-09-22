#ifndef CAMERA_CLIENT_H
#define CAMERA_CLIENT_H

#include <string>
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTimer>
#include <QThread>
#include <QFile>
#include <opencv2/opencv.hpp>
#include <common.h>
#include <pi_camera.h>


/**
 * @brief Interaction object with the server via TCP
 */
class CameraClient : public QObject
{
    Q_OBJECT
public:
    explicit CameraClient( QObject* parent = 0 );
    virtual ~CameraClient();

    /**
     * @brief Connects to the server
     * @param hostName - a server ip address
     * @param port - a server port
     * @param waitConnectionMsecs - connection timeout in millisecond
     */
    void connectToServer( const QString& hostName, quint16 port, int waitConnectionMsecs );

    /**
     * @brief Closes connection with the server
     */
    void closeConnection();

public slots:
    /**
     * @brief Slot for sending an image to the server
     */
    void slotSendImage();

signals:
    /**
     * @brief This signal is emitted for starting Raspberry Pi camera
     * @param fps - a camera operating frequency
     */
    void signalStartCamera( double fps );

    /**
     * @brief This signal is emitted for stopping Raspberry Pi camera
     */
    void signalStopCamera();

/**
 * @brief These public slots are used to execute command after corresponding tcpSocket signals
 */
public slots:
    void slotConnected();
    void slotReadyRead();
    void slotDisconnected();
    void slotError(QAbstractSocket::SocketError err);

private:
    /**
     * @brief Connects to the server
     * @return connection result
     */
    bool slotConnectToServer();

    /**
     * @brief Processing an input message from the server
     * @param in - input tcp stream
     */
    void processMessage( QDataStream& in );

    /**
     * @brief Writes message to log
     * @param message - a log message
     */
    void addMessageToLog( const QString& message );

private:
    QTcpSocket tcpSocket;           // TCP socket for communication with the server
    QTimer connectionTimer;         // timer for reconnection
    QString hostName;               // a server ip address
    quint16 port;                   // a server port
    int waitConnectionMsecs;        // connection timeout in millisecond
    qint64 nBitMessageSize;         // a bit size of a current message from the server, the
                                    // first 64 bits of each message

    PiCamera* piCamera;             // Raspberry Pi camera
    QThread piCameraThread;         // a thread of piCamera execution

    std::string compressBuff;       // Buffer for output of image compressing

    QFile logFile;                  // log file
};

#endif // CAMERA_CLIENT_H