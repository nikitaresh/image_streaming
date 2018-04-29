
#include <client_hendler.h>
#include <QDataStream>
#include <QHostAddress>


ISClientHendler::ISClientHendler( qintptr ID, ServerStorage& storage, QObject* parent )
    : QObject(parent), serverStorage(storage), pClientSocket(nullptr),
      socketDescriptor(ID), clientType(CT_UNKNOWN), ip(0), nBitMessageSize(0),
      streamingTimer(nullptr), cameraIP(0)
{
}

ISClientHendler::~ISClientHendler()
{
}

void ISClientHendler::execClient()
{
    pClientSocket = new QTcpSocket(this);
    if( pClientSocket == nullptr ) {
        addErrorToLog("execClient(): error: pClientSocket == nullptr");
        return;
    }

    streamingTimer = new QTimer(this);
    if( streamingTimer == nullptr ) {
        addErrorToLog("execClient(): error: streamingTimer == nullptr");
        return;
    }
    streamingTimer->setSingleShot( false );

    bool isAllBind = true;
    isAllBind = isAllBind && connect( streamingTimer, &QTimer::timeout,
                                      this, &ISClientHendler::sendImageToViewer );

    // set the ID
    if( !pClientSocket->setSocketDescriptor(this->socketDescriptor) )
    {
        addErrorToLog( "execClient(): pClientSocket->setSocketDescriptor(...) error code: "
                      + QString::number(pClientSocket->error()) );
        return;
    }

    ip = pClientSocket->peerAddress().toIPv4Address();
    addMessageToLog( "New connecion with ip: " + QHostAddress(ip).toString() );

    // connect socket and signal
    isAllBind = isAllBind && connect( pClientSocket, SIGNAL(readyRead()),
                                      this, SLOT(readyRead()), Qt::DirectConnection );
    isAllBind = isAllBind && connect( pClientSocket, SIGNAL(disconnected()),
                                      this, SLOT(disconnected()) );

    serverStorage.updateClientHendler(ip, this);
}

ClientType ISClientHendler::getClientType() const
{
    return clientType;
}


