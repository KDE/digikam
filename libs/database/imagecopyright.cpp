/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-12
 * Description : Access to copyright info of an image in the database
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

// KDE includes

#include <klocale.h>
#include <kglobal.h>

// Local includes

#include "ddebug.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "imagecopyright.h"

namespace Digikam
{

ImageCopyright::ImageCopyright(qlonglong imageid)
    : m_id(imageid)
{
}

QStringList ImageCopyright::creator()
{
    QList<CopyrightInfo> infos = DatabaseAccess().db()->getImageCopyright(m_id, "creator");
    QStringList list;
    foreach(const CopyrightInfo &info, infos)
    {
        list << info.value;
    }
    return list;
}

void ImageCopyright::setCreator(const QString &creator, ReplaceMode mode)
{
    AlbumDB::CopyrightPropertyUnique uniqueness;
    if (mode == ReplaceAllEntries)
        uniqueness = AlbumDB::PropertyUnique;
    else
        uniqueness = AlbumDB::PropertyNoConstraint;

    DatabaseAccess().db()->setImageCopyrightProperty(m_id, "creator", creator, QString(), uniqueness);
}

QString ImageCopyright::provider()
{
    return readSimpleProperty("provider");
}

void ImageCopyright::setProvider(const QString &provider)
{
    setSimpleProperty("provider", provider);
}

QString ImageCopyright::copyrightNotice(const QString &languageCode)
{
    return readLanguageProperty("copyrightNotice", languageCode);
}

void ImageCopyright::setCopyrightNotice(const QString &notice, const QString &languageCode, ReplaceMode mode)
{
    setLanguageProperty("copyrightNotice", notice, languageCode, mode);
}

QString ImageCopyright::rightsUsageTerm(const QString &languageCode)
{
    return readLanguageProperty("rightsUsageTerm", languageCode);
}

void ImageCopyright::setRightsUsageTerm(const QString &term, const QString &languageCode, ReplaceMode mode)
{
    setLanguageProperty("rightsUsageTerm", term, languageCode, mode);
}

QString ImageCopyright::source()
{
    return readSimpleProperty("source");
}

void ImageCopyright::setSource(const QString &source)
{
    setSimpleProperty("source", source);
}

QString ImageCopyright::creatorJobTitle()
{
    return readSimpleProperty("creatorJobTitle");
}

void ImageCopyright::setCreatorJobTitle(const QString &title)
{
    setSimpleProperty("creatorJobTitle", title);
}

QString ImageCopyright::instructions()
{
    return readSimpleProperty("instructions");
}

void ImageCopyright::setInstructions(const QString &instructions)
{
    setSimpleProperty("instructions", instructions);
}


QString ImageCopyright::readSimpleProperty(const QString &property)
{
    QList<CopyrightInfo> infos = DatabaseAccess().db()->getImageCopyright(m_id, property);
    if (infos.isEmpty())
        return QString();
    else
        return infos.first().value;
}

void ImageCopyright::setSimpleProperty(const QString &property, const QString &value)
{
    DatabaseAccess().db()->setImageCopyrightProperty(m_id, property, value, QString(), AlbumDB::PropertyUnique);
}

QString ImageCopyright::readLanguageProperty(const QString &property, const QString &languageCode)
{
    QList<CopyrightInfo> infos = DatabaseAccess().db()->getImageCopyright(m_id, property);
    int index = languageMatch(infos, languageCode);
    if (index == -1)
        return QString();
    else
        return infos[index].value;
}

void ImageCopyright::setLanguageProperty(const QString &property, const QString &value, const QString &languageCode, ReplaceMode mode)
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

int ImageCopyright::languageMatch(const QList<CopyrightInfo> infos, const QString &languageCode) const
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
    fullCodeMatch = -1;
    langCodeMatch = -1;
    defaultCodeMatch = -1;
    firstMatch = -1;

    if (infos.isEmpty())
    {
        return -1;
    }
    else
        firstMatch = 0; // index of first entry - at least we have one

    // First we search for a full match
    // Second for a match of the language code
    // Third for the default code
    // Fourth we return the first comment

    QLatin1String defaultCode("x-default");

    for (int i=0; i<infos.size(); i++)
    {
        const CopyrightInfo &info = infos[i];

        if (info.extraValue == fullCode)
        {
            fullCodeMatch = i;
            break;
        }
        else if (info.extraValue.startsWith(langCode) && langCodeMatch == -1)
            langCodeMatch = i;
        else if (info.extraValue == defaultCode)
            defaultCodeMatch = i;
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




}

