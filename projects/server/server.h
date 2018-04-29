#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QFile>
#include <server_storage.h>
#include <client_hendler.h>


/**
 * @brief Image Streaming Server
 */
class ISServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit ISServer( QObject* parent = 0 );
    virtual ~ISServer();

    /**
     * @brief Starts the server with specified port
     * @param port - a server port
     * @return result of execution
     */
    bool startServer( int port );

    /**
     * @brief Stops the server
     */
    void stopServer();

public slots:
    /**
     * @brief Slot for writing a message to log
     * @param message - a log message
     */
    void slotClientHendlerMsg( const QString& message );

protected:
    /**
     * @brief This function is called when a new connection is available
     * @param socketDescriptor - descriptor of a new tcp client
     */
    void incomingConnection( qintptr socketDescriptor );

private:
    /**
     * @brief Writes message to log
     * @param message - a log message
     */
    void addMessageToLog( const QString& message );

private:
    ServerStorage serverStorage;    // a storage of clients and images from cameras

    QFile logFile;                  // log file
};

#endif // SERVER_H
