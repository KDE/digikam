#include <qstring.h>

#include "thumbnailsize.h"

ThumbnailSize::ThumbnailSize(Size val)
{
    mSize = val;
}

ThumbnailSize::ThumbnailSize(const QString& size)
{
    if (size == "Small")
        mSize = Small;
    else if (size == "Medium")
        mSize = Medium;
    else
        mSize = Large;
}

int ThumbnailSize::getSizeAsNumber() const
{
    switch(mSize) {
    case Small:
        return 64;
    case Medium:
        return 90;
    case Large:
        return 120;
    default:
        return 120;
    }
}

ThumbnailSize::ThumbnailSize(const ThumbnailSize& thumbSize)
{
    if (this != &thumbSize)
        mSize = thumbSize.mSize;
}

ThumbnailSize& ThumbnailSize::operator=(const ThumbnailSize&
                                        thumbSize)
{
    if (this != &thumbSize) {
        mSize = thumbSize.mSize;
    }
    return *this;
}

bool ThumbnailSize::operator==(const ThumbnailSize& thumbSize)
{
    return (mSize == thumbSize.mSize);
}

bool ThumbnailSize::operator!=(const ThumbnailSize& thumbSize)
{
    return (mSize != thumbSize.mSize);
}

double ThumbnailSize::scaleFactor() const
{
    switch(mSize) {
    case Small:
        return (double(64)/double(120));
    case Medium:
        return (double(90)/double(120));
    case Large:
        return 1.0;
    default:
        return 1.0;
    }
}

void ThumbnailSize::setSize(Size size)
{
    switch(size) {
    case Small:
        mSize = size;
    case Medium:
        mSize = size;
    case Large:
        mSize = size;
    default:
        mSize = Large;
    }
}

void ThumbnailSize::setSize(const QString& size)
{
    if (size == "Small")
        mSize = Small;
    else if (size == "Medium")
        mSize = Medium;
    else
        mSize = Large;
}

