/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011      by Leif Huhn <leif at dkstat dot com>
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

#include "dmetadata.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QUuid>

// Local includes

#include "filereadwritelock.h"
#include "metaenginesettings.h"
#include "template.h"
#include "dimg.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

DMetadata::DMetadata()
    : MetaEngine()
{
    registerMetadataSettings();
}

DMetadata::DMetadata(const QString& filePath)
    : MetaEngine()
{
    registerMetadataSettings();
    load(filePath);
}

DMetadata::DMetadata(const MetaEngineData& data)
    : MetaEngine(data)
{
    registerMetadataSettings();
}

DMetadata::~DMetadata()
{
}

void DMetadata::registerMetadataSettings()
{
    setSettings(MetaEngineSettings::instance()->settings());
}

void DMetadata::setSettings(const MetaEngineSettingsContainer& settings)
{
    setUseXMPSidecar4Reading(settings.useXMPSidecar4Reading);
    setWriteRawFiles(settings.writeRawFiles);
    setMetadataWritingMode(settings.metadataWritingMode);
    setUpdateFileTimeStamp(settings.updateFileTimeStamp);
}

bool DMetadata::load(const QString& filePath)
{
    // In first, we trying to get metadata using Exiv2,
    // else we will use other engine to extract minimal information.

    FileReadLocker lock(filePath);

    if (!MetaEngine::load(filePath))
    {
        if (!loadUsingRawEngine(filePath))
        {
            if (!loadUsingFFmpeg(filePath))
            {
                return false;
            }
        }
    }

    return true;
}

bool DMetadata::save(const QString& filePath, bool setVersion) const
{
    FileWriteLocker lock(filePath);
    return MetaEngine::save(filePath, setVersion);
}

bool DMetadata::applyChanges(bool setVersion) const
{
    FileWriteLocker lock(getFilePath());
    return MetaEngine::applyChanges(setVersion);
}

int DMetadata::getMSecsInfo() const
{
    int ms  = 0;
    bool ok = mSecTimeStamp("Exif.Photo.SubSecTime", ms);
    if (ok) return ms;

    ok      = mSecTimeStamp("Exif.Photo.SubSecTimeOriginal", ms);
    if (ok) return ms;

    ok      = mSecTimeStamp("Exif.Photo.SubSecTimeDigitized", ms);
    if (ok) return ms;

    return 0;
}

bool DMetadata::mSecTimeStamp(const char* const exifTagName, int& ms) const
{
    bool ok     = false;
    QString val = getExifTagString(exifTagName);

    if (!val.isEmpty())
    {
        int sub = val.toUInt(&ok);

        if (ok)
        {
            int _ms = (int)(QString::fromLatin1("0.%1").arg(sub).toFloat(&ok) * 1000.0);

            if (ok)
            {
                ms = _ms;
                qCDebug(DIGIKAM_METAENGINE_LOG) << "msec timestamp: " << ms;
            }
        }
    }

    return ok;
}

bool DMetadata::setImageHistory(QString& imageHistoryXml) const
{
    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.digiKam.ImageHistory", imageHistoryXml))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}

QString DMetadata::getImageHistory() const
{
    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.digiKam.ImageHistory", false);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Loading image history " << value;
        return value;
    }

    return QString();
}

