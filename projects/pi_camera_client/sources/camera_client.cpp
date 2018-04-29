
#include <camera_client.h>
#include <QDataStream>
#include <QDateTime>

CameraClient::CameraClient( QObject* parent )
    : QObject(parent), tcpSocket(this), connectionTimer(this), 
    hostName(""), port(0), waitConnectionMsecs(0), nBitMessageSize(0), 
    piCamera(nullptr), logFile(this)
{
    logFile.setFileName( "pi_cam_client_log.txt" );
    logFile.open( QIODevice::Append );

    piCamera = new PiCamera;
    if( piCamera == nullptr ) {
        addMessageToLog("CameraClient::CameraClient(...): piCamera == nullptr");
        return;
    }
    piCamera->moveToThread(&piCameraThread);

    bool isAllBind = true;
    isAllBind = isAllBind && connect( &tcpSocket, &QTcpSocket::connected,
                                      this, &CameraClient::slotConnected );
    isAllBind = isAllBind && connect( &tcpSocket, &QTcpSocket::readyRead,
                                      this, &CameraClient::slotReadyRead );
    isAllBind = isAllBind && connect( &tcpSocket, &QTcpSocket::disconnected,
                                      this, &CameraClient::slotDisconnected );
    isAllBind = isAllBind && connect( &tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                                      this, SLOT(slotError(QAbstractSocket::SocketError)) );
    isAllBind = isAllBind && connect( this, &CameraClient::signalStartCamera,
                                      piCamera, &PiCamera::slotStartCamera );
    isAllBind = isAllBind && connect( this, &CameraClient::signalStopCamera,
                                      piCamera, &PiCamera::slotStopCamera );
    isAllBind = isAllBind && connect( piCamera, &PiCamera::signalNewImage,
                                      this, &CameraClient::slotSendImage );
    isAllBind = isAllBind && connect( &connectionTimer, &QTimer::timeout,
                                      this, &CameraClient::slotConnectToServer );

    connectionTimer.setSingleShot(false);
    piCameraThread.start();
}

CameraClient::~CameraClient()
{
    closeConnection();

    logFile.close();
}

bool CameraClient::connectToServer( const QString& hostName_, quint16 port_, int waitConnectionMsecs_ )
{
    hostName = hostName_;
    port = port_;
    waitConnectionMsecs = waitConnectionMsecs_;
    bool isConnected = slotConnectToServer();
    if( !isConnected ) {
        addMessageToLog( "ERROR: connectToServer(...) failed: slotConnectToServer() return false" );
        return false;
    }

    addMessageToLog("connectToServer(): connected");
    return true;
}

void CameraClient::closeConnection()
{
    piCameraThread.quit();
    piCameraThread.wait();

    tcpSocket.close();
    tcpSocket.waitForDisconnected();
}

//

void CameraClient::slotConnected()
{
    QByteArray byteArray;
    QDataStream stream( &byteArray, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_AUTHORIZATION) << qint32(CT_CAMERA);

    if( stream.device() == nullptr ) {
        addMessageToLog("ERROR: slotConnected() error: stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(byteArray.size() - sizeof(qint64));
    tcpSocket.write( byteArray );
}

void CameraClient::slotReadyRead()
{
    QDataStream in(&tcpSocket);
    while(true)
    {
        if ( nBitMessageSize == 0 ) {
            if( tcpSocket.bytesAvailable() < sizeof(qint64) ) {
                break;
            }
            in >> nBitMessageSize;
        }

        if( tcpSocket.bytesAvailable() < nBitMessageSize ) {
            break;
        }

        processMessage( in );
        nBitMessageSize = 0;
    }
}

void CameraClient::slotDisconnected()
{
    connectionTimer.start(waitConnectionMsecs);
}

void CameraClient::slotError( QAbstractSocket::SocketError err )
{
    QString errorStr = "ERROR: slotError(...): tcpSocket signal error ID: " + QString::number(err);
    addMessageToLog( errorStr );
}

void CameraClient::slotSendImage()
{
    if( piCamera == nullptr || tcpSocket.state() != QAbstractSocket::ConnectedState ) {
        return;
    }

    cv::Mat3b image = piCamera->getImage();
    if( image.empty() ) {
        return;
    }

    if( tcpSocket.bytesToWrite() != 0 ) {
        return;
    }

    qint32 imgType = image.type();
    qint32 imgRows = image.rows;
    qint32 imgCols = image.cols;
    qint32 imgDataSize = 3 * image.rows * image.cols;

    QByteArray arrBlock;
    QDataStream stream( &arrBlock, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_IMAGE) << imgType << imgRows << imgCols << imgDataSize;
    stream.writeRawData( (char*)image.data, imgDataSize );

    if( stream.device() == nullptr ) {
        addMessageToLog("ERROR: slotSendImage() error: stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(arrBlock.size() - sizeof(qint64));
    tcpSocket.write( arrBlock );
    tcpSocket.waitForBytesWritten();
}

//

bool CameraClient::slotConnectToServer()
{
    if( tcpSocket.state() == QAbstractSocket::ConnectedState ) {
        connectionTimer.stop();
        return true;
    }

    tcpSocket.connectToHost( hostName, port );
    bool isConnected = tcpSocket.waitForConnected( waitConnectionMsecs );
    return isConnected;
}

void CameraClient::processMessage( QDataStream& in )
{
    qint32 messageType = 0;
    in >> messageType;

    switch( MessageType(messageType) )
    {
    case MT_AUTHORIZATION:
    {
        quint8 authResult = 0;
        in >> authResult;
        addMessageToLog( "processMessage(): authResult: " + QString::number(authResult) );
        break;
    }
    case MT_MANAGING:
    {
        qint32 command = 0;
        in >> command;
        qreal fps = 0;
        switch( CommandControlType(command) )
        {
        case CCT_START_CAM:
            in >> fps;
            emit signalStartCamera(fps);
            addMessageToLog( "processMessage(): start camara, fps: " + QString::number(fps) );
            break;
        case CCT_STOP_CAM:
            emit signalStopCamera();
            addMessageToLog( "processMessage(): stop camara" );
            break;
        default:
            addMessageToLog("ERROR: processMessage(...): messageType MT_MANAGING, bad command");
            break;
        }
        break;
    }
    default:
        addMessageToLog( "ERROR: processMessage(...): bad messageType!" );
        break;
    }
}

void CameraClient::addMessageToLog( const QString& message )
{
    const QString logTimeFormt = "yyyy.MM.dd hh:mm:ss.zzz";
    QDateTime time = QDateTime::currentDateTime();
    QString logMessage = time.toString(logTimeFormt) + "    " + message + "\n";

    if( logFile.isOpen() )
    {
        logFile.write( logMessage.toStdString().c_str() );
        logFile.flush();
    }

    qDebug() << message;
}
