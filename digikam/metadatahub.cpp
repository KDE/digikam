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
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "metadatahub.h"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <kdebug.h>

// Libkexiv2 includes

#include <libkexiv2/version.h>

// Local includes

#include "imageinfo.h"
#include "template.h"
#include "templatemanager.h"
#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "imageattributeswatch.h"

namespace Digikam
{

class MetadataHubPriv
{
public:

    MetadataHubPriv()
    {
        dateTimeStatus   = MetadataHub::MetadataInvalid;
        ratingStatus     = MetadataHub::MetadataInvalid;
        commentsStatus   = MetadataHub::MetadataInvalid;
        templateStatus   = MetadataHub::MetadataInvalid;

        rating           = -1;
        highestRating    = -1;

        count            = 0;

        dbmode           = MetadataHub::ManagedTags;

        dateTimeChanged  = false;
        commentsChanged  = false;
        ratingChanged    = false;
        templateChanged  = false;
        tagsChanged      = false;
    }

    bool                                  dateTimeChanged;
    bool                                  commentsChanged;
    bool                                  ratingChanged;
    bool                                  templateChanged;
    bool                                  tagsChanged;

    int                                   rating;
    int                                   highestRating;
    int                                   count;

    QDateTime                             dateTime;
    QDateTime                             lastDateTime;

    KExiv2::AltLangMap                    comments;

    Template                              metadataTemplate;

    QMap<TAlbum*, MetadataHub::TagStatus> tags;

    QStringList                           tagList;

    MetadataHub::Status                   dateTimeStatus;
    MetadataHub::Status                   commentsStatus;
    MetadataHub::Status                   ratingStatus;
    MetadataHub::Status                   templateStatus;
    MetadataHub::DatabaseMode             dbmode;

