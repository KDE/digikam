/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface - comments helpers.
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

// Qt includes

#include <QLocale>

// Local includes

#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

CaptionsMap DMetadata::getImageComments(const DMetadataSettingsContainer& settings) const
{
    if (getFilePath().isEmpty())
    {
        return CaptionsMap();
    }

    CaptionsMap            captionsMap;
    MetaEngine::AltLangMap authorsMap;
    MetaEngine::AltLangMap datesMap;
    MetaEngine::AltLangMap commentsMap;
    QString                commonAuthor;

    // In first try to get captions properties from digiKam XMP namespace

    if (supportXmp())
    {
        authorsMap = getXmpTagStringListLangAlt("Xmp.digiKam.CaptionsAuthorNames",    false);
        datesMap   = getXmpTagStringListLangAlt("Xmp.digiKam.CaptionsDateTimeStamps", false);

        if (authorsMap.isEmpty() && commonAuthor.isEmpty())
        {
            QString xmpAuthors = getXmpTagString("Xmp.acdsee.author", false);

            if (!xmpAuthors.isEmpty())
            {
                authorsMap.insert(QLatin1String("x-default"), xmpAuthors);
            }
        }
    }

    // Get author name from IPTC DescriptionWriter. Private namespace above gets precedence.
    QVariant descriptionWriter = getMetadataField(MetadataInfo::DescriptionWriter);

    if (!descriptionWriter.isNull())
    {
        commonAuthor = descriptionWriter.toString();
    }

    // In first, we check XMP alternative language tags to create map of values.

    bool xmpSupported  = hasXmp();
    bool iptcSupported = hasIptc();
    bool exivSupported = hasExif();

    for (NamespaceEntry entry : settings.getReadMapping(QString::fromUtf8(DM_COMMENT_CONTAINER)))
    {
        if (entry.isDisabled)
            continue;

        QString commentString;
        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:
                switch(entry.specialOpts)
                {
                    case NamespaceEntry::COMMENT_ALTLANG:
                        if (xmpSupported)
                            commentString = getXmpTagStringLangAlt(nameSpace, QString(), false);
                        break;
                    case NamespaceEntry::COMMENT_ATLLANGLIST:
                        if (xmpSupported)
                            commentsMap = getXmpTagStringListLangAlt(nameSpace, false);
                        break;
                    case NamespaceEntry::COMMENT_XMP:
                        if (xmpSupported)
                            commentString = getXmpTagString("Xmp.acdsee.notes", false);
                        break;
                    case NamespaceEntry::COMMENT_JPEG:
                        // Now, we trying to get image comments, outside of XMP.
                        // For JPEG, string is extracted from JFIF Comments section.
                        // For PNG, string is extracted from iTXt chunk.
                        commentString = getCommentsDecoded();
                    default:
                        break;
                }
                break;
            case NamespaceEntry::IPTC:
                if (iptcSupported)
                    commentString = getIptcTagString(nameSpace, false);
                break;
            case NamespaceEntry::EXIF:
                if (exivSupported)
                    commentString = getExifComment();
                break;
            default:
                break;
        }

        if (!commentString.isEmpty() &&!commentString.trimmed().isEmpty())
        {
            commentsMap.insert(QLatin1String("x-default"), commentString);
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }

        if (!commentsMap.isEmpty())
        {
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }
    }

    return captionsMap;
}

bool DMetadata::setImageComments(const CaptionsMap& comments, const DMetadataSettingsContainer& settings) const
{
/*
    // See bug #139313: An empty string is also a valid value
    if (comments.isEmpty())
          return false;
*/

    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Comment: " << comments;

    // In first, set captions properties to digiKam XMP namespace

    if (supportXmp())
    {
        if (!setXmpTagStringListLangAlt("Xmp.digiKam.CaptionsAuthorNames", comments.authorsList()))
        {
            return false;
        }

        QString defaultAuthor  = comments.value(QLatin1String("x-default")).author;
        removeXmpTag("Xmp.acdsee.author");

        if (!defaultAuthor.isNull())
        {
            if (!setXmpTagString("Xmp.acdsee.author", defaultAuthor))
            {
                return false;
            }
        }

        if (!setXmpTagStringListLangAlt("Xmp.digiKam.CaptionsDateTimeStamps", comments.datesList()))
        {
            return false;
        }
    }

    QString defaultComment        = comments.value(QLatin1String("x-default")).caption;
    QList<NamespaceEntry> toWrite = settings.getReadMapping(QString::fromUtf8(DM_COMMENT_CONTAINER));

    if (!settings.unifyReadWrite())
        toWrite = settings.getWriteMapping(QString::fromUtf8(DM_COMMENT_CONTAINER));

    for (NamespaceEntry entry : toWrite)
    {
        if (entry.isDisabled)
            continue;

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:
                if (entry.namespaceName.contains(QLatin1String("Xmp.")))
                    removeXmpTag(nameSpace);

                switch(entry.specialOpts)
                {
                    case NamespaceEntry::COMMENT_ALTLANG:
                        if (!defaultComment.isNull())
                        {
                            if (!setXmpTagStringLangAlt(nameSpace, defaultComment, QString()))
                            {
                                qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image comment failed" << nameSpace;
                                return false;
                            }
                        }
                        break;

                    case NamespaceEntry::COMMENT_ATLLANGLIST:
                        if (!setXmpTagStringListLangAlt(nameSpace, comments.toAltLangMap()))
                        {
                            return false;
                        }
                        break;

                    case NamespaceEntry::COMMENT_XMP:
                        if (!defaultComment.isNull())
                        {
                            if (!setXmpTagString(nameSpace, defaultComment))
                            {
                                return false;
                            }
                        }
                        break;

                    case NamespaceEntry::COMMENT_JPEG:
                        // In first we set image comments, outside of Exif, XMP, and IPTC.
                        if (!setComments(defaultComment.toUtf8()))
                        {
                            return false;
                        }
                        break;

                    default:
                        break;
                }
                break;

            case NamespaceEntry::IPTC:
                removeIptcTag(nameSpace);

                if (!defaultComment.isNull())
                {
                    defaultComment.truncate(2000);

                    if (!setIptcTagString(nameSpace, defaultComment))
                    {
                        return false;
                    }
                }
                break;

            case NamespaceEntry::EXIF:
                if (!setExifComment(defaultComment))
                {
                    return false;
                }
                break;

            default:
                break;
        }
    }

    return true;
}

