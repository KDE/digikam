/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-05
 * Description : Metadata handling
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATAHUB_H
#define METADATAHUB_H

// Qt includes

#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QMap>

// KDE includes

#include <kurl.h>

// Local includes

#include "dmetadata.h"
#include "dimg.h"

using namespace KExiv2Iface;

namespace Digikam
{

class AlbumSettings;
class ImageInfo;
class Template;
class TAlbum;
class MetadataHubPriv;

class MetadataWriteSettings
{
    /**
        The class MetadataWriteSettings encapsulates all metadata related settings that are available
        from the AlbumSettings.
        This allows supply changed arguments to MetadataHub without changing the global settings
    */
public:

    /**
        Constructs a MetadataWriteSettings object with all boolean values set to false,
        all QString values set to QString()
    */
    MetadataWriteSettings();

    /**
        Constructs a MetadataWriteSettings object from the given AlbumSettings object
    */
    MetadataWriteSettings(AlbumSettings *albumsettings);

    bool saveComments;
    bool saveDateTime;
    bool saveRating;
    bool saveTemplate;
    bool saveTags;
    bool writeRawFiles;
    bool updateFileTimeStamp;
};

// ----------------------------------------------------------------------------------------

class MetadataHub
{
public:

    /**
        The status enum describes the result of joining several metadata sets.
        If only one set has been added, the status is always MetadataAvailable.
        If no set has been added, the status is always MetadataInvalid
    */
    enum Status
    {
        MetadataInvalid,   /// not yet filled with any value
        MetadataAvailable, /// only one data set has been added, or a common value is available
        MetadataDisjoint   /// No common value is available. For rating and dates, the interval is available.
    };

    /**
        Describes the complete status of a Tag: The metadata status, and the fact if it has the tag or not.
    */
    class TagStatus
    {
        public:
            TagStatus(Status status, bool hasTag = false) : status(status), hasTag(hasTag) {};
            TagStatus() : status(MetadataInvalid), hasTag(false) {};

            Status status;
            bool   hasTag;

            bool operator==(TagStatus otherstatus)
            {
                return otherstatus.status == status &&
                        otherstatus.hasTag == hasTag;
            }
            bool operator==(Status otherstatus) { return otherstatus == status; }
    };

    enum DatabaseMode
    {
        /**
            Use this mode if
                - the album manager is not available and/or
                - metadata sets may contain tags that are not available from the AlbumManager
            This situation occurs if new tags are imported from XMP/IPTC keywords.
            This means that the album manager is not accessed, all methods depending on TAlbum*
            (tags(), tagIDs(), setTag()) shall not be used.
            The method write(ImageInfo) will create not yet existing tags in the database.
        */
        NewTagsImport,
        /**
            Use this mode if all tags are available from the AlbumManager.
            This situation occurs if you load from ImageInfo objects.
            All methods can be used.
        */
        ManagedTags
    };

    enum WriteMode
    {
        /**
            Write all available information
        */
        FullWrite,
        /**
            Do a full write if and only if
                - metadata fields changed
                - the changed fields shall be written according to write settings
            "Changed" in this context means changed by one of the set... methods,
            the load() methods are ignored for this attribute.
            This mode allows to avoid write operations when e.g. the user does not want
            keywords to be written and only changes keywords.
        */
        FullWriteIfChanged,
        /**
            Write only the changed parts.
            Metadata fields which cannot be changed from MetadataHub (photographer ID etc.)
            will never be written
        */
        PartialWrite
    };

    /**
        Constructs a MetadataHub.
        @param dbmode Determines if the database may be accessed or not. See the enum description above.
    */
    MetadataHub(DatabaseMode dbmode = ManagedTags);
    ~MetadataHub();
    MetadataHub& operator=(const MetadataHub &);
    MetadataHub(const MetadataHub &);

    void reset();

    // --------------------------------------------------

    /**
        Add metadata information contained in the ImageInfo object.
        This method (or in combination with the other load methods)
        can be called multiple times on the same MetadataHub object.
        In this case, the metadata will be combined.
    */
    void load(const ImageInfo& info);

    /**
        Add metadata information from the DMetadata object
    */
    void load(const DMetadata& metadata);

    /**
        Load metadata information from the given file.
        (Uses DMetadata, QFileInfo)
        @returns True if the metadata could be loaded
    */
    bool load(const QString& filePath);

    // --------------------------------------------------

    /**
        Applies the set of metadata contained in this MetadataHub
        to the given ImageInfo object.
        @return Returns true if the info object has been changed
    */
    bool write(ImageInfo info, WriteMode writeMode = FullWrite);

    /**
        Applies the set of metadata contained in this MetadataHub
        to the given DMetadata object.
        The MetadataWriteSettings determine whether data is actually
        set or not.
        The following metadata fields may be set (depending on settings):
        - Comment
        - Date
        - Rating
        - Tags
        - Photographer ID (data from settings)
        - Credits (data from settings)

        The data fields taken from this MetadataHub object are only set if
        their status is MetadataAvailable.
        If the status is MetadataInvalid or MetadataDisjoint, the respective
        metadata field is not touched.
        @return Returns true if the metadata object has been touched
    */
    bool write(DMetadata& metadata, WriteMode writeMode = FullWrite,
               const MetadataWriteSettings& settings = defaultWriteSettings());

