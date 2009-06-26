/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-12
 * Description : Access to copyright info of an image in the database
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagecopyright.h"

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "imagescanner.h"

namespace Digikam
{

ImageCopyright::ImageCopyright(qlonglong imageid)
              : m_id(imageid)
{
}

ImageCopyright::ImageCopyright()
              : m_id(0)
{
}

QStringList ImageCopyright::creator()
{
    QList<CopyrightInfo> infos = DatabaseAccess().db()->getImageCopyright(m_id, 
         ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreator));
    QStringList list;
    foreach(const CopyrightInfo& info, infos)
    {
        list << info.value;
    }
    return list;
}

void ImageCopyright::setCreator(const QString& creator, ReplaceMode mode)
{
    AlbumDB::CopyrightPropertyUnique uniqueness;
    if (mode == ReplaceAllEntries)
        uniqueness = AlbumDB::PropertyUnique;
    else
        uniqueness = AlbumDB::PropertyNoConstraint;

    DatabaseAccess().db()->setImageCopyrightProperty(m_id, ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreator),
                                                     creator, QString(), uniqueness);
}

void ImageCopyright::removeCreators()
{
    removeProperties(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreator));
}

QString ImageCopyright::provider()
{
    return readSimpleProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvider));
}

void ImageCopyright::setProvider(const QString& provider)
{
    setSimpleProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvider), provider);
}

void ImageCopyright::removeProvider()
{
    removeProperties(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvider));
}

QString ImageCopyright::copyrightNotice(const QString& languageCode)
{
    return readLanguageProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCopyrightNotice), languageCode);
}

KExiv2Iface::KExiv2::AltLangMap ImageCopyright::allCopyrightNotices()
{
    return readLanguageProperties(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCopyrightNotice));
}

void ImageCopyright::setCopyrightNotice(const QString& notice, const QString& languageCode, ReplaceMode mode)
{
    setLanguageProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCopyrightNotice), notice, languageCode, mode);
}

void ImageCopyright::removeCopyrightNotices()
{
    removeProperties(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCopyrightNotice));
}

QString ImageCopyright::rightsUsageTerms(const QString& languageCode)
{
    return readLanguageProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreRightsUsageTerms), languageCode);
}

KExiv2Iface::KExiv2::AltLangMap ImageCopyright::allRightsUsageTerms()
{
    return readLanguageProperties(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreRightsUsageTerms));
}

void ImageCopyright::setRightsUsageTerms(const QString& term, const QString& languageCode, ReplaceMode mode)
{
    setLanguageProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreRightsUsageTerms), term, languageCode, mode);
}

void ImageCopyright::removeRightsUsageTerms()
{
    removeProperties(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreRightsUsageTerms));
}

QString ImageCopyright::source()
{
    return readSimpleProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSource));
}

void ImageCopyright::setSource(const QString& source)
{
    setSimpleProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSource), source);
}

void ImageCopyright::removeSource()
{
    removeProperties(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSource));
}

QString ImageCopyright::creatorJobTitle()
{
    return readSimpleProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreatorJobTitle));
}

void ImageCopyright::setCreatorJobTitle(const QString& title)
{
    setSimpleProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreatorJobTitle), title);
}

void ImageCopyright::removeCreatorJobTitle()
{
    removeProperties(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreatorJobTitle));
}

QString ImageCopyright::instructions()
{
    return readSimpleProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreInstructions));
}

void ImageCopyright::setInstructions(const QString& instructions)
{
    setSimpleProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreInstructions), instructions);
}

void ImageCopyright::removeInstructions()
{
    removeProperties(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreInstructions));
}

QString ImageCopyright::readSimpleProperty(const QString& property)
{
    QList<CopyrightInfo> infos = DatabaseAccess().db()->getImageCopyright(m_id, property);
    if (infos.isEmpty())
        return QString();
    else
        return infos.first().value;
}

void ImageCopyright::setSimpleProperty(const QString& property, const QString& value)
{
    DatabaseAccess().db()->setImageCopyrightProperty(m_id, property, value, QString(), AlbumDB::PropertyUnique);
}

