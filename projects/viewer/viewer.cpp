
#include <viewer.h>
#include "ui_viewer.h"
#include <QScreen>
#include <QHostAddress>
#include <QMessageBox>


Viewer::Viewer( QWidget* parent )
    : QWidget(parent), ui(new Ui::Viewer), viewerClient(nullptr),
      isClientConnected(false), isCameraRunning(false), isImagesStream(false)
{
    if( ui != nullptr ) {
        ui->setupUi(this);
    }

    QScreen* primaryScreen = QApplication::primaryScreen();
    if( primaryScreen != nullptr && this->width() != 0 && this->height() != 0 )
    {
        int screenWidth = primaryScreen->availableGeometry().width();
        int screenHeight = primaryScreen->availableGeometry().height();
        double kx = screenWidth / (1.0 * this->width());
        double ky = screenHeight / (1.0 * this->height());
        transformQWidjet(ui->liveImageStream, kx, ky);
        transformQWidjet(ui->connectionButton, kx, ky);
        transformQWidjet(ui->camerasComboBox, kx, ky);
        transformQWidjet(ui->cameraGroupBox, kx, ky);
        transformQWidjet(ui->cameraFpsStringLineEdit, kx, ky);
        transformQWidjet(ui->runCameraButton, kx, ky);
        transformQWidjet(ui->streamFpsStringLineEdit, kx, ky);
        transformQWidjet(ui->startStreamingButton, kx, ky);
        transformQWidjet(ui->exitButton, kx, ky);

        this->resize( screenWidth, screenHeight );
        this->setMinimumSize( screenWidth, screenHeight );
        this->setMaximumSize( screenWidth, screenHeight );
    }

    qRegisterMetaType<QImage>("QImage");

    viewerClient = new ViewerClient();
    if( viewerClient == nullptr ) {
        return;
    }

    viewerClient->moveToThread(&viewerClientThread);

    bool isAllBind = true;
    isAllBind = isAllBind && connect( &viewerClientThread, &QThread::finished,
                                      viewerClient, &QObject::deleteLater );
    isAllBind = isAllBind && connect( this, &Viewer::signalConnectToServer,
                                      viewerClient, &ViewerClient::slotConnectToServer );
    isAllBind = isAllBind && connect( this, &Viewer::signalDisconnect,
                                      viewerClient, &ViewerClient::slotDisconnect );
    isAllBind = isAllBind && connect( this, &Viewer::signalRunCamera,
                                      viewerClient, &ViewerClient::slotRunCamera );
    isAllBind = isAllBind && connect( this, &Viewer::signalStopCamera,
                                      viewerClient, &ViewerClient::slotStopCamera );
    isAllBind = isAllBind && connect( this, &Viewer::signalStartStreaming,
                                      viewerClient, &ViewerClient::slotStartStreaming );
    isAllBind = isAllBind && connect( this, &Viewer::signalStopStreaming,
                                      viewerClient, &ViewerClient::slotStopStreaming );

    isAllBind = isAllBind && connect( viewerClient, &ViewerClient::signalConnected,
                                      this, &Viewer::slotClientConnected );
    isAllBind = isAllBind && connect( viewerClient, &ViewerClient::signalDisconnected,
                                      this, &Viewer::slotClientDisconnected );
    isAllBind = isAllBind && connect( viewerClient, &ViewerClient::signalNewImage,
                                      this, &Viewer::slotNewImage );
    isAllBind = isAllBind && connect( viewerClient, &ViewerClient::signalError,
                                      this, &Viewer::slotClientError );


    viewerClientThread.start();
}

Viewer::~Viewer()
{
    emit signalDisconnect();

    viewerClientThread.quit();
    viewerClientThread.wait();

    delete ui;
}


void Viewer::slotClientConnected()
{
    if( ui == nullptr || ui->camerasComboBox == nullptr || ui->connectionButton == nullptr ) {
        return;
    }

    isClientConnected = true;

    QVector<quint32> cameras = viewerClient->getCameras();
    for( int index = 0; index < cameras.size(); ++index ) {
        ui->camerasComboBox->addItem( QHostAddress(cameras[index]).toString(), cameras[index] );
    }

    ui->camerasComboBox->setEnabled(true);
    ui->connectionButton->setText("Disconnect");
}

