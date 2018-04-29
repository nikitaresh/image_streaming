#ifndef VIEWER_H
#define VIEWER_H

#include <QWidget>
#include <viewer_client.h>
#include <QThread>

namespace Ui {
class Viewer;
}

/**
 * @brief Widget for displaying data from the server
 */
class Viewer : public QWidget
{
    Q_OBJECT

public:
    explicit Viewer( QWidget* parent = 0 );
    ~Viewer();

public slots:
    /**
     * @brief Slot is called after connection to the server and successful authorization
     */
    void slotClientConnected();

    /**
     * @brief Slot is called after disconnected from the server
     */
    void slotClientDisconnected();

    /**
     * @brief Slot is called after receiving a new image
     * @param qImage - a new image from the server
     */
    void slotNewImage( const QImage& qImage );

    /**
     * @brief Slot is called to process the client error
     * @param result - a text error description
     */
    void slotClientError( const QString& result );

signals:
    /**
     * @brief This signal is emitted for connection to the server
     * @param ip - a server IP adress
     * @param port - a server port
     */
    void signalConnectToServer( const QString& ip, quint16 port );
    
    /**
     * @brief This signal is emitted to disconnect from the server
     */
    void signalDisconnect();
    
    /**
     * @brief This signal is emitted to start a camera
     * @param ip - a camera IP adress
     * @param fps - a camera operating frequency
     */
    void signalRunCamera( quint32 ip, qreal fps );
    
    /**
     * @brief This signal is emitted to stop a camera
     * @param ip - a camera IP adress
     */
    void signalStopCamera( quint32 ip );

    /**
     * @brief This signal is emitted to start streaming from the server to this viewer
     * @param ip - a camera IP adress
     * @param fps - a stream operating frequency
     */
    void signalStartStreaming( quint32 ip, qreal fps );

    /**
     * @brief This signal is emitted to stop streaming from the server to this viewer
     */
    void signalStopStreaming();

public:
    /**
     * @brief Method for converting widget coordinates with specified transform coefficients
     * @param obj - widget
     * @param kx - transformation factor along the x axis
     * @param ky - transformation factor along the y axis
     */
    static void transformQWidjet( QWidget* obj, double kx, double ky );

/**
 * @brief These private slots are used for event processing of graphic objects
 */
private slots:
    void on_connectionButton_clicked();
    void on_camerasComboBox_activated( int index );
    void on_runCameraButton_clicked();
    void on_startStreamingButton_clicked();
    void on_exitButton_clicked();

private:
    Ui::Viewer* ui;                 // the main GUI

    ViewerClient* viewerClient;     // object for communication with the server
    QThread viewerClientThread;     // a thread of viewerClient execution
    bool isClientConnected;         // connection status of viewerClient
    bool isCameraRunning;           // running status of a chosen camera
    bool isImagesStream;            // streaming status

    quint32 choosenIP;              // a chosen camera by the camerasComboBox widjet

};

#endif // VIEWER_H