    template <class T> void loadWithInterval(const T &data, T &storage, T &highestStorage, MetadataHub::Status& status);
    template <class T> void loadSingleValue(const T &data, T &storage, MetadataHub::Status& status);
};

// ------------------------------------------------------------------------------------------------

MetadataWriteSettings::MetadataWriteSettings()
{
    saveComments        = false;
    saveDateTime        = false;
    saveRating          = false;
    saveTemplate        = false;
    saveTags            = false;
    writeRawFiles       = false;
    updateFileTimeStamp = false;
}

MetadataWriteSettings::MetadataWriteSettings(AlbumSettings *albumSettings)
{
    saveComments        = albumSettings->getSaveComments();
    saveDateTime        = albumSettings->getSaveDateTime();
    saveRating          = albumSettings->getSaveRating();
    saveTemplate        = albumSettings->getSaveTemplate();
    saveTags            = albumSettings->getSaveTags();
    writeRawFiles       = albumSettings->getWriteRawFiles();
    updateFileTimeStamp = albumSettings->getUpdateFileTimeStamp();
}

// ------------------------------------------------------------------------------------------

MetadataHub::MetadataHub(DatabaseMode dbmode)
           : d(new MetadataHubPriv)
{
    d->dbmode = dbmode;
}

MetadataHub::MetadataHub(const MetadataHub& other)
           : d(new MetadataHubPriv(*other.d))
{
}

MetadataHub::~MetadataHub()
{
    delete d;
}

MetadataHub& MetadataHub::operator=(const MetadataHub& other)
{
    (*d) = (*other.d);
    return *this;
}

void MetadataHub::reset()
{
    (*d) = MetadataHubPriv();
}

// --------------------------------------------------

void MetadataHub::load(const ImageInfo& info)
{
    d->count++;

    KExiv2::AltLangMap commentMap;
    {
        DatabaseAccess access;
        ImageComments comments = info.imageComments(access);
        commentMap             = comments.toAltLangMap();
    }
    Template t;
    {
        ImageCopyright cr = info.imageCopyright();
        t                 = TemplateManager::defaultManager()->findByContents(cr.toMetadataTemplate());
    }
    kDebug(50003) << "Found Metadata Template: " << t.templateTitle();

    load(info.dateTime(), commentMap, info.rating(), t);

    AlbumManager *man = AlbumManager::instance();
    QList<int> tagIds = info.tagIds();
    QList<TAlbum*> loadedTags;

    if (d->dbmode == ManagedTags)
    {
        QList<TAlbum *> loadedTags;
        for (QList<int>::const_iterator it = tagIds.constBegin(); it != tagIds.constEnd(); ++it)
        {
            TAlbum *album = man->findTAlbum(*it);
            if (!album)
            {
                kWarning(50003) << "Tag id " << *it << " not found in database.";
                continue;
            }
            loadedTags << album;
        }

        loadTags(loadedTags);
    }
    else
    {
        loadTags(man->tagPaths(info.tagIds(), false));
    }
}

void MetadataHub::load(const DMetadata& metadata)
{
    d->count++;

    KExiv2::AltLangMap comments;
    QStringList        keywords;
    QDateTime          datetime;
    int                rating;

    // Try to get comments from image :
    // In first, from Xmp comments tag,
    // In second, from standard JPEG JFIF comments section,
    // In third, from Exif comments tag,
    // In four, from Iptc comments tag.
    comments = metadata.getImageComments();

    // Try to get date and time from image :
    // In first, from Exif date & time tags,
    // In second, from Xmp date & time tags, or
    // In third, from Iptc date & time tags.
    // else use file system time stamp.
    datetime = metadata.getImageDateTime();
    if ( !datetime.isValid() )
    {
        QFileInfo info( metadata.getFilePath() );
        datetime = info.lastModified();
    }

    // Try to get image rating from Xmp tag, or Iptc Urgency tag
    rating = metadata.getImageRating();

    Template tref = metadata.getMetadataTemplate();
    Template t    = TemplateManager::defaultManager()->findByContents(tref);

    kDebug(50003) << "Found Metadata Template: " << t.templateTitle();

    load(datetime, comments, rating, t.isNull() ? tref : t);

    // Try to get image tags from Xmp using digiKam namespace tags.

    if (d->dbmode == ManagedTags)
    {
        QStringList tagPaths;
        if (metadata.getImageTagsPath(tagPaths))
        {
            AlbumManager *man = AlbumManager::instance();
            QList<TAlbum *> loadedTags;

            for (QStringList::const_iterator it = tagPaths.constBegin(); it != tagPaths.constEnd(); ++it)
            {
                TAlbum *album = man->findTAlbum(*it);
                if (!album)
                {
                    kWarning(50003) << "Tag id " << *it << " not found in database. Use NewTagsImport mode?";
                    continue;
                }
                loadedTags << album;
            }

            loadTags(loadedTags);
        }
    }
    else
    {
        QStringList tagPaths;
        if (metadata.getImageTagsPath(tagPaths))
            loadTags(tagPaths);
    }
}

bool MetadataHub::load(const QString& filePath)
{
    DMetadata metadata;
    bool success = metadata.load(filePath);
    load(metadata); // increments count
    return success;
}

// private common code to merge tags
void MetadataHub::loadTags(const QList<TAlbum *>& loadedTags)
{
    // get copy of tags
    QList<TAlbum *> previousTags = d->tags.keys();

    // first go through all tags contained in this set
    for (QList<TAlbum *>::const_iterator it = loadedTags.constBegin(); it != loadedTags.constEnd(); ++it)
    {
        // that is a reference
        TagStatus &status = d->tags[*it];
        // if it was not contained in the list, the default constructor will mark it as invalid
        if (status == MetadataInvalid)
        {
            if (d->count == 1)
                // there were no previous sets that could have contained the set
                status = TagStatus(MetadataAvailable, true);
            else
                // previous sets did not contain the tag, we do => disjoint
                status = TagStatus(MetadataDisjoint, true);
        }
        else if (status == TagStatus(MetadataAvailable, false))
        {
            // set to explicitly not contained, but we contain it => disjoint
            status = TagStatus(MetadataDisjoint, true);
        }
        // else if mapIt.value() ==  MetadataAvailable, true: all right, we contain it too
        // else if mapIt.value() ==  MetadataDisjoint: it's already disjoint

        // remove from the list to signal that this tag has been handled
        previousTags.removeAll(*it);
    }

    // Those tags which had been set as MetadataAvailable before,
    // but are not contained in this set, have to be set to MetadataDisjoint
    for (QList<TAlbum *>::const_iterator it = previousTags.constBegin(); it != previousTags.constEnd(); ++it)
    {
        QMap<TAlbum *, TagStatus>::iterator mapIt = d->tags.find(*it);
        if (mapIt != d->tags.end() && mapIt.value() == TagStatus(MetadataAvailable, true))
        {
            mapIt.value() = TagStatus(MetadataDisjoint, true);
        }
    }
}

// private code to merge tags with d->tagList
void MetadataHub::loadTags(const QStringList& loadedTagPaths)
{
    if (d->count == 1)
    {
        d->tagList = loadedTagPaths;
    }
    else
    {
        // a simple intersection
        QStringList toBeAdded;
        for (QStringList::iterator it = d->tagList.begin(); it != d->tagList.end(); ++it)
        {
            if (loadedTagPaths.indexOf(*it) == -1)
            {
                // it's not in the loadedTagPaths list. Remove it from intersection list.
                it = d->tagList.erase(it);
            }
            // else, it is in both lists, so no need to change d->tagList, it's already added.
        }
    }
}

// private common code to load dateTime, comment, rating
void MetadataHub::load(const QDateTime& dateTime, const KExiv2::AltLangMap& comments, int rating, const Template& t)
{
    if (dateTime.isValid())
    {
        d->loadWithInterval<QDateTime>(dateTime, d->dateTime, d->lastDateTime, d->dateTimeStatus);
    }

    d->loadWithInterval<int>(rating, d->rating, d->highestRating, d->ratingStatus);

    d->loadSingleValue<KExiv2::AltLangMap>(comments, d->comments, d->commentsStatus);

    d->loadSingleValue<Template>(t, d->metadataTemplate, d->templateStatus);
}

// template method to share code for dateTime and rating
template <class T> void MetadataHubPriv::loadWithInterval(const T& data, T& storage, T& highestStorage, MetadataHub::Status& status)
{
    switch (status)
    {
        case MetadataHub::MetadataInvalid:
            storage = data;
            status  = MetadataHub::MetadataAvailable;
            break;
        case MetadataHub::MetadataAvailable:
            // we have two values. If they are equal, status is unchanged
            if (data == storage)
                break;
            // they are not equal. We need to enter the disjoint state.
            status = MetadataHub::MetadataDisjoint;
            if (data > storage)
            {
                highestStorage = data;
            }
            else
            {
                highestStorage = storage;
                storage        = data;
            }
            break;
        case MetadataHub::MetadataDisjoint:
            // smaller value is stored in storage
            if (data < storage)
                storage = data;
            else if (highestStorage < data)
                highestStorage = data;
            break;
    }
}

// template method used by comment and template
template <class T> void MetadataHubPriv::loadSingleValue(const T& data, T& storage, MetadataHub::Status& status)
{
    switch (status)
    {
        case MetadataHub::MetadataInvalid:
            storage = data;
            status  = MetadataHub::MetadataAvailable;
            break;
        case MetadataHub::MetadataAvailable:
            // we have two values. If they are equal, status is unchanged
            if (data == storage)
                break;
            // they are not equal. We need to enter the disjoint state.
            status = MetadataHub::MetadataDisjoint;
            break;
        case MetadataHub::MetadataDisjoint:
            break;
    }
}

// --------------------------------------------------

bool MetadataHub::write(ImageInfo info, WriteMode writeMode)
{
    bool changed = false;

    // find out in advance if we have something to write - needed for FullWriteIfChanged mode
    bool saveComment  = (d->commentsStatus == MetadataAvailable);
    bool saveDateTime = (d->dateTimeStatus == MetadataAvailable);
    bool saveRating   = (d->ratingStatus   == MetadataAvailable);
    bool saveTemplate = (d->templateStatus == MetadataAvailable);
    bool saveTags     = false;
    for (QMap<TAlbum *, TagStatus>::iterator it = d->tags.begin(); it != d->tags.end(); ++it)
    {
        if (it.value() == MetadataAvailable)
        {
            saveTags = true;
            break;
        }
    }

    bool writeAllFields;
    if (writeMode == FullWrite)
        writeAllFields = true;
    else if (writeMode == FullWriteIfChanged)
        writeAllFields = (
                           (saveComment  && d->commentsChanged) ||
                           (saveDateTime && d->dateTimeChanged) ||
                           (saveRating   && d->ratingChanged)   ||
                           (saveTemplate && d->templateChanged) ||
                           (saveTags     && d->tagsChanged)
                         );
    else // PartialWrite
        writeAllFields = false;

    if (saveComment && (writeAllFields || d->commentsChanged))
    {
        DatabaseAccess access;
        ImageComments comments = info.imageComments(access);
        // we set a map of alt-languages comments with default author and date null
        comments.replaceComments(d->comments);
        changed = true;
    }
    if (saveDateTime && (writeAllFields || d->dateTimeChanged))
    {
        info.setDateTime(d->dateTime);
        changed = true;
    }
    if (saveRating && (writeAllFields || d->ratingChanged))
    {
        info.setRating(d->rating);
        changed = true;
    }
    if (saveTemplate && writeAllFields)
    {
        ImageCopyright cr = info.imageCopyright();
        QString title = d->metadataTemplate.templateTitle();
        if (title == Template::removeTemplateTitle())
        {
            cr.removeCreators();
            cr.removeProvider();
            cr.removeCopyrightNotices();
            cr.removeRightsUsageTerms();
            cr.removeSource();
            cr.removeCreatorJobTitle();
            cr.removeInstructions();
        }
        if (title.isEmpty())
        {
            // Nothing to do.
        }
        else
        {
            cr.removeCreators();
            foreach(QString author, d->metadataTemplate.authors())
                cr.setAuthor(author, ImageCopyright::AddEntryToExisting);

            cr.setCredit(d->metadataTemplate.credit());

            cr.removeCopyrightNotices();
            KExiv2::AltLangMap copyrights = d->metadataTemplate.copyright();
            KExiv2::AltLangMap::const_iterator it;
            for (it = copyrights.constBegin() ; it != copyrights.constEnd() ; ++it)
                cr.setRights(it.value(), it.key(), ImageCopyright::AddEntryToExisting);

            cr.removeRightsUsageTerms();
            KExiv2::AltLangMap usages = d->metadataTemplate.rightUsageTerms();
            KExiv2::AltLangMap::const_iterator it2;
            for (it2 = usages.constBegin() ; it2 != usages.constEnd() ; ++it2)
                cr.setRightsUsageTerms(it2.value(), it2.key(), ImageCopyright::AddEntryToExisting);

            cr.setSource(d->metadataTemplate.source());
            cr.setAuthorsPosition(d->metadataTemplate.authorsPosition());
            cr.setInstructions(d->metadataTemplate.instructions());
        }
        changed = true;
    }

    if (writeAllFields || d->tagsChanged)
    {
        if (d->dbmode == ManagedTags)
        {
            for (QMap<TAlbum *, TagStatus>::iterator it = d->tags.begin(); it != d->tags.end(); ++it)
            {
                if (it.value() == MetadataAvailable)
                {
                    if (it.value().hasTag)
                        info.setTag(it.key()->id());
                    else
                        info.removeTag(it.key()->id());
                    changed = true;
                }
            }
        }
        else
        {
            // tags not yet contained in database will be created
            info.addTagPaths(d->tagList);
            changed = changed || !d->tagList.isEmpty();
        }
    }
    return changed;
}

bool MetadataHub::write(DMetadata& metadata, WriteMode writeMode, const MetadataWriteSettings& settings)
{
    bool dirty = false;

    metadata.setWriteRawFiles(settings.writeRawFiles);

#if KEXIV2_VERSION >= 0x000600
    metadata.setUpdateFileTimeStamp(settings.updateFileTimeStamp);
#endif

    // find out in advance if we have something to write - needed for FullWriteIfChanged mode
    bool saveComment  = (settings.saveComments && d->commentsStatus == MetadataAvailable);
    bool saveDateTime = (settings.saveDateTime && d->dateTimeStatus == MetadataAvailable);
    bool saveRating   = (settings.saveRating   && d->ratingStatus   == MetadataAvailable);
    bool saveTemplate = (settings.saveTemplate && d->templateStatus == MetadataAvailable);
    bool saveTags     = false;

    if (settings.saveTags)
    {
        saveTags = false;
        // find at least one tag to write
        for (QMap<TAlbum *, TagStatus>::iterator it = d->tags.begin(); it != d->tags.end(); ++it)
        {
            if (it.value() == MetadataAvailable)
            {
                saveTags = true;
                break;
            }
        }
    }

    bool writeAllFields;
    if (writeMode == FullWrite)
        writeAllFields = true;
    else if (writeMode == FullWriteIfChanged)
        writeAllFields = (
                           (saveComment  && d->commentsChanged) ||
                           (saveDateTime && d->dateTimeChanged) ||
                           (saveRating   && d->ratingChanged)   ||
                           (saveTemplate && d->templateChanged) ||
                           (saveTags     && d->tagsChanged)
                         );
    else // PartialWrite
        writeAllFields = false;

    if (saveComment && (writeAllFields || d->commentsChanged))
    {
        // Store comments in image as JFIF comments, Exif comments, Iptc Caption, and Xmp.
        dirty |= metadata.setImageComments(d->comments);
    }

    if (saveDateTime && (writeAllFields || d->dateTimeChanged))
    {
        // Store Image Date & Time as Exif and Iptc tags.
        dirty |= metadata.setImageDateTime(d->dateTime, false);
    }

    if (saveRating && (writeAllFields || d->ratingChanged))
    {
        // Store Image rating as Iptc tag.
        dirty |= metadata.setImageRating(d->rating);
    }

    if (saveTemplate && (writeAllFields || d->templateChanged))
    {
        QString title = d->metadataTemplate.templateTitle();
        if (title == Template::removeTemplateTitle())
        {
            dirty |= metadata.removeMetadataTemplate();
        }
        else if (title.isEmpty())
        {
            // Nothing to do.
        }
        else
        {
            // Store metadata template as XMP tag.
            dirty |= metadata.removeMetadataTemplate();
            dirty |= metadata.setMetadataTemplate(d->metadataTemplate);
        }
    }

    if (saveTags && (writeAllFields || d->tagsChanged))
    {
        // Store tag paths as Iptc keywords tags.

        // DatabaseMode == ManagedTags is assumed.
        // To fix this constraint (not needed currently), an oldKeywords parameter is needed

        // create list of keywords to be added and to be removed
        QStringList oldTagsPathList, newTagsPathList, oldKeywords, newKeywords;
        for (QMap<TAlbum *, TagStatus>::iterator it = d->tags.begin(); it != d->tags.end(); ++it)
        {
            // it is important that MetadataDisjoint keywords are not touched
            if (it.value() == MetadataAvailable)
            {
                // This works for single and multiple selection.
                // In both situations, tags which had originally been loaded
                // have explicitly been removed with setTag.
                if (it.value().hasTag)
                {
                    newTagsPathList.append(it.key()->tagPath(false));
                    newKeywords.append(it.key()->title());
                }
                else
                {
                    oldTagsPathList.append(it.key()->tagPath(false));
                    oldKeywords.append(it.key()->title());
                }
            }
        }

        // NOTE: See B.K.O #175321 : we remove all old keyword from IPTC and XMP before to
        // synchronize metadata, else contents is not coherent.

        // We set Iptc keywords using tags name.
        dirty |= metadata.setIptcKeywords(metadata.getIptcKeywords(), newKeywords);

        // We add Xmp keywords using tags name.
        dirty |= metadata.removeXmpKeywords(metadata.getXmpKeywords());
        dirty |= metadata.setXmpKeywords(newKeywords);

        // We set Tags Path list in digiKam Xmp private namespace using tags path.
        dirty |= metadata.setImageTagsPath(newTagsPathList);
    }

    return dirty;
}

bool MetadataHub::write(const QString& filePath, WriteMode writeMode, const MetadataWriteSettings& settings)
{
    // if no DMetadata object is needed at all, don't construct one -
    // important optimization if writing to file is turned off in setup!
    if (!willWriteMetadata(writeMode, settings))
        return false;

    DMetadata metadata(filePath);
    if (write(metadata, writeMode, settings))
    {
        bool success = metadata.applyChanges();
        ImageAttributesWatch::instance()->fileMetadataChanged(filePath);
        return success;
    }
    return false;
}

bool MetadataHub::write(DImg& image, WriteMode writeMode, const MetadataWriteSettings& settings)
{
    // if no DMetadata object is needed at all, don't construct one
    if (!willWriteMetadata(writeMode, settings))
        return false;

    // See DImgLoader::readMetadata() and saveMetadata()
    DMetadata metadata;
    metadata.setComments(image.getComments());
    metadata.setExif(image.getExif());
    metadata.setIptc(image.getIptc());
    metadata.setXmp(image.getXmp());

    if (write(metadata, writeMode, settings))
    {
        // Do not insert null data into metaData map:
        // Even if byte array is null, if there is a key in the map, it will
        // be interpreted as "There was data, so write it again to the file".
        if (metadata.hasComments())
            image.setComments(metadata.getComments());
        if (metadata.hasExif())
            image.setExif(metadata.getExif());
        if (metadata.hasIptc())
            image.setIptc(metadata.getIptc());
        if (metadata.hasXmp())
            image.setXmp(metadata.getXmp());
        return true;
    }
    return false;
}

bool MetadataHub::willWriteMetadata(WriteMode writeMode, const MetadataWriteSettings& settings) const
{
    // This is the same logic as in write(DMetadata) but without actually writing.
    // Adapt if the method above changes

    bool saveComment  = (settings.saveComments && d->commentsStatus == MetadataAvailable);
    bool saveDateTime = (settings.saveDateTime && d->dateTimeStatus == MetadataAvailable);
    bool saveRating   = (settings.saveRating   && d->ratingStatus   == MetadataAvailable);
    bool saveTemplate = (settings.saveTemplate && d->templateStatus == MetadataAvailable);
    bool saveTags     = false;

    if (settings.saveTags)
    {
        saveTags = false;
        // find at least one tag to write
        for (QMap<TAlbum *, TagStatus>::iterator it = d->tags.begin(); it != d->tags.end(); ++it)
        {
            if (it.value() == MetadataAvailable)
            {
                saveTags = true;
                break;
            }
        }
    }

    bool writeAllFields;
    if (writeMode == FullWrite)
        writeAllFields = true;
    else if (writeMode == FullWriteIfChanged)
        writeAllFields = (
                           (saveComment  && d->commentsChanged) ||
                           (saveDateTime && d->dateTimeChanged) ||
                           (saveRating   && d->ratingChanged)   ||
                           (saveTemplate && d->templateChanged) ||
                           (saveTags     && d->tagsChanged)
                         );
    else // PartialWrite
        writeAllFields = false;

    return (
            (saveComment &&  (writeAllFields || d->commentsChanged)) ||
            (saveDateTime && (writeAllFields || d->dateTimeChanged)) ||
            (saveRating &&   (writeAllFields || d->ratingChanged))   ||
            (saveTags &&     (writeAllFields || d->tagsChanged))     ||
            (saveTemplate && (writeAllFields || d->templateChanged))
           );
}

MetadataWriteSettings MetadataHub::defaultWriteSettings()
{
    if (AlbumSettings::instance())
        return MetadataWriteSettings(AlbumSettings::instance());
    else
        // is this check necessary?
        return MetadataWriteSettings();
}

// --------------------------------------------------

MetadataHub::Status MetadataHub::dateTimeStatus() const
{
    return d->dateTimeStatus;
}

MetadataHub::Status MetadataHub::commentsStatus() const
{
    return d->commentsStatus;
}

MetadataHub::Status MetadataHub::ratingStatus() const
{
    return d->ratingStatus;
}

MetadataHub::Status MetadataHub::templateStatus() const
{
    return d->templateStatus;
}

MetadataHub::TagStatus MetadataHub::tagStatus(int albumId) const
{
    if (d->dbmode == NewTagsImport)
        return TagStatus(MetadataInvalid);
    return tagStatus(AlbumManager::instance()->findTAlbum(albumId));
}

MetadataHub::TagStatus MetadataHub::tagStatus(const QString& tagPath) const
{
    if (d->dbmode == NewTagsImport)
        return TagStatus(MetadataInvalid);
    return tagStatus(AlbumManager::instance()->findTAlbum(tagPath));
}

MetadataHub::TagStatus MetadataHub::tagStatus(TAlbum *album) const
{
    if (!album)
        return TagStatus(MetadataInvalid);
    QMap<TAlbum *, TagStatus>::iterator mapIt = d->tags.find(album);
    if (mapIt == d->tags.end())
        return TagStatus(MetadataInvalid);
    return mapIt.value();
}

bool MetadataHub::dateTimeChanged() const
{
    return d->dateTimeChanged;
}

bool MetadataHub::commentsChanged() const
{
    return d->commentsChanged;
}

bool MetadataHub::ratingChanged() const
{
    return d->ratingChanged;
}

bool MetadataHub::templateChanged() const
{
    return d->templateChanged;
}

bool MetadataHub::tagsChanged() const
{
    return d->tagsChanged;
}

QDateTime MetadataHub::dateTime() const
{
    return d->dateTime;
}

KExiv2::AltLangMap MetadataHub::comments() const
{
    return d->comments;
}

int MetadataHub::rating() const
{
    return d->rating;
}

Template MetadataHub::metadataTemplate() const
{
    return d->metadataTemplate;
}

void MetadataHub::dateTimeInterval(QDateTime& lowest, QDateTime& highest) const
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

void MetadataHub::ratingInterval(int& lowest, int& highest) const
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

QStringList MetadataHub::keywords() const
{
    if (d->dbmode == NewTagsImport)
        return d->tagList;
    else
    {
        QStringList tagList;
        for (QMap<TAlbum *, TagStatus>::iterator it = d->tags.begin(); it != d->tags.end(); ++it)
        {
            if (it.value() == TagStatus(MetadataAvailable, true))
                tagList.append(it.key()->tagPath(false));
        }
        return tagList;
    }
}

QMap<TAlbum *, MetadataHub::TagStatus> MetadataHub::tags() const
{
    // DatabaseMode == ManagedTags is assumed
    return d->tags;
}

QMap<int, MetadataHub::TagStatus> MetadataHub::tagIDs() const
{
    // DatabaseMode == ManagedTags is assumed
    QMap<int, TagStatus> intmap;
    for (QMap<TAlbum *, TagStatus>::const_iterator it = d->tags.constBegin(); it != d->tags.constEnd(); ++it)
    {
        intmap.insert(it.key()->id(), it.value());
    }
    return intmap;
}

// --------------------------------------------------

void MetadataHub::setDateTime(const QDateTime& dateTime, Status status)
{
    d->dateTimeStatus  = status;
    d->dateTime        = dateTime;
    d->dateTimeChanged = true;
}

void MetadataHub::setComments(const KExiv2::AltLangMap& comments, Status status)
{
    d->commentsStatus  = status;
    d->comments        = comments;
    d->commentsChanged = true;
}

void MetadataHub::setRating(int rating, Status status)
{
    d->ratingStatus   = status;
    d->rating         = rating;
    d->ratingChanged  = true;
}

void MetadataHub::setMetadataTemplate(const Template &t, Status status)
{
    d->templateStatus   = status;
    d->metadataTemplate = t;
    d->templateChanged  = true;
}

void MetadataHub::setTag(TAlbum *tag, bool hasTag, Status status)
{
    // DatabaseMode == ManagedTags is assumed
    d->tags[tag]   = TagStatus(status, hasTag);
    d->tagsChanged = true;
}

void MetadataHub::setTag(int albumID, bool hasTag, Status status)
{
    // DatabaseMode == ManagedTags is assumed
    TAlbum *album = AlbumManager::instance()->findTAlbum(albumID);
    if (!album)
    {
        kWarning(50003) << "Tag ID " << albumID << " not found in database.";
        return;
    }
    setTag(album, hasTag, status);
}

void MetadataHub::resetChanged()
{
    d->dateTimeChanged = false;
    d->commentsChanged = false;
    d->ratingChanged   = false;
    d->templateChanged = false;
    d->tagsChanged     = false;
}

} // namespace Digikam
