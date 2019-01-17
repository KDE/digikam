/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - tags helpers.
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::getItemTagsPath(QStringList& tagsPath,
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
                        //qCDebug(DIGIKAM_METAENGINE_LOG) << "Tags Path imported from Imach: " << tagsPath;
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

bool DMetadata::setItemTagsPath(const QStringList& tagsPath, const DMetadataSettingsContainer& settings) const
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

        foreach (const QString& tags, xmlTags)
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
            //qCDebug(DIGIKAM_METAENGINE_LOG) << "Tags Path imported from ACDSee: " << tagsPath;
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

    //qCDebug(DIGIKAM_METAENGINE_LOG) << "xmlACDSee" << xmlACDSee;

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

} // namespace Digikam