bool DMetadata::hasImageHistoryTag() const
{
    if (hasXmp())
    {
        if (QString(getXmpTagString("Xmp.digiKam.ImageHistory", false)).length() > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

QString DMetadata::getImageUniqueId() const
{
    QString exifUid;

    if (hasXmp())
    {
        QString uuid = getXmpTagString("Xmp.digiKam.ImageUniqueID");

        if (!uuid.isEmpty())
        {
            return uuid;
        }

        exifUid = getXmpTagString("Xmp.exif.ImageUniqueId");
    }

    if (exifUid.isEmpty())
    {
        exifUid = getExifTagString("Exif.Photo.ImageUniqueID");
    }

    // same makers may choose to use a "click counter" to generate the id,
    // which is then weak and not a universally unique id
    // The Exif ImageUniqueID is 128bit, or 32 hex digits.
    // If the first 20 are zero, it's probably a counter,
    // the left 12 are sufficient for more then 10^14 clicks.
    if (!exifUid.isEmpty() && !exifUid.startsWith(QLatin1String("00000000000000000000")))
    {
        if (getExifTagString("Exif.Image.Make").contains(QLatin1String("SAMSUNG"), Qt::CaseInsensitive))
        {
            // Generate for Samsung a new random 32 hex digits unique ID.
            QString imageUniqueID(QUuid::createUuid().toString());
            imageUniqueID.remove(QLatin1Char('-'));
            imageUniqueID.remove(0, 1).chop(1);

            return imageUniqueID;
        }

        return exifUid;
    }

    // Exif.Image.ImageID can also be a pathname, so it's not sufficiently unique

    QString dngUid = getExifTagString("Exif.Image.RawDataUniqueID");

    if (!dngUid.isEmpty())
    {
        return dngUid;
    }

    return QString();
}

bool DMetadata::setImageUniqueId(const QString& uuid) const
{
    if (supportXmp())
    {
        return setXmpTagString("Xmp.digiKam.ImageUniqueID", uuid);
    }

    return false;
}

PhotoInfoContainer DMetadata::getPhotographInformation() const
{
    PhotoInfoContainer photoInfo;

    if (hasExif() || hasXmp())
    {
        photoInfo.dateTime = getImageDateTime();

        // -----------------------------------------------------------------------------------

        photoInfo.make     = getExifTagString("Exif.Image.Make");

        if (photoInfo.make.isEmpty())
        {
            photoInfo.make = getXmpTagString("Xmp.tiff.Make");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.model    = getExifTagString("Exif.Image.Model");

        if (photoInfo.model.isEmpty())
        {
            photoInfo.model = getXmpTagString("Xmp.tiff.Model");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.lens     = getLensDescription();

        // -----------------------------------------------------------------------------------

        photoInfo.aperture = getExifTagString("Exif.Photo.FNumber");

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getExifTagString("Exif.Photo.ApertureValue");
        }

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getXmpTagString("Xmp.exif.FNumber");
        }

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getXmpTagString("Xmp.exif.ApertureValue");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureTime = getExifTagString("Exif.Photo.ExposureTime");

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getExifTagString("Exif.Photo.ShutterSpeedValue");
        }

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getXmpTagString("Xmp.exif.ExposureTime");
        }

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getXmpTagString("Xmp.exif.ShutterSpeedValue");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureMode    = getExifTagString("Exif.Photo.ExposureMode");

        if (photoInfo.exposureMode.isEmpty())
        {
            photoInfo.exposureMode = getXmpTagString("Xmp.exif.ExposureMode");
        }

        if (photoInfo.exposureMode.isEmpty())
        {
            photoInfo.exposureMode = getExifTagString("Exif.CanonCs.MeteringMode");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureProgram = getExifTagString("Exif.Photo.ExposureProgram");

        if (photoInfo.exposureProgram.isEmpty())
        {
            photoInfo.exposureProgram = getXmpTagString("Xmp.exif.ExposureProgram");
        }

        if (photoInfo.exposureProgram.isEmpty())
        {
            photoInfo.exposureProgram = getExifTagString("Exif.CanonCs.ExposureProgram");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.focalLength     = getExifTagString("Exif.Photo.FocalLength");

        if (photoInfo.focalLength.isEmpty())
        {
            photoInfo.focalLength = getXmpTagString("Xmp.exif.FocalLength");
        }

        if (photoInfo.focalLength.isEmpty())
        {
            photoInfo.focalLength = getExifTagString("Exif.Canon.FocalLength");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.focalLength35mm = getExifTagString("Exif.Photo.FocalLengthIn35mmFilm");

        if (photoInfo.focalLength35mm.isEmpty())
        {
            photoInfo.focalLength35mm = getXmpTagString("Xmp.exif.FocalLengthIn35mmFilm");
        }

        // -----------------------------------------------------------------------------------

        QStringList ISOSpeedTags;

        ISOSpeedTags << QLatin1String("Exif.Photo.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Exif.Photo.ExposureIndex");
        ISOSpeedTags << QLatin1String("Exif.Image.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Xmp.exif.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Xmp.exif.ExposureIndex");
        ISOSpeedTags << QLatin1String("Exif.CanonSi.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.CanonCs.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon1.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon2.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon3.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.NikonIi.ISO");
        ISOSpeedTags << QLatin1String("Exif.NikonIi.ISO2");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCsNew.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCsOld.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCs5D.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCs7D.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Sony1Cs.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony2Cs.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony1Cs2.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony2Cs2.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony1MltCsA100.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Pentax.ISO");
        ISOSpeedTags << QLatin1String("Exif.Olympus.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Samsung2.ISO");

        photoInfo.sensitivity = getExifTagStringFromTagsList(ISOSpeedTags);

        // -----------------------------------------------------------------------------------

        photoInfo.flash = getExifTagString("Exif.Photo.Flash");

        if (photoInfo.flash.isEmpty())
        {
            photoInfo.flash = getXmpTagString("Xmp.exif.Flash");
        }

        if (photoInfo.flash.isEmpty())
        {
            photoInfo.flash = getExifTagString("Exif.CanonCs.FlashActivity");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.whiteBalance = getExifTagString("Exif.Photo.WhiteBalance");

        if (photoInfo.whiteBalance.isEmpty())
        {
            photoInfo.whiteBalance = getXmpTagString("Xmp.exif.WhiteBalance");
        }

        // -----------------------------------------------------------------------------------

        double l, L, a;
        photoInfo.hasCoordinates = getGPSInfo(a, l, L);
    }

    return photoInfo;
}

bool DMetadata::getImageTagsPath(QStringList& tagsPath,
                                 const DMetadataSettingsContainer& settings) const
{
    for (NamespaceEntry entry : settings.getReadMapping(QString::fromUtf8(DM_TAG_CONTAINER)))
    {
        if (entry.isDisabled)
            continue;

        int index                                  = 0;
        QString currentNamespace                   = entry.namespaceName;
        NamespaceEntry::SpecialOptions currentOpts = entry.specialOpts;

        // Some namespaces have altenative paths, we must search them both

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:

                while(index < 2)
                {
                    const std::string myStr = currentNamespace.toStdString();
                    const char* nameSpace   = myStr.data();

                    switch(currentOpts)
                    {
                        case NamespaceEntry::TAG_XMPBAG:
                            tagsPath = getXmpTagStringBag(nameSpace, false);
                            break;
                        case NamespaceEntry::TAG_XMPSEQ:
                            tagsPath = getXmpTagStringSeq(nameSpace, false);
                            break;
                        case NamespaceEntry::TAG_ACDSEE:
                            getACDSeeTagsPath(tagsPath);
                            break;
                        // not used here, to suppress warnings
                        case NamespaceEntry::COMMENT_XMP:
                        case NamespaceEntry::COMMENT_ALTLANG:
                        case NamespaceEntry::COMMENT_ATLLANGLIST:
                        case NamespaceEntry::NO_OPTS:
                        default:
                            break;
                    }

                    if (!tagsPath.isEmpty())
                    {
                        if (entry.separator != QLatin1String("/"))
                        {
                            tagsPath = tagsPath.replaceInStrings(entry.separator, QLatin1String("/"));
                        }

                        return true;
                    }
                    else if (!entry.alternativeName.isEmpty())
                    {
                        currentNamespace = entry.alternativeName;
                        currentOpts      = entry.secondNameOpts;
                    }
                    else
                    {
                        break; // no alternative namespace, go to next one
                    }

                    index++;
                }

                break;

            case NamespaceEntry::IPTC:
                // Try to get Tags Path list from IPTC keywords.
                // digiKam 0.9.x has used IPTC keywords to store Tags Path list.
                // This way is obsolete now since digiKam support XMP because IPTC
                // do not support UTF-8 and have strings size limitation. But we will
                // let the capability to import it for interworking issues.
                tagsPath = getIptcKeywords();

                if (!tagsPath.isEmpty())
                {
                    // Work around to Imach tags path list hosted in IPTC with '.' as separator.
                    QStringList ntp = tagsPath.replaceInStrings(entry.separator, QLatin1String("/"));

                    if (ntp != tagsPath)
                    {
                        tagsPath = ntp;
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "Tags Path imported from Imach: " << tagsPath;
                    }

                    return true;
                }

                break;

            case NamespaceEntry::EXIF:
            {
                // Try to get Tags Path list from Exif Windows keywords.
                QString keyWords = getExifTagString("Exif.Image.XPKeywords", false);

                if (!keyWords.isEmpty())
                {
                    tagsPath = keyWords.split(entry.separator);

                    if (!tagsPath.isEmpty())
                    {
                        return true;
                    }
                }

                break;
            }

            default:
                break;
        }
    }

    return false;
}

bool DMetadata::setImageTagsPath(const QStringList& tagsPath, const DMetadataSettingsContainer& settings) const
{
    // NOTE : with digiKam 0.9.x, we have used IPTC Keywords for that.
    // Now this way is obsolete, and we use XMP instead.

    // Set the new Tags path list. This is set, not add-to like setXmpKeywords.
    // Unlike the other keyword fields, we do not need to merge existing entries.
    QList<NamespaceEntry> toWrite = settings.getReadMapping(QString::fromUtf8(DM_TAG_CONTAINER));

    if (!settings.unifyReadWrite())
        toWrite = settings.getWriteMapping(QString::fromUtf8(DM_TAG_CONTAINER));

    for (NamespaceEntry entry : toWrite)
    {
        if (entry.isDisabled)
            continue;

        QStringList newList;

        // get keywords from tags path, for type tag
        for (QString tagPath : tagsPath)
        {
            newList.append(tagPath.split(QLatin1Char('/')).last());
        }

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:

                if (supportXmp())
                {
                    if (entry.tagPaths != NamespaceEntry::TAG)
                    {
                        newList = tagsPath;

                        if (entry.separator.compare(QLatin1String("/")) != 0)
                        {
                            newList = newList.replaceInStrings(QLatin1String("/"), entry.separator);
                        }
                    }

                    const std::string myStr = entry.namespaceName.toStdString();
                    const char* nameSpace   = myStr.data();

                    switch(entry.specialOpts)
                    {
                        case NamespaceEntry::TAG_XMPSEQ:

                            if (!setXmpTagStringSeq(nameSpace, newList))
                            {
                                qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image paths failed" << nameSpace;
                                return false;
                            }

                            break;

                        case NamespaceEntry::TAG_XMPBAG:

                            if (!setXmpTagStringBag(nameSpace, newList))
                            {
                                qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image paths failed" << nameSpace;
                                return false;
                            }

                            break;

                        case NamespaceEntry::TAG_ACDSEE:

                            if (!setACDSeeTagsPath(newList))
                            {
                                qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image paths failed" << nameSpace;
                                return false;
                            }

                        default:
                            break;
                    }
                }

                break;

            case NamespaceEntry::IPTC:

                if (entry.namespaceName == QLatin1String("Iptc.Application2.Keywords"))
                {
                    if (!setIptcKeywords(getIptcKeywords(), newList))
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image paths failed" << entry.namespaceName;
                        return false;
                    }
                }

            default:
                break;
        }
    }

    return true;
}

bool DMetadata::getACDSeeTagsPath(QStringList &tagsPath) const
{
    // Try to get Tags Path list from ACDSee 8 Pro categories.
    QString xmlACDSee = getXmpTagString("Xmp.acdsee.categories", false);

    if (!xmlACDSee.isEmpty())
    {
        xmlACDSee.remove(QLatin1String("</Categories>"));
        xmlACDSee.remove(QLatin1String("<Categories>"));
        xmlACDSee.replace(QLatin1Char('/'), QLatin1Char('\\'));

        QStringList xmlTags = xmlACDSee.split(QLatin1String("<Category Assigned"));
        int category        = 0;

        foreach(const QString& tags, xmlTags)
        {
            if (!tags.isEmpty())
            {
                int count  = tags.count(QLatin1String("<\\Category>"));
                int length = tags.length() - (11 * count) - 5;

                if (category == 0)
                {
                    tagsPath << tags.mid(5, length);
                }
                else
                {
                    tagsPath.last().append(QLatin1Char('/') + tags.mid(5, length));
                }

                category = category - count + 1;

                if (tags.left(5) == QLatin1String("=\"1\">") && category > 0)
                {
                    tagsPath << tagsPath.last().section(QLatin1Char('/'), 0, category - 1);
                }
            }
        }

        if (!tagsPath.isEmpty())
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Tags Path imported from ACDSee: " << tagsPath;
            return true;
        }
    }

    return false;
}

bool DMetadata::setACDSeeTagsPath(const QStringList &tagsPath) const
{
    // Converting Tags path list to ACDSee 8 Pro categories.
    const QString category(QLatin1String("<Category Assigned=\"%1\">"));
    QStringList splitTags;
    QStringList xmlTags;

    foreach(const QString& tags, tagsPath)
    {
        splitTags   = tags.split(QLatin1Char('/'));
        int current = 0;

        for (int index = 0; index < splitTags.size(); index++)
        {
            int tagIndex = xmlTags.indexOf(category.arg(0) + splitTags[index]);

            if (tagIndex == -1)
            {
                tagIndex = xmlTags.indexOf(category.arg(1) + splitTags[index]);
            }

            splitTags[index].insert(0, category.arg(index == splitTags.size() - 1 ? 1 : 0));

            if (tagIndex == -1)
            {
                if (index == 0)
                {
                    xmlTags << splitTags[index];
                    xmlTags << QLatin1String("</Category>");
                    current = xmlTags.size() - 1;
                }
                else
                {
                    xmlTags.insert(current, splitTags[index]);
                    xmlTags.insert(current + 1, QLatin1String("</Category>"));
                    current++;
                }
            }
            else
            {
                if (index == splitTags.size() - 1)
                {
                    xmlTags[tagIndex] = splitTags[index];
                }

                current = tagIndex + 1;
            }
        }
    }

    QString xmlACDSee = QLatin1String("<Categories>") + xmlTags.join(QLatin1String("")) + QLatin1String("</Categories>");
    qCDebug(DIGIKAM_METAENGINE_LOG) << "xmlACDSee" << xmlACDSee;
    removeXmpTag("Xmp.acdsee.categories");

    if (!xmlTags.isEmpty())
    {
        if (!setXmpTagString("Xmp.acdsee.categories", xmlACDSee))
        {
            return false;
        }
    }

    return true;
}

QString DMetadata::getLensDescription() const
{
    QString     lens;
    QStringList lensExifTags;

    // In first, try to get Lens information from makernotes.

    lensExifTags.append(QLatin1String("Exif.CanonCs.LensType"));      // Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.CanonCs.Lens"));          // Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Canon.0x0095"));          // Alternative Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd1.LensIDNumber")); // Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd2.LensIDNumber")); // Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd3.LensIDNumber")); // Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Minolta.LensID"));        // Minolta Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Photo.LensModel"));       // Sony Cameras Makernote (and others?).
    lensExifTags.append(QLatin1String("Exif.Sony1.LensID"));          // Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Sony2.LensID"));          // Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.SonyMinolta.LensID"));    // Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Pentax.LensType"));       // Pentax Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.PentaxDng.LensType"));    // Pentax Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Panasonic.0x0051"));      // Panasonic Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Panasonic.0x0310"));      // Panasonic Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Sigma.LensRange"));       // Sigma Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Samsung2.LensType"));     // Samsung Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Photo.0xFDEA"));          // Non-standard Exif tag set by Camera Raw.
    lensExifTags.append(QLatin1String("Exif.OlympusEq.LensModel"));   // Olympus Cameras Makernote.

    // Olympus Cameras Makernote. FIXME is this necessary? exiv2 returns complete name, which doesn't match with lensfun information, see bug #311295
    //lensExifTags.append("Exif.OlympusEq.LensType");

    // TODO : add Fuji camera Makernotes.

    // -------------------------------------------------------------------
    // Try to get Lens Data information from Exif.

    for (QStringList::const_iterator it = lensExifTags.constBegin(); it != lensExifTags.constEnd(); ++it)
    {
        lens = getExifTagString((*it).toLatin1().constData());

        if ( !lens.isEmpty() &&
             !(lens.startsWith(QLatin1Char('(')) &&
               lens.endsWith(QLatin1Char(')'))
              )
           )   // To prevent undecoded tag values from Exiv2 as "(65535)".
        {
            return lens;
        }
    }

    // -------------------------------------------------------------------
    // Try to get Lens Data information from XMP.
    // XMP aux tags.
    lens = getXmpTagString("Xmp.aux.Lens");

    if (lens.isEmpty())
    {
        // XMP M$ tags (Lens Maker + Lens Model).
        lens = getXmpTagString("Xmp.MicrosoftPhoto.LensManufacturer");

        if (!lens.isEmpty())
        {
            lens.append(QLatin1Char(' '));
        }

        lens.append(getXmpTagString("Xmp.MicrosoftPhoto.LensModel"));
    }

    return lens;
}

IccProfile DMetadata::getIccProfile() const
{
    // Check if Exif data contains an ICC color profile.
    QByteArray data = getExifTagData("Exif.Image.InterColorProfile");

    if (!data.isNull())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Found an ICC profile in Exif metadata";
        return IccProfile(data);
    }

    // Else check the Exif color-space tag and use default profiles that we ship
    switch (getImageColorWorkSpace())
    {
        case DMetadata::WORKSPACE_SRGB:
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Exif color-space tag is sRGB. Using default sRGB ICC profile.";
            return IccProfile::sRGB();
        }

        case DMetadata::WORKSPACE_ADOBERGB:
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Exif color-space tag is AdobeRGB. Using default AdobeRGB ICC profile.";
            return IccProfile::adobeRGB();
        }

        default:
            break;
    }

    return IccProfile();
}

