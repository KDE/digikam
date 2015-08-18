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
#include "disjointmetadata.h"

#include <QMap>
#include <QStringList>
#include <QDateTime>

#include "captionvalues.h"
#include "template.h"
#include "templatemanager.h"
#include "tagscache.h"

#include "databaseaccess.h"
#include "imagecomments.h"
#include "imageinfo.h"

namespace Digikam
{

class DisjointMetadata::Private
{
public:
    Private()
    {
        dateTimeStatus    = DisjointMetadata::MetadataInvalid;
        pickLabelStatus   = DisjointMetadata::MetadataInvalid;
        colorLabelStatus  = DisjointMetadata::MetadataInvalid;
        ratingStatus      = DisjointMetadata::MetadataInvalid;
        titlesStatus      = DisjointMetadata::MetadataInvalid;
        commentsStatus    = DisjointMetadata::MetadataInvalid;
        templateStatus    = DisjointMetadata::MetadataInvalid;

        pickLabel         = -1;
        colorLabel        = -1;
        highestPickLabel  = -1;
        highestColorLabel = -1;

        rating            = -1;
        highestRating     = -1;

        count             = 0;

        dateTimeChanged   = false;
        titlesChanged     = false;
        commentsChanged   = false;
        pickLabelChanged  = false;
        colorLabelChanged = false;
        ratingChanged     = false;
        templateChanged   = false;
        tagsChanged       = false;
    }

    bool                              dateTimeChanged;
    bool                              titlesChanged;
    bool                              commentsChanged;
    bool                              pickLabelChanged;
    bool                              colorLabelChanged;
    bool                              ratingChanged;
    bool                              templateChanged;
    bool                              tagsChanged;

    int                               pickLabel;
    int                               highestPickLabel;
    int                               colorLabel;
    int                               highestColorLabel;
    int                               rating;
    int                               highestRating;
    int                               count;

    QDateTime                         dateTime;
    QDateTime                         lastDateTime;

    CaptionsMap                       titles;
    CaptionsMap                       comments;

    Template                          metadataTemplate;

    QMap<int, DisjointMetadata::Status> tags;

    QStringList                       tagList;

    QMultiMap<QString, QVariant>      faceTagsList;

    DisjointMetadata::Status               dateTimeStatus;
    DisjointMetadata::Status               titlesStatus;
    DisjointMetadata::Status               commentsStatus;
    DisjointMetadata::Status               pickLabelStatus;
    DisjointMetadata::Status               colorLabelStatus;
    DisjointMetadata::Status               ratingStatus;
    DisjointMetadata::Status               templateStatus;

public:
    template <class T> void loadSingleValue(const T& data,
                                            T& storage,
                                            DisjointMetadata::Status& status);
};

DisjointMetadata::DisjointMetadata()
    :d(new Private())
{

}

DisjointMetadata::~DisjointMetadata()
{

}

DisjointMetadata& DisjointMetadata::operator=(const DisjointMetadata &other)
{
    (*d) = (*other.d);
    return *this;
}

void DisjointMetadata::load(const ImageInfo &info)
{
    CaptionsMap commentMap;
    CaptionsMap titleMap;

    {
        DatabaseAccess access;
        ImageComments comments = info.imageComments(access);
        commentMap             = comments.toCaptionsMap();
        titleMap               = comments.toCaptionsMap(DatabaseComment::Title);
    }

    Template tref = info.metadataTemplate();
    Template t    = TemplateManager::defaultManager()->findByContents(tref);
    //qCDebug(DIGIKAM_GENERAL_LOG) << "Found Metadata Template: " << t.templateTitle();

    load(info.dateTime(),
         titleMap,
         commentMap,
         info.colorLabel(),
         info.pickLabel(),
         info.rating(),
         t.isNull() ? tref : t);

    QList<int> tagIds = info.tagIds();
    loadTags(tagIds);
}


//-----------------------------Status -------------------------
DisjointMetadata::Status DisjointMetadata::dateTimeStatus() const
{
    return d->dateTimeStatus;
}

DisjointMetadata::Status DisjointMetadata::titlesStatus() const
{
    return d->titlesStatus;
}

DisjointMetadata::Status DisjointMetadata::commentsStatus() const
{
    return d->commentsStatus;
}

DisjointMetadata::Status DisjointMetadata::pickLabelStatus() const
{
    return d->pickLabelStatus;
}

DisjointMetadata::Status DisjointMetadata::colorLabelStatus() const
{
    return d->colorLabelStatus;
}

DisjointMetadata::Status DisjointMetadata::ratingStatus() const
{
    return d->ratingStatus;
}

DisjointMetadata::Status DisjointMetadata::templateStatus() const
{
    return d->templateStatus;
}

DisjointMetadata::Status DisjointMetadata::tagStatus(int albumId) const
{
    return d->tags.value(albumId, DisjointMetadata::MetadataInvalid);
}

DisjointMetadata::Status DisjointMetadata::tagStatus(const QString &tagPath) const
{
    return tagStatus(TagsCache::instance()->tagForPath(tagPath));
}

//---- Changed status ---------------------

bool DisjointMetadata::dateTimeChanged() const
{
    return d->dateTimeChanged;
}

bool DisjointMetadata::titlesChanged() const
{
    return d->titlesChanged;
}

bool DisjointMetadata::commentsChanged() const
{
    return d->commentsChanged;
}

bool DisjointMetadata::pickLabelChanged() const
{
    return d->pickLabelChanged;
}

bool DisjointMetadata::colorLabelChanged() const
{
    return d->colorLabelChanged;
}

bool DisjointMetadata::ratingChanged() const
{
    return d->ratingChanged;
}

bool DisjointMetadata::templateChanged() const
{
    return d->templateChanged;
}

bool DisjointMetadata::tagsChanged() const
{
    return d->tagsChanged;
}

void DisjointMetadata::setDateTime(const QDateTime &dateTime, DisjointMetadata::Status status)
{
    d->dateTimeStatus  = status;
    d->dateTime        = dateTime;
    d->dateTimeChanged = true;
}

void DisjointMetadata::setTitles(const CaptionsMap &titles, DisjointMetadata::Status status)
{
    d->titlesStatus  = status;
    d->titles        = titles;
    d->titlesChanged = true;
}

void DisjointMetadata::setComments(const CaptionsMap &comments, DisjointMetadata::Status status)
{
    d->commentsStatus  = status;
    d->comments        = comments;
    d->commentsChanged = true;
}

void DisjointMetadata::setPickLabel(int pickId, DisjointMetadata::Status status)
{
    d->pickLabelStatus  = status;
    d->pickLabel        = pickId;
    d->pickLabelChanged = true;
}

void DisjointMetadata::setColorLabel(int colorId, DisjointMetadata::Status status)
{
    d->colorLabelStatus  = status;
    d->colorLabel        = colorId;
    d->colorLabelChanged = true;
}

void DisjointMetadata::setRating(int rating, DisjointMetadata::Status status)
{
    d->ratingStatus   = status;
    d->rating         = rating;
    d->ratingChanged  = true;
}

void DisjointMetadata::setMetadataTemplate(const Template &t, DisjointMetadata::Status status)
{
    d->templateStatus   = status;
    d->metadataTemplate = t;
    d->templateChanged  = true;
}

void DisjointMetadata::setTag(int albumID, DisjointMetadata::Status status)
{
    d->tags[albumID] = status;
    d->tagsChanged = true;
}


void DisjointMetadata::resetChanged()
{

    d->dateTimeChanged   = false;
    d->titlesChanged     = false;
    d->commentsChanged   = false;
    d->pickLabelChanged  = false;
    d->colorLabelChanged = false;
    d->ratingChanged     = false;
    d->templateChanged   = false;
    d->tagsChanged       = false;

}

bool DisjointMetadata::write(ImageInfo info, WriteMode writeMode)
{
//    applyChangeNotifications();

    bool changed = false;

    // find out in advance if we have something to write - needed for FullWriteIfChanged mode
    bool saveTitle      = (d->titlesStatus     == MetadataAvailable);
    bool saveComment    = (d->commentsStatus   == MetadataAvailable);
    bool saveDateTime   = (d->dateTimeStatus   == MetadataAvailable);
    bool savePickLabel  = (d->pickLabelStatus  == MetadataAvailable);
    bool saveColorLabel = (d->colorLabelStatus == MetadataAvailable);
    bool saveRating     = (d->ratingStatus     == MetadataAvailable);
    bool saveTemplate   = (d->templateStatus   == MetadataAvailable);
    bool saveTags       = true;


    bool writeAllFields;

    if (writeMode == FullWrite)
    {
        writeAllFields = true;
    }
    else if (writeMode == FullWriteIfChanged)
    {
        writeAllFields = (
                             (saveTitle      && d->titlesChanged)     ||
                             (saveComment    && d->commentsChanged)   ||
                             (saveDateTime   && d->dateTimeChanged)   ||
                             (savePickLabel  && d->pickLabelChanged)  ||
                             (saveColorLabel && d->colorLabelChanged) ||
                             (saveRating     && d->ratingChanged)     ||
                             (saveTemplate   && d->templateChanged)   ||
                             (saveTags       && d->tagsChanged)
                         );
    }
    else // PartialWrite
    {
        writeAllFields = false;
    }

    if (saveTitle && (writeAllFields || d->titlesChanged))
    {
        DatabaseAccess access;
        ImageComments comments = info.imageComments(access);
        comments.replaceComments(d->titles, DatabaseComment::Title);
        changed                = true;
    }

    if (saveComment && (writeAllFields || d->commentsChanged))
    {
        DatabaseAccess access;
        ImageComments comments = info.imageComments(access);
        comments.replaceComments(d->comments);
        changed                = true;
    }

    if (saveDateTime && (writeAllFields || d->dateTimeChanged))
    {
        info.setDateTime(d->dateTime);
        changed = true;
    }

    if (savePickLabel && (writeAllFields || d->pickLabelChanged))
    {
        info.setPickLabel(d->pickLabel);
        changed = true;
    }

    if (saveColorLabel && (writeAllFields || d->colorLabelChanged))
    {
        info.setColorLabel(d->colorLabel);
        changed = true;
    }

    if (saveRating && (writeAllFields || d->ratingChanged))
    {
        info.setRating(d->rating);
        changed = true;
    }

    if (saveTemplate && writeAllFields)
    {
        QString title = d->metadataTemplate.templateTitle();

        if (title == Template::removeTemplateTitle())
        {
            info.removeMetadataTemplate();
        }

        if (title.isEmpty())
        {
            // Nothing to do.
        }
        else
        {
            info.setMetadataTemplate(d->metadataTemplate);
        }

        changed = true;
    }

    if (writeAllFields || d->tagsChanged)
    {
        QList<int> keys = d->tags.keys();
        foreach (int key, keys)
        {
            if (d->tags.value(key) == MetadataAvailable)
            {
                info.setTag(key);
                changed = true;
            }
            if(d->tags.value(key) == MetadataInvalid)
            {
                info.removeTag(key);
                changed = true;
            }
        }
    }

    return changed;
}

void DisjointMetadata::load(const QDateTime &dateTime,const CaptionsMap &titles,
                            const CaptionsMap &comment, int colorLabel,
                            int pickLabel, int rating, const Template &t)
{
    if (dateTime.isValid())
    {
        d->loadSingleValue<QDateTime>(dateTime, d->dateTime,d->dateTimeStatus);
    }

    d->loadSingleValue<int>(pickLabel, d->pickLabel, d->pickLabelStatus);

    d->loadSingleValue<int>(colorLabel, d->colorLabel, d->colorLabelStatus);

    d->loadSingleValue<int>(rating, d->rating,d->ratingStatus);

    d->loadSingleValue<CaptionsMap>(titles, d->titles, d->titlesStatus);

    d->loadSingleValue<CaptionsMap>(comment, d->comments, d->commentsStatus);

    d->loadSingleValue<Template>(t, d->metadataTemplate, d->templateStatus);
}

void DisjointMetadata::loadTags(const QList<int> &loadedTagIds)
{
    // If tags map is empty, set them all as Available
    if(d->tags.isEmpty())
    {
        foreach(int tagId, loadedTagIds)
        {
            if(TagsCache::instance()->isInternalTag(tagId))
            {
                continue;
            }
            d->tags[tagId] = MetadataAvailable;
        }
        return;
    }
    // We search for metadata available tags, and
    // it is not present in current list, set it to
    // disjoint
    QList<int> keySet = d->tags.keys();
    foreach(int key, keySet)
    {
        if(d->tags.value(key) == MetadataAvailable)
        {
            if(!loadedTagIds.contains(key))
            {
                d->tags[key] = MetadataDisjoint;
            }
        }
    }

    // new tags which are not yet in the set,
    // are added as Disjoint
    foreach(int tagId, loadedTagIds)
    {
        if(!d->tags.contains(tagId))
        {
            d->tags[tagId] = MetadataDisjoint;
        }
    }
}


QDateTime DisjointMetadata::dateTime() const
{
    return d->dateTime;
}

CaptionsMap DisjointMetadata::titles() const
{
    return d->titles;
}

CaptionsMap DisjointMetadata::comments() const
{
    return d->comments;
}

int DisjointMetadata::pickLabel() const
{
    return d->pickLabel;
}

int DisjointMetadata::colorLabel() const
{
    return d->colorLabel;
}

int DisjointMetadata::rating() const
{
    return d->rating;
}

Template DisjointMetadata::metadataTemplate() const
{
    return d->metadataTemplate;
}

void DisjointMetadata::dateTimeInterval(QDateTime& lowest, QDateTime& highest) const
{
    switch (d->dateTimeStatus)
    {
        case MetadataInvalid:
            lowest = highest = QDateTime();
            break;
        case MetadataAvailable:
            lowest = highest = d->dateTime;
            break;
        case MetadataDisjoint:
            lowest  = d->dateTime;
            highest = d->lastDateTime;
            break;
    }
}

void DisjointMetadata::pickLabelInterval(int& lowest, int& highest) const
{
    switch (d->pickLabelStatus)
    {
        case MetadataInvalid:
            lowest = highest = -1;
            break;
        case MetadataAvailable:
            lowest = highest = d->pickLabel;
            break;
        case MetadataDisjoint:
            lowest  = d->pickLabel;
            highest = d->highestPickLabel;
            break;
    }
}

void DisjointMetadata::colorLabelInterval(int& lowest, int& highest) const
{
    switch (d->colorLabelStatus)
    {
        case MetadataInvalid:
            lowest = highest = -1;
            break;
        case MetadataAvailable:
            lowest = highest = d->colorLabel;
            break;
        case MetadataDisjoint:
            lowest  = d->colorLabel;
            highest = d->highestColorLabel;
            break;
    }
}

void DisjointMetadata::ratingInterval(int& lowest, int& highest) const
{
    switch (d->ratingStatus)
    {
        case MetadataInvalid:
            lowest = highest = -1;
            break;
        case MetadataAvailable:
            lowest = highest = d->rating;
            break;
        case MetadataDisjoint:
            lowest  = d->rating;
            highest = d->highestRating;
            break;
    }
}

QStringList DisjointMetadata::keywords() const
{
    QStringList tagList;

    QList<int> keys = d->tags.keys();
    foreach (int key, keys)
    {
        if (d->tags.value(key) == MetadataAvailable)
        {
            tagList.append(TagsCache::instance()->tagPath(key, TagsCache::NoLeadingSlash));
        }
    }

    return tagList;
}

QMap<int, DisjointMetadata::Status> DisjointMetadata::tags() const
{
    // DatabaseMode == ManagedTags is assumed
    return d->tags;
}

QMap<int, DisjointMetadata::Status> DisjointMetadata::tagIDs() const
{
    // DatabaseMode == ManagedTags is assumed
    return QMap<int, DisjointMetadata::Status>(d->tags);
}


template <class T> void DisjointMetadata::Private::loadSingleValue(const T& data, T& storage,
                                                              DisjointMetadata::Status& status)
{
    switch (status)
    {
        case DisjointMetadata::MetadataInvalid:
            storage = data;
            status  = DisjointMetadata::MetadataAvailable;
            break;
        case DisjointMetadata::MetadataAvailable:

            // we have two values. If they are equal, status is unchanged
            if (data == storage)
            {
                break;
            }

            // they are not equal. We need to enter the disjoint state.
            status = DisjointMetadata::MetadataDisjoint;
            break;
        case DisjointMetadata::MetadataDisjoint:
            break;
    }
}

}
