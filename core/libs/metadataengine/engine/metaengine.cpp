/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface
 *               Exiv2: http://www.exiv2.org
 *               Exif : http://www.exif.org/Exif2-2.PDF
 *               Iptc : http://www.iptc.org/std/IIM/4.1/specification/IIMV4.1.pdf
 *               Xmp  : http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf
 *                      http://www.iptc.org/std/Iptc4xmpCore/1.0/specification/Iptc4xmpCore_1.0-spec-XMPSchema_8.pdf
 *               Paper: http://www.metadataworkinggroup.com/pdf/mwg_guidance.pdf
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "metaengine.h"
#include "metaengine_p.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"

namespace Digikam
{

MetaEngine::MetaEngine()
    : d(new Private)
{
}

MetaEngine::MetaEngine(const MetaEngine& metadata)
    : d(new Private)
{
    d->copyPrivateData(metadata.d);
}

MetaEngine::MetaEngine(const MetaEngineData& data)
    : d(new Private)
{
    setData(data);
}

MetaEngine::MetaEngine(const QString& filePath)
    : d(new Private)
{
    load(filePath);
}

MetaEngine::~MetaEngine()
{
    delete d;
}

MetaEngine& MetaEngine::operator=(const MetaEngine& metadata)
{
    d->copyPrivateData(metadata.d);

    return *this;
}

//-- Statics methods ----------------------------------------------

bool MetaEngine::initializeExiv2()
{
#ifdef _XMP_SUPPORT_

    if (!Exiv2::XmpParser::initialize())
        return false;

    registerXmpNameSpace(QLatin1String("http://ns.adobe.com/lightroom/1.0/"),  QLatin1String("lr"));
    registerXmpNameSpace(QLatin1String("http://www.digikam.org/ns/kipi/1.0/"), QLatin1String("kipi"));
    registerXmpNameSpace(QLatin1String("http://ns.microsoft.com/photo/1.2/"),  QLatin1String("MP"));
    registerXmpNameSpace(QLatin1String("http://ns.acdsee.com/iptc/1.0/"),      QLatin1String("acdsee"));
    registerXmpNameSpace(QLatin1String("http://www.video"),                    QLatin1String("video"));

#endif // _XMP_SUPPORT_

    return true;
}

bool MetaEngine::cleanupExiv2()
{
    // Fix memory leak if Exiv2 support XMP.
#ifdef _XMP_SUPPORT_

    unregisterXmpNameSpace(QLatin1String("http://ns.adobe.com/lightroom/1.0/"));
    unregisterXmpNameSpace(QLatin1String("http://www.digikam.org/ns/kipi/1.0/"));
    unregisterXmpNameSpace(QLatin1String("http://ns.microsoft.com/photo/1.2/"));
    unregisterXmpNameSpace(QLatin1String("http://ns.acdsee.com/iptc/1.0/"));
    unregisterXmpNameSpace(QLatin1String("http://www.video"));

    Exiv2::XmpParser::terminate();

#endif // _XMP_SUPPORT_

    return true;
}

bool MetaEngine::supportXmp()
{
#ifdef _XMP_SUPPORT_
    return true;
#else
    return false;
#endif // _XMP_SUPPORT_
}

bool MetaEngine::supportMetadataWritting(const QString& typeMime)
{
    if (typeMime == QLatin1String("image/jpeg"))
    {
        return true;
    }
    else if (typeMime == QLatin1String("image/tiff"))
    {
        return true;
    }
    else if (typeMime == QLatin1String("image/png"))
    {
        return true;
    }
    else if (typeMime == QLatin1String("image/jp2"))
    {
        return true;
    }
    else if (typeMime == QLatin1String("image/x-raw"))
    {
        return true;
    }
    else if (typeMime == QLatin1String("image/pgf"))
    {
        return true;
    }

    return false;
}

QString MetaEngine::Exiv2Version()
{
#if EXIV2_TEST_VERSION(0,27,0)
    return QLatin1String(Exiv2::versionString().c_str());
#else
    return QLatin1String(Exiv2::version());
#endif
}

//-- General methods ----------------------------------------------

MetaEngineData MetaEngine::data() const
{
    MetaEngineData data;
    data.d = d->data;
    return data;
}

void MetaEngine::setData(const MetaEngineData& data)
{
    if (data.d)
    {
        d->data = data.d;
    }
    else
    {
        // MetaEngineData can have a null pointer,
        // but we never want a null pointer in Private.
        d->data->clear();
    }
}

bool MetaEngine::loadFromData(const QByteArray& imgData)
{
    if (imgData.isEmpty())
        return false;

    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((Exiv2::byte*)imgData.data(), imgData.size());

        d->filePath.clear();
        image->readMetadata();

        // Size and mimetype ---------------------------------

        d->pixelSize = QSize(image->pixelWidth(), image->pixelHeight());
        d->mimeType  = QLatin1String(image->mimeType().c_str());

        // Image comments ---------------------------------

        d->imageComments() = image->comment();

        // Exif metadata ----------------------------------

        d->exifMetadata() = image->exifData();

        // Iptc metadata ----------------------------------

        d->iptcMetadata() = image->iptcData();

#ifdef _XMP_SUPPORT_

        // Xmp metadata -----------------------------------

        d->xmpMetadata() = image->xmpData();

#endif // _XMP_SUPPORT_

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot load metadata using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::isEmpty() const
{
    if (!hasComments() && !hasExif() && !hasIptc() && !hasXmp())
        return true;

    return false;
}

QSize MetaEngine::getPixelSize() const
{
    return d->pixelSize;
}

QString MetaEngine::getMimeType() const
{
    return d->mimeType;
}

void MetaEngine::setWriteRawFiles(const bool on)
{
    d->writeRawFiles = on;
}

bool MetaEngine::writeRawFiles() const
{
    return d->writeRawFiles;
}

void MetaEngine::setUseXMPSidecar4Reading(const bool on)
{
    d->useXMPSidecar4Reading = on;
}

bool MetaEngine::useXMPSidecar4Reading() const
{
    return d->useXMPSidecar4Reading;
}

void MetaEngine::setMetadataWritingMode(const int mode)
{
    d->metadataWritingMode = mode;
}

int MetaEngine::metadataWritingMode() const
{
    return d->metadataWritingMode;
}

void MetaEngine::setUpdateFileTimeStamp(bool on)
{
    d->updateFileTimeStamp = on;
}

bool MetaEngine::updateFileTimeStamp() const
{
    return d->updateFileTimeStamp;
}

bool MetaEngine::setProgramId() const
{
    QString version(digiKamVersion());
    QLatin1String software("digiKam");
    return setImageProgramId(software, version);
}

} // namespace Digikam