void ISClientHendler::slotStartCamera( const qreal& fps )
{
    if( clientType != CT_CAMERA ) {
        return;
    }

    QByteArray arrBlock;
    QDataStream stream( &arrBlock, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_MANAGING) << qint32(CCT_START_CAM) << fps;

    if( pClientSocket == nullptr || stream.device() == nullptr ) {
        addErrorToLog("slotStartCamera(...) error: pClientSocket == nullptr || stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(arrBlock.size() - sizeof(qint64));
    pClientSocket->write( arrBlock );
}

void ISClientHendler::slotStopCamera()
{
    if( clientType != CT_CAMERA ) {
        return;
    }

    QByteArray arrBlock;
    QDataStream stream( &arrBlock, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_MANAGING) << qint32(CCT_STOP_CAM);

    if( pClientSocket == nullptr || stream.device() == nullptr ) {
        addErrorToLog("slotStopCamera() error: pClientSocket == nullptr || stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(arrBlock.size() - sizeof(qint64));
    pClientSocket->write( arrBlock );
}


void ISClientHendler::sendImageToViewer()
{
    if( clientType != CT_VIEWER ) {
        return;
    }

    if( pClientSocket == nullptr || pClientSocket->bytesToWrite() != 0 ) {
        return;
    }

    QSharedPointer<ISImage> image = serverStorage.getImage( cameraIP );
    if( image.isNull() ) {
        return;
    }

    qint32 imgType = image.data()->type;
    qint32 imgRows = image.data()->rows;
    qint32 imgCols = image.data()->cols;
    qint32 imgDataSize = image.data()->dataSize;

    QByteArray arrBlock;
    QDataStream stream( &arrBlock, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_IMAGE) << imgType << imgRows << imgCols << imgDataSize;
    stream.writeRawData( image.data()->getData(), imgDataSize );

    if( stream.device() == nullptr ) {
        addErrorToLog("sendImageToViewer() error: stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(arrBlock.size() - sizeof(qint64));

    pClientSocket->write( arrBlock );
}

//

void ISClientHendler::readyRead()
{
    if( pClientSocket == nullptr ) {
        return;
    }

    QDataStream in(pClientSocket);
    while(true)
    {
        if ( nBitMessageSize == 0 ) {
            if( pClientSocket->bytesAvailable() < sizeof(qint64) ) {
                break;
            }
            in >> nBitMessageSize;
        }

        if( pClientSocket->bytesAvailable() < nBitMessageSize ) {
            break;
        }

        processMessage( in );
        nBitMessageSize = 0;
    }
}

void ISClientHendler::disconnected()
{
    addMessageToLog( "disconnected: " + QHostAddress(ip).toString() );
    serverStorage.eraseClientHendler(ip);
    emit signalDisconnected();
}

//

void ISClientHendler::processMessage( QDataStream& in )
{
    qint32 messageType = 0;
    in >> messageType;

    switch( messageType )
    {
    case MT_AUTHORIZATION:
    {
        bool authResult = false;

        if( clientType != CT_UNKNOWN ) {
            addErrorToLog("processMessage(): MT_AUTHORIZATION: re-authorization");
        }
        else
        {
            qint32 clientTypeMsg;
            in >> clientTypeMsg;
            if( clientTypeMsg > CT_UNKNOWN || clientTypeMsg <= CT_VIEWER ) {
                clientType = static_cast<ClientType>(clientTypeMsg);
                authResult = true;
                addMessageToLog("MT_AUTHORIZATION: client type: " + QString::number(clientType));
            }
            else {
                addErrorToLog("processMessage(): MT_AUTHORIZATION: wrong client type");
            }
        }

        answerAuthorization(authResult);
        break;
    }
    case MT_MANAGING:
    {
        processManageMessage(in);
        break;
    }
    case MT_IMAGE:
    {
        switch( clientType )
        {
        case CT_CAMERA:
            processImageMessage(in);
            break;
        default:
            addErrorToLog( "processMessage(): messageType MT_IMAGE not from clientType CT_CAMERA" );
            break;
        }
        break;
    }
    default:
        addErrorToLog( "processMessage(): bad messageType" );
        break;
    }
}


void ISClientHendler::processManageMessage( QDataStream& in )
{
    qint32 command = 0;
    quint32 camIP = 0;
    qreal fps = 0;
    in >> command;

    switch( command )
    {
    case CCT_GET_LIST_OF_CAMS:
        answerListOfCameras();
        addMessageToLog("processManageMessage(...): CCT_GET_LIST_OF_CAMS");
        break;
    case CCT_START_CAM:
        in >> camIP >> fps;
        startCamera( camIP, fps );
        addMessageToLog("processManageMessage(...): CCT_START_CAM with ip: " +
                        QHostAddress(camIP).toString() + " and fps: " + QString::number(fps));
        break;
    case CCT_STOP_CAM:
        in >> camIP;
        stopCamera( camIP );
        addMessageToLog("processManageMessage(...): CCT_STOP_CAM with ip: " +
                        QHostAddress(camIP).toString());
        break;
    case CCT_START_STREAM:
        in >> camIP >> fps;
        startImagesStreaming( camIP, fps );
        addMessageToLog("processManageMessage(...): CCT_START_STREAM from ip: " +
                        QHostAddress(camIP).toString() + " and fps: " + QString::number(fps));
        break;
    case CCT_STOP_STREAM:
        stopImagesStreaming();
        addMessageToLog("processManageMessage(...): CCT_STOP_STREAM");
        break;
    default:
        addErrorToLog("processManageMessage(): bad command");
        break;
    }
}

void ISClientHendler::processImageMessage( QDataStream& in )
{
    qint32 imgType = 0, imgRows = 0, imgCols = 0, imgDataSize = 0;
    in >> imgType >> imgRows >> imgCols >> imgDataSize;
    QSharedPointer<ISImage> image(new ISImage(imgType, imgRows, imgCols, imgDataSize));
    if( !image.isNull() )
    {
        in.readRawData( image->getData(), imgDataSize );
        serverStorage.updateImages(ip, image);
    }
    else {
        addErrorToLog("processImageMessage(): image == nullptr");
    }
}

void ISClientHendler::answerAuthorization( bool authResult )
{
    QByteArray arrBlock;
    QDataStream stream( &arrBlock, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_AUTHORIZATION) << quint8(authResult);

    if( pClientSocket == nullptr || stream.device() == nullptr ) {
        addErrorToLog("answerAuthorization(...) error: pClientSocket == nullptr || stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(arrBlock.size() - sizeof(qint64));
    pClientSocket->write( arrBlock );
}

void ISClientHendler::answerListOfCameras()
{
    QByteArray arrBlock;
    QDataStream stream( &arrBlock, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_MANAGING) << qint32(CCT_GET_LIST_OF_CAMS);

    QVector<quint32> cameras = serverStorage.getCameras();
    qint32 numCameras = static_cast<qint32>(cameras.size());
    stream << numCameras;
    for( qint32 index = 0; index < numCameras; ++index ) {
        stream << cameras[index];
    }

    if( pClientSocket == nullptr || stream.device() == nullptr ) {
        addErrorToLog("answerListOfCameras()) error: pClientSocket == nullptr || stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(arrBlock.size() - sizeof(qint64));
    pClientSocket->write( arrBlock );
}

void ISClientHendler::startCamera( quint32 camIP, const qreal& fps )
{
    ISClientHendler* clientHendler = serverStorage.getClientHendler(camIP);
    if( clientHendler != nullptr )
    {
        connect( this, &ISClientHendler::signalStartCamera,
                 clientHendler, &ISClientHendler::slotStartCamera );

        emit signalStartCamera(fps);

        disconnect( this, &ISClientHendler::signalStartCamera,
                    clientHendler, &ISClientHendler::slotStartCamera );
    }
}

void ISClientHendler::stopCamera( quint32 camIP )
{
    ISClientHendler* clientHendler = serverStorage.getClientHendler(camIP);
    if( clientHendler != nullptr )
    {
        connect( this, &ISClientHendler::signalStopCamera,
                 clientHendler, &ISClientHendler::slotStopCamera );

        emit signalStopCamera();

        disconnect( this, &ISClientHendler::signalStopCamera,
                    clientHendler, &ISClientHendler::slotStopCamera );
    }
}

void ISClientHendler::startImagesStreaming( quint32 camIP, const qreal& fps )
{
    if( streamingTimer == nullptr || fps == 0 ) {
        addErrorToLog("startImagesStreaming(...) error: streamingTimer == nullptr || fps == 0");
        return;
    }

    cameraIP = camIP;

    int msec = static_cast<int>(1000.0 / fps);
    streamingTimer->start(msec);
}

void ISClientHendler::stopImagesStreaming()
{
    if( streamingTimer == nullptr ) {
        addErrorToLog("stopImagesStreaming() error: streamingTimer == nullptr");
        return;
    }

    cameraIP = 0;
    streamingTimer->stop();
}

void ISClientHendler::addMessageToLog( const QString& message )
{
    QString msg = "ClientMsg : { ip: " + QHostAddress(ip).toString() + " msg: " + message + " } ";
    emit signalLogMessage(msg);
}

void ISClientHendler::addErrorToLog( const QString& message )
{
    QString msg = "ClientError : { ip: " + QHostAddress(ip).toString() + " msg: " + message + " } ";
    emit signalLogMessage(msg);
}
