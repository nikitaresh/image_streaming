
#include <viewer_client.h>
#include <QByteArray>
#include <QDataStream>


ViewerClient::ViewerClient( QObject* parent )
    : QObject(parent), tcpSocket(this), nBitMessageSize(0)
{
    bool isAllBind = true;
    isAllBind = isAllBind && connect( &tcpSocket, &QTcpSocket::connected,
                                      this, &ViewerClient::slotConnected );
    isAllBind = isAllBind && connect( &tcpSocket, &QTcpSocket::readyRead,
                                      this, &ViewerClient::slotReadyRead );
    isAllBind = isAllBind && connect( &tcpSocket, &QTcpSocket::disconnected,
                                      this, &ViewerClient::slotDisconnected );
    isAllBind = isAllBind && connect( &tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                                      this, SLOT(slotError(QAbstractSocket::SocketError)) );
}

ViewerClient::~ViewerClient()
{
    slotDisconnect();
}

QVector<quint32> ViewerClient::getCameras() const
{
    return cameras;
}

bool ViewerClient::slotConnectToServer( const QString& ip, quint16 port )
{
    if( tcpSocket.state() == QAbstractSocket::ConnectedState ) {
        slotDisconnect();
    }

    int waitConnectionMsecs = 1000;
    tcpSocket.connectToHost( ip, port );
    bool isConnected = tcpSocket.waitForConnected( waitConnectionMsecs );
    if( !isConnected ) {
        emit signalError( "connectToServer(...) failed: connectToHost(...) return false" );
        return false;
    }

    nBitMessageSize = 0;
    return true;
}

void ViewerClient::slotDisconnect()
{
    tcpSocket.close();
}

void ViewerClient::slotRunCamera( quint32 ip, qreal fps )
{
    QByteArray arrBlock;
    QDataStream stream( &arrBlock, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_MANAGING) << qint32(CCT_START_CAM);
    stream << ip << fps;
    if( stream.device() == nullptr ) {
        emit signalError("slotRunCamera(...) error: stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(arrBlock.size() - sizeof(qint64));

    tcpSocket.write( arrBlock );
}

void ViewerClient::slotStopCamera( quint32 ip )
{
    QByteArray arrBlock;
    QDataStream stream( &arrBlock, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_MANAGING) << qint32(CCT_STOP_CAM);
    stream << ip;
    if( stream.device() == nullptr ) {
        emit signalError("slotStopCamera(...) error: stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(arrBlock.size() - sizeof(qint64));

    tcpSocket.write( arrBlock );
}

void ViewerClient::slotStartStreaming( quint32 ip, qreal fps )
{
    QByteArray arrBlock;
    QDataStream stream( &arrBlock, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_MANAGING) << qint32(CCT_START_STREAM);
    stream << ip << fps;
    if( stream.device() == nullptr ) {
        emit signalError("slotStartStreaming(...) error: stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(arrBlock.size() - sizeof(qint64));

    tcpSocket.write( arrBlock );
}

void ViewerClient::slotStopStreaming()
{
    QByteArray arrBlock;
    QDataStream stream( &arrBlock, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_MANAGING) << qint32(CCT_STOP_STREAM);
    if( stream.device() == nullptr ) {
        emit signalError("slotStopStreaming() error: stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(arrBlock.size() - sizeof(qint64));

    tcpSocket.write( arrBlock );
}

void ViewerClient::slotConnected()
{
    QByteArray byteArray;
    QDataStream stream( &byteArray, QIODevice::WriteOnly );
    stream << qint64(0) << qint32(MT_AUTHORIZATION) << qint32(CT_VIEWER);
    if( stream.device() == nullptr ) {
        emit signalError("slotConnected() error: stream.device() == nullptr");
        return;
    }
    stream.device()->seek(0);
    stream << qint64(byteArray.size() - sizeof(qint64));

    tcpSocket.write( byteArray );
}

void ViewerClient::slotReadyRead()
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

void ViewerClient::slotDisconnected()
{
    emit signalDisconnected();
}

void ViewerClient::slotError( QAbstractSocket::SocketError err )
{
    QString message = "tcpSocket error: code: " + QString::number(err);
    emit signalError(message);
}

void ViewerClient::processMessage( QDataStream& inputStream )
{
    qint32 messageType = 0;
    inputStream >> messageType;

    switch( messageType )
    {
    case MT_AUTHORIZATION:
    {
        quint8 authResult = 0;
        inputStream >> authResult;
        if( authResult == 1 )
        {
            QByteArray arrBlock;
            QDataStream stream( &arrBlock, QIODevice::WriteOnly );
            stream << qint64(0) << qint32(MT_MANAGING) << qint32(CCT_GET_LIST_OF_CAMS);
            if( stream.device() != nullptr )
            {
                stream.device()->seek(0);
                stream << qint64(arrBlock.size() - sizeof(qint64));
                tcpSocket.write( arrBlock );
            }
            else {
                tcpSocket.close();
            }
        }
        else {
            tcpSocket.close();
        }
        break;
    }
    case MT_MANAGING:
    {
        qint32 command = 0;
        inputStream >> command;
        if( command == CCT_GET_LIST_OF_CAMS )
        {
            qint32 numCameras = 0;
            inputStream >> numCameras;
            cameras.clear();
            cameras.reserve(numCameras);
            for( qint32 index = 0; index < numCameras; ++index )
            {
                quint32 ip;
                inputStream >> ip;
                cameras.push_back( ip );
            }

            emit signalConnected();
        }
        else {
            emit signalError("processMessage(...): case MT_MANAGING: command != CCT_GET_LIST_OF_CAMS");
        }
        break;
    }
    case MT_IMAGE:
    {
        qint32 imgType = 0, imgRows = 0, imgCols = 0, imgDataSize = 0;
        inputStream >> imgType >> imgRows >> imgCols >> imgDataSize;
        const qint32 RGBTypeOpenCV = 16; // cv::Mat3b has this image type value in OpenCV
        bool isBadImgFormat = (imgType != RGBTypeOpenCV) || (imgDataSize != 3 * imgRows * imgCols);
        if( isBadImgFormat ) {
            emit signalError("processMessage(...): case MT_IMAGE: bad image format");
            return;
        }

        QImage qImage(imgCols, imgRows, QImage::Format_RGB888);
        if( qImage.isNull() ) {
            emit signalError("processMessage(...): case MT_IMAGE: error while creating a qImage");
            return;
        }

        inputStream.readRawData((char*)(qImage.bits()), qImage.byteCount());
        emit signalNewImage(qImage);
        break;
    }
    default:
        emit signalError("processMessage(...): bad messageType");
        break;
    }
}