void Viewer::slotClientDisconnected()
{
    bool isBadPtr = (ui == nullptr) || (ui->camerasComboBox == nullptr) ||
            (ui->cameraFpsStringLineEdit == nullptr) || (ui->runCameraButton == nullptr) ||
            (ui->streamFpsStringLineEdit == nullptr) || (ui->startStreamingButton == nullptr) ||
            (ui->connectionButton == nullptr);
    if( isBadPtr ) {
        return;
    }

    isClientConnected = false;

    ui->camerasComboBox->clear();
    ui->camerasComboBox->setEnabled(false);
    ui->cameraFpsStringLineEdit->setEnabled(false);
    ui->runCameraButton->setEnabled(false);
    ui->streamFpsStringLineEdit->setEnabled(false);
    ui->startStreamingButton->setEnabled(false);

    ui->connectionButton->setText("Connect");
    slotNewImage(QImage());

    isCameraRunning = false;
    isImagesStream = false;
    ui->runCameraButton->setText("Run Camera");
    ui->startStreamingButton->setText("Start Stream");
}

void Viewer::slotNewImage( const QImage& qImage )
{
    if( ui == nullptr || ui->liveImageStream == nullptr ) {
        return;
    }

    ui->liveImageStream->setPixmap( QPixmap::fromImage( qImage ) );
}

void Viewer::slotClientError( const QString& result )
{
    QMessageBox( QMessageBox::NoIcon, "slotClientError", result ).exec();
}

void Viewer::transformQWidjet( QWidget* obj, double kx, double ky )
{
    if( obj == nullptr ) {
        return;
    }

    QRect rect;
    rect.setX( static_cast<int>(kx * obj->x()) + 0.5 );
    rect.setY( static_cast<int>(ky * obj->y()) + 0.5 );
    rect.setWidth( static_cast<int>(kx * obj->width()) + 0.5 );
    rect.setHeight( static_cast<int>(ky * obj->height()) + 0.5 );

    obj->setGeometry( rect );
}


void Viewer::on_connectionButton_clicked()
{
    if( isClientConnected ) {
        emit signalDisconnect();
    }
    else
    {
        QString ip = "192.168.3.30";    // Change the value to your server IP address
        quint16 port = 555;             // Check a server port
        emit signalConnectToServer(ip, port);
    }

    isCameraRunning = false;
    isImagesStream = false;
}

void Viewer::on_camerasComboBox_activated(int index)
{
    bool isBadPtr = (ui == nullptr) || (ui->camerasComboBox == nullptr) ||
            (ui->cameraGroupBox == nullptr) || (ui->cameraFpsStringLineEdit == nullptr) ||
            (ui->runCameraButton == nullptr) || (ui->streamFpsStringLineEdit == nullptr) ||
            (ui->startStreamingButton == nullptr);
    if( isBadPtr ) {
        return;
    }

    int numElement = ui->camerasComboBox->count();

    if( index < 0 || index >= numElement ) {
        choosenIP = 0;
        ui->cameraGroupBox->setEnabled(false);
    }
    else
    {
        choosenIP = static_cast<quint32>(ui->camerasComboBox->itemData(index).toInt());
        ui->cameraGroupBox->setEnabled(true);
        ui->cameraFpsStringLineEdit->setEnabled(true);
        ui->runCameraButton->setEnabled(true);
        ui->streamFpsStringLineEdit->setEnabled(true);
        ui->startStreamingButton->setEnabled(true);
    }
}

void Viewer::on_runCameraButton_clicked()
{
    bool isBadPtr = (ui == nullptr) || (ui->runCameraButton == nullptr) ||
            (ui->cameraFpsStringLineEdit == nullptr);
    if( isBadPtr ) {
        return;
    }

    if( isCameraRunning )
    {
        emit signalStopCamera(choosenIP);
        ui->runCameraButton->setText("Run Camera");
    }
    else
    {
        qreal fps = ui->cameraFpsStringLineEdit->text().toFloat();
        emit signalRunCamera(choosenIP, fps);
        ui->runCameraButton->setText("Stop Camera");
    }

    isCameraRunning = !isCameraRunning;
}

void Viewer::on_startStreamingButton_clicked()
{
    bool isBadPtr = (ui == nullptr) || (ui->startStreamingButton == nullptr) ||
            (ui->streamFpsStringLineEdit == nullptr);
    if( isBadPtr ) {
        return;
    }

    if( isImagesStream )
    {
        emit signalStopStreaming();
        ui->startStreamingButton->setText("Start Stream");
    }
    else
    {
        qreal fps = ui->streamFpsStringLineEdit->text().toFloat();
        emit signalStartStreaming(choosenIP, fps);
        ui->startStreamingButton->setText("Stop Stream");
    }

    isImagesStream = !isImagesStream;
}

void Viewer::on_exitButton_clicked()
{
    QApplication::exit();
}
