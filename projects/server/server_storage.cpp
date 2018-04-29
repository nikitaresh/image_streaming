
#include <server_storage.h>
#include <client_hendler.h>


ServerStorage::ServerStorage()
{
}

ServerStorage::~ServerStorage()
{
}

ISClientHendler* ServerStorage::getClientHendler( quint32 ip )
{
    clientsMutex.lock();
    QMap<quint32, ISClientHendler*>::iterator it = clients.find( ip );
    clientsMutex.unlock();
    if( it == clients.end() ) {
        return nullptr;
    }

    return it.value();
}

const QSharedPointer<ISImage> ServerStorage::getImage( quint32 camIP )
{
    imagesMutex.lock();
    auto it = images.find( camIP );
    imagesMutex.unlock();
    if( it == images.end() ) {
        return QSharedPointer<ISImage>();
    }

    return it.value();
}

void ServerStorage::updateClientHendler( quint32 ip, ISClientHendler* clientHendler )
{
    clientsMutex.lock();
    auto it = clients.find( ip );
    if( it == clients.end() ) {
        clients.insert(ip, clientHendler);
    }
    else {
        it.value() = clientHendler;
    }
    clientsMutex.unlock();
}

void ServerStorage::updateImages( quint32 camIP, QSharedPointer<ISImage> newImage )
{
    imagesMutex.lock();
    auto it = images.find( camIP );
    if( it == images.end() ) {
        images.insert(camIP, newImage);
    }
    else {
        it.value() = newImage;
    }
    imagesMutex.unlock();
}

QVector<quint32> ServerStorage::getCameras()
{
    QVector<quint32> cameras;
    clientsMutex.lock();
    for( auto it = clients.begin(); it != clients.end(); ++it )
    {
        if( it.value()->getClientType() == CT_CAMERA ) {
            cameras.push_back(it.key());
        }
    }
    clientsMutex.unlock();

    return cameras;
}

void ServerStorage::eraseClientHendler( quint32 ip )
{
    clientsMutex.lock();
    clients.remove(ip);
    clientsMutex.unlock();

    imagesMutex.lock();
    images.remove(ip);
    imagesMutex.unlock();
}
