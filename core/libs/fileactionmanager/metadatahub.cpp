/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-05
 * Description : Metadata handling
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014-2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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
#include <QMutex>
#include <QMutexLocker>

// Local includes

#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredbwatch.h"
#include "imageinfo.h"
#include "imagecomments.h"
#include "template.h"
#include "templatemanager.h"
#include "applicationsettings.h"
#include "imageattributeswatch.h"
#include "tagscache.h"
#include "facetagseditor.h"
#include "metadatahubmngr.h"

#ifdef HAVE_KFILEMETADATA
#   include "baloowrap.h"
#endif

namespace Digikam
{

class MetadataHub::Private
{
public:

    Private()
    {
        dateTimeStatus    = MetadataHub::MetadataInvalid;
        pickLabelStatus   = MetadataHub::MetadataInvalid;
        colorLabelStatus  = MetadataHub::MetadataInvalid;
        ratingStatus      = MetadataHub::MetadataInvalid;
        titlesStatus      = MetadataHub::MetadataInvalid;
        commentsStatus    = MetadataHub::MetadataInvalid;
        templateStatus    = MetadataHub::MetadataInvalid;

        pickLabel         = -1;
        colorLabel        = -1;
        rating            = -1;
        count             = 0;

    }

public:

    int                               pickLabel;
    int                               colorLabel;
    int                               rating;
    int                               count;

    QDateTime                         dateTime;

    CaptionsMap                       titles;
    CaptionsMap                       comments;

    Template                          metadataTemplate;

    QMap<int, MetadataHub::Status>    tags;

    QStringList                       tagList;

    QMultiMap<QString, QVariant>      faceTagsList;

    MetadataHub::Status               dateTimeStatus;
    MetadataHub::Status               titlesStatus;
    MetadataHub::Status               commentsStatus;
    MetadataHub::Status               pickLabelStatus;
    MetadataHub::Status               colorLabelStatus;
    MetadataHub::Status               ratingStatus;
    MetadataHub::Status               templateStatus;

public:

