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

#ifndef DIGIKAM_COLLECTION_SCANNER_P_H
#define DIGIKAM_COLLECTION_SCANNER_P_H

#include "collectionscanner.h"

// C++ includes

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QStringList>
#include <QSet>
#include <QTime>
#include <QWriteLocker>

// Local includes

#include "drawfiles.h"
#include "digikam_debug.h"
#include "coredb.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "collectionscannerobserver.h"
#include "coredbaccess.h"
#include "coredbbackend.h"
#include "coredbtransaction.h"
#include "coredboperationgroup.h"
#include "itemcomments.h"
#include "itemcopyright.h"
#include "iteminfo.h"
#include "itemscanner.h"
#include "metaenginesettings.h"
#include "tagscache.h"
#include "thumbsdbaccess.h"
#include "thumbsdb.h"

namespace Digikam
{

bool s_modificationDateEquals(const QDateTime& a, const QDateTime& b);

// --------------------------------------------------------------------

class Q_DECL_HIDDEN NewlyAppearedFile
{

public:

    NewlyAppearedFile();
    NewlyAppearedFile(int albumId, const QString& fileName);

    bool operator==(const NewlyAppearedFile& other) const;

public:

    int     albumId;
    QString fileName;
};

// --------------------------------------------------------------------

inline uint qHash(const NewlyAppearedFile& file)
{
    return ::qHash(file.albumId) ^ ::qHash(file.fileName);
}

// --------------------------------------------------------------------

class Q_DECL_HIDDEN CollectionScannerHintContainerImplementation : public CollectionScannerHintContainer
{
public:

    virtual void recordHints(const QList<AlbumCopyMoveHint>& hints) override;
    virtual void recordHints(const QList<ItemCopyMoveHint>& hints) override;
    virtual void recordHints(const QList<ItemChangeHint>& hints) override;
    virtual void recordHint(const ItemMetadataAdjustmentHint& hint) override;

    virtual void clear() override;

    bool hasAnyNormalHint(qlonglong id);
    bool hasAlbumHints();
    bool hasModificationHint(qlonglong id);
    bool hasRescanHint(qlonglong id);
    bool hasMetadataAboutToAdjustHint(qlonglong id);
    bool hasMetadataAdjustedHint(qlonglong id);

public:

    QReadWriteLock                                                        lock;

    QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album> albumHints;
    QHash<NewlyAppearedFile, qlonglong>                                   itemHints;
    QSet<qlonglong>                                                       modifiedItemHints;
    QSet<qlonglong>                                                       rescanItemHints;
    QHash<qlonglong, QDateTime>                                           metadataAboutToAdjustHints;
    QHash<qlonglong, QDateTime>                                           metadataAdjustedHints;
};

// --------------------------------------------------------------------

class Q_DECL_HIDDEN CollectionScanner::Private
{

public:

    explicit Private();

public:

    void resetRemovedItemsTime();
    void removedItems();

    bool checkObserver();
    bool checkDeferred(const QFileInfo& info);

    void finishScanner(ItemScanner& scanner);

public:

    QSet<QString>                                 nameFilters;
    QSet<QString>                                 imageFilterSet;
    QSet<QString>                                 videoFilterSet;
    QSet<QString>                                 audioFilterSet;
    QSet<QString>                                 ignoreDirectory;

    QList<int>                                    scannedAlbums;
    bool                                          wantSignals;
    bool                                          needTotalFiles;

    QDateTime                                     removedItemsTime;

    CollectionScannerHintContainerImplementation* hints;
    QHash<int, int>                               establishedSourceAlbums;
    bool                                          updatingHashHint;

    bool                                          recordHistoryIds;
    QSet<qlonglong>                               needResolveHistorySet;
    QSet<qlonglong>                               needTaggingHistorySet;

    bool                                          deferredFileScanning;
    QSet<QString>                                 deferredAlbumPaths;

    CollectionScannerObserver*                    observer;
};

} // namespace Digikam

#endif // DIGIKAM_COLLECTION_SCANNER_P_H
