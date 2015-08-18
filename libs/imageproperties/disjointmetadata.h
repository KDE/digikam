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

    DisjointMetadata();
    ~DisjointMetadata();

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