    template <class T> void loadSingleValue(const T& data, T& storage, MetadataHub::Status& status);
};

// ------------------------------------------------------------------------------------------

MetadataHub::MetadataHub()
    : d(new Private)
{
}

MetadataHub::MetadataHub(const MetadataHub& other)
    : d(new Private(*other.d))
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

MetadataHub* MetadataHub::clone() const
{
    return new MetadataHub(*this);
}

void MetadataHub::reset()
{
    (*d) = Private();
}

// --------------------------------------------------

void MetadataHub::load(const ImageInfo& info)
{
    d->count++;
    //qCDebug(DIGIKAM_GENERAL_LOG) << "---------------------------------Load from ImageInfo ----------------";

    CaptionsMap commentMap;
    CaptionsMap titleMap;

    {
        CoreDbAccess access;
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

    loadFaceTags(info, info.dimensions());
}

// private common code to merge tags
void MetadataHub::loadTags(const QList<int>& loadedTags)
{
    d->tags.clear();

    foreach(int tagId, loadedTags)
    {
        if (TagsCache::instance()->isInternalTag(tagId))
        {
            continue;
        }

        d->tags[tagId] = MetadataAvailable;
    }
}

// private common code to load dateTime, comment, color label, pick label, rating
void MetadataHub::load(const QDateTime& dateTime,
                       const CaptionsMap& titles, const CaptionsMap& comments,
                       int colorLabel, int pickLabel,
                       int rating, const Template& t)
{
    if (dateTime.isValid())
    {
        d->loadSingleValue<QDateTime>(dateTime, d->dateTime, d->dateTimeStatus);
    }

    d->loadSingleValue<int>(pickLabel, d->pickLabel, d->pickLabelStatus);

    d->loadSingleValue<int>(colorLabel, d->colorLabel, d->colorLabelStatus);

    d->loadSingleValue<int>(rating, d->rating, d->ratingStatus);

    d->loadSingleValue<CaptionsMap>(titles, d->titles, d->titlesStatus);

    d->loadSingleValue<CaptionsMap>(comments, d->comments, d->commentsStatus);

    d->loadSingleValue<Template>(t, d->metadataTemplate, d->templateStatus);
}


// template method used by comment and template
template <class T> void MetadataHub::Private::loadSingleValue(const T& data, T& storage,
                                                              MetadataHub::Status& status)
{
    switch (status)
    {
        case MetadataHub::MetadataInvalid:
            storage = data;
            status  = MetadataHub::MetadataAvailable;
            break;

        case MetadataHub::MetadataAvailable:
            qCDebug(DIGIKAM_GENERAL_LOG) << "You should not load more than one image info in metadatahub";
            break;
    }
}

// ------------------------------------------------------------------------------------------------------------

/** safe **/
bool MetadataHub::writeToMetadata(const ImageInfo& info, WriteComponent writeMode, bool ignoreLazySync, const MetadataSettingsContainer &settings)
{
    applyChangeNotifications();

    // if no DMetadata object is needed at all, don't construct one -
    // important optimization if writing to file is turned off in setup!
    if (!willWriteMetadata(writeMode, settings))
    {
        return false;
    }

    if (!ignoreLazySync && settings.useLazySync)
    {
        MetadataHubMngr::instance()->addPending(info);
        return true;
    }

    writeToBaloo(info.filePath());

    DMetadata metadata(info.filePath());

    if (write(metadata, writeMode, settings))
    {
        bool success = metadata.applyChanges();
        ImageAttributesWatch::instance()->fileMetadataChanged(QUrl::fromLocalFile(info.filePath()));
        return success;
    }

    return false;
}

bool MetadataHub::write(DMetadata& metadata, WriteComponent writeMode, const MetadataSettingsContainer& settings)
{
    applyChangeNotifications();

    bool dirty = false;

    metadata.setSettings(settings);

    // find out in advance if we have something to write - needed for FullWriteIfChanged mode
    bool saveTitle      = (settings.saveComments   && (d->titlesStatus     == MetadataAvailable) && writeMode.testFlag(WRITE_TITLE));
    bool saveComment    = (settings.saveComments   && (d->commentsStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_COMMENTS));
    bool saveDateTime   = (settings.saveDateTime   && (d->dateTimeStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_DATETIME));
    bool savePickLabel  = (settings.savePickLabel  && (d->pickLabelStatus  == MetadataAvailable) && writeMode.testFlag(WRITE_PICKLABEL));
    bool saveColorLabel = (settings.saveColorLabel && (d->colorLabelStatus == MetadataAvailable) && writeMode.testFlag(WRITE_COLORLABEL));
    bool saveRating     = (settings.saveRating     && (d->ratingStatus     == MetadataAvailable) && writeMode.testFlag(WRITE_RATING));
    bool saveTemplate   = (settings.saveTemplate   && (d->templateStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_TEMPLATE));
    bool saveTags       = settings.saveTags && writeMode.testFlag(WRITE_TAGS);
    bool saveFaces      = settings.saveFaceTags && writeMode.testFlag((WRITE_TAGS));


    if (saveTitle)
    {
        // Store titles in image as Iptc Object name and Xmp.
        dirty |= metadata.setImageTitles(d->titles);
    }

    if (saveComment)
    {
        // Store comments in image as JFIF comments, Exif comments, Iptc Caption, and Xmp.
        dirty |= metadata.setImageComments(d->comments);
    }

    if (saveDateTime)
    {
        // Store Image Date & Time as Exif and Iptc tags.
        dirty |= metadata.setImageDateTime(d->dateTime, false);
    }

    if (savePickLabel)
    {
        // Store Image Pick Label as XMP tag.
        dirty |= metadata.setImagePickLabel(d->pickLabel);
    }

    if (saveColorLabel)
    {
        // Store Image Color Label as XMP tag.
        dirty |= metadata.setImageColorLabel(d->colorLabel);
    }

    if (saveRating)
    {
        // Store Image rating as Iptc tag.
        dirty |= metadata.setImageRating(d->rating);
    }

    if (saveTemplate)
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

    dirty |= metadata.setImageFacesMap(d->faceTagsList,saveFaces);

    dirty |= writeTags(metadata,saveTags);

    return dirty;
}

bool MetadataHub::write(const QString& filePath, WriteComponent writeMode, bool ignoreLazySync, const MetadataSettingsContainer& settings)
{
    applyChangeNotifications();

    // if no DMetadata object is needed at all, don't construct one -
    // important optimization if writing to file is turned off in setup!
    if (!willWriteMetadata(writeMode, settings))
    {
        return false;
    }

    if (!ignoreLazySync && settings.useLazySync)
    {
        ImageInfo info = ImageInfo::fromLocalFile(filePath);
        MetadataHubMngr::instance()->addPending(info);
        return true;
    }

    writeToBaloo(filePath);

    DMetadata metadata(filePath);

    if (write(metadata, writeMode, settings))
    {
        bool success = metadata.applyChanges();
        ImageAttributesWatch::instance()->fileMetadataChanged(QUrl::fromLocalFile(filePath));
        return success;
    }

    return false;
}

bool MetadataHub::write(DImg& image, WriteComponent writeMode, bool ignoreLazySync, const MetadataSettingsContainer& settings)
{
    applyChangeNotifications();

    // if no DMetadata object is needed at all, don't construct one
    if (!willWriteMetadata(writeMode, settings))
    {
        return false;
    }

    // See DImgLoader::readMetadata() and saveMetadata()
    DMetadata metadata;
    metadata.setData(image.getMetadata());

    QString filePath = image.originalFilePath();

    if (filePath.isEmpty())
    {
        filePath = image.lastSavedFilePath();
    }

    if (!ignoreLazySync && settings.useLazySync && !filePath.isEmpty())
    {
        ImageInfo info = ImageInfo::fromLocalFile(filePath);
        MetadataHubMngr::instance()->addPending(info);
        return true;
    }

    if (!filePath.isEmpty())
    {
        writeToBaloo(filePath);
    }

    return write(metadata, writeMode, settings);
}

bool MetadataHub::writeTags(const QString& filePath, WriteComponent writeMode,
                            const MetadataSettingsContainer& settings)
{
    applyChangeNotifications();

    // if no DMetadata object is needed at all, don't construct one -
    // important optimization if writing to file is turned off in setup!
    if (!willWriteMetadata(writeMode, settings))
    {
        return false;
    }

    DMetadata metadata(filePath);
    metadata.setSettings(settings);
    bool saveFaces = settings.saveFaceTags;
    bool saveTags  = settings.saveTags;

    if (saveFaces)
    {
        metadata.setImageFacesMap(d->faceTagsList, true);
    }
    else
    {
        metadata.setImageFacesMap(d->faceTagsList, false);
    }

    writeToBaloo(filePath);

    if (writeTags(metadata, saveTags))
    {
        bool success = metadata.applyChanges();
        ImageAttributesWatch::instance()->fileMetadataChanged(QUrl::fromLocalFile(filePath));
        return success;
    }
    else
    {
        return false;
    }
}

bool MetadataHub::writeTags(DMetadata& metadata, bool saveTags)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Writting tags";
    bool dirty = false;

    if (saveTags)
    {
        // Store tag paths as Iptc keywords tags.

        // DatabaseMode == ManagedTags is assumed.
        // To fix this constraint (not needed currently), an oldKeywords parameter is needed

        // create list of keywords to be added and to be removed
        QStringList tagsPathList, newKeywords;

        QList<int> keys = d->tags.keys();

        foreach (int tagId, keys)
        {
            if (!TagsCache::instance()->canBeWrittenToMetadata(tagId))
            {
                continue;
            }

            // WARNING: Do not use write(QFilePath ...) when multiple image info are loaded
            // otherwise disjoint tags will not be used, use writeToMetadata(ImageInfo...)
            if (d->tags.value(tagId) == MetadataAvailable)
            {
                // This works for single and multiple selection.
                // In both situations, tags which had originally been loaded
                // have explicitly been removed with setTag.
                QString tagName = TagsCache::instance()->tagName(tagId);
                QString tagPath = TagsCache::instance()->tagPath(tagId, TagsCache::NoLeadingSlash);

                if (!tagsPathList.contains(tagPath))
                {
                    tagsPathList << tagPath;
                }

                if (!tagName.isEmpty())
                {
                    newKeywords << tagName;
                }
            }
        }

        tagsPathList = cleanupTags(tagsPathList);
        newKeywords = cleanupTags(newKeywords);

        if (!newKeywords.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "-------------------------- New Keywords" << newKeywords;
            // NOTE: See bug #175321 : we remove all old keyword from IPTC and XMP before to
            // synchronize metadata, else contents is not coherent.
            dirty |= metadata.setImageTagsPath(tagsPathList);
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Delete all keywords";
            // Delete all IPTC and XMP keywords
            dirty |= metadata.setImageTagsPath(QStringList());
        }
    }

    return dirty;
}

QStringList MetadataHub::cleanupTags(const QStringList& toClean)
{
    QSet<QString> deduplicator;

    for (int index = 0; index < toClean.size(); index++)
    {
        QString keyword = toClean.at(index);

        if (!keyword.isEmpty())
        {

            // _Digikam_root_tag_ is present in some photos tagged with older
            // version of digiKam, must be removed
            if (keyword.contains(QRegExp(QLatin1String("(_Digikam_root_tag_/|/_Digikam_root_tag_|_Digikam_root_tag_)"))))
            {
                keyword = keyword.replace(QRegExp(QLatin1String("(_Digikam_root_tag_/|/_Digikam_root_tag_|_Digikam_root_tag_)")),
                                          QLatin1String(""));
            }

            deduplicator.insert(keyword);
        }
    }

    return deduplicator.toList();
}

bool MetadataHub::willWriteMetadata(WriteComponent writeMode, const MetadataSettingsContainer& settings) const
{
    // This is the same logic as in write(DMetadata) but without actually writing.
    // Adapt if the method above changes
    bool saveTitle      = (settings.saveComments   && (d->titlesStatus     == MetadataAvailable) && writeMode.testFlag(WRITE_TITLE));
    bool saveComment    = (settings.saveComments   && (d->commentsStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_COMMENTS));
    bool saveDateTime   = (settings.saveDateTime   && (d->dateTimeStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_DATETIME));
    bool savePickLabel  = (settings.savePickLabel  && (d->pickLabelStatus  == MetadataAvailable) && writeMode.testFlag(WRITE_PICKLABEL));
    bool saveColorLabel = (settings.saveColorLabel && (d->colorLabelStatus == MetadataAvailable) && writeMode.testFlag(WRITE_COLORLABEL));
    bool saveRating     = (settings.saveRating     && (d->ratingStatus     == MetadataAvailable) && writeMode.testFlag(WRITE_RATING));
    bool saveTemplate   = (settings.saveTemplate   && (d->templateStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_TEMPLATE));
    bool saveTags       = settings.saveTags && writeMode.testFlag(WRITE_TAGS);




    return (
               saveTitle       ||
               saveComment     ||
               saveDateTime    ||
               savePickLabel   ||
               saveColorLabel  ||
               saveRating      ||
               saveTags        ||
               saveTemplate
           );
}

void MetadataHub::writeToBaloo(const QString& filePath, const MetadataSettingsContainer& settings)
{
#ifdef HAVE_KFILEMETADATA

    BalooWrap* const baloo = BalooWrap::instance();
    int rating             = -1;
    QString* comment       = 0;

    if (!baloo->getSyncToBaloo())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No write to baloo +++++++++++++++++++++++++++++++++++++";
        return;
    }

