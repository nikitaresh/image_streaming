
#include <camera_client.h>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    CameraClient cameraClient;

    const QString ip = "192.168.3.30";
    const int port = 555;
    const int waitConnectionMsecs = 1000;
    cameraClient.connectToServer( ip, port, waitConnectionMsecs );

    return app.exec();
}