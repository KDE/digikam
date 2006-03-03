/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2005-06-14
 * Description : main DImg framework implementation
 *
 * Copyright 2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier 
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIMG_H
#define DIMG_H

// QT includes.

#include <qcstring.h>
#include <qsize.h>
#include <qrect.h>
#include <qimage.h>
#include <qpixmap.h>

// Local includes.

#include "digikam_export.h"
#include "rawdecodingsettings.h"
#include "dimgloaderobserver.h"
#include "dcolor.h"

class QString;
class QVariant;

namespace Digikam
{

class DImgPrivate;

class DIGIKAM_EXPORT DImg
{
public:

    enum FORMAT
    {
        NONE = 0,
        JPEG,
        PNG,
        TIFF,
        RAW,
        PPM,
        QIMAGE
    };

    enum METADATA
    {
        JPG_EXIF,
        JPG_IPTC,
        JPG_COM,
        TIF_TAG_ARTIST,
        TIF_TAG_COPYRIGHT,
        TIF_TAG_DATETIME,
        TIF_TAG_DOCUMENTNAME,
        TIF_TAG_HOSTCOMPUTER,
        TIF_TAG_IMAGEDESCRIPTION,
        TIF_TAG_INKNAMES,
        TIF_TAG_MAKE,
        TIF_TAG_MODEL,
        TIF_TAG_PAGENAME,
        TIF_TAG_SOFTWARE,
        TIF_TAG_TARGETPRINTER
    };

    enum ANGLE
    {
        ROT90,
        ROT180,
        ROT270
    };

    enum FLIP
    {
        HORIZONTAL,
        VERTICAL
    };

    /** Create null image */
    DImg();

    /** Load image */
    DImg(const QString& filePath, DImgLoaderObserver *observer = 0,
         RawDecodingSettings rawDecodingSettings=RawDecodingSettings());

    /** Copy image */
    DImg(const DImg& image);

    /** Create image from data.
        If data is 0, a new buffer will be allocated, otherwise the given data will be used:
        If copydata is true, the data will be copied to a newly allocated buffer.
        If copyData is false, this DImg object will take ownership of the data pointer.
    */
    DImg(uint width, uint height, bool sixteenBit, bool alpha=false, uchar* data = 0, bool copyData = true);

   ~DImg();

    DImg&       operator=(const DImg& image);

    /** Replaces image data of this object. Metadata is unchanged. Parameters like constructor above. */
    void        putImageData(uint width, uint height, bool sixteenBit, bool alpha, uchar *data, bool copyData = true);

    /** Reset to null image */
    void        reset(void);

    /** Returns the data of this image. 
        Ownership of the buffer is passed to the caller, this image will be null afterwards.
    */
    uchar*      stripImageData();

    bool        load(const QString& filePath, DImgLoaderObserver *observer = 0,
                     RawDecodingSettings rawDecodingSettings=RawDecodingSettings());

    bool        save(const QString& filePath, const QString& format, DImgLoaderObserver *observer = 0);

    bool        isNull()         const;
    uint        width()          const;
    uint        height()         const;
    uchar*      bits()           const;
    uchar*      scanLine(uint i) const;
    bool        hasAlpha()       const;
    bool        sixteenBit()     const;
    uint        numBytes()       const;

    /** Return the number of bytes depth of one pixel : 4 (non sixteenBit) or 8 (sixteen) */
    int         bytesDepth() const;

    /** Return the number of bits depth of one color component for one pixel : 8 (non sixteenBit) or 16 (sixteen) */
    int         bitsDepth()  const;

    /** Access a single pixel of the image.
        These functions add some safety checks and then use the methods from DColor.
        In optimized code working directly on the data,
        better use the inline methods from DColor.
    */
    DColor      getPixelColor(uint x, uint y);
    void        setPixelColor(uint x, uint y, DColor color);

    /**
    Return true if the original image file format cannot be saved. 
    This is depending of DImgLoader::save() implementation. For example
    RAW file formats are supported by DImg uing dcraw than cannot support 
    writing operations.
    */
    bool       isReadOnly() const;

