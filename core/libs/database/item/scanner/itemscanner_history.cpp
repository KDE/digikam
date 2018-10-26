/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - history metadata helper.
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemscanner_p.h"

namespace Digikam
{
    
void ItemScanner::scanImageHistory()
{
    /** Stage 1 of history scanning */

    d->commit.historyXml = d->metadata.getImageHistory();
    d->commit.uuid       = d->metadata.getImageUniqueId();
}

void ItemScanner::commitImageHistory()
{
    if (!d->commit.historyXml.isEmpty())
    {
        CoreDbAccess().db()->setImageHistory(d->scanInfo.id, d->commit.historyXml);
        // Delay history resolution by setting this tag:
        // Resolution depends on the presence of other images, possibly only when the scanning process has finished
        CoreDbAccess().db()->addItemTag(d->scanInfo.id, TagsCache::instance()->
                                        getOrCreateInternalTag(InternalTagName::needResolvingHistory()));
        d->hasHistoryToResolve = true;
    }

    if (!d->commit.uuid.isNull())
    {
        CoreDbAccess().db()->setImageUuid(d->scanInfo.id, d->commit.uuid);
    }
}

void ItemScanner::scanImageHistoryIfModified()
{
    // If a file has a modified history, it must have a new UUID
    QString previousUuid = CoreDbAccess().db()->getImageUuid(d->scanInfo.id);
    QString currentUuid  = d->metadata.getImageUniqueId();

    if (!currentUuid.isEmpty() && previousUuid != currentUuid)
    {
        scanImageHistory();
    }
}

bool ItemScanner::resolveImageHistory(qlonglong id, QList<qlonglong>* needTaggingIds)
{
    ImageHistoryEntry history = CoreDbAccess().db()->getImageHistory(id);
    return resolveImageHistory(id, history.history, needTaggingIds);
}

bool ItemScanner::resolveImageHistory(qlonglong imageId, const QString& historyXml,
                                       QList<qlonglong>* needTaggingIds)
{
    /** Stage 2 of history scanning */

    if (historyXml.isNull())
    {
        return true;    // "true" means nothing is left to resolve
    }

    DImageHistory history = DImageHistory::fromXml(historyXml);

    if (history.isNull())
    {
        return true;
    }

    ItemHistoryGraph graph;
    graph.addScannedHistory(history, imageId);

    if (!graph.hasEdges())
    {
        return true;
    }

    QPair<QList<qlonglong>, QList<qlonglong> > cloud = graph.relationCloudParallel();
    CoreDbAccess().db()->addImageRelations(cloud.first, cloud.second, DatabaseRelation::DerivedFrom);

    int needResolvingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needResolvingHistory());
    int needTaggingTag   = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    // remove the needResolvingHistory tag from all images in graph
    CoreDbAccess().db()->removeTagsFromItems(graph.allImageIds(), QList<int>() << needResolvingTag);

    // mark a single image from the graph (sufficient for find the full relation cloud)
    QList<ItemInfo> roots = graph.rootImages();

    if (!roots.isEmpty())
    {
        CoreDbAccess().db()->addItemTag(roots.first().id(), needTaggingTag);

        if (needTaggingIds)
        {
            *needTaggingIds << roots.first().id();
        }
    }

    return !graph.hasUnresolvedEntries();
}