CaptionsMap DMetadata::getImageTitles() const
{
    if (getFilePath().isEmpty())
        return CaptionsMap();

    CaptionsMap            captionsMap;
    MetaEngine::AltLangMap authorsMap;
    MetaEngine::AltLangMap datesMap;
    MetaEngine::AltLangMap titlesMap;
    QString                commonAuthor;

    // Get author name from IPTC DescriptionWriter. Private namespace above gets precedence.
    QVariant descriptionWriter = getMetadataField(MetadataInfo::DescriptionWriter);

    if (!descriptionWriter.isNull())
        commonAuthor = descriptionWriter.toString();

    // In first, we check XMP alternative language tags to create map of values.

    if (hasXmp())
    {
        titlesMap = getXmpTagStringListLangAlt("Xmp.dc.title", false);

        if (!titlesMap.isEmpty())
        {
            captionsMap.setData(titlesMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }

        QString xmpTitle = getXmpTagString("Xmp.acdsee.caption" ,false);

        if (!xmpTitle.isEmpty() && !xmpTitle.trimmed().isEmpty())
        {
            titlesMap.insert(QLatin1String("x-default"), xmpTitle);
            captionsMap.setData(titlesMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }
    }

    // We trying to get IPTC title

    if (hasIptc())
    {
        QString iptcTitle = getIptcTagString("Iptc.Application2.ObjectName", false);

        if (!iptcTitle.isEmpty() && !iptcTitle.trimmed().isEmpty())
        {
            titlesMap.insert(QLatin1String("x-default"), iptcTitle);
            captionsMap.setData(titlesMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }
    }

    return captionsMap;
}

bool DMetadata::setImageTitles(const CaptionsMap& titles) const
{
    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Title: " << titles;

    QString defaultTitle = titles[QLatin1String("x-default")].caption;

    // In First we write comments into XMP. Language Alternative rule is not yet used.

    if (supportXmp())
    {
        // NOTE : setXmpTagStringListLangAlt remove xmp tag before to add new values
        if (!setXmpTagStringListLangAlt("Xmp.dc.title", titles.toAltLangMap()))
        {
            return false;
        }

        removeXmpTag("Xmp.acdsee.caption");

        if (!defaultTitle.isEmpty())
        {
            if (!setXmpTagString("Xmp.acdsee.caption", defaultTitle))
            {
                return false;
            }
        }
    }

    // In Second we write comments into IPTC.
    // Note that Caption IPTC tag is limited to 64 char and ASCII charset.

    removeIptcTag("Iptc.Application2.ObjectName");

    if (!defaultTitle.isNull())
    {
        defaultTitle.truncate(64);

        // See if we have any non printable chars in there. If so, skip IPTC
        // to avoid confusing other apps and web services with invalid tags.
        bool hasInvalidChar = false;

        for (QString::const_iterator c = defaultTitle.constBegin(); c != defaultTitle.constEnd(); ++c)
        {
            if (!(*c).isPrint())
            {
                hasInvalidChar = true;
                break;
            }
        }

        if (!hasInvalidChar)
        {
            if (!setIptcTagString("Iptc.Application2.ObjectName", defaultTitle))
                return false;
        }
    }

    return true;
}

MetaEngine::AltLangMap DMetadata::toAltLangMap(const QVariant& var)
{
    MetaEngine::AltLangMap map;

    if (var.isNull())
    {
        return map;
    }

    switch (var.type())
    {
        case QVariant::String:
            map.insert(QLatin1String("x-default"), var.toString());
            break;
        case QVariant::Map:
        {
            QMap<QString, QVariant> varMap = var.toMap();

            for (QMap<QString, QVariant>::const_iterator it = varMap.constBegin(); it != varMap.constEnd(); ++it)
            {
                map.insert(it.key(), it.value().toString());
            }

            break;
        }
        default:
            break;
    }

    return map;
}

} // namespace Digikam
