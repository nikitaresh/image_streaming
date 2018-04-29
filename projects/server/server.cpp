
#include <server.h>
#include <QDateTime>
#include <QThread>
#include <QDebug>

ISServer::ISServer( QObject* parent )
    : QTcpServer(parent)
{
    logFile.setFileName( "server_log.txt" );
    logFile.open( QIODevice::Append );
}

ISServer::~ISServer()
{
    if( isListening() ) {
        stopServer();
    }

    logFile.close();
}

bool ISServer::startServer( int port )
{
    bool isServerStarted = this->listen( QHostAddress::Any, port );
    if( isServerStarted ) {
        addMessageToLog( "Start server" );
    }
    else {
        addMessageToLog( "Server not started!" );
    }

    return isServerStarted;
}

void ISServer::stopServer()
{
    close();
    addMessageToLog( "Stop server" );
}


void ISServer::slotClientHendlerMsg( const QString& message )
{
    addMessageToLog(message);
}

void ISServer::incomingConnection( qintptr socketDescriptor )
{
    addMessageToLog( "New connection with descriptor: " + QString::number(socketDescriptor) );

    ISClientHendler* pClientHendler = new ISClientHendler( socketDescriptor, serverStorage );
    QThread* pClientThread = new QThread;
    if( pClientHendler == nullptr || pClientThread == nullptr ) {
        addMessageToLog("incomingConnection(...) error: pClientHendler == nullptr || pClientThread == nullptr");
        return;
    }

    pClientHendler->moveToThread(pClientThread);

    bool isAllBind = true;
    isAllBind = isAllBind && connect( pClientThread, &QThread::started,
                                      pClientHendler, &ISClientHendler::execClient );
    isAllBind = isAllBind && connect( pClientHendler, &ISClientHendler::signalDisconnected,
                                      pClientThread, &QThread::quit );
    isAllBind = isAllBind && connect( pClientHendler, &ISClientHendler::signalLogMessage,
                                      this, &ISServer::slotClientHendlerMsg );
    isAllBind = isAllBind && connect( pClientHendler, &ISClientHendler::signalDisconnected,
                                      pClientHendler, &ISClientHendler::deleteLater );
    isAllBind = isAllBind && connect( pClientThread, &QThread::finished,
                                      pClientThread, &QThread::deleteLater );

    pClientThread->start();
}

void ISServer::addMessageToLog( const QString& message )
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
