/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : DMetadata Settings Container.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#ifndef DMETADATASETTINGSCONTAINER_H
#define DMETADATASETTINGSCONTAINER_H

// Qt includes

#include <QFlags>
#include <QString>
#include <QMap>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

/**
 * @brief The NamespaceEntry class provide a simple container
 *        for dmetadata namespaces variables, such
 *        as names, what types of data expects and extra
 *        xml tags
 */

const char* const DM_TAG_CONTAINER     = I18N_NOOP("Tags");
const char* const DM_RATING_CONTAINER  = I18N_NOOP("Rating");
const char* const DM_COMMENT_CONTAINER = I18N_NOOP("Comment");

class NamespaceEntry
{

public:

    enum NsSubspace
    {
        EXIF = 0,
        IPTC = 1,
        XMP  = 2
    };

    enum TagType
    {
        TAG     = 0,
        TAGPATH = 1
    };

    enum SpecialOptions
    {
        NO_OPTS             = 0,
        COMMENT_ALTLANG     = 1,
        COMMENT_ATLLANGLIST = 2,
        COMMENT_XMP         = 3,
        COMMENT_JPEG        = 4,
        TAG_XMPBAG          = 5,
        TAG_XMPSEQ          = 6,
        TAG_ACDSEE          = 7
    };

    enum NamespaceType
    {
        TAGS    = 0,
        RATING  = 1,
        COMMENT = 2
    };

public:

    NamespaceEntry()
    {
        specialOpts    = NO_OPTS;
        secondNameOpts = NO_OPTS;
        isDefault      = true;
        isDisabled     = false;
        nsType         = TAGS;
        subspace       = XMP;
        index          = -1;
        tagPaths       = TAGPATH;
    }

    NamespaceEntry(const NamespaceEntry& copy)
    {
        this->namespaceName   = copy.namespaceName;
        this->alternativeName = copy.alternativeName;
        this->tagPaths        = copy.tagPaths;
        this->separator       = copy.separator;
        this->nsType          = copy.nsType;
        this->convertRatio    = QList<int>(copy.convertRatio);
        this->specialOpts     = copy.specialOpts;
        this->secondNameOpts  = copy.secondNameOpts;
        this->index           = copy.index;
        this->subspace        = copy.subspace;
        this->isDefault       = copy.isDefault;
        this->isDisabled      = copy.isDisabled;
    }

    ~NamespaceEntry()
    {
    }

public:

    NamespaceType  nsType;
    NsSubspace     subspace;
    bool           isDefault;
    bool           isDisabled;
    int            index;

    /**
     * Tag Options
     */
    QString        namespaceName;
    QString        alternativeName;
    TagType        tagPaths;
    QString        separator;

    /**
     * Rating Options
     */
    QList<int>     convertRatio;

    SpecialOptions specialOpts;
    SpecialOptions secondNameOpts;
};

/**
    The class DMetadataSettingsContainer is designed to dynamically add namespaces.
*/
class DIGIKAM_EXPORT DMetadataSettingsContainer
{
public:

    DMetadataSettingsContainer();
    DMetadataSettingsContainer(const DMetadataSettingsContainer& other);
    ~DMetadataSettingsContainer();
    DMetadataSettingsContainer& operator=(const DMetadataSettingsContainer& other);

public:

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;

    /**
     * @brief defaultValues - default namespaces used by digiKam
     */
    void defaultValues();

    bool unifyReadWrite() const;
    void setUnifyReadWrite(bool b);

    void addMapping(const QLatin1String& key);

    QList<NamespaceEntry>& getReadMapping(const QLatin1String& key) const;

    QList<NamespaceEntry>& getWriteMapping(const QLatin1String& key) const;

    QList<QLatin1String> mappingKeys() const;

private:

    void defaultTagValues();
    void defaultRatingValues();
    void defaultCommentValues();
    void readOneGroup(KConfigGroup& group, const QString& name, QList<NamespaceEntry>& container);
    void writeOneGroup(KConfigGroup& group, const QString& name, QList<NamespaceEntry>& container) const;

    class Private;
    Private* d;
};

}  // namespace Digikam

#endif  // DMETADATASETTINGSCONTAINER_H