bool DMetadata::setIccProfile(const IccProfile& profile)
{
    if (profile.isNull())
    {
        removeExifTag("Exif.Image.InterColorProfile");
    }
    else
    {
        QByteArray data = IccProfile(profile).data();

        if (!setExifTagData("Exif.Image.InterColorProfile", data))
        {
            return false;
        }
    }

    removeExifColorSpace();

    return true;
}

inline QVariant DMetadata::fromExifOrXmp(const char* const exifTagName, const char* const xmpTagName) const
{
    QVariant var;

    if (exifTagName)
    {
        var = getExifTagVariant(exifTagName, false);

        if (!var.isNull())
        {
            return var;
        }
    }

    if (xmpTagName)
    {
        var = getXmpTagVariant(xmpTagName);

        if (!var.isNull())
        {
            return var;
        }
    }

    return var;
}

inline QVariant DMetadata::fromIptcOrXmp(const char* const iptcTagName, const char* const xmpTagName) const
{
    if (iptcTagName)
    {
        QString iptcValue = getIptcTagString(iptcTagName);

        if (!iptcValue.isNull())
        {
            return iptcValue;
        }
    }

    if (xmpTagName)
    {
        QVariant var = getXmpTagVariant(xmpTagName);

        if (!var.isNull())
        {
            return var;
        }
    }

    return QVariant(QVariant::String);
}

inline QVariant DMetadata::fromXmpList(const char* const xmpTagName) const
{
    QVariant var = getXmpTagVariant(xmpTagName);

    if (var.isNull())
    {
        return QVariant(QVariant::StringList);
    }

    return var;
}

inline QVariant DMetadata::fromXmpLangAlt(const char* const xmpTagName) const
{
    QVariant var = getXmpTagVariant(xmpTagName);

    if (var.isNull())
    {
        return QVariant(QVariant::Map);
    }

    return var;
}

inline QVariant DMetadata::toStringListVariant(const QStringList& list) const
{
    if (list.isEmpty())
    {
        return QVariant(QVariant::StringList);
    }

    return list;
}

QVariant DMetadata::getMetadataField(MetadataInfo::Field field) const
{
    switch (field)
    {
        case MetadataInfo::Comment:
            return getImageComments()[QLatin1String("x-default")].caption;
        case MetadataInfo::CommentJfif:
            return getCommentsDecoded();
        case MetadataInfo::CommentExif:
            return getExifComment();
        case MetadataInfo::CommentIptc:
            return fromIptcOrXmp("Iptc.Application2.Caption", 0);

        case MetadataInfo::Description:
        {
            QVariant var = fromXmpLangAlt("Xmp.dc.description");

            if (!var.isNull())
            {
                return var;
            }

            var = fromXmpLangAlt("Xmp.tiff.ImageDescription");

            if (!var.isNull())
            {
                return var;
            }

            return fromIptcEmulateLangAlt("Iptc.Application2.Caption");
        }
        case MetadataInfo::Headline:
            return fromIptcOrXmp("Iptc.Application2.Headline", "Xmp.photoshop.Headline");
        case MetadataInfo::Title:
        {
            QString str = getImageTitles()[QLatin1String("x-default")].caption;

            if (str.isEmpty())
            {
                return QVariant(QVariant::Map);
            }

            QMap<QString, QVariant> map;
            map[QLatin1String("x-default")] = str;
            return map;
        }
        case MetadataInfo::DescriptionWriter:
            return fromIptcOrXmp("Iptc.Application2.Writer", "Xmp.photoshop.CaptionWriter");

        case MetadataInfo::Keywords:
        {
            QStringList list;
            getImageTagsPath(list);
            return toStringListVariant(list);
        }

        case MetadataInfo::Faces:
        {
            QMultiMap<QString,QVariant> faceMap;
            getImageFacesMap(faceMap);
            QVariant var(faceMap);
            return var;
        }

        case MetadataInfo::Rating:
            return getImageRating();
        case MetadataInfo::CreationDate:
            return getImageDateTime();
        case MetadataInfo::DigitizationDate:
            return getDigitizationDateTime(true);
        case MetadataInfo::Orientation:
            return (int)getImageOrientation();

        case MetadataInfo::Make:
        {
            QVariant var = fromExifOrXmp("Exif.Image.Make", "Xmp.tiff.Make");
            return QVariant(var.toString().trimmed());
        }
        case MetadataInfo::Model:
        {
            QVariant var = fromExifOrXmp("Exif.Image.Model", "Xmp.tiff.Model");
            return QVariant(var.toString().trimmed());
        }
        case MetadataInfo::Lens:
            return getLensDescription();
        case MetadataInfo::Aperture:
        {
            QVariant var = fromExifOrXmp("Exif.Photo.FNumber", "Xmp.exif.FNumber");

            if (var.isNull())
            {
                var = fromExifOrXmp("Exif.Photo.ApertureValue", "Xmp.exif.ApertureValue");

                if (!var.isNull())
                {
                    var = apexApertureToFNumber(var.toDouble());
                }
            }

            return var;
        }
        case MetadataInfo::FocalLength:
            return fromExifOrXmp("Exif.Photo.FocalLength", "Xmp.exif.FocalLength");
        case MetadataInfo::FocalLengthIn35mm:
            return fromExifOrXmp("Exif.Photo.FocalLengthIn35mmFilm", "Xmp.exif.FocalLengthIn35mmFilm");
        case MetadataInfo::ExposureTime:
        {
            QVariant var = fromExifOrXmp("Exif.Photo.ExposureTime", "Xmp.exif.ExposureTime");

            if (var.isNull())
            {
                var = fromExifOrXmp("Exif.Photo.ShutterSpeedValue", "Xmp.exif.ShutterSpeedValue");

                if (!var.isNull())
                {
                    var = apexShutterSpeedToExposureTime(var.toDouble());
                }
            }

            return var;
        }
        case MetadataInfo::ExposureProgram:
            return fromExifOrXmp("Exif.Photo.ExposureProgram", "Xmp.exif.ExposureProgram");
        case MetadataInfo::ExposureMode:
            return fromExifOrXmp("Exif.Photo.ExposureMode", "Xmp.exif.ExposureMode");
        case MetadataInfo::Sensitivity:
        {
            QVariant var = fromExifOrXmp("Exif.Photo.ISOSpeedRatings", "Xmp.exif.ISOSpeedRatings");
            //if (var.isNull())
            // TODO: has this ISO format??? We must convert to the format of ISOSpeedRatings!
            //  var = fromExifOrXmp("Exif.Photo.ExposureIndex", "Xmp.exif.ExposureIndex");
            return var;
        }
        case MetadataInfo::FlashMode:
            return fromExifOrXmp("Exif.Photo.Flash", "Xmp.exif.Flash");
        case MetadataInfo::WhiteBalance:
            return fromExifOrXmp("Exif.Photo.WhiteBalance", "Xmp.exif.WhiteBalance");
        case MetadataInfo::MeteringMode:
            return fromExifOrXmp("Exif.Photo.MeteringMode", "Xmp.exif.MeteringMode");
        case MetadataInfo::SubjectDistance:
            return fromExifOrXmp("Exif.Photo.SubjectDistance", "Xmp.exif.SubjectDistance");
        case MetadataInfo::SubjectDistanceCategory:
            return fromExifOrXmp("Exif.Photo.SubjectDistanceRange", "Xmp.exif.SubjectDistanceRange");
        case MetadataInfo::WhiteBalanceColorTemperature:
            //TODO: ??
            return QVariant(QVariant::Int);

        case MetadataInfo::Longitude:
            return getGPSLongitudeString();
        case MetadataInfo::LongitudeNumber:
        {
            double longitude;

            if (getGPSLongitudeNumber(&longitude))
            {
                return longitude;
            }
            else
            {
                return QVariant(QVariant::Double);
            }
        }
        case MetadataInfo::Latitude:
            return getGPSLatitudeString();
        case MetadataInfo::LatitudeNumber:
        {
            double latitude;

            if (getGPSLatitudeNumber(&latitude))
            {
                return latitude;
            }
            else
            {
                return QVariant(QVariant::Double);
            }
        }
        case MetadataInfo::Altitude:
        {
            double altitude;

            if (getGPSAltitude(&altitude))
            {
                return altitude;
            }
            else
            {
                return QVariant(QVariant::Double);
            }
        }
        case MetadataInfo::PositionOrientation:
        case MetadataInfo::PositionTilt:
        case MetadataInfo::PositionRoll:
        case MetadataInfo::PositionAccuracy:
            // TODO or unsupported?
            return QVariant(QVariant::Double);
        case MetadataInfo::PositionDescription:
            // TODO or unsupported?
            return QVariant(QVariant::String);

        case MetadataInfo::IptcCoreCopyrightNotice:
        {
            QVariant var = fromXmpLangAlt("Xmp.dc.rights");

            if (!var.isNull())
            {
                return var;
            }

            var = fromXmpLangAlt("Xmp.tiff.Copyright");

            if (!var.isNull())
            {
                return var;
            }

            return fromIptcEmulateLangAlt("Iptc.Application2.Copyright");
        }
        case MetadataInfo::IptcCoreCreator:
        {
            QVariant var = fromXmpList("Xmp.dc.creator");

            if (!var.isNull())
            {
                return var;
            }

            QString artist = getXmpTagString("Xmp.tiff.Artist");

            if (!artist.isNull())
            {
                QStringList list;
                list << artist;
                return list;
            }

            return fromIptcEmulateList("Iptc.Application2.Byline");
        }
        case MetadataInfo::IptcCoreProvider:
            return fromIptcOrXmp("Iptc.Application2.Credit", "Xmp.photoshop.Credit");
        case MetadataInfo::IptcCoreRightsUsageTerms:
            return fromXmpLangAlt("Xmp.xmpRights.UsageTerms");
        case MetadataInfo::IptcCoreSource:
            return fromIptcOrXmp("Iptc.Application2.Source", "Xmp.photoshop.Source");

        case MetadataInfo::IptcCoreCreatorJobTitle:
            return fromIptcOrXmp("Iptc.Application2.BylineTitle", "Xmp.photoshop.AuthorsPosition");
        case MetadataInfo::IptcCoreInstructions:
            return fromIptcOrXmp("Iptc.Application2.SpecialInstructions", "Xmp.photoshop.Instructions");

        case MetadataInfo::IptcCoreLocationInfo:
        {
            IptcCoreLocationInfo location = getIptcCoreLocation();

            if (location.isNull())
            {
                return QVariant();
            }

            return QVariant::fromValue(location);
        }
        case MetadataInfo::IptcCoreCountryCode:
            return fromIptcOrXmp("Iptc.Application2.CountryCode", "Xmp.iptc.CountryCode");
        case MetadataInfo::IptcCoreCountry:
            return fromIptcOrXmp("Iptc.Application2.CountryName", "Xmp.photoshop.Country");
        case MetadataInfo::IptcCoreCity:
            return fromIptcOrXmp("Iptc.Application2.City", "Xmp.photoshop.City");
        case MetadataInfo::IptcCoreLocation:
            return fromIptcOrXmp("Iptc.Application2.SubLocation", "Xmp.iptc.Location");
        case MetadataInfo::IptcCoreProvinceState:
            return fromIptcOrXmp("Iptc.Application2.ProvinceState", "Xmp.photoshop.State");

        case MetadataInfo::IptcCoreIntellectualGenre:
            return fromIptcOrXmp("Iptc.Application2.ObjectAttribute", "Xmp.iptc.IntellectualGenre");
        case MetadataInfo::IptcCoreJobID:
            return fromIptcOrXmp("Iptc.Application2.TransmissionReference", "Xmp.photoshop.TransmissionReference");
        case MetadataInfo::IptcCoreScene:
            return fromXmpList("Xmp.iptc.Scene");
        case MetadataInfo::IptcCoreSubjectCode:
            return toStringListVariant(getIptcCoreSubjects());

        case MetadataInfo::IptcCoreContactInfo:
        {
            IptcCoreContactInfo info = getCreatorContactInfo();

            if (info.isNull())
            {
                return QVariant();
            }

            return QVariant::fromValue(info);
        }
        case MetadataInfo::IptcCoreContactInfoCity:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity");
        case MetadataInfo::IptcCoreContactInfoCountry:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry");
        case MetadataInfo::IptcCoreContactInfoAddress:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr");
        case MetadataInfo::IptcCoreContactInfoPostalCode:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode");
        case MetadataInfo::IptcCoreContactInfoProvinceState:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrRegion");
        case MetadataInfo::IptcCoreContactInfoEmail:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiEmailWork");
        case MetadataInfo::IptcCoreContactInfoPhone:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiTelWork");
        case MetadataInfo::IptcCoreContactInfoWebUrl:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork");

        case MetadataInfo::AspectRatio:
        {
            long num             = 0;
            long den             = 1;

            // NOTE: there is a bug in Exiv2 xmp::video tag definition as "Rational" value is defined as "Ratio"...
            //QList<QVariant> list = getXmpTagVariant("Xmp.video.AspectRatio").toList();

            QString ar       = getXmpTagString("Xmp.video.AspectRatio");
            QStringList list = ar.split(QLatin1Char('/'));

            if (list.size() >= 1)
                num = list[0].toInt();

            if (list.size() >= 2)
                den = list[1].toInt();

            return QString::number((double)num / (double)den);
        }
        case MetadataInfo::AudioBitRate:
            return fromXmpLangAlt("Xmp.audio.SampleRate");
        case MetadataInfo::AudioChannelType:
            return fromXmpLangAlt("Xmp.audio.ChannelType");
        case MetadataInfo::AudioCodec:
            return fromXmpLangAlt("Xmp.audio.Codec");
        case MetadataInfo::Duration:
            return fromXmpLangAlt("Xmp.video.duration"); // duration is in ms
        case MetadataInfo::FrameRate:
            return fromXmpLangAlt("Xmp.video.FrameRate");
        case MetadataInfo::VideoCodec:
            return fromXmpLangAlt("Xmp.video.Codec");
        case MetadataInfo::VideoBitDepth:
            return fromXmpLangAlt("Xmp.video.BitDepth");
        case MetadataInfo::VideoHeight:
            return fromXmpLangAlt("Xmp.video.Height");
        case MetadataInfo::VideoWidth:
            return fromXmpLangAlt("Xmp.video.Width");
        case MetadataInfo::VideoColorSpace:
        {
            QString cs = getXmpTagString("Xmp.video.ColorSpace");

            if (cs == QLatin1String("sRGB"))
                return QString::number(VIDEOCOLORMODEL_SRGB);
            else if (cs == QLatin1String("CCIR-601"))
                return QString::number(VIDEOCOLORMODEL_BT601);
            else if (cs == QLatin1String("CCIR-709"))
                return QString::number(VIDEOCOLORMODEL_BT709);
            else if (cs == QLatin1String("Other"))
                return QString::number(VIDEOCOLORMODEL_OTHER);
            else
                return QVariant(QVariant::Int);
        }
        default:
            return QVariant();
    }
}

