#ifndef SERVER_STORAGE_H
#define SERVER_STORAGE_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QSharedPointer>
#include <QMutex>
#include <common.h>
#include <isimage.h>

class ISClientHendler;


/**
 * @brief Container of clients and images from cameras
 */
class ServerStorage
{
public:
    ServerStorage();
    ~ServerStorage();

    /**
     * @brief Returns a client with specified IP address
     * @param ip - a client ip address
     * @return non-owning client pointer, if a client isn't found returns nullptr
     */
    ISClientHendler* getClientHendler( quint32 ip );

    /**
     * @brief Returns an image is received from the camera with specified IP address
     * @param camIP - a camera ip address
     * @return QSharedPointer of the image, if an image isn't found returns QSharedPointer of nullptr
     */
    const QSharedPointer<ISImage> getImage( quint32 camIP );


    /**
     * @brief Updates a client with specified IP address
     * @param ip - a client ip address
     * @param clientHendler - non-owning client pointer
     */
    void updateClientHendler( quint32 ip, ISClientHendler* clientHendler );

    /**
     * @brief Updates an image is received from the camera with specified IP address
     * @param camIP - a camera ip address
     * @param newImage - QSharedPointer of the image
     */
    void updateImages( quint32 camIP, QSharedPointer<ISImage> newImage );


    /**
     * @brief Returns ip addresses of all cameras from the container
     * @return vector of camera ip addresses
     */
    QVector<quint32> getCameras();


    /**
     * @brief Erases a client with specified IP address from the container
     * @param ip - a client ip address
     */
    void eraseClientHendler( quint32 ip );

private:
    QMutex clientsMutex;    // mutex for accessing the client storage
    QMap<quint32, ISClientHendler*> clients; // client storage: <ip, non-owning client pointer>

    QMutex imagesMutex;     // mutex for accessing the image storage
    QMap< quint32, QSharedPointer<ISImage> > images; // image storage: <ip, smart pointer to the last image>
};

#endif // SERVER_STORAGE_H
