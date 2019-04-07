/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Tag Album helpers.
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#include "albummanager_p.h"

namespace Digikam
{

void AlbumManager::scanTAlbums()
{
    d->scanTAlbumsTimer->stop();

    // list TAlbums directly from the db
    // first insert all the current TAlbums into a map for quick lookup
    typedef QMap<int, TAlbum*> TagMap;
    TagMap                     tmap;

    tmap.insert(0, d->rootTAlbum);

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* t = (TAlbum*)(*it);
        tmap.insert(t->id(), t);
        ++it;
    }

    // Retrieve the list of tags from the database
    TagInfo::List tList = CoreDbAccess().db()->scanTags();

    // sort the list. needed because we want the tags can be read in any order,
    // but we want to make sure that we are ensure to find the parent TAlbum
    // for a new TAlbum

    {
        QHash<int, TAlbum*> tagHash;

        // insert items into a dict for quick lookup
        for (TagInfo::List::const_iterator iter = tList.constBegin() ; iter != tList.constEnd() ; ++iter)
        {
            TagInfo info        = *iter;
            TAlbum* const album = new TAlbum(info.name, info.id);
            album->m_icon       = info.icon;
            album->m_iconId     = info.iconId;
            album->m_pid        = info.pid;
            tagHash.insert(info.id, album);
        }

        tList.clear();

        // also add root tag
        TAlbum* const rootTag = new TAlbum(QLatin1String("root"), 0, true);
        tagHash.insert(0, rootTag);

        // build tree
        for (QHash<int, TAlbum*>::const_iterator iter = tagHash.constBegin() ; iter != tagHash.constEnd() ; ++iter)
        {
            TAlbum* album = *iter;

            if (album->m_id == 0)
            {
                continue;
            }

            TAlbum* const parent = tagHash.value(album->m_pid);

            if (parent)
            {
                album->setParent(parent);
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to find parent tag for tag "
                           << album->m_title
                           << " with pid "
                           << album->m_pid;
            }
        }

        tagHash.clear();

        // now insert the items into the list. becomes sorted
        AlbumIterator it(rootTag);

        while (it.current())
        {
            TagInfo info;
            TAlbum* const album = static_cast<TAlbum*>(it.current());

            if (album)
            {
                info.id     = album->m_id;
                info.pid    = album->m_pid;
                info.name   = album->m_title;
                info.icon   = album->m_icon;
                info.iconId = album->m_iconId;
            }

            tList.append(info);
            ++it;
        }

        // this will also delete all child albums
        delete rootTag;
    }

    for (TagInfo::List::const_iterator it = tList.constBegin() ; it != tList.constEnd() ; ++it)
    {
        TagInfo info = *it;

        // check if we have already added this tag
        if (tmap.contains(info.id))
        {
            continue;
        }

        // Its a new album. Find the parent of the album
        TagMap::const_iterator iter = tmap.constFind(info.pid);

        if (iter == tmap.constEnd())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to find parent tag for tag "
                                           << info.name
                                           << " with pid "
                                           << info.pid;
            continue;
        }

        TAlbum* const parent = iter.value();

        // Create the new TAlbum
        TAlbum* const album = new TAlbum(info.name, info.id, false);
        album->m_icon       = info.icon;
        album->m_iconId     = info.iconId;
        insertTAlbum(album, parent);

        // also insert it in the map we are doing lookup of parent tags
        tmap.insert(info.id, album);
    }

    if (!tList.isEmpty())
    {
        emit signalAlbumsUpdated(Album::TAG);
    }

    getTagItemsCount();
}

void AlbumManager::getTagItemsCount()
{
    d->tagItemCountTimer->stop();

    if (!ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        return;
    }

    tagItemsCount();
    personItemsCount();
}

void AlbumManager::tagItemsCount()
{
    if (d->tagListJob)
    {
        d->tagListJob->cancel();
        d->tagListJob = 0;
    }

    TagsDBJobInfo jInfo;
    jInfo.setFoldersJob();

    d->tagListJob = DBJobsManager::instance()->startTagsJobThread(jInfo);

    connect(d->tagListJob, SIGNAL(finished()),
            this, SLOT(slotTagsJobResult()));

    connect(d->tagListJob, SIGNAL(foldersData(QMap<int,int>)),
            this, SLOT(slotTagsJobData(QMap<int,int>)));
}

