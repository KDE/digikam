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
class NamespaceEntry{

public:

    enum NsSubspace {EXIV = 0, IPTC = 1, XMP = 2 };
    enum TagType {TAG = 0, TAGPATH = 1};
    enum SpecialOptions {NO_OPTS = 0,
                         COMMENT_ALTLANG = 1,
                         COMMENT_ATLLANGLIST = 2,
                         COMMENT_XMP = 3,
                         TAG_XMPBAG = 4,
                         TAG_XMPSEQ = 5};

    enum NamespaceType {TAGS = 0, RATING = 1, COMMENT = 2};

    NamespaceEntry(){
        this->isDefault = true;
        this->isDisabled = false;
    }

    /**
     * @brief NamespaceEntry -constructor for tag type namespace
     * @param name - namespace name
     * @param isPath - namespace with keywords or full path
     * @param separator - separator used for tag paths
     * @param extraXml  - xml to format tags
     */
    NamespaceEntry(QString name, TagType isPath, QString separator, QString extraXml,
                   NamespaceType nsType, int index)
    {
        this->namespaceName = name;
        this->tagPaths = isPath;
        this->separator = separator;
        this->extraXml = extraXml;
        this->nsType = nsType;
        this->index = index;
        this->specialOpts = NO_OPTS;
        this->secondNameOpts = NO_OPTS;
        this->isDefault = true;
        this->isDisabled = false;
    }

    /**
     * @brief NamespaceEntry -constructor to build a rating
     * @param name - namespace name
     * @param convertRatio - convert ration ex: 1:1 if namespace store 5 star rating
     *                       or 25:1 if rating must be stored from 0-100
     */
    NamespaceEntry(QString name, QList<int> convertRatio, NamespaceType nsType, int index)
    {
        this->namespaceName = name;
        this->convertRatio  = QList<int>(convertRatio);
        this->nsType = nsType;
        this->index = index;
        this->specialOpts = NO_OPTS;
        this->secondNameOpts = NO_OPTS;
        this->isDefault = true;
        this->isDisabled = false;
    }

    /**
     * @brief NamespaceEntry -constructor to build a comment
     * @param name - namespace name
     */
    NamespaceEntry(QString name, NamespaceType nsType, SpecialOptions comm, int index)
    {
        this->namespaceName = name;
        this->nsType = nsType;
        this->index = index;
        this->specialOpts = comm;
        this->secondNameOpts = NO_OPTS;
        this->isDefault = true;
        this->isDisabled = false;
    }

    NamespaceEntry(const NamespaceEntry& copy)
    {
        this->namespaceName = copy.namespaceName;
        this->alternativeName = copy.alternativeName;
        this->tagPaths      = copy.tagPaths;
        this->separator     = copy.separator;
        this->extraXml      = copy.extraXml;
        this->nsType        = copy.nsType;
        this->convertRatio  = QList<int>(copy.convertRatio);
        this->specialOpts   = copy.specialOpts;
        this->secondNameOpts = copy.secondNameOpts;
        this->index         = copy.index;
        this->subspace      = copy.subspace;
        this->isDefault     = copy.isDefault;
        this->isDisabled    = copy.isDisabled;
    }

    ~NamespaceEntry(){}

    NamespaceType nsType;
    NsSubspace    subspace;
    bool          isDefault;
    bool          isDisabled;
    int index;

    /**
     * Tag Options
     */
    QString namespaceName;
    QString alternativeName;
    TagType tagPaths;
    QString separator;
    QString extraXml;


    /**
     * Rating Options
     */
    QList<int> convertRatio;


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
    ~DMetadataSettingsContainer()
    {
    }

public:

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;

    /**
     * @brief defaultValues - default namespaces used by digiKam
     */
    void defaultValues();

    QList<NamespaceEntry> readTagNamespaces;
    QList<NamespaceEntry> readRatingNamespaces;
    QList<NamespaceEntry> readCommentNamespaces;

    QList<NamespaceEntry> writeTagNamespaces;
    QList<NamespaceEntry> writeRatingNamespaces;
    QList<NamespaceEntry> writeCommentNamespaces;
    bool unifyReadWrite;

private:
    void defaultTagValues();
    void defaultRatingValues();
    void defaultCommentValues();
    void readOneGroup(KConfigGroup& group, QString name, QList<NamespaceEntry> &container);
    void writeOneGroup(KConfigGroup& group, QString name, QList<NamespaceEntry> container) const;

};

}  // namespace Digikam


#endif  // METADATASETTINGSCONTAINER_H
