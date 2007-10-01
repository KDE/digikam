/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
 * Description : scan pictures interface.
 * 
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// Qt includes.

#include <QApplication>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QProgressBar>
#include <QTime>

// KDE includes.

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
#include "albumsettings.h"
#include "albumdb.h"
#include "albummanager.h"
#include "scanlib.h"
#include "scanlib.moc"

/** @file scanlib.cpp*/

namespace Digikam
{

ScanLib::ScanLib() 
       : QObject()
{
    m_progressDlg = new DProgressDlg(0);
    m_progressDlg->setInitialSize(QSize(500, 100));
    m_progressDlg->setActionListVSBarVisible(false);
    m_progressDlg->setWhatsThis( i18n("This shows the progress of the "
        "scan. During the scan, all files on disk are registered in a "
        "database. This is required for sorting on exif-date and speeds up "
        "the overall performance of digiKam.") );

    // these two lines prevent the dialog to be shown in
    // findFoldersWhichDoNotExist() method.
    m_progressDlg->setMaximum(1);
    m_progressDlg->setValue(1);

    m_scanner.setNameFilters(AlbumSettings::instance()->getAllFileFilter());

    connect(&m_scanner, SIGNAL(totalFilesToScan(int)),
            this, SLOT(slotTotalFilesToScan(int)));

    connect(&m_scanner, SIGNAL(startScanningAlbum(const QString&, const QString&)),
            this, SLOT(slotStartScanningAlbum(const QString&, const QString&)));

    connect(&m_scanner, SIGNAL(finishedScanningAlbum(const QString&, const QString&, int)),
            this, SLOT(slotFinishedScanningAlbum(const QString&, const QString&, int)));

    connect(&m_scanner, SIGNAL(scanningFile(const QString&)),
            this, SLOT(slotScanningFile(const QString&)));
}

ScanLib::~ScanLib()
{
    delete m_progressDlg;
}

void ScanLib::startScan()
{
    QTime time;
    QPixmap pix = KIconLoader::global()->loadIcon("system-run", KIconLoader::NoGroup, 32);

    QString message = i18n("Finding non-existing Albums");
    m_progressDlg->addedAction(pix, message);
    time.start();
    findFoldersWhichDoNotExist();
    timing(message, time.elapsed());

    message = i18n("Finding items not in the database or disk");
    m_progressDlg->addedAction(pix, message);
    time.start();
    findMissingItems();
    timing(message, time.elapsed());

    message = i18n("Updating items without date");
    m_progressDlg->addedAction(pix, message);
    time.start();
    updateItemsWithoutDate();
    timing(message, time.elapsed());

    deleteStaleEntries();

    m_scanner.markDatabaseAsScanned();
}

void ScanLib::timing(const QString& text, int elaspedms)
{
    DDebug() << "ScanLib: " << text
             << ": " << elaspedms
             << " ms" << endl;
}

void ScanLib::findFoldersWhichDoNotExist()
{
    m_scanner.scanForStaleAlbums();
    QStringList toBeDeleted = m_scanner.formattedListOfStaleAlbums();

    if (!toBeDeleted.isEmpty())
    {
        int rc = KMessageBox::warningYesNoList(0,
            i18np("<p>There is an album in the database which does not appear to "
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
                  QString::number(toBeDeleted.count())),
            toBeDeleted,
            i18n("Albums are Missing"));

        if (rc != KMessageBox::Yes)
            exit(0);

        m_scanner.removeStaleAlbums();
    }
}

void ScanLib::findMissingItems()
{
    m_progressDlg->setAllowCancel(false);
    m_progressDlg->showCancelButton(false);
    m_progressDlg->setValue(0);
    m_progressDlg->setLabel(i18n("Scanning items, please wait..."));
    qApp->processEvents();

    m_scanner.scanAlbums();

    m_progressDlg->hide();
    kapp->processEvents();
}

void ScanLib::slotTotalFilesToScan(int count)
{
    m_progressDlg->setMaximum(count);
    if (count > 0)
        m_progressDlg->show();
    qApp->processEvents();
}

void ScanLib::slotStartScanningAlbum(const QString &albumRoot, const QString &album)
{
    QPixmap pix = KIconLoader::global()->loadIcon("folder-image", KIconLoader::NoGroup, 32);
    m_progressDlg->addedAction(pix, albumRoot + album);
    qApp->processEvents();
}

void ScanLib::slotFinishedScanningAlbum(const QString &, const QString &, int filesScanned)
{
    m_progressDlg->advance(filesScanned);
    qApp->processEvents();
}

void ScanLib::slotScanningFile(const QString &)
{
    m_progressDlg->advance(1);
    if (m_progressDlg->value() % 30 == 0)
        qApp->processEvents();
}

void ScanLib::updateItemsWithoutDate()
{
    m_progressDlg->setAllowCancel( false );
    m_progressDlg->showCancelButton (false );
    m_progressDlg->setValue(0);
    m_progressDlg->setLabel(i18n("Updating items, please wait..."));
    kapp->processEvents();

    m_scanner.updateItemsWithoutDate();

    m_progressDlg->hide();
    qApp->processEvents();
}

void ScanLib::deleteStaleEntries()
{
    QStringList listToBeDeleted = m_scanner.formattedListOfStaleFiles();

    if ( !listToBeDeleted.isEmpty() )
    {
        int rc = KMessageBox::warningYesNoList(0,
          i18np("<p>There is an item in the database which does not "
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

}  // namespace Digikam