    bool saveComment = (settings.saveComments   && d->commentsStatus   == MetadataAvailable);
    bool saveRating  = (settings.saveRating     && d->ratingStatus     == MetadataAvailable);

    QStringList newKeywords;

    for (QMap<int, MetadataHub::Status>::iterator it = d->tags.begin(); it != d->tags.end(); ++it)
    {
        if (!TagsCache::instance()->canBeWrittenToMetadata(it.key()))
        {
            continue;
        }

        // it is important that MetadataDisjoint keywords are not touched

        if (it.value() == MetadataAvailable)
        {
            QString tagName = TagsCache::instance()->tagName(it.key());

            if (!tagName.isEmpty())
            {
                newKeywords << tagName;
            }
        }
    }

    if (saveComment)
    {
        comment = new QString(d->comments.value(QLatin1String("x-default")).caption);
    }

    if (saveRating)
    {
        rating = d->rating;
    }

    newKeywords = cleanupTags(newKeywords);
    QUrl url = QUrl::fromLocalFile(filePath);
    baloo->setAllData(url, &newKeywords, comment, rating);
#else
    Q_UNUSED(filePath);
    Q_UNUSED(settings);
#endif
}

void MetadataHub::applyChangeNotifications()
{
}

void Digikam::MetadataHub::loadFaceTags(const ImageInfo& info, const QSize& size)
{
    FaceTagsEditor editor;
    //qCDebug(DIGIKAM_GENERAL_LOG) << "Image Dimensions ----------------" << info.dimensions();

    QList<FaceTagsIface> facesList = editor.confirmedFaceTagsIfaces(info.id());
    d->faceTagsList.clear();

    if (!facesList.isEmpty())
    {
        foreach(const FaceTagsIface& dface, facesList)
        {
            QString faceName = FaceTags::faceNameForTag(dface.tagId());

            if (faceName.isEmpty())
                continue;

            QRect  temprect  = dface.region().toRect();
            QRectF faceRect  = TagRegion::absoluteToRelative(temprect,size);
            d->faceTagsList.insertMulti(faceName, QVariant(faceRect));
        }

    }
}

