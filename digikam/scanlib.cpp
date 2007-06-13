/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
 * Description : scan pictures interface.
 * 
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
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

// C Ansi includes.

extern "C"
{
#include <sys/time.h>
}

// C++ includes

#include <ctime>
#include <cstdlib>


// Qt includes.

#include <qapplication.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qwhatsthis.h>
#include <qpixmap.h>

// KDE includes.

#include <kprogress.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>

// Local includes.

#include "ddebug.h"
#include "collectionscanner.h"
#include "databaseaccess.h"
#include "databasetransaction.h"
#include "dprogressdlg.h"
#include "dmetadata.h"
#include "albumdb.h"
#include "albummanager.h"
#include "scanlib.h"

/** @file scanlib.cpp*/

namespace Digikam
{

ScanLib::ScanLib()
{
    m_progressBar = new DProgressDlg(0);
    m_progressBar->setInitialSize(QSize(500, 100), true);
    m_progressBar->setActionListVSBarVisible(false);
    QWhatsThis::add( m_progressBar, i18n("This shows the progress of the "
        "scan. During the scan, all files on disk are registered in a "
        "database. This is required for sorting on exif-date and speeds up "
        "the overall performance of digiKam.") );

    // these two lines prevent the dialog to be shown in
    // findFoldersWhichDoNotExist() method.
    m_progressBar->progressBar()->setTotalSteps(1);
    m_progressBar->progressBar()->setProgress(1);

    connect(&m_scanner, SIGNAL(totalFilesToScan(int)),
            this, SLOT(slotTotalFilesToScan(int)));
    connect(&m_scanner, SIGNAL(startScanningAlbum(const QString &, const QString &)),
            this, SLOT(slotStartScanningAlbum(const QString &, const QString &)));
    connect(&m_scanner, SIGNAL(finishedScanningAlbum(const QString &, const QString &, int)),
            this, SLOT(slotFinishedScanningAlbum(const QString &, const QString &, int)));
    connect(&m_scanner, SIGNAL(scanningFile(const QString &)),
            this, SLOT(slotScanningFile(const QString &)));
}

ScanLib::~ScanLib()
{
    delete m_progressBar;
}

void ScanLib::startScan()
{
    struct timeval tv1, tv2;
    QPixmap pix = KApplication::kApplication()->iconLoader()->loadIcon(
                  "run", KIcon::NoGroup, 32);

    QString message = i18n("Finding non-existing Albums");
    m_progressBar->addedAction(pix, message);
    gettimeofday(&tv1, 0);
    findFoldersWhichDoNotExist();
    gettimeofday(&tv2, 0);
    timing(message, tv1, tv2);

    message = i18n("Finding items not in the database or disk");
    m_progressBar->addedAction(pix, message);
    gettimeofday(&tv1, 0);
    findMissingItems();
    gettimeofday(&tv2, 0);
    timing(message, tv1, tv2);

    message = i18n("Updating items without date");
    m_progressBar->addedAction(pix, message);
    gettimeofday(&tv1, 0);
    updateItemsWithoutDate();
    gettimeofday(&tv2, 0);
    timing(message, tv1, tv2);

    deleteStaleEntries();

    m_scanner.markDatabaseAsScanned();
}

void ScanLib::findFoldersWhichDoNotExist()
{
    m_scanner.scanForStaleAlbums();
    QStringList toBeDeleted = m_scanner.formattedListOfStaleAlbums();

    if (!toBeDeleted.isEmpty())
    {
        int rc = KMessageBox::warningYesNoList(0,
            i18n("<p>There is an album in the database which does not appear to "
                 "be on disk. This album should be removed from the database, "
                 "however you may lose information because all images "
                 "associated with this album will be removed from the database "
                 "as well.<p>"
                 "digiKam cannot continue without removing the items "
                 "from the database because all views depend on the information "
                 "in the database. Do you want them to be removed from the "
                 "database?",
                 "<p>There are %n albums in the database which do not appear to "
                 "be on disk. These albums should be removed from the database, "
                 "however you may lose information because all images "
                 "associated with these albums will be removed from the database "
                 "as well.<p>"
                 "digiKam cannot continue without removing the items "
                 "from the database because all views depend on the information "
                 "in the database. Do you want them to be removed from the "
                 "database?",
                 toBeDeleted.count()),
            toBeDeleted,
            i18n("Albums are Missing"));

        if (rc != KMessageBox::Yes)
            exit(0);

        m_scanner.removeStaleAlbums();
    }
}

void ScanLib::findMissingItems()
{
    m_progressBar->setAllowCancel( false );
    m_progressBar->showCancelButton (false );
    m_progressBar->progressBar()->setProgress( 0 );
    m_progressBar->setLabel(i18n("Scanning items, please wait..."));
    kapp->processEvents();

    m_scanner.scanAlbums();

    m_progressBar->hide();
    kapp->processEvents();
}

void ScanLib::slotTotalFilesToScan(int count)
{
    m_progressBar->progressBar()->setTotalSteps( count );
    if (count > 0)
        m_progressBar->show();
    kapp->processEvents();
}

void ScanLib::slotStartScanningAlbum(const QString &albumRoot, const QString &album)
{
    QPixmap pix = KApplication::kApplication()->iconLoader()->loadIcon(
                  "folder_image", KIcon::NoGroup, 32);
    m_progressBar->addedAction(pix, albumRoot + album);
    kapp->processEvents();
}

void ScanLib::slotFinishedScanningAlbum(const QString &, const QString &, int filesScanned)
{
    m_progressBar->progressBar()->advance(filesScanned);
    kapp->processEvents();
}

void ScanLib::slotScanningFile(const QString &)
{
    m_progressBar->progressBar()->advance(1);
    if (m_progressBar->progressBar()->progress() % 30 == 0)
        kapp->processEvents();
}

void ScanLib::updateItemsWithoutDate()
{
    m_progressBar->setAllowCancel( false );
    m_progressBar->showCancelButton (false );
    m_progressBar->progressBar()->setProgress(0);
    m_progressBar->setLabel(i18n("Updating items, please wait..."));
    kapp->processEvents();

    m_scanner.updateItemsWithoutDate();

    m_progressBar->hide();
    kapp->processEvents();
}

void ScanLib::deleteStaleEntries()
{
    QStringList listToBeDeleted = m_scanner.formattedListOfStaleFiles();

    if ( !listToBeDeleted.isEmpty() )
    {
        int rc = KMessageBox::warningYesNoList(0,
          i18n("<p>There is an item in the database which does not "
               "appear to be on disk or is located in the root album of "
               "the path. This file should be removed from the "
               "database, however you may lose information.<p>"
               "digiKam cannot continue without removing the item from "
               "the database because all views depend on the information "
               "in the database. Do you want it to be removed from the "
               "database?",
               "<p>There are %n items in the database which do not "
               "appear to be on disk or are located in the root album of "
               "the path. These files should be removed from the "
               "database, however you may lose information.<p>"
               "digiKam cannot continue without removing these items from "
               "the database because all views depend on the information "
               "in the database. Do you want them to be removed from the "
               "database?",
               listToBeDeleted.count()),
          listToBeDeleted,
          i18n("Files are Missing"));

        if (rc != KMessageBox::Yes)
            exit(0);

        m_scanner.removeStaleFiles();
    }
}

void ScanLib::timing(const QString& text, struct timeval tv1, struct timeval tv2)
{
    DDebug() << "ScanLib: "
              << text + ": "
              << (((tv2.tv_sec-tv1.tv_sec)*1000000 +
                   (tv2.tv_usec-tv1.tv_usec))/1000)
              << " ms" << endl;
}

}  // namespace Digikam
