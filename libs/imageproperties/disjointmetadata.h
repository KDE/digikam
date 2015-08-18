/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-17
 * Description : Helper class for Image Description Editor Tab
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
#ifndef DISJOINTMETADATA_H
#define DISJOINTMETADATA_H

#include <QString>
#include <QMap>

class QDateTime;

namespace Digikam
{
class ImageInfo;
class CaptionsMap;
class Template;

class DisjointMetadata
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

    DisjointMetadata();
    ~DisjointMetadata();

    DisjointMetadata& operator=(const DisjointMetadata& other);

    void load(const ImageInfo& info);


    //** Status **//
    Status dateTimeStatus()   const;
    Status titlesStatus()     const;
    Status commentsStatus()   const;
    Status pickLabelStatus()  const;
    Status colorLabelStatus() const;
    Status ratingStatus()     const;
    Status templateStatus()   const;

    Status tagStatus(int albumId) const;
    Status tagStatus(const QString& tagPath) const;

    /**
        Returns if the metadata field has been changed
        with the corresponding set... method
    */
    bool dateTimeChanged()   const;
    bool titlesChanged()     const;
    bool commentsChanged()   const;
    bool pickLabelChanged()  const;
    bool colorLabelChanged() const;
    bool ratingChanged()     const;
    bool templateChanged()   const;
    bool tagsChanged()       const;

    /**
        Set dateTime to the given value, and the dateTime status to MetadataAvailable
    */
    void setDateTime(const QDateTime& dateTime, Status status = MetadataAvailable);
    void setTitles(const CaptionsMap& titles, Status status = MetadataAvailable);
    void setComments(const CaptionsMap& comments, Status status = MetadataAvailable);
    void setPickLabel(int pickId, Status status = MetadataAvailable);
    void setColorLabel(int colorId, Status status = MetadataAvailable);
    void setRating(int rating, Status status = MetadataAvailable);
    void setMetadataTemplate(const Template& t, Status status = MetadataAvailable);
    void setTag(int albumID, Status status = MetadataAvailable);

    /**
        Returns the dateTime.
        If status is MetadataDisjoint, the earliest date is returned.
                                       (see dateTimeInterval())
        If status is MetadataInvalid, an invalid date is returned.
    */
    QDateTime   dateTime() const;

    /**
        Returns a map all alternate language titles.
        If status is MetadataDisjoint, the first loaded map is returned.
        If status is MetadataInvalid, CaptionMap() is returned.
    */
    CaptionsMap titles() const;

    /**
        Returns a map all alternate language omments .
        If status is MetadataDisjoint, the first loaded map is returned.
        If status is MetadataInvalid, CaptionMap() is returned.
    */
    CaptionsMap comments() const;

    /**
        Returns the Pick Label id (see PickLabel values in globals.h).
        If status is MetadataDisjoint, the None Label is returned.
                                       (see pickLabelInterval())
        If status is MetadataInvalid, -1 is returned.
    */
    int         pickLabel() const;

    /**
        Returns the Color Label id (see ColorLabel values in globals.h).
        If status is MetadataDisjoint, the None Label is returned.
                                       (see colorLabelInterval())
        If status is MetadataInvalid, -1 is returned.
    */
    int         colorLabel() const;

    /**
        Returns the rating.
        If status is MetadataDisjoint, the lowest rating is returned.
                                       (see ratingInterval())
        If status is MetadataInvalid, -1 is returned.
    */
    int         rating() const;

    /**
        Returns the metadata template.
        If status is MetadataDisjoint, the first loaded template is returned.
        If status is MetadataInvalid, 0 is returned.
    */
    Template metadataTemplate() const;

    /**
        Returns the earliest and latest date.
        If status is MetadataAvailable, the values are the same.
        If status is MetadataInvalid, invalid dates are returned.
    */
    void                dateTimeInterval(QDateTime& lowest, QDateTime& highest) const;

    /**
        Returns the lowest and highest Pick Label id (see PickLabel values from globals.h).
        If status is MetadataAvailable, the values are the same.
        If status is MetadataInvalid, -1 is returned.
    */
    void                pickLabelInterval(int& lowest, int& highest) const;

    /**
        Returns the lowest and highest Color Label id (see ColorLabel values from globals.h).
        If status is MetadataAvailable, the values are the same.
        If status is MetadataInvalid, -1 is returned.
    */
    void                colorLabelInterval(int& lowest, int& highest) const;

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
    QMap<int, Status> tags() const;

    /**
        Similar to the method above.
        This method is less efficient internally.
    */
    QMap<int, Status>   tagIDs() const;

    void resetChanged();

    /**
        Applies the set of metadata contained in this MetadataHub
        to the given ImageInfo object.
        @return Returns true if the info object has been changed
    */
    bool write(ImageInfo info, WriteMode writeMode = FullWrite);
protected:
    void load(const QDateTime& dateTime,
              const CaptionsMap& titles, const CaptionsMap& comment,
              int colorLabel, int pickLabel,
              int rating, const Template& t);
    void loadTags(const QList<int>& loadedTagIds);

private:
    class Private;
    Private *d;
};

}
#endif // DISJOINTMETADATA_H
