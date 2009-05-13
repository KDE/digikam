/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : DImg image loader interface
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QMap>
#include <QString>
#include <QByteArray>
#include <QVariant>

// Local includes

#include "digikam_export.h"
#include "dimg.h"

namespace Digikam
{

class DImgLoaderObserver;

class DImgLoader
{
public:

    enum LoadFlag
    {
        LoadImageInfo  = 1,
        LoadMetadata   = 2,
        LoadICCData    = 4,
        LoadImageData  = 8,
        LoadUniqueHash = 16,
        LoadAll        = LoadImageInfo | LoadMetadata | LoadUniqueHash | LoadICCData | LoadImageData
    };
    Q_DECLARE_FLAGS(LoadFlags, LoadFlag)

    virtual ~DImgLoader() {};

    void setLoadFlags(LoadFlags flags);

    virtual bool load(const QString& filePath, DImgLoaderObserver *observer) = 0;
    virtual bool save(const QString& filePath, DImgLoaderObserver *observer) = 0;

    virtual bool hasAlpha()   const = 0;
    virtual bool sixteenBit() const = 0;
    virtual bool isReadOnly() const = 0;
    virtual bool hasLoadedData() const;

    static QByteArray uniqueHash(const QString& filePath, const DImg& img, bool loadMetadata);

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
    LoadFlags               m_loadFlags;

private:

    DImgLoader();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DImgLoader::LoadFlags)

}  // namespace Digikam

#endif /* DIMGLOADER_H */