QVariantList DMetadata::getMetadataFields(const MetadataFields& fields) const
{
    QVariantList list;

    foreach(MetadataInfo::Field field, fields) // krazy:exclude=foreach
    {
        list << getMetadataField(field);
    }

    return list;
}

QString DMetadata::valueToString(const QVariant& value, MetadataInfo::Field field)
{
    MetaEngine exiv2Iface;

    switch (field)
    {
        case MetadataInfo::Rating:
            return value.toString();
        case MetadataInfo::CreationDate:
        case MetadataInfo::DigitizationDate:
            return value.toDateTime().toString(Qt::LocaleDate);
        case MetadataInfo::Orientation:
        {
            switch (value.toInt())
            {
                    // Example why the English text differs from the enum names: ORIENTATION_ROT_90.
                    // Rotation by 90 degrees is right (clockwise) rotation.
                    // But: The enum names describe what needs to be done to get the image right again.
                    // And an image that needs to be rotated 90 degrees is currently rotated 270 degrees = left.

                case ORIENTATION_UNSPECIFIED:
                    return i18n("Unspecified");
                case ORIENTATION_NORMAL:
                    return i18nc("Rotation of an unrotated image", "Normal");
                case ORIENTATION_HFLIP:
                    return i18n("Flipped Horizontally");
                case ORIENTATION_ROT_180:
                    return i18n("Rotated by 180 Degrees");
                case ORIENTATION_VFLIP:
                    return i18n("Flipped Vertically");
                case ORIENTATION_ROT_90_HFLIP:
                    return i18n("Flipped Horizontally and Rotated Left");
                case ORIENTATION_ROT_90:
                    return i18n("Rotated Left");
                case ORIENTATION_ROT_90_VFLIP:
                    return i18n("Flipped Vertically and Rotated Left");
                case ORIENTATION_ROT_270:
                    return i18n("Rotated Right");
                default:
                    return i18n("Unknown");
            }
            break;
        }
        case MetadataInfo::Make:
            return exiv2Iface.createExifUserStringFromValue("Exif.Image.Make", value);
        case MetadataInfo::Model:
            return exiv2Iface.createExifUserStringFromValue("Exif.Image.Model", value);
        case MetadataInfo::Lens:
            // heterogeneous source, non-standardized string
            return value.toString();
        case MetadataInfo::Aperture:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.FNumber", value);
        case MetadataInfo::FocalLength:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.FocalLength", value);
        case MetadataInfo::FocalLengthIn35mm:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.FocalLengthIn35mmFilm", value);
        case MetadataInfo::ExposureTime:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.ExposureTime", value);
        case MetadataInfo::ExposureProgram:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.ExposureProgram", value);
        case MetadataInfo::ExposureMode:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.ExposureMode", value);
        case MetadataInfo::Sensitivity:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.ISOSpeedRatings", value);
        case MetadataInfo::FlashMode:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.Flash", value);
        case MetadataInfo::WhiteBalance:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.WhiteBalance", value);
        case MetadataInfo::MeteringMode:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.MeteringMode", value);
        case MetadataInfo::SubjectDistance:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.SubjectDistance", value);
        case MetadataInfo::SubjectDistanceCategory:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.SubjectDistanceRange", value);
        case MetadataInfo::WhiteBalanceColorTemperature:
            return i18nc("Temperature in Kelvin", "%1 K", value.toInt());

        case MetadataInfo::AspectRatio:
        case MetadataInfo::AudioBitRate:
        case MetadataInfo::AudioChannelType:
        case MetadataInfo::AudioCodec:
        case MetadataInfo::Duration:
        case MetadataInfo::FrameRate:
        case MetadataInfo::VideoCodec:
            return value.toString();

        case MetadataInfo::Longitude:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            if (!convertToUserPresentableNumbers(value.toString(), &degrees, &minutes, &seconds, &directionRef))
            {
                return QString();
            }

            QString direction = (QLatin1Char(directionRef) == QLatin1Char('W')) ?
                                i18nc("For use in longitude coordinate", "West") : i18nc("For use in longitude coordinate", "East");
            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::LongitudeNumber:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            convertToUserPresentableNumbers(false, value.toDouble(), &degrees, &minutes, &seconds, &directionRef);
            QString direction = (QLatin1Char(directionRef) == QLatin1Char('W')) ?
                                i18nc("For use in longitude coordinate", "West") : i18nc("For use in longitude coordinate", "East");
            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::Latitude:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            if (!convertToUserPresentableNumbers(value.toString(), &degrees, &minutes, &seconds, &directionRef))
            {
                return QString();
            }

            QString direction = (QLatin1Char(directionRef) == QLatin1Char('N')) ?
                                i18nc("For use in latitude coordinate", "North") : i18nc("For use in latitude coordinate", "South");
            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::LatitudeNumber:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            convertToUserPresentableNumbers(false, value.toDouble(), &degrees, &minutes, &seconds, &directionRef);
            QString direction = (QLatin1Char(directionRef) == QLatin1Char('N')) ?
                                i18nc("For use in latitude coordinate", "North") : i18nc("For use in latitude coordinate", "South");
            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::Altitude:
        {
            QString meters = QString::fromLatin1("%L1").arg(value.toDouble(), 0, 'f', 2);
            // xgettext: no-c-format
            return i18nc("Height in meters", "%1m", meters);
        }

        case MetadataInfo::PositionOrientation:
        case MetadataInfo::PositionTilt:
        case MetadataInfo::PositionRoll:
        case MetadataInfo::PositionAccuracy:
            //TODO
            return value.toString();

        case MetadataInfo::PositionDescription:
            return value.toString();

            // Lang Alt
        case MetadataInfo::IptcCoreCopyrightNotice:
        case MetadataInfo::IptcCoreRightsUsageTerms:
        case MetadataInfo::Description:
        case MetadataInfo::Title:
        {
            QMap<QString, QVariant> map = value.toMap();

            // the most common cases
            if (map.isEmpty())
            {
                return QString();
            }
            else if (map.size() == 1)
            {
                return map.begin().value().toString();
            }

            // Try "en-us"
            QString spec = QLocale().name().toLower().replace(QLatin1Char('_'), QLatin1Char('-'));

            if (map.contains(spec))
            {
                return map[spec].toString();
            }

            // Try "en-"
            QStringList keys    = map.keys();
            QString spec2       = QLocale().name().toLower();
            QRegExp exp(spec2.left(spec2.indexOf(QLatin1Char('_'))) + QLatin1Char('-'));
            QStringList matches = keys.filter(exp);

            if (!matches.isEmpty())
            {
                return map[matches.first()].toString();
            }

            // return default
            if (map.contains(QLatin1String("x-default")))
            {
                return map[QLatin1String("x-default")].toString();
            }

            // return first entry
            return map.begin().value().toString();
        }

        // List
        case MetadataInfo::IptcCoreCreator:
        case MetadataInfo::IptcCoreScene:
        case MetadataInfo::IptcCoreSubjectCode:
            return value.toStringList().join(QLatin1Char(' '));

            // Text
        case MetadataInfo::Comment:
        case MetadataInfo::CommentJfif:
        case MetadataInfo::CommentExif:
        case MetadataInfo::CommentIptc:
        case MetadataInfo::Headline:
        case MetadataInfo::DescriptionWriter:
        case MetadataInfo::IptcCoreProvider:
        case MetadataInfo::IptcCoreSource:
        case MetadataInfo::IptcCoreCreatorJobTitle:
        case MetadataInfo::IptcCoreInstructions:
        case MetadataInfo::IptcCoreCountryCode:
        case MetadataInfo::IptcCoreCountry:
        case MetadataInfo::IptcCoreCity:
        case MetadataInfo::IptcCoreLocation:
        case MetadataInfo::IptcCoreProvinceState:
        case MetadataInfo::IptcCoreIntellectualGenre:
        case MetadataInfo::IptcCoreJobID:
            return value.toString();

        default:
            break;
    }

    return QString();
}