void ItemScanner::tagItemHistoryGraph(qlonglong id)
{
    /** Stage 3 of history scanning */

    ItemInfo info(id);

    if (info.isNull())
    {
        return;
    }
    //qCDebug(DIGIKAM_DATABASE_LOG) << "tagItemHistoryGraph" << id;

    // Load relation cloud, history of info and of all leaves of the tree into the graph, fully resolved
    ItemHistoryGraph graph    = ItemHistoryGraph::fromInfo(info, ItemHistoryGraph::LoadAll, ItemHistoryGraph::NoProcessing);
    qCDebug(DIGIKAM_DATABASE_LOG) << graph;

    int originalVersionTag     = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::originalVersion());
    int currentVersionTag      = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::currentVersion());
    int intermediateVersionTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::intermediateVersion());

    int needTaggingTag         = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    // Remove all relevant tags
    CoreDbAccess().db()->removeTagsFromItems(graph.allImageIds(), QList<int>() << originalVersionTag
        << currentVersionTag << intermediateVersionTag << needTaggingTag);

    if (!graph.hasEdges())
    {
        return;
    }

    // get category info
    QList<qlonglong>                                        originals, intermediates, currents;
    QHash<ItemInfo, HistoryImageId::Types>                 types = graph.categorize();
    QHash<ItemInfo, HistoryImageId::Types>::const_iterator it;

    for (it = types.constBegin() ; it != types.constEnd() ; ++it)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Image" << it.key().id() << "type" << it.value();
        HistoryImageId::Types types = it.value();

        if (types & HistoryImageId::Original)
        {
            originals << it.key().id();
        }

        if (types & HistoryImageId::Intermediate)
        {
            intermediates << it.key().id();
        }

        if (types & HistoryImageId::Current)
        {
            currents << it.key().id();
        }
    }

    if (!originals.isEmpty())
    {
        CoreDbAccess().db()->addTagsToItems(originals, QList<int>() << originalVersionTag);
    }

    if (!intermediates.isEmpty())
    {
        CoreDbAccess().db()->addTagsToItems(intermediates, QList<int>() << intermediateVersionTag);
    }

    if (!currents.isEmpty())
    {
        CoreDbAccess().db()->addTagsToItems(currents, QList<int>() << currentVersionTag);
    }
}

DImageHistory ItemScanner::resolvedImageHistory(const DImageHistory& history, bool mustBeAvailable)
{
    DImageHistory h;

    foreach (const DImageHistory::Entry& e, history.entries())
    {
        // Copy entry, without referredImages
        DImageHistory::Entry entry;
        entry.action = e.action;

        // resolve referredImages
        foreach (const HistoryImageId& id, e.referredImages)
        {
            QList<qlonglong> imageIds = resolveHistoryImageId(id);

            // append each image found in collection to referredImages
            foreach (const qlonglong& imageId, imageIds)
            {
                ItemInfo info(imageId);

                if (info.isNull())
                {
                    continue;
                }

                if (mustBeAvailable)
                {
                    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId(info.albumRootId());

                    if (!location.isAvailable())
                    {
                        continue;
                    }
                }

                HistoryImageId newId = info.historyImageId();
                newId.setType(id.m_type);
                entry.referredImages << newId;
            }
        }

        // add to history
        h.entries() << entry;
    }

    return h;
}

bool ItemScanner::sameReferredImage(const HistoryImageId& id1, const HistoryImageId& id2)
{
    if (!id1.isValid() || !id2.isValid())
    {
        return false;
    }

    /*
     * We give the UUID the power of equivalence that none of the other criteria has:
     * For two images a,b with uuids x,y, where x and y not null,
     *  a (same image as) b   <=>   x == y
     */
    if (id1.hasUuid() && id2.hasUuid())
    {
        return id1.m_uuid == id2.m_uuid;
    }

    if (id1.hasUniqueHashIdentifier()        &&
        id1.m_uniqueHash == id2.m_uniqueHash &&
        id1.m_fileSize   == id2.m_fileSize)
    {
        return true;
    }

    if (id1.hasFileName() && id1.hasCreationDate() &&
        id1.m_fileName     == id2.m_fileName       &&
        id1.m_creationDate == id2.m_creationDate)
    {
        return true;
    }

    if (id1.hasFileOnDisk()              &&
        id1.m_filePath == id2.m_filePath &&
        id1.m_fileName == id2.m_fileName)
    {
        return true;
    }

    return false;
}

// Returns true if both have the same UUID, or at least one of the two has no UUID
// Returns false iff both have a UUID and the UUIDs differ
static bool uuidDoesNotDiffer(const HistoryImageId& referenceId, qlonglong id)
{
    if (referenceId.hasUuid())
    {
        QString uuid = CoreDbAccess().db()->getImageUuid(id);

        if (!uuid.isEmpty())
        {
            return referenceId.m_uuid == uuid;
        }
    }

    return true;
}

