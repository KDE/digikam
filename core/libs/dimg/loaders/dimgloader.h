/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : DImg image loader interface
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes

#include <limits>

// Qt includes

#include <QMap>
#include <QString>
#include <QByteArray>
#include <QVariant>

// Local includes

#include "digikam_debug.h"
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

public:

    void setLoadFlags(LoadFlags flags);

    virtual ~DImgLoader();

    virtual bool load(const QString& filePath, DImgLoaderObserver* const observer) = 0;
    virtual bool save(const QString& filePath, DImgLoaderObserver* const observer) = 0;

    virtual bool hasLoadedData() const;
    virtual bool hasAlpha()      const = 0;
    virtual bool sixteenBit()    const = 0;
    virtual bool isReadOnly()    const = 0;

    static QByteArray     uniqueHashV2(const QString& filePath, const DImg* const img = 0);
    static QByteArray     uniqueHash(const QString& filePath, const DImg& img, bool loadMetadata);
    static HistoryImageId createHistoryImageId(const QString& filePath, const DImg& img, const DMetadata& metadata);

    static unsigned char*  new_failureTolerant(size_t unsecureSize);
    static unsigned char*  new_failureTolerant(quint64 w, quint64 h, uint typesPerPixel);
    static unsigned short* new_short_failureTolerant(size_t unsecureSize);
    static unsigned short* new_short_failureTolerant(quint64 w, quint64 h, uint typesPerPixel);

    /** Value returned : -1 : unsupported platform
     *                    0 : parse failure from supported platform
     *                    1 : parse done with success from supported platform
     */
    static qint64 checkAllocation(qint64 fullSize);

    template <typename Type> static Type* new_failureTolerant(size_t unsecureSize);
    template <typename Type> static Type* new_failureTolerant(quint64 w, quint64 h, uint typesPerPixel);

protected:

    explicit DImgLoader(DImg* const image);

    unsigned char*&         imageData();
    unsigned int&           imageWidth();
    unsigned int&           imageHeight();

    bool                    imageHasAlpha()   const;
    bool                    imageSixteenBit() const;

    int                     imageBitsDepth()  const;
    int                     imageBytesDepth() const;

    void                    imageSetIccProfile(const IccProfile& profile);
    QVariant                imageGetAttribute(const QString& key) const;
    void                    imageSetAttribute(const QString& key, const QVariant& value);

    QMap<QString, QString>& imageEmbeddedText()                      const;
    QString                 imageGetEmbbededText(const QString& key) const;
    void                    imageSetEmbbededText(const QString& key, const QString& text);

    void                    loadingFailed();
    bool                    checkExifWorkingColorSpace() const;
    void                    purgeExifWorkingColorSpace();
    void                    storeColorProfileInMetadata();

    virtual bool            readMetadata(const QString& filePath, DImg::FORMAT ff);
    virtual bool            saveMetadata(const QString& filePath);
    virtual int             granularity(DImgLoaderObserver* const observer, int total, float progressSlice = 1.0);

protected:

    DImg*     m_image;
    LoadFlags m_loadFlags;

private:

    DImgLoader();
};

// ---------------------------------------------------------------------------------------------------

/// Allows safe multiplication of requested pixel number and bytes per pixel, avoiding particularly
/// 32bit overflow and exceeding the size_t type
template <typename Type>
Q_INLINE_TEMPLATE Type* DImgLoader::new_failureTolerant(quint64 w, quint64 h, uint typesPerPixel)
{
    quint64 requested = w * h * quint64(typesPerPixel);
    quint64 maximum   = std::numeric_limits<size_t>::max();

    if (requested > maximum)
    {
        qCCritical(DIGIKAM_DIMG_LOG) << "Requested memory of" << requested*quint64(sizeof(Type))
                                        << "is larger than size_t supported by platform.";
        return 0;
    }

    return new_failureTolerant<Type>(requested);
}

template <typename Type>
Q_INLINE_TEMPLATE Type* DImgLoader::new_failureTolerant(size_t size)
{
    qint64 res = checkAllocation(size);

    switch(res)
    {
        case 0:       // parse failure from supported platform
            return 0;
            break;
        case -1:      // unsupported platform
            // We will try to continue to allocate
            break;
        default:     // parse done with success from supported platform
            break;
    }

    Type* reserved = 0;

    try
    {
        reserved = new Type[size];
    }
    catch (std::bad_alloc& ex)
    {
        qCCritical(DIGIKAM_DIMG_LOG) << "Failed to allocate chunk of memory of size" << size << ex.what();
        reserved = 0;
    }

    return reserved;
}

Q_DECLARE_OPERATORS_FOR_FLAGS(DImgLoader::LoadFlags)

}  // namespace Digikam

#endif /* DIMGLOADER_H */