QStringList DMetadata::valuesToString(const QVariantList& values, const MetadataFields& fields)
{
    int size = values.size();
    Q_ASSERT(size == values.size());

    QStringList list;
    for (int i = 0; i < size; ++i)
    {
        list << valueToString(values.at(i), fields.at(i));
    }

    return list;
}

QMap<int, QString> DMetadata::possibleValuesForEnumField(MetadataInfo::Field field)
{
    QMap<int, QString> map;
    int min, max;

    switch (field)
    {
        case MetadataInfo::Orientation:                      /// Int, enum from libMetaEngine
            min = ORIENTATION_UNSPECIFIED;
            max = ORIENTATION_ROT_270;
            break;
        case MetadataInfo::ExposureProgram:                  /// Int, enum from Exif
            min = 0;
            max = 8;
            break;
        case MetadataInfo::ExposureMode:                     /// Int, enum from Exif
            min = 0;
            max = 2;
            break;
        case MetadataInfo::WhiteBalance:                     /// Int, enum from Exif
            min = 0;
            max = 1;
            break;
        case MetadataInfo::MeteringMode:                     /// Int, enum from Exif
            min = 0;
            max = 6;
            map[255] = valueToString(255, field);
            break;
        case MetadataInfo::SubjectDistanceCategory:          /// int, enum from Exif
            min = 0;
            max = 3;
            break;
        case MetadataInfo::FlashMode:                        /// Int, bit mask from Exif
            // This one is a bit special.
            // We return a bit mask for binary AND searching.
            map[0x1] = i18n("Flash has been fired");
            map[0x40] = i18n("Flash with red-eye reduction mode");
            //more: TODO?
            return map;
        default:
            qCWarning(DIGIKAM_METAENGINE_LOG) << "Unsupported field " << field << " in DMetadata::possibleValuesForEnumField";
            return map;
    }

    for (int i = min; i <= max; ++i)
    {
        map[i] = valueToString(i, field);
    }

    return map;
}

