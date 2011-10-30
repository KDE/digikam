/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : DImg image loader interface
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
class DMetadata;

class DImgLoader
{
public:

    enum LoadFlag
    {
        LoadImageInfo    = 1,
        LoadMetadata     = 2,
        LoadICCData      = 4,
        LoadImageData    = 8,
        LoadUniqueHash   = 16,
        LoadImageHistory = 32,
        LoadAll          = LoadImageInfo | LoadMetadata | LoadUniqueHash | LoadICCData | LoadImageData | LoadImageHistory
    };
    Q_DECLARE_FLAGS(LoadFlags, LoadFlag)

    virtual ~DImgLoader() {};

    void setLoadFlags(LoadFlags flags);

    virtual bool load(const QString& filePath, DImgLoaderObserver* observer) = 0;
    virtual bool save(const QString& filePath, DImgLoaderObserver* observer) = 0;

    virtual bool hasAlpha()      const = 0;
    virtual bool sixteenBit()    const = 0;
    virtual bool isReadOnly()    const = 0;
    virtual bool hasLoadedData() const;

    static QByteArray uniqueHashV2(const QString& filePath, const DImg* img = 0);
    static QByteArray uniqueHash(const QString& filePath, const DImg& img, bool loadMetadata);
    static HistoryImageId createHistoryImageId(const QString& filePath, const DImg& img, const DMetadata& metadata);

    static unsigned char*   new_failureTolerant(size_t unsecureSize);
    static unsigned short*  new_short_failureTolerant(size_t unsecureSize);

    static int checkAllocation(qint64 fullSize);

protected:

    DImgLoader(DImg* image);

    unsigned char*&         imageData();
    unsigned int&           imageWidth();
    unsigned int&           imageHeight();

    bool                    imageHasAlpha() const;
    bool                    imageSixteenBit() const;

    int                     imageBitsDepth() const;
    int                     imageBytesDepth() const;

    void                    imageSetIccProfile(const IccProfile& profile);
    QVariant                imageGetAttribute(const QString& key);
    void                    imageSetAttribute(const QString& key, const QVariant& value);

    QMap<QString, QString>& imageEmbeddedText();
    KExiv2Data              imageMetadata();
    QString                 imageGetEmbbededText(const QString& key);
    void                    imageSetEmbbededText(const QString& key, const QString& text);

    virtual bool            readMetadata(const QString& filePath, DImg::FORMAT ff);
    virtual bool            saveMetadata(const QString& filePath);
    void                    loadingFailed();
    virtual int             granularity(DImgLoaderObserver* observer, int total, float progressSlice = 1.0);

    bool                    checkExifWorkingColorSpace();

protected:

    DImg*                   m_image;
    LoadFlags               m_loadFlags;

private:

    DImgLoader();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DImgLoader::LoadFlags)

}  // namespace Digikam

#endif /* DIMGLOADER_H */