QMultiMap<QString, QVariant> Digikam::MetadataHub::getFaceTags()
{
    return d->faceTagsList;
}

QMultiMap<QString, QVariant> Digikam::MetadataHub::loadIntegerFaceTags(const ImageInfo& info)
{
    FaceTagsEditor editor;
    QMultiMap<QString, QVariant> faceTagsList;

    QList<FaceTagsIface> facesList = editor.confirmedFaceTagsIfaces(info.id());
    faceTagsList.clear();

    if (!facesList.isEmpty())
    {
        foreach(const FaceTagsIface& dface, facesList)
        {
            QString faceName = FaceTags::faceNameForTag(dface.tagId());

            if (faceName.isEmpty())
                continue;

            QRect  temprect  = dface.region().toRect();
            faceTagsList.insertMulti(faceName, QVariant(temprect));
        }
    }
    return faceTagsList;
}

void Digikam::MetadataHub::setFaceTags(QMultiMap<QString, QVariant> newFaceTags, QSize size)
{
    d->faceTagsList.clear();
    QMultiMap<QString, QVariant>::iterator it;

    for(it = newFaceTags.begin() ; it != newFaceTags.end() ; it++)
    {
        QRect  temprect  = it.value().toRect();
        QRectF faceRect  = TagRegion::absoluteToRelative(temprect,size);
        d->faceTagsList.insertMulti(it.key(), faceRect);
    }
}


