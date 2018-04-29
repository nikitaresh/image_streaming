
#include <isimage.h>
#include <QMutex>


ISImage::ISImage( qint32 type_, qint32 rows_, qint32 cols_, qint32 dataSize_ )
    : type(type_), rows(rows_), cols(cols_), dataSize(dataSize_), data(new char[dataSize])
{
}

ISImage::~ISImage()
{
}

char* ISImage::getData() const
{
    return data.data();
}
