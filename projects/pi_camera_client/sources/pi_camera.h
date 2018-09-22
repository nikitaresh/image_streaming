#ifndef PI_CAMERA_H
#define PI_CAMERA_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <opencv2/opencv.hpp>

#ifdef __linux__
    #include <raspicam.h>
#endif

/**
 * @brief Raspberry Pi Camera API based on raspicam API
 */
class PiCamera : public QObject
{
    Q_OBJECT
public:
    explicit PiCamera( QObject* parent = 0 );
    virtual ~PiCamera();

    /**
     * @brief Returns a current image
     * @return OpenCV Mat3b image, if there is no new image then returns an empty image container
     */
    cv::Mat3b getImage();

    /**
    * @brief Returns an image width and height
    */
    unsigned int getImageWidth() const;
    unsigned int getImageHeight() const;

public slots:
    /**
     * @brief Slot for starting Raspberry Pi camera
     * @param fps - a camera operating frequency
     */
    void slotStartCamera( double fps );

    /**
     * @brief Slot for stopping Raspberry Pi camera
     */
    void slotStopCamera();

signals:
    /**
     * @brief This signal is emitted when new image is available
     */
    void signalNewImage();

    /**
     * @brief This signal is emitted when an error occurred
     * @param errorMessage - a message description of an error
     */
    void signalError( const QString& errorMessage );

private slots:
    /**
     * @brief Grabs and retrieves an image from Raspberry Pi camera
     */
    void readImage();

private:
    const unsigned int imageWidth;      // width of images
    const unsigned int imageHeight;     // height of images
    cv::Mat3b currImage;                // a read image
    QMutex imageMutex;                  // a mutex for access to currImage
    double camFPS;                      // a camera operating frequency
    QTimer processTimer;                // a timer for continuous image reading
#ifdef __linux__
    raspicam::RaspiCam Camera;          // base class of raspicam API
#endif
};

#endif // PI_CAMERA_H