#ifndef IS_IMAGE_H
#define IS_IMAGE_H

#include <QScopedPointer>

/**
 * @brief Image container
 */
class ISImage
{
public:
    /**
     * @brief The only constructor of this image container
     * @param type - an image type
     * @param rows - a number image rows
     * @param cols - a number image cols
     * @param dataSize - num bites for image data
     */
    ISImage( qint32 type, qint32 rows, qint32 cols, qint32 dataSize );
    ~ISImage();

    /**
     * @brief Gets data pointer
     * @return data pointer
     */
    char* getData() const;

public:
    const qint32 type;      // an image type
    const qint32 rows;      // a number image rows
    const qint32 cols;      // a number image cols
    const qint32 dataSize;  // num bites for image data

private:
    ISImage();  // forbidden default constructor

private:
    QScopedPointer<char> data;  // an image data
};


#endif // IS_IMAGE_H