    /** Metadata manipulation methods */
    QByteArray getExif() const;
    QByteArray getIptc() const;
    QByteArray metadata(METADATA key) const;
    QByteArray getICCProfil() const;

    void       setAttribute(const QString& key, const QVariant& value);
    QVariant   attribute(const QString& key);

    void       setEmbeddedText(const QString& key, const QString& text);
    QString    embeddedText(const QString& key);

    /** Save/Get camera informations witch taking the pictures.*/
    void       setCameraModel(QString model);
    QString    cameraModel();

    void       setCameraConstructor(QString constructor);
    QString    cameraConstructor();

    /** Return a deep copy of full image */
    DImg       copy();

    /** Return a region of image */
    DImg       copy(QRect rect);
    DImg       copy(uint x, uint y, uint w, uint h);

    /** Copy a region of pixels from a source image to this image.
        Parameters:
        sx|sy  Coordinates in the source image of the rectangle to be copied
        w h    Width and height of the rectangle (Default: whole source image)
        dx|dy  Coordinates in this image of the rectangle in which the region will be copied
               (Default: 0|0)
        The bit depth of source and destination must be identical.
    */
    void       bitBltImage(const DImg* src, int dx, int dy);
    void       bitBltImage(const DImg* src, int sx, int sy, int dx, int dy);
    void       bitBltImage(const DImg* src, int sx, int sy, int w, int h, int dx, int dy);
    void       bitBltImage(const uchar* src, int sx, int sy, int w, int h, int dx, int dy,
                           uint swidth, uint sheight, int sdepth);

    /** Merge a pixels region to an image using alpha channel */
    void       bitBlend_RGBA2RGB(DImg& region, int x, int y, int w, int h);

    /** QImage wrapper methods */
    QImage     copyQImage();
    QImage     copyQImage(QRect rect);
    QImage     copyQImage(uint x, uint y, uint w, uint h);

    /** Crop image to the specified region */
    void       crop(QRect rect);
    void       crop(int x, int y, int w, int h);

    /** Set width and height of this image, smoothScale it to the given size */
    void       resize(int w, int h);

    /** Return a version of this image scaled to the specified size with the specified mode.
        See QSize documentation for information on available modes
    */
    DImg       smoothScale(uint width, uint height, QSize::ScaleMode scaleMode=QSize::ScaleFree);

    /** Take the region specified by the rectangle sx|sy, width and height sw * sh,
        and scale it to an image with size dw * dh
    */
    DImg       smoothScaleSection(uint sx, uint sy, uint sw, uint sh,
                                  uint dw, uint dh);

    void       rotate(ANGLE angle);
    void       flip(FLIP direction);

    QPixmap    convertToPixmap();

    /** Detaches from shared image data and makes sure that this image
        is the only one referring to the data. 
        If multiple images share common data, this image makes a copy
        of the data and detaches itself from the sharing mechanism.
        Nothing is done if there is just a single reference.
    */
    void       detach();

    /** Convert depth of image. Depth is bytesDepth * bitsDepth.
        If depth is 32, converts to 8 bits,
        if depth is 64, converts to 16 bits.
    */
    void       convertDepth(int depth);

    /** Wrapper methods for convertDepth */
    void       convertToSixteenBit();
    void       convertToEightBit();
    void       convertToDepthOfImage(const DImg *otherImage);

    /** Fill whole image with specified color.
        The bit depth of the color must be identical to the depth of this image.
    */
    void       fill(DColor color);

private:

    DImgPrivate *m_priv;

private:

    FORMAT     fileFormat(const QString& filePath);
    void       copyMetaData(const DImgPrivate *src);
    void       copyImageData(const DImgPrivate *src);
    void       setImageData(bool null, uint width, uint height, bool sixteenBit, bool alpha);
    void       setImageDimension(uint width, uint height);
    int        allocateData();
    DImg(const DImg &image, int w, int h);
    static void bitBlt(const uchar *src, uchar *dest, int sx, int sy, int w, int h, int dx, int dy,
                       uint swidth, uint sheight, uint dwidth, uint dheight, int sdepth, int ddepth);

    friend class DImgLoader;
};

}  // NameSpace Digikam

#endif /* DIMG_H */
