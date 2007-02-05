/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-06-14
 * Description : DImg image loader interface
 *
 * Copyright 2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier 
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

#ifndef DIMGLOADER_H
#define DIMGLOADER_H

// Qt includes.

#include <qmap.h>
#include <qstring.h>
#include <qcstring.h>
#include <qvariant.h>

// Local includes.

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class DImgLoaderObserver;

class DIGIKAM_EXPORT DImgLoader
{
public:

    virtual ~DImgLoader() {};

    virtual bool load(const QString& filePath, DImgLoaderObserver *observer) = 0;
    virtual bool save(const QString& filePath, DImgLoaderObserver *observer) = 0;

    virtual bool hasAlpha()   const = 0;
    virtual bool sixteenBit() const = 0;
    virtual bool isReadOnly() const = 0;

protected:

    DImgLoader(DImg* image);

    unsigned char*&         imageData();
    unsigned int&           imageWidth();
    unsigned int&           imageHeight();

    bool                    imageHasAlpha();
    bool                    imageSixteenBit();

    int                     imageBitsDepth();
    int                     imageBytesDepth();

    QMap<int, QByteArray>&  imageMetaData();
    QVariant                imageGetAttribute(const QString& key);
    void                    imageSetAttribute(const QString& key, const QVariant& value);

    QMap<QString, QString>& imageEmbeddedText();    
    QString                 imageGetEmbbededText(const QString& key);
    void                    imageSetEmbbededText(const QString& key, const QString& text);

    virtual bool            readMetadata(const QString& filePath, DImg::FORMAT ff);
    virtual bool            saveMetadata(const QString& filePath);
    virtual int             granularity(DImgLoaderObserver *observer, int total, float progressSlice = 1.0);

    bool                    checkExifWorkingColorSpace();

protected:

    DImg                   *m_image;

private:

    DImgLoader();
};

}  // NameSpace Digikam

#endif /* DIMGLOADER_H */
