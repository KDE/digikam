/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2005-06-14
 * Description : digiKam 8/16 bits image management API
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
#include "dcolorcomposer.h"

class QString;
class QVariant;

namespace Digikam
{

class DImgPrivate;
class IccTransform;

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
        COM,    // JFIF comments section data.
        EXIF,   // EXIF meta-data.
        IPTC,   // IPTC meta-data.
        ICC     // ICC color profile.        
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

    /** Copy image: Creates a shallow copy that refers to the same shared data.
        The two images will be equal. Call detach() or copy() to create deep copies.
    */
    DImg(const DImg& image);

    /** Create image from data.
        If data is 0, a new buffer will be allocated, otherwise the given data will be used:
        If copydata is true, the data will be copied to a newly allocated buffer.
        If copyData is false, this DImg object will take ownership of the data pointer.

        If there is an alpha channel, the data shall be in non-premultiplied form (unassociated alpha).
    */
    DImg(uint width, uint height, bool sixteenBit, bool alpha=false, uchar* data = 0, bool copyData = true);

   ~DImg();

    /** Equivalent to the copy constructor */
    DImg&       operator=(const DImg& image);

    /** Detaches from shared data and makes sure that this image
        is the only one referring to the data.
        If multiple images share common data, this image makes a copy
        of the data and detaches itself from the sharing mechanism.
        Nothing is done if there is just a single reference.
    */
    void       detach();

    /** Returns whether two images are equal.
        Two images are equal if and only if they refer to the same shared data.
        (Thus, DImg() == DImg() is not true, both instances refer two their
         own shared data. image == DImg(image) is true.)
        If two or more images refer to the same data, they have the same
        image data, bits() returns the same data, they have the same metadata,
        and a change to one image also affects the others.
        Call detach() to split one image from the group of equal images.
    */
    bool        operator==(const DImg& image) const;



    /** Replaces image data of this object. Metadata is unchanged. Parameters like constructor above. */
    void        putImageData(uint width, uint height, bool sixteenBit, bool alpha, uchar *data, bool copyData = true);

    /** Overloaded function, provided for convenience, behaves essentially
        like the function above if data is not 0.
        Uses current width, height, sixteenBit, and alpha values.
        If data is 0, the current data is deleted and the image is set to null
        (But metadata unchanged).
    */
    void        putImageData(uchar *data, bool copyData = true);

    /** Reset metadata and image data to null image */
    void        reset(void);

    /** Reset metadata, but do not change image data */
    void        resetMetaData(void);

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
    QSize       size()           const;
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
    DColor      getPixelColor(uint x, uint y) const;
    void        setPixelColor(uint x, uint y, DColor color);

    /**
    Return true if the original image file format cannot be saved. 
    This is depending of DImgLoader::save() implementation. For example
    RAW file formats are supported by DImg uing dcraw than cannot support 
    writing operations.
    */
    bool       isReadOnly() const;

    /** Metadata manipulation methods */
    QByteArray getComments()  const;
    QByteArray getExif()      const;
    QByteArray getIptc()      const;
    QByteArray getICCProfil() const;
    void       setComments(const QByteArray& commentsData);
    void       setExif(const QByteArray& exifData);
    void       setIptc(const QByteArray& iptcData);
    void       setICCProfil(const QByteArray& profile);

    QByteArray metadata(METADATA key) const;

    bool       getICCProfilFromFile(const QString& filePath);
    bool       setICCProfilToFile(const QString& filePath);

    void       setAttribute(const QString& key, const QVariant& value);
    QVariant   attribute(const QString& key) const;

    void       setEmbeddedText(const QString& key, const QString& text);
    QString    embeddedText(const QString& key) const;

    /** Save/Get camera informations witch taking the pictures.*/
    void       setCameraModel(QString model);
    QString    cameraModel() const;

    void       setCameraConstructor(QString constructor);
    QString    cameraConstructor() const;



    /** Return a deep copy of full image */
    DImg       copy();

    /** Return a deep copy of the image, but do not include metadata. */
    DImg       copyImageData();

    /** Return an image that containes a deep copy of
        this image's metadata and the information associated
        with the image data (width, height, hasAlpha, sixteenBit),
        but no image data, i.e. isNull() is true.
    */
    DImg       copyMetaData();

    /** Return a region of image */
    DImg       copy(QRect rect);
    DImg       copy(int x, int y, int w, int h);




    /** Copy a region of pixels from a source image to this image.
        Parameters:
        sx|sy  Coordinates in the source image of the rectangle to be copied
        w h    Width and height of the rectangle (Default, or when both are -1: whole source image)
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

    /** Blend src image on this image (this is dest) with the specified composer
        and multiplication flags. See documentation of DColorComposer for more info.
        For the other arguments, see documentation of bitBltImage above.
    */
    void       bitBlendImage(DColorComposer *composer, const DImg* src,
                             int sx, int sy, int w, int h, int dx, int dy,
                             DColorComposer::MultiplicationFlags multiplicationFlags =
                             DColorComposer::NoMultiplication);

    /** QImage wrapper methods */
    QImage     copyQImage();
    QImage     copyQImage(QRect rect);
    QImage     copyQImage(int x, int y, int w, int h);

    /** Crop image to the specified region */
    void       crop(QRect rect);
    void       crop(int x, int y, int w, int h);

    /** Set width and height of this image, smoothScale it to the given size */
    void       resize(int w, int h);

    /** Return a version of this image scaled to the specified size with the specified mode.
        See QSize documentation for information on available modes
    */
    DImg       smoothScale(int width, int height, QSize::ScaleMode scaleMode=QSize::ScaleFree);

    /** Take the region specified by the rectangle sx|sy, width and height sw * sh,
        and scale it to an image with size dw * dh
    */
    DImg       smoothScaleSection(int sx, int sy, int sw, int sh,
                                  int dw, int dh);

    void       rotate(ANGLE angle);
    void       flip(FLIP direction);

    QPixmap    convertToPixmap();
    QPixmap    convertToPixmap(IccTransform* monitorICCtrans);

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
    static void bitBlt(const uchar *src, uchar *dest,
                         int sx, int sy, int w, int h, int dx, int dy,
                         uint swidth, uint sheight, uint dwidth, uint dheight,
                         bool sixteenBit, int sdepth, int ddepth);
    static void bitBlend(DColorComposer *composer, const uchar *src, uchar *dest,
                         int sx, int sy, int w, int h, int dx, int dy,
                         uint swidth, uint sheight, uint dwidth, uint dheight,
                         bool sixteenBit, int sdepth, int ddepth,
                         DColorComposer::MultiplicationFlags multiplicationFlags);
    static bool normalizeRegionArguments(int &sx, int &sy, int &w, int &h, int &dx, int &dy,
                                         uint swidth, uint sheight, uint dwidth, uint dheight);

    friend class DImgLoader;
};

}  // NameSpace Digikam

#endif /* DIMG_H */