static QList<qlonglong> mergedIdLists(const HistoryImageId& referenceId,
                                      const QList<qlonglong>& uuidList,
                                      const QList<qlonglong>& candidates)
{
    QList<qlonglong> results;
    // uuidList are definite results
    results = uuidList;

    // Add a candidate if it has the same UUID, or either reference or candidate  have a UUID
    // (other way round: do not add a candidate which positively has a different UUID)
    foreach (const qlonglong& candidate, candidates)
    {
        if (results.contains(candidate))
        {
            continue; // already in list, skip
        }

        if (uuidDoesNotDiffer(referenceId, candidate))
        {
            results << candidate;
        }
    }

    return results;
}

QList<qlonglong> ItemScanner::resolveHistoryImageId(const HistoryImageId& historyId)
{
    // first and foremost: UUID
    QList<qlonglong> uuidList;

    if (historyId.hasUuid())
    {
        uuidList = CoreDbAccess().db()->getItemsForUuid(historyId.m_uuid);

        // If all images had a UUID, we would be finished and could return here with a result:
/*
        if (!uuidList.isEmpty())
        {
            return uuidList;
        }
*/
        // But as identical images may have no UUID yet, we need to continue
    }

    // Second: uniqueHash + fileSize. Sufficient to assume that a file is identical, but subject to frequent change.
    if (historyId.hasUniqueHashIdentifier() && CoreDbAccess().db()->isUniqueHashV2())
    {
        QList<ItemScanInfo> infos = CoreDbAccess().db()->getIdenticalFiles(historyId.m_uniqueHash, historyId.m_fileSize);

        if (!infos.isEmpty())
        {
            QList<qlonglong> ids;

            foreach (const ItemScanInfo& info, infos)
            {
                if (info.status != DatabaseItem::Status::Trashed && info.status != DatabaseItem::Status::Obsolete)
                {
                    ids << info.id;
                }
            }

            return mergedIdLists(historyId, uuidList, ids);
        }
    }

    // As a third combination, we try file name and creation date. Susceptible to renaming,
    // but not to metadata changes.
    if (historyId.hasFileName() && historyId.hasCreationDate())
    {
        QList<qlonglong> ids = CoreDbAccess().db()->findByNameAndCreationDate(historyId.m_fileName, historyId.m_creationDate);

        if (!ids.isEmpty())
        {
            return mergedIdLists(historyId, uuidList, ids);
        }
    }

    // Another possibility: If the original UUID is given, we can find all relations for the image with this UUID,
    // and make an assumption from this group of images. Currently not implemented.

    // resolve old-style by full file path
    if (historyId.hasFileOnDisk())
    {
        QFileInfo file(historyId.filePath());

        if (file.exists())
        {
            CollectionLocation location = CollectionManager::instance()->locationForPath(historyId.path());

            if (!location.isNull())
            {
                QString album      = CollectionManager::instance()->album(file.path());
                QString name       = file.fileName();
                ItemShortInfo info = CoreDbAccess().db()->getItemShortInfo(location.id(), album, name);

                if (info.id)
                {
                    return mergedIdLists(historyId, uuidList, QList<qlonglong>() << info.id);
                }
            }
        }
    }

    return uuidList;
}

bool ItemScanner::hasHistoryToResolve() const
{
    return d->hasHistoryToResolve;
}

QString ItemScanner::uniqueHash() const
{
    // the QByteArray is an ASCII hex string
    if (d->scanInfo.category == DatabaseItem::Image)
    {
        if (CoreDbAccess().db()->isUniqueHashV2())
            return QString::fromUtf8(d->img.getUniqueHashV2());
        else
            return QString::fromUtf8(d->img.getUniqueHash());
    }
    else
    {
        if (CoreDbAccess().db()->isUniqueHashV2())
            return QString::fromUtf8(DImg::getUniqueHashV2(d->fileInfo.filePath()));
        else
            return QString::fromUtf8(DImg::getUniqueHash(d->fileInfo.filePath()));
    }
}

} // namespace Digikam
