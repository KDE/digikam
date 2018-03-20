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

#ifndef DISJOINT_METADATA_H
#define DISJOINT_METADATA_H

// Qt includes

#include <QString>
#include <QMap>
#include <QObject>
#include <QDateTime>

// Local includes

#include "metadatasettings.h"

namespace Digikam
{

class ImageInfo;
class CaptionsMap;
class Template;

class DisjointMetadata : public QObject
{
    Q_OBJECT

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

    DisjointMetadata(QObject* parent = 0);
    DisjointMetadata(const DisjointMetadata& other);
    ~DisjointMetadata();

    DisjointMetadata& operator=(const DisjointMetadata& other);

    void reset();

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
        Any tag that was set on one of the loaded images is contained in the map.
        (If a tag is not contained in the map, it was not set on any of the loaded images)
        If the tag was set on all loaded images, the status is MetadataAvailable.
        If the tag was set on at least one, but not all of the loaded images, the status is MetadataDisjoint.
     */
    QMap<int, Status> tags() const;

    void resetChanged();

    /**
        Applies the set of metadata contained in this MetadataHub
        to the given ImageInfo object.
        @return Returns true if the info object has been changed
     */
    bool write(ImageInfo info, WriteMode writeMode = FullWrite);
    /**
        With the currently applied changes, the given writeMode and settings,
        returns if write(DMetadata), write(QString) or write(DImg) will actually
        apply any changes.
     */
    bool willWriteMetadata(WriteMode writeMode,
                           const MetadataSettingsContainer& settings = MetadataSettings::instance()->settings()) const;

    /**
     * @brief changedFlags - used for selective metadata write. The result will be passed to metadatahub and it will
     *                     - write it to disk
     * @return - metadatahub flags encoded as int
     */
    int changedFlags();

protected:

    void load(const QDateTime& dateTime,
              const CaptionsMap& titles,
              const CaptionsMap& comment,
              int colorLabel, int pickLabel,
              int rating, const Template& t);

    void loadTags( QList<int>& loadedTagIds);
    void notifyTagDeleted(int id);

// Former MetadataHubOnTheRoad implementation
protected Q_SLOTS:

    void slotTagDeleted(int tagId);
    void slotInvalidate();

private:

    virtual void applyChangeNotifications();

private:

    class Private;
    Private *d;
};

} // namespace Digikam

#endif // DISJOINT_METADATA_H