    /**
        Constructs a DMetadata object for given filePath,
        calls the above method, writes the changes out to the file,
        and notifies the ImageAttributesWatch.
        @return Returns if the file has been touched
    */
    bool write(const QString& filePath, WriteMode writeMode = FullWrite,
               const MetadataWriteSettings& settings = defaultWriteSettings());

    /**
        Constructs a DMetadata object from the metadata stored in the given DImg object,
        calls the above method, and changes the stored metadata in the DImg object.
        @return Returns if the DImg object has been touched
    */
    bool write(DImg& image, WriteMode writeMode = FullWrite,
               const MetadataWriteSettings& settings = defaultWriteSettings());

    /**
        Constructs a MetadataWriteSettings object from the global AlbumSettings object.
    */
    static MetadataWriteSettings defaultWriteSettings();

    /**
        With the currently applied changes, the given writeMode and settings,
        returns if write(DMetadata), write(QString) or write(DImg) will actually
        apply any changes.
    */
    bool willWriteMetadata(WriteMode writeMode,
                           const MetadataWriteSettings& settings = defaultWriteSettings()) const;

    // --------------------------------------------------

    Status dateTimeStatus() const;
    Status commentsStatus() const;
    Status ratingStatus()   const;
    Status templateStatus() const;

    TagStatus tagStatus(TAlbum *album) const;
    TagStatus tagStatus(int albumId) const;
    TagStatus tagStatus(const QString& tagPath) const;

    /**
        Returns if the metadata field has been changed
        with the corresponding set... method
    */
    bool dateTimeChanged() const;
    bool commentsChanged() const;
    bool ratingChanged()   const;
    bool templateChanged() const;
    bool tagsChanged()     const;

    /**
        Returns the dateTime.
        If status is MetadataDisjoint, the earliest date is returned.
                                       (see dateTimeInterval())
        If status is MetadataInvalid, an invalid date is returned.
    */
    QDateTime           dateTime() const;
    /**
        Returns a map all alternate language omments .
        If status is MetadataDisjoint, the first loaded map is returned.
        If status is MetadataInvalid, KExiv2::AltLangMap() is returned.
    */
    KExiv2::AltLangMap  comments() const;
    /**
        Returns the rating.
        If status is MetadataDisjoint, the lowest rating is returned.
                                       (see ratingInterval())
        If status is MetadataInvalid, -1 is returned.
    */
    int                 rating() const;
    /**
        Returns the metadata template.
        If status is MetadataDisjoint, FIXME
        If status is MetadataInvalid, 0 is returned.
    */
    Template* metadataTemplate() const;
    /**
        Returns the earliest and latest date.
        If status is MetadataAvailable, the values are the same.
        If status is MetadataInvalid, invalid dates are returned.
    */
    void                dateTimeInterval(QDateTime& lowest, QDateTime& highest) const;
    /**
        Returns the lowest and highest rating.
        If status is MetadataAvailable, the values are the same.
        If status is MetadataInvalid, -1 is returned.
    */
    void                ratingInterval(int& lowest, int& highest) const;

    /**
        Returns a QStringList with all tags with status MetadataAvailable.
        (i.e., the intersection of tags from all loaded metadata sets)
    */
    QStringList         keywords() const;

    /**
        Returns a map with the status for each tag.
        Tags not contained in the list are considered to have the status MetadataInvalid,
        that means no loaded metadata set contained this tag.
        If a tag in the map has the status MetadataAvailable and it has the tag,
        all loaded sets contained the tag.
        If a tag in the map has the status MetadataAvailable and it does not have the tag,
        no loaded sets contains this tag (has been explicitly set so)
        If a tag in the map has the status MetadataDisjoint, some but not all loaded
        sets contained the tag. The hasTag value is true then.
        If MapMode (set in constructor) is false, returns an empty map.
    */
    QMap<TAlbum *, TagStatus> tags() const;
    /**
        Similar to the method above.
        This method is less efficient internally.
    */
    QMap<int, TagStatus>   tagIDs() const;

    // --------------------------------------------------

    /**
        Set dateTime to the given value, and the dateTime status to MetadataAvailable
    */
    void setDateTime(const QDateTime& dateTime, Status status = MetadataAvailable);
    void setComments(const KExiv2::AltLangMap& comments, Status status = MetadataAvailable);
    void setRating(int rating, Status status = MetadataAvailable);
    void setMetadataTemplate(Template* t, Status status = MetadataAvailable);
    void setTag(TAlbum *tag, bool hasTag, Status status = MetadataAvailable);
    void setTag(int albumID, bool hasTag, Status status = MetadataAvailable);

    /**
        Resets the information that metadata fields have been changed with one of the
        set... methods (see commentChanged, dateTimeChanged etc.)
    */
    void resetChanged();

private:

    void load(const QDateTime& dateTime, const KExiv2::AltLangMap& comment, int rating, Template* t);
    void loadTags(const QList<TAlbum *>& loadedTags);
    void loadTags(const QStringList& loadedTagPaths);

private:

    MetadataHubPriv* const d;
};

} // namespace Digikam

#endif // METADATAHUB_H
