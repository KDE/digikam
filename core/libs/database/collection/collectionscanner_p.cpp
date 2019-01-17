/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database - private containers.
 *
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "collectionscanner_p.h"

namespace Digikam
{

bool s_modificationDateEquals(const QDateTime& a, const QDateTime& b)
{
    if (a != b)
    {
        // allow a "modify window" of one second.
        // FAT filesystems store the modify date in 2-second resolution.
        int diff = a.secsTo(b);

        if (abs(diff) > 1)
        {
            return false;
        }
    }

    return true;
}

// --------------------------------------------------------------------

NewlyAppearedFile::NewlyAppearedFile()
    : albumId(0)
{
}

NewlyAppearedFile::NewlyAppearedFile(int albumId, const QString& fileName)
    : albumId(albumId),
      fileName(fileName)
{
}

bool NewlyAppearedFile::operator==(const NewlyAppearedFile& other) const
{
    return (albumId  == other.albumId) &&
           (fileName == other.fileName);
}

// --------------------------------------------------------------------

bool CollectionScannerHintContainerImplementation::hasAnyNormalHint(qlonglong id)
{
    QReadLocker locker(&lock);

    return modifiedItemHints.contains(id)          ||
           rescanItemHints.contains(id)            ||
           metadataAboutToAdjustHints.contains(id) ||
           metadataAdjustedHints.contains(id);
}

bool CollectionScannerHintContainerImplementation::hasAlbumHints()
{
    QReadLocker locker(&lock);
    return !albumHints.isEmpty();
}

bool CollectionScannerHintContainerImplementation::hasModificationHint(qlonglong id)
{
    QReadLocker locker(&lock);
    return modifiedItemHints.contains(id);
}

bool CollectionScannerHintContainerImplementation::hasRescanHint(qlonglong id)
{
    QReadLocker locker(&lock);
    return rescanItemHints.contains(id);
}

bool CollectionScannerHintContainerImplementation::hasMetadataAboutToAdjustHint(qlonglong id)
{
    QReadLocker locker(&lock);
    return metadataAboutToAdjustHints.contains(id);
}

bool CollectionScannerHintContainerImplementation::hasMetadataAdjustedHint(qlonglong id)
{
    QReadLocker locker(&lock);
    return metadataAdjustedHints.contains(id);
}

void CollectionScannerHintContainerImplementation::recordHints(const QList<AlbumCopyMoveHint>& hints)
{
    QWriteLocker locker(&lock);

    foreach (const AlbumCopyMoveHint& hint, hints)
    {
        // automagic casting to src and dst
        albumHints[hint] = hint;
    }
}

void CollectionScannerHintContainerImplementation::recordHints(const QList<ItemCopyMoveHint>& hints)
{
    QWriteLocker locker(&lock);

    foreach (const ItemCopyMoveHint& hint, hints)
    {
        QList<qlonglong> ids = hint.srcIds();
        QStringList dstNames = hint.dstNames();

        for (int i = 0 ; i < ids.size() ; ++i)
        {
            itemHints[NewlyAppearedFile(hint.albumIdDst(), dstNames.at(i))] = ids.at(i);
        }
    }
}

void CollectionScannerHintContainerImplementation::recordHints(const QList<ItemChangeHint>& hints)
{
    QWriteLocker locker(&lock);

    foreach (const ItemChangeHint& hint, hints)
    {
        const QList<qlonglong>& ids = hint.ids();

        for (int i = 0 ; i < ids.size() ; ++i)
        {
            if (hint.isModified())
            {
                modifiedItemHints << ids.at(i);
            }
            else
            {
                rescanItemHints << ids.at(i);
            }
        }
    }
}

void CollectionScannerHintContainerImplementation::recordHint(const ItemMetadataAdjustmentHint& hint)
{
    if (hint.isAboutToEdit())
    {
        ItemInfo info(hint.id());

        if (!
            (s_modificationDateEquals(hint.modificationDate(), info.modDateTime())
             && hint.fileSize() == info.fileSize())
           )
        {
            // refuse to create a hint as a rescan is required already before any metadata edit
            // or, in case of multiple edits, there is already a hint with an older date, then all is fine.
            return;
        }

        QWriteLocker locker(&lock);
        metadataAboutToAdjustHints[hint.id()] = hint.modificationDate();
    }
    else if (hint.isEditingFinished())
    {
        QWriteLocker locker(&lock);
        QHash<qlonglong, QDateTime>::iterator it = metadataAboutToAdjustHints.find(hint.id());

        if (it == metadataAboutToAdjustHints.end())
        {
            return;
        }

        QDateTime date                   = it.value();
        metadataAboutToAdjustHints.erase(it);

        metadataAdjustedHints[hint.id()] = hint.modificationDate();
    }
    else // Aborted
    {
         QWriteLocker locker(&lock);
         QDateTime formerDate = metadataAboutToAdjustHints.take(hint.id());
    }
}

void CollectionScannerHintContainerImplementation::clear()
{
    QWriteLocker locker(&lock);

    albumHints.clear();
    itemHints.clear();
    modifiedItemHints.clear();
    rescanItemHints.clear();
    metadataAboutToAdjustHints.clear();
    metadataAdjustedHints.clear();
}

// --------------------------------------------------------------------

CollectionScanner::Private::Private()
    : wantSignals(false),
      needTotalFiles(false),
      hints(0),
      updatingHashHint(false),
      recordHistoryIds(false),
      deferredFileScanning(false),
      observer(0)
{
}

void CollectionScanner::Private::resetRemovedItemsTime()
{
    removedItemsTime = QDateTime();
}

void CollectionScanner::Private::removedItems()
{
    removedItemsTime = QDateTime::currentDateTime();
}

bool CollectionScanner::Private::checkObserver()
{
    if (observer)
    {
        return observer->continueQuery();
    }

    return true;
}

bool CollectionScanner::Private::checkDeferred(const QFileInfo& info)
{
    if (deferredFileScanning)
    {
        deferredAlbumPaths << info.path();
        return true;
    }

    return false;
}

void CollectionScanner::Private::finishScanner(ItemScanner& scanner)
{
    // Perform the actual write operation to the database
    {
        CoreDbOperationGroup group;
        scanner.commit();
    }

    if (recordHistoryIds && scanner.hasHistoryToResolve())
    {
        needResolveHistorySet << scanner.id();
    }
}

} // namespace Digikam