// NOTE: Unused code
//void MetadataHub::load(const DMetadata& metadata)
//{
//    d->count++;

//    CaptionsMap comments;
//    CaptionsMap titles;
//    QStringList keywords;
//    QDateTime   datetime;
//    int         pickLabel;
//    int         colorLabel;
//    int         rating;

//    titles = metadata.getImageTitles();

//    // Try to get comments from image :
//    // In first, from Xmp comments tag,
//    // In second, from standard JPEG JFIF comments section,
//    // In third, from Exif comments tag,
//    // In four, from Iptc comments tag.
//    comments = metadata.getImageComments();

//    // Try to get date and time from image :
//    // In first, from Exif date & time tags,
//    // In second, from Xmp date & time tags, or
//    // In third, from Iptc date & time tags.
//    // else use file system time stamp.
//    datetime = metadata.getImageDateTime();

//    if ( !datetime.isValid() )
//    {
//        QFileInfo info( metadata.getFilePath() );
//        datetime = info.lastModified();
//    }

//    // Try to get image pick label from Xmp tag
//    pickLabel = metadata.getImagePickLabel();

//    // Try to get image color label from Xmp tag
//    colorLabel = metadata.getImageColorLabel();

//    // Try to get image rating from Xmp tag, or Iptc Urgency tag
//    rating = metadata.getImageRating();

//    Template tref = metadata.getMetadataTemplate();
//    Template t    = TemplateManager::defaultManager()->findByContents(tref);

//    qCDebug(DIGIKAM_GENERAL_LOG) << "Found Metadata Template: " << t.templateTitle();

//    load(datetime, titles, comments, colorLabel, pickLabel, rating, t.isNull() ? tref : t);

//    // Try to get image tags from Xmp using digiKam namespace tags.

//    QStringList tagPaths;

//    if (metadata.getImageTagsPath(tagPaths))
//    {
//        QList<int> tagIds = TagsCache::instance()->tagsForPaths(tagPaths);
//        loadTags(tagIds);
//    }
//}

//bool MetadataHub::load(const QString& filePath, const MetadataSettingsContainer& settings)
//{

//    DMetadata metadata;
//    metadata.setSettings(settings);
//    bool success = metadata.load(filePath);
//    load(metadata); // increments count
//    return success;
//}

/*
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
*/

} // namespace Digikam
