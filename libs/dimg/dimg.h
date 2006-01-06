/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-06-14
 * Description : main DImg framework implementation
 *
 * Copyright 2005 by Renchi Raju, Gilles Caulier
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
 * ============================================================ */

#ifndef DIMG_H
#define DIMG_H

// QT includes.

#include <qcstring.h>
#include <qrect.h>
#include <qimage.h>
#include <qpixmap.h>

// Local includes.

#include "digikam_export.h"
#include "rawdecodingsettings.h"
#include "dimgloaderobserver.h"

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

    DImg();
    
    DImg(const QString& filePath, DImgLoaderObserver *observer = 0,
         RawDecodingSettings rawDecodingSettings=RawDecodingSettings());
         
    DImg(const DImg& image);
    DImg(uint width, uint height, bool sixteenBit, bool alpha=false, uchar* data = 0);
   ~DImg();
    
    DImg& operator=(const DImg& image);

    void        reset(void);

    bool        load(const QString& filePath, DImgLoaderObserver *observer = 0,
                     RawDecodingSettings rawDecodingSettings=RawDecodingSettings());
                     
    bool        save(const QString& filePath, const char* format, DImgLoaderObserver *observer = 0);

    bool        isNull()     const;
    uint        width()      const;
    uint        height()     const;
    uchar*      bits()       const;
    bool        hasAlpha()   const;
    bool        sixteenBit() const;
    uint        numBytes()   const;

    /** Return the number of bytes depth of one pixel : 4 (non sixteenBit) or 8 (sixteen) */
    int         bytesDepth() const;

    /** Return the number of bits depth of one color component for one pixel : 8 (non sixteenBit) or 16 (sixteen) */
    int         bitsDepth()  const;

    /**
    Return true if the original image file format cannot be saved. 
    This is depending of DImgLoader::save() implementation. For example
    RAW file formats are supported by DImg uing dcraw than cannot support 
    writing operations.
    */
    bool       isReadOnly() const;

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

    /** Copy a pixels region to an image */
    void       bitBltImage(DImg* src, int dx, int dy);

    /** Merge a pixels region to an image */
    void       bitBlend(DImg& region, int x, int y, int w, int h);

    /** QImage wrapper methods */
    QImage     copyQImage();
    QImage     copyQImage(QRect rect);
    QImage     copyQImage(uint x, uint y, uint w, uint h);

    /** Cropping image methods */
    void       crop(QRect rect);
    void       crop(int x, int y, int w, int h);

    void       resize(int w, int h);

    DImg       smoothScale(uint width, uint height);
    DImg       smoothScaleSection(uint sx, uint sy, uint sw, uint sh,
                                  uint dw, uint dh);

    void       rotate(ANGLE angle);
    void       flip(FLIP direction);

    QPixmap    convertToPixmap();

    void       detach();

    void       convertDepth(int depth);

private:

    DImgPrivate *m_priv;

private:

    FORMAT fileFormat(const QString& filePath);

    friend class DImgLoader;
};

}  // NameSpace Digikam

#endif /* DIMG_H */
