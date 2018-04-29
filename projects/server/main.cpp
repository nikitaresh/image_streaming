#include <QCoreApplication>
#include <server.h>

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    ISServer isServer;
    int port = 555;
    isServer.startServer(port);

    return a.exec();
}
