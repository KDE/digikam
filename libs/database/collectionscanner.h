/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef COLLECTIONSCANNER_H
#define COLLECTIONSCANNER_H

// Qt includes

#include <qstring.h>
#include <qobject.h>

// Local includes

#include "digikam_export.h"
#include "databaseaccess.h"

namespace Digikam
{

class DIGIKAM_EXPORT CollectionScanner : public QObject
{
    Q_OBJECT
public:

    // Scanning from ioslave
    void scanAlbum(const QString &albumRoot, const QString &album);
    void scanOneAlbum(const QString &albumRoot, const QString &album);
    void removeInvalidAlbums(const QString &albumRoot);

    // Scanning from Scanlib
    void scanAlbums();
    void scanAlbumScanLib(const QString& filePath);
    void scanAlbumScanLib(const QString &albumRoot, const QString& album);

    void scanForStaleAlbums();
    void scanForStaleAlbums(const QString &albumRoot);
    QStringList formattedListOfStaleAlbums();
    void removeStaleAlbums();

    QStringList formattedListOfStaleFiles();
    void removeStaleFiles();

    void updateItemsWithoutDate();

    void markDatabaseAsScanned();

    // Tools
    static void addItem(int albumID, const QString& albumRoot, const QString &album, const QString &name);
    static void addItem(DatabaseAccess &access, int albumID,
                        const QString& albumRoot, const QString &album, const QString &name);
    static void updateItemDate(int albumID, const QString& albumRoot, const QString &album, const QString &fileName);
    static void updateItemDate(Digikam::DatabaseAccess &access, int albumID,
                               const QString& albumRoot, const QString &album, const QString &fileName);

signals:

    void totalFilesToScan(int count);
    void startScanningAlbum(const QString &albumRoot, const QString &album);
    void finishedScanningAlbum(const QString &albumRoot, const QString &album, int filesScanned);

    void totalFilesToUpdate(int count);
    void scanningFile(const QString &filePath);

protected:

    int countItemsInFolder(const QString& directory);

    QValueList< QPair<QString,int> >  m_filesToBeDeleted;
    QMap<QString, int> m_foldersToBeDeleted;

};


}

#endif