double DMetadata::apexApertureToFNumber(double aperture)
{
    // convert from APEX. See Exif spec, Annex C.
    if (aperture == 0.0)
    {
        return 1;
    }
    else if (aperture == 1.0)
    {
        return 1.4;
    }
    else if (aperture == 2.0)
    {
        return 2;
    }
    else if (aperture == 3.0)
    {
        return 2.8;
    }
    else if (aperture == 4.0)
    {
        return 4;
    }
    else if (aperture == 5.0)
    {
        return 5.6;
    }
    else if (aperture == 6.0)
    {
        return 8;
    }
    else if (aperture == 7.0)
    {
        return 11;
    }
    else if (aperture == 8.0)
    {
        return 16;
    }
    else if (aperture == 9.0)
    {
        return 22;
    }
    else if (aperture == 10.0)
    {
        return 32;
    }

    return exp(log(2) * aperture / 2.0);
}

double DMetadata::apexShutterSpeedToExposureTime(double shutterSpeed)
{
    // convert from APEX. See Exif spec, Annex C.
    if (shutterSpeed == -5.0)
    {
        return 30;
    }
    else if (shutterSpeed == -4.0)
    {
        return 15;
    }
    else if (shutterSpeed == -3.0)
    {
        return 8;
    }
    else if (shutterSpeed == -2.0)
    {
        return 4;
    }
    else if (shutterSpeed == -1.0)
    {
        return 2;
    }
    else if (shutterSpeed == 0.0)
    {
        return 1;
    }
    else if (shutterSpeed == 1.0)
    {
        return 0.5;
    }
    else if (shutterSpeed == 2.0)
    {
        return 0.25;
    }
    else if (shutterSpeed == 3.0)
    {
        return 0.125;
    }
    else if (shutterSpeed == 4.0)
    {
        return 1.0 / 15.0;
    }
    else if (shutterSpeed == 5.0)
    {
        return 1.0 / 30.0;
    }
    else if (shutterSpeed == 6.0)
    {
        return 1.0 / 60.0;
    }
    else if (shutterSpeed == 7.0)
    {
        return 0.008;    // 1/125
    }
    else if (shutterSpeed == 8.0)
    {
        return 0.004;    // 1/250
    }
    else if (shutterSpeed == 9.0)
    {
        return 0.002;    // 1/500
    }
    else if (shutterSpeed == 10.0)
    {
        return 0.001;    // 1/1000
    }
    else if (shutterSpeed == 11.0)
    {
        return 0.0005;    // 1/2000
    }
    // additions by me
    else if (shutterSpeed == 12.0)
    {
        return 0.00025;    // 1/4000
    }
    else if (shutterSpeed == 13.0)
    {
        return 0.000125;    // 1/8000
    }

    return exp( - log(2) * shutterSpeed);
}