AlbumList AlbumManager::allTAlbums() const
{
    AlbumList list;

    if (d->rootTAlbum)
    {
        list.append(d->rootTAlbum);
    }

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

QList<TAlbum*> AlbumManager::currentTAlbums() const
{
    /**
     * This method is not yet used
     */
    QList<TAlbum*> talbums;
    QList<Album*>::iterator it;

    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
    {
        TAlbum* const temp = dynamic_cast<TAlbum*>(*it);

        if (temp)
            talbums.append(temp);
    }

    return talbums;
}

TAlbum* AlbumManager::findTAlbum(int id) const
{
    if (!d->rootTAlbum)
    {
        return 0;
    }

    int gid = d->rootTAlbum->globalID() + id;

    return static_cast<TAlbum*>((d->allAlbumsIdHash.value(gid)));
}

TAlbum* AlbumManager::findTAlbum(const QString& tagPath) const
{
    // handle gracefully with or without leading slash
    bool withLeadingSlash = tagPath.startsWith(QLatin1Char('/'));
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* const talbum = static_cast<TAlbum*>(*it);

        if (talbum->tagPath(withLeadingSlash) == tagPath)
        {
            return talbum;
        }

        ++it;
    }

    return 0;

}

TAlbum* AlbumManager::createTAlbum(TAlbum* parent, const QString& name,
                                   const QString& iconkde, QString& errMsg)
{
    if (!parent)
    {
        errMsg = i18n("No parent found for tag");
        return 0;
    }

    // sanity checks
    if (name.isEmpty())
    {
        errMsg = i18n("Tag name cannot be empty");
        return 0;
    }

    if (name.contains(QLatin1Char('/')))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return 0;
    }

    // first check if we have another album with the same name
    if (hasDirectChildAlbumWithTitle(parent, name))
    {
        errMsg = i18n("Tag name already exists");
        return 0;
    }

    ChangingDB changing(d);
    int id = CoreDbAccess().db()->addTag(parent->id(), name, iconkde, 0);

    if (id == -1)
    {
        errMsg = i18n("Failed to add tag to database");
        return 0;
    }

    TAlbum* const album = new TAlbum(name, id, false);
    album->m_icon       = iconkde;

    insertTAlbum(album, parent);

    TAlbum* personParentTag = findTAlbum(FaceTags::personParentTag());

    if (personParentTag && personParentTag->isAncestorOf(album))
    {
        FaceTags::ensureIsPerson(album->id());
    }

    emit signalAlbumsUpdated(Album::TAG);

    return album;
}

AlbumList AlbumManager::findOrCreateTAlbums(const QStringList& tagPaths)
{
    // find tag ids for tag paths in list, create if they don't exist
    QList<int> tagIDs = TagsCache::instance()->getOrCreateTags(tagPaths);

    // create TAlbum objects for the newly created tags
    scanTAlbums();

    AlbumList resultList;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        resultList.append(findTAlbum(*it));
    }

    return resultList;
}

bool AlbumManager::deleteTAlbum(TAlbum* album, QString& errMsg, bool askUser)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot delete Root Tag");
        return false;
    }

    QList<qlonglong> imageIds;

    if (askUser)
    {
        imageIds = CoreDbAccess().db()->getItemIDsInTag(album->id());
    }

    {
        CoreDbAccess access;
        ChangingDB changing(d);
        access.db()->deleteTag(album->id());

        Album* subAlbum = 0;
        AlbumIterator it(album);

        while ((subAlbum = it.current()) != 0)
        {
            access.db()->deleteTag(subAlbum->id());
            ++it;
        }
    }

    removeTAlbum(album);
    emit signalAlbumsUpdated(Album::TAG);

    if (askUser)
    {
        askUserForWriteChangedTAlbumToFiles(imageIds);
    }

    return true;
}