QString ImageCopyright::readLanguageProperty(const QString& property, const QString& languageCode)
{
    QList<CopyrightInfo> infos = DatabaseAccess().db()->getImageCopyright(m_id, property);
    int index = languageMatch(infos, languageCode);
    if (index == -1)
        return QString();
    else
        return infos[index].value;
}

KExiv2Iface::KExiv2::AltLangMap ImageCopyright::readLanguageProperties(const QString& property)
{
    KExiv2Iface::KExiv2::AltLangMap map;
    QList<CopyrightInfo> infos = DatabaseAccess().db()->getImageCopyright(m_id, property);
    foreach (const CopyrightInfo &info, infos)
        map[info.extraValue] = info.value;
    return map;
}

void ImageCopyright::setLanguageProperty(const QString& property, const QString& value, 
                                         const QString& languageCode, ReplaceMode mode)
{
    AlbumDB::CopyrightPropertyUnique uniqueness;
    if (mode == ReplaceAllEntries)
        uniqueness = AlbumDB::PropertyUnique;
    else if (mode == ReplaceLanguageEntry)
        uniqueness = AlbumDB::PropertyExtraValueUnique;
    else
        uniqueness = AlbumDB::PropertyNoConstraint;

    QString language = languageCode;
    if (language.isNull())
        language = "x-default";

    DatabaseAccess().db()->setImageCopyrightProperty(m_id, property, value, language, uniqueness);
}

void ImageCopyright::removeProperties(const QString &property)
{
    DatabaseAccess().db()->removeImageCopyrightProperties(m_id, property);
}

void ImageCopyright::removeLanguageProperty(const QString &property, const QString &languageCode)
{
    DatabaseAccess().db()->removeImageCopyrightProperties(m_id, property, languageCode);
}

int ImageCopyright::languageMatch(const QList<CopyrightInfo> infos, const QString& languageCode) const
{
    QString langCode;
    QString fullCode = languageCode;
    if (languageCode.isNull())
    {
        // find local language
        KLocale *locale = KGlobal::locale();
        langCode = locale->language().toLower() + '-';
        fullCode = langCode + locale->country().toLower();
    }
    else if (languageCode == "x-default")
    {
        langCode = languageCode;
    }
    else
    {
        // en-us => en-
        langCode = languageCode.section('-', 0, 0, QString::SectionIncludeTrailingSep);
    }

    int fullCodeMatch, langCodeMatch, defaultCodeMatch, firstMatch;
    fullCodeMatch    = -1;
    langCodeMatch    = -1;
    defaultCodeMatch = -1;
    firstMatch       = -1;

    if (infos.isEmpty())
    {
        return -1;
    }
    else
    {
        firstMatch = 0; // index of first entry - at least we have one
    }

    // First we search for a full match
    // Second for a match of the language code
    // Third for the default code
    // Fourth we return the first comment

    QLatin1String defaultCode("x-default");

    for (int i=0; i<infos.size(); ++i)
    {
        const CopyrightInfo& info = infos[i];

        if (info.extraValue == fullCode)
        {
            fullCodeMatch = i;
            break;
        }
        else if (info.extraValue.startsWith(langCode) && langCodeMatch == -1)
        {
            langCodeMatch = i;
        }
        else if (info.extraValue == defaultCode)
        {
            defaultCodeMatch = i;
        }
    }

    int chosen = fullCodeMatch;
    if (chosen == -1)
        chosen = langCodeMatch;
    if (chosen == -1)
        chosen = defaultCodeMatch;
    if (chosen == -1)
        chosen = firstMatch;

    return chosen;
}

Template ImageCopyright::toMetadataTemplate()
{
    Template t;
    t.setAuthors(author());
    t.setAuthorsPosition(authorsPosition());
    t.setCredit(credit());
    t.setCopyright(rights("x-default"));
    t.setRightUsageTerms(rightsUsageTerms("x-default"));
    t.setSource(source());
    t.setInstructions(instructions());
    return t;
}

} // namespace Digikam