bool DMetadata::addToXmpTagStringBag(const char* const xmpTagName, const QStringList& entriesToAdd) const
{
    //#ifdef _XMP_SUPPORT_

    QStringList oldEntries = getXmpTagStringBag(xmpTagName, false);
    QStringList newEntries = entriesToAdd;

    // Create a list of keywords including old one which already exists.
    for (QStringList::const_iterator it = oldEntries.constBegin(); it != oldEntries.constEnd(); ++it )
    {
        if (!newEntries.contains(*it))
        {
            newEntries.append(*it);
        }
    }

    if (setXmpTagStringBag(xmpTagName, newEntries))
    {
        return true;
    }

    //#endif // _XMP_SUPPORT_

    return false;
}

bool DMetadata::removeFromXmpTagStringBag(const char* const xmpTagName, const QStringList& entriesToRemove) const
{
    //#ifdef _XMP_SUPPORT_

    QStringList currentEntries = getXmpTagStringBag(xmpTagName, false);
    QStringList newEntries;

    // Create a list of current keywords except those that shall be removed
    for (QStringList::const_iterator it = currentEntries.constBegin(); it != currentEntries.constEnd(); ++it )
    {
        if (!entriesToRemove.contains(*it))
        {
            newEntries.append(*it);
        }
    }

    if (setXmpTagStringBag(xmpTagName, newEntries))
    {
        return true;
    }

    //#endif // _XMP_SUPPORT_

    return false;
}

QStringList DMetadata::getXmpKeywords() const
{
    return (getXmpTagStringBag("Xmp.dc.subject", false));
}

bool DMetadata::setXmpKeywords(const QStringList& newKeywords) const
{
    return setXmpTagStringBag("Xmp.dc.subject", newKeywords);
}

bool DMetadata::removeXmpKeywords(const QStringList& keywordsToRemove)
{
    return removeFromXmpTagStringBag("Xmp.dc.subject", keywordsToRemove);
}

QStringList DMetadata::getXmpSubCategories() const
{
    return (getXmpTagStringBag("Xmp.photoshop.SupplementalCategories", false));
}

bool DMetadata::setXmpSubCategories(const QStringList& newSubCategories) const
{
    return addToXmpTagStringBag("Xmp.photoshop.SupplementalCategories", newSubCategories);
}

bool DMetadata::removeXmpSubCategories(const QStringList& subCategoriesToRemove)
{
    return removeFromXmpTagStringBag("Xmp.photoshop.SupplementalCategories", subCategoriesToRemove);
}

QStringList DMetadata::getXmpSubjects() const
{
    return (getXmpTagStringBag("Xmp.iptc.SubjectCode", false));
}

bool DMetadata::setXmpSubjects(const QStringList& newSubjects) const
{
    return addToXmpTagStringBag("Xmp.iptc.SubjectCode", newSubjects);
}

bool DMetadata::removeXmpSubjects(const QStringList& subjectsToRemove)
{
    return removeFromXmpTagStringBag("Xmp.iptc.SubjectCode", subjectsToRemove);
}

bool DMetadata::removeExifColorSpace() const
{
    bool ret =  true;
    ret     &= removeExifTag("Exif.Photo.ColorSpace");
    ret     &= removeXmpTag("Xmp.exif.ColorSpace");

    return ret;
}

QString DMetadata::getExifTagStringFromTagsList(const QStringList& tagsList) const
{
    QString val;

    foreach(const QString& tag, tagsList)
    {
        val = getExifTagString(tag.toLatin1().constData());

        if (!val.isEmpty())
            return val;
    }

    return QString();
}

bool DMetadata::removeExifTags(const QStringList& tagFilters)
{
    MetaDataMap m = getExifTagsDataList(tagFilters);

    if (m.isEmpty())
        return false;

    for (MetaDataMap::iterator it = m.begin() ; it != m.end() ; ++it)
    {
        removeExifTag(it.key().toLatin1().constData());
    }

    return true;
}

bool DMetadata::removeXmpTags(const QStringList& tagFilters)
{
    MetaDataMap m = getXmpTagsDataList(tagFilters);

    if (m.isEmpty())
        return false;

    for (MetaDataMap::iterator it = m.begin() ; it != m.end() ; ++it)
    {
        removeXmpTag(it.key().toLatin1().constData());
    }

    return true;
}

bool DMetadata::hasValidField(const QVariantList& list) const
{
    for (QVariantList::const_iterator it = list.constBegin();
         it != list.constEnd(); ++it)
    {
        if (!(*it).isNull())
        {
            return true;
        }
    }

    return false;
}

} // namespace Digikam