bool AlbumManager::renameTAlbum(TAlbum* album,
                                const QString& name,
                                QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot edit root tag");
        return false;
    }

    if (name.contains(QLatin1Char('/')))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return false;
    }

    // first check if we have another sibling with the same name
    if (hasDirectChildAlbumWithTitle(album->m_parent, name))
    {
        errMsg = i18n("Another tag with the same name already exists.\n"
                      "Please choose another name.");
        return false;
    }

    ChangingDB changing(d);
    CoreDbAccess().db()->setTagName(album->id(), name);
    album->setTitle(name);
    emit signalAlbumRenamed(album);

    askUserForWriteChangedTAlbumToFiles(album);

    return true;
}

bool AlbumManager::moveTAlbum(TAlbum* album, TAlbum* newParent, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (!newParent)
    {
        errMsg = i18n("Attempt to move TAlbum to nowhere");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot move root tag");
        return false;
    }

    if (!ProgressManager::instance()->isEmpty())
    {
        errMsg = i18n("Other process are not finished yet.\n"
                      "Please try again later.");
        return false;
    }

    if (hasDirectChildAlbumWithTitle(newParent, album->title()))
    {
        QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                 qApp->applicationName(),
                 i18n("Another tag with the same name already exists.\n"
                      "Do you want to merge the tags?"),
                 QMessageBox::Yes | QMessageBox::No,
                 qApp->activeWindow());

        int result = msgBox->exec();
        delete msgBox;

        if (result == QMessageBox::Yes)
        {
            TAlbum* const destAlbum = findTAlbum(newParent->tagPath() +
                                                 QLatin1Char('/')     +
                                                 album->title());

            return mergeTAlbum(album, destAlbum, false, errMsg);
        }
        else
        {
            return true;
        }
    }

    d->currentlyMovingAlbum = album;
    emit signalAlbumAboutToBeMoved(album);

    emit signalAlbumAboutToBeDeleted(album);

    if (album->parent())
    {
        album->parent()->removeChild(album);
    }

    album->setParent(0);
    emit signalAlbumDeleted(album);
    emit signalAlbumHasBeenDeleted(reinterpret_cast<quintptr>(album));

    emit signalAlbumAboutToBeAdded(album, newParent, newParent->lastChild());
    ChangingDB changing(d);
    CoreDbAccess().db()->setTagParentID(album->id(), newParent->id());
    album->setParent(newParent);
    emit signalAlbumAdded(album);

    emit signalAlbumMoved(album);
    emit signalAlbumsUpdated(Album::TAG);
    d->currentlyMovingAlbum = 0;

    TAlbum* personParentTag = findTAlbum(FaceTags::personParentTag());

    if (personParentTag && personParentTag->isAncestorOf(album))
    {
        FaceTags::ensureIsPerson(album->id());
    }

    askUserForWriteChangedTAlbumToFiles(album);

    return true;
}

bool AlbumManager::mergeTAlbum(TAlbum* album, TAlbum* destAlbum, bool dialog, QString& errMsg)
{
    if (!album || !destAlbum)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum || destAlbum == d->rootTAlbum)
    {
        errMsg = i18n("Cannot merge root tag");
        return false;
    }

    if (album->firstChild())
    {
        errMsg = i18n("Only a tag without children can be merged!");
        return false;
    }

    if (!ProgressManager::instance()->isEmpty())
    {
        errMsg = i18n("Other process are not finished yet.\n"
                      "Please try again later.");
        return false;
    }

    if (dialog)
    {
        QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                 qApp->applicationName(),
                 i18n("Do you want to merge tag '%1' into tag '%2'?",
                      album->title(), destAlbum->title()),
                 QMessageBox::Yes | QMessageBox::No,
                 qApp->activeWindow());

        int result = msgBox->exec();
        delete msgBox;

        if (result == QMessageBox::No)
        {
            return true;
        }
    }

    int oldId   = album->id();
    int mergeId = destAlbum->id();

    if (oldId == mergeId)
    {
        return true;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QList<qlonglong> imageIds = CoreDbAccess().db()->getItemIDsInTag(oldId);

    CoreDbOperationGroup group;
    group.setMaximumTime(200);

    foreach (const qlonglong& imageId, imageIds)
    {
        QList<FaceTagsIface> facesList = FaceTagsEditor().databaseFaces(imageId);
        bool foundFace                 = false;

        foreach (const FaceTagsIface& face, facesList)
        {
            if (face.tagId() == oldId)
            {
                foundFace = true;
                FaceTagsEditor().removeFace(face);
                FaceTagsEditor().add(imageId, mergeId, face.region(), false);
            }
        }

        if (!foundFace)
        {
            ItemInfo info(imageId);
            info.removeTag(oldId);
            info.setTag(mergeId);
            group.allowLift();
        }
    }

    QApplication::restoreOverrideCursor();

    if (!deleteTAlbum(album, errMsg, false))
    {
        return false;
    }

    askUserForWriteChangedTAlbumToFiles(imageIds);

    return true;
}

bool AlbumManager::updateTAlbumIcon(TAlbum* album, const QString& iconKDE,
                                    qlonglong iconID, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such tag");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot edit root tag");
        return false;
    }

    {
        CoreDbAccess access;
        ChangingDB changing(d);
        access.db()->setTagIcon(album->id(), iconKDE, iconID);
        album->m_icon   = iconKDE;
        album->m_iconId = iconID;
    }

    emit signalAlbumIconChanged(album);

    return true;
}

AlbumList AlbumManager::getRecentlyAssignedTags(bool includeInternal) const
{
    QList<int> tagIDs = CoreDbAccess().db()->getRecentlyAssignedTags();

    AlbumList resultList;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        TAlbum* const album = findTAlbum(*it);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            resultList.append(album);
        }
    }

    return resultList;
}

QStringList AlbumManager::tagPaths(const QList<int>& tagIDs,
                                   bool leadingSlash,
                                   bool includeInternal) const
{
    QStringList tagPaths;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        TAlbum* album = findTAlbum(*it);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            tagPaths.append(album->tagPath(leadingSlash));
        }
    }

    return tagPaths;
}

QStringList AlbumManager::tagNames(const QList<int>& tagIDs, bool includeInternal) const
{
    QStringList tagNames;

    foreach (int id, tagIDs)
    {
        TAlbum* const album = findTAlbum(id);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            tagNames << album->title();
        }
    }

    return tagNames;
}

QHash<int, QString> AlbumManager::tagPaths(bool leadingSlash, bool includeInternal) const
{
    QHash<int, QString> hash;
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* const t = (TAlbum*)(*it);

        if (includeInternal || !t->isInternalTag())
        {
            hash.insert(t->id(), t->tagPath(leadingSlash));
        }

        ++it;
    }

    return hash;
}

QHash<int, QString> AlbumManager::tagNames(bool includeInternal) const
{
    QHash<int, QString> hash;
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* const t = (TAlbum*)(*it);

        if (includeInternal || !t->isInternalTag())
        {
            hash.insert(t->id(), t->title());
        }

        ++it;
    }

    return hash;
}

QList< int > AlbumManager::subTags(int tagId, bool recursive)
{
    TAlbum* const album = this->findTAlbum(tagId);

    return album->childAlbumIds(recursive);
}

AlbumList AlbumManager::findTagsWithProperty(const QString& property)
{
    AlbumList list;

    QList<int> ids = TagsCache::instance()->tagsWithProperty(property);

    foreach (int id, ids)
    {
        TAlbum* const album = findTAlbum(id);

        if (album)
        {
            list << album;
        }
    }

    return list;
}

AlbumList AlbumManager::findTagsWithProperty(const QString& property, const QString& value)
{
    AlbumList list;

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        if (static_cast<TAlbum*>(*it)->property(property) == value)
        {
            list << *it;
        }

        ++it;
    }

    return list;
}

QMap<int, int> AlbumManager::getTAlbumsCount() const
{
    return d->tAlbumsCount;
}

void AlbumManager::insertTAlbum(TAlbum* album, TAlbum* parent)
{
    if (!album)
    {
        return;
    }

    emit signalAlbumAboutToBeAdded(album, parent, parent ? parent->lastChild() : 0);

    if (parent)
    {
        album->setParent(parent);
    }

    d->allAlbumsIdHash.insert(album->globalID(), album);

    emit signalAlbumAdded(album);
}

void AlbumManager::removeTAlbum(TAlbum* album)
{
    if (!album)
    {
        return;
    }

    // remove all children of this album
    Album* child        = album->firstChild();
    TAlbum* toBeRemoved = 0;

    while (child)
    {
        Album* next = child->next();
        toBeRemoved = static_cast<TAlbum*>(child);

        if (toBeRemoved)
        {
            removeTAlbum(toBeRemoved);
            toBeRemoved = 0;
        }

        child = next;
    }

    emit signalAlbumAboutToBeDeleted(album);
    d->allAlbumsIdHash.remove(album->globalID());

    if (!d->currentAlbums.isEmpty())
    {
        if (album == d->currentAlbums.first())
        {
            d->currentAlbums.clear();
            emit signalAlbumCurrentChanged(d->currentAlbums);
        }
    }

    emit signalAlbumDeleted(album);

    quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
    delete album;

    emit signalAlbumHasBeenDeleted(deletedAlbum);
}

void AlbumManager::slotTagsJobResult()
{
    if (!d->tagListJob)
    {
        return;
    }

    if (d->tagListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list face tags";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->personListJob->errorsList().first(),
                             0, i18n("digiKam"));
    }

    d->tagListJob = 0;
}

void AlbumManager::slotTagsJobData(const QMap<int, int>& tagsStatMap)
{
    if (tagsStatMap.isEmpty())
    {
        return;
    }

    d->tAlbumsCount = tagsStatMap;
    emit signalTAlbumsDirty(tagsStatMap);
}

void AlbumManager::slotTagChange(const TagChangeset& changeset)
{
    if (d->changingDB || !d->rootTAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case TagChangeset::Added:
        case TagChangeset::Moved:
        case TagChangeset::Deleted:
        case TagChangeset::Reparented:

            if (!d->scanTAlbumsTimer->isActive())
            {
                d->scanTAlbumsTimer->start();
            }

            break;

        case TagChangeset::Renamed:
        case TagChangeset::IconChanged:
            /**
             * @todo what happens here?
             */
            break;

        case TagChangeset::PropertiesChanged:
        {
            TAlbum* tag = findTAlbum(changeset.tagId());

            if (tag)
            {
                emit signalTagPropertiesChanged(tag);
            }

            break;
        }

        case TagChangeset::Unknown:
            break;
    }
}

void AlbumManager::slotImageTagChange(const ImageTagChangeset& changeset)
{
    if (!d->rootTAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case ImageTagChangeset::Added:
        case ImageTagChangeset::Removed:
        case ImageTagChangeset::RemovedAll:
        // Add properties changed.
        // Reason: in people sidebar, the images are not
        // connected with the ImageTag table but by
        // ImageTagProperties entries.
        // Thus, the count of entries in face tags are not
        // updated. This adoption should fix the problem.
        case ImageTagChangeset::PropertiesChanged:

            if (!d->tagItemCountTimer->isActive())
            {
                d->tagItemCountTimer->start();
            }

            break;

        default:
            break;
    }
}

void AlbumManager::askUserForWriteChangedTAlbumToFiles(TAlbum* const album)
{
    QList<qlonglong> imageIds = CoreDbAccess().db()->getItemIDsInTag(album->id());
    askUserForWriteChangedTAlbumToFiles(imageIds);
}

void AlbumManager::askUserForWriteChangedTAlbumToFiles(const QList<qlonglong>& imageIds)
{
    MetaEngineSettings* const settings = MetaEngineSettings::instance();

    if ((!settings->settings().saveTags &&
         !settings->settings().saveFaceTags) || imageIds.isEmpty())
    {
        return;
    }

    if (imageIds.count() > 100)
    {
        int result = d->longTimeMessageBoxResult;

        if (result == -1)
        {
            QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                     qApp->applicationName(),
                     i18n("This operation can take a long time in the background.\n"
                          "Do you want to write the metadata to %1 files now?",
                          imageIds.count()),
                     QMessageBox::Yes | QMessageBox::No,
                     qApp->activeWindow());
            QCheckBox* const chkBox      = new QCheckBox(i18n("Do not ask again for this session"), msgBox);
            msgBox->setCheckBox(chkBox);

            result = msgBox->exec();

            if (chkBox->isChecked())
            {
                d->longTimeMessageBoxResult = result;
            }

            delete msgBox;
        }

        if (result != QMessageBox::Yes)
        {
            return;
        }
    }

    ItemInfoList infos(imageIds);
    MetadataSynchronizer* const tool = new MetadataSynchronizer(infos, MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->start();
}

} // namespace Digikam
