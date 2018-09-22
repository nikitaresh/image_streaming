
#include <pi_camera.h>

#ifdef __linux__
static void setupCamera( raspicam::RaspiCam& Camera, unsigned int width, unsigned int height )
{
    Camera.setWidth( width );
    Camera.setHeight( height );
    Camera.setBrightness( 50 );
    Camera.setSharpness( 0 );
    Camera.setContrast( 0 );
    Camera.setSaturation( 0 );
    Camera.setShutterSpeed( 0 );
    Camera.setISO( 400 );
    Camera.setExposureCompensation( 0 );
    Camera.setAWB_RB( 1, 1 );
}
#endif

PiCamera::PiCamera( QObject* parent )
    : imageWidth(320), imageHeight(240), camFPS(10), processTimer(this)
{

#ifdef __linux__
    setupCamera(Camera, imageWidth, imageHeight);
#endif

    bool isAllBind = true;
    isAllBind = isAllBind && connect( &processTimer, &QTimer::timeout,
                                      this, &PiCamera::readImage );

    processTimer.setSingleShot( false );
}

PiCamera::~PiCamera()
{
    slotStopCamera();
}

cv::Mat3b PiCamera::getImage()
{
    cv::Mat3b image;

    imageMutex.lock();
    image = currImage;
    currImage = cv::Mat3b();
    imageMutex.unlock();

    return image;
}

unsigned int PiCamera::getImageWidth() const
{
    return imageWidth;
}

unsigned int PiCamera::getImageHeight() const
{
    return imageHeight;
}

void PiCamera::slotStartCamera( double fps )
{
    if( fps == 0 ) {
        emit signalError( "slotStartCamera(...) error: fps == 0" );
        return;
    }

    slotStopCamera();

#ifdef __linux__
    if ( !Camera.open() ) {
        emit signalError( "slotStartCamera(...): error opening camera" );
        return;
    }

    if( Camera.getImageBufferSize() != 3 * imageWidth * imageHeight ) {
        emit signalError( "slotStartCamera(...) error: Camera.getImageBufferSize() != 3 * imageWidth * imageHeight" );
        return;
    }
#endif

    int msSleep = static_cast<int>( 1000.0 / camFPS + 0.5 );
    processTimer.start(msSleep);
}

void PiCamera::slotStopCamera()
{
    processTimer.stop();

#ifdef __linux__
    Camera.release();
#endif
}

void PiCamera::readImage()
{
    cv::Mat3b newImage(imageHeight, imageWidth);
#ifdef __linux__
    Camera.grab();
    Camera.retrieve( newImage.data );
#endif

    imageMutex.lock();
    currImage = newImage;
    imageMutex.unlock();

    emit signalNewImage();
}