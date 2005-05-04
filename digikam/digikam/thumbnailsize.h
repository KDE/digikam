#ifndef THUMBNAILSIZE_H
#define THUMBNAILSIZE_H

class ThumbnailSize {

public:

    enum Size {
        Tiny   = 32,
        Small  = 64,
        Medium = 100,
        Large  = 160,
        Huge   = 256
    };

    ThumbnailSize() {
        mSize = Medium;
    }

    ThumbnailSize(Size size) {
        mSize = size;
    }

    ~ThumbnailSize() {
    }

    ThumbnailSize(const ThumbnailSize& thumbsize) {
        if (this != &thumbsize)
            mSize = thumbsize.mSize;
    }

    ThumbnailSize& operator=(const ThumbnailSize& thumbsize) {
        if (this != &thumbsize)
            mSize = thumbsize.mSize;
        return *this;
    }

    bool operator!=(const ThumbnailSize& thumbsize) {
        return mSize != thumbsize.mSize;
    }

    Size size() const {
        return mSize;
    }

private:

    Size mSize;


};

#endif
