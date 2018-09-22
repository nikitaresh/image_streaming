#ifndef VIEWER_CLIENT_H
#define VIEWER_CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QImage>
#include <QString>
#include <QVector>
#include <common.h>
#include <string>

/**
\brief  Interaction object with the server via TCP
*/
class ViewerClient : public QObject
{
    Q_OBJECT

public:
    explicit ViewerClient( QObject* parent = 0 );
    ~ViewerClient();

    /**
     * @brief Returns vector of available cameras
     * @return QVector of availiable cameras
     */
    QVector<quint32> getCameras() const;

public slots:
    /**
     * @brief Slot for establish connection with the server
     * @param ip - a server IP adress
     * @param port - a server port
     * @return connection result: true - connected, false - otherwise
     */
    bool slotConnectToServer( const QString& ip, quint16 port );

    /**
     * @brief Slot for close connection with the server
     */
    void slotDisconnect();

    /**
     * @brief Slot for starting a camera
     * @param ip - a camera IP adress
     * @param fps - a camera operating frequency
     */
    void slotRunCamera( quint32 ip, qreal fps );

    /**
     * @brief Slot for stopping a camera
     * @param ip - a camera IP adress
     */
    void slotStopCamera( quint32 ip );

    /**
     * @brief Slot for starting stream from the server to this viewer
     * @param ip - a camera IP adress
     * @param fps - a stream operating frequency
     */
    void slotStartStreaming( quint32 ip, qreal fps );

    /**
     * @brief Slot for stopping stream from the server to this viewer
     */
    void slotStopStreaming();

signals:
    /**
     * @brief This signal is emitted after establish connection, successful
     * authorisation and receiving list of available cameras from the server
     */
    void signalConnected();

    /**
     * @brief This signal is emitted after the connection to the server is disconnected
     */
    void signalDisconnected();

    /**
     * @brief This signal is emitted after receiving a new image
     * @param qImage - a new image from the server
     */
    void signalNewImage( const QImage& qImage );

    /**
     * @brief This signal is emitted when an error occurs
     * @param result - a text error description
     */
    void signalError( const QString& result );

/**
 * @brief These private slots are used to execute command after corresponding tcpSocket signals
 */
private slots:
    void slotConnected();
    void slotReadyRead();
    void slotDisconnected();
    void slotError( QAbstractSocket::SocketError err );

private:
    /**
     * @brief Process a message from the server
     * @param in - input data stream associated with tcpSocket
     */
    void processMessage( QDataStream& inputStream );

private:
    QTcpSocket tcpSocket;       // TCP socket for communication with the server
    qint64 nBitMessageSize;     // a bit size of a current message from the server, the
                                // first 64 bits of each message

    QVector<quint32> cameras;   // array of available cameras are requested after authorisation

    std::string compressBuff;       // buffer of compressed image data recieved from Pi
    std::string uncompressedBuff;   // buffer of uncompressed image data
};

#endif // VIEWER_CLIENT_H
