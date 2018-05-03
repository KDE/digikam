/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Import tools
 *
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikamapp.h"
#include "digikamapp_p.h"

namespace Digikam
{

QString DigikamApp::scannerTargetPlace()
{
    QString place    = QDir::homePath();
    QStringList pics = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    if (!pics.isEmpty())
        place = pics.first();

    Album* const album = AlbumManager::instance()->currentAlbums().first();

    if (album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(album);

        if (p)
        {
            place = p->folderPath();
        }
    }
    else
    {
        QStringList cols = CollectionManager::instance()->allAvailableAlbumRootPaths();

        if (!cols.isEmpty())
            place = cols.first();
    }

    return place;
}

void DigikamApp::updateQuickImportAction()
{
    d->quickImportMenu->clear();

    foreach(QAction* const action, d->solidCameraActionGroup->actions())
    {
        d->quickImportMenu->addAction(action);
    }

    foreach(QAction* const action, d->solidUsmActionGroup->actions())
    {
        d->quickImportMenu->addAction(action);
    }

    foreach(QAction* const action, d->manualCameraActionGroup->actions())
    {
        d->quickImportMenu->addAction(action);
    }

    if (d->quickImportMenu->actions().isEmpty())
    {
        d->quickImportMenu->setEnabled(false);
    }
    else
    {
        disconnect(d->quickImportMenu->menuAction(), SIGNAL(triggered()), 0, 0);

        QAction*  primaryAction = 0;
        QDateTime latest;

        foreach(QAction* const action, d->quickImportMenu->actions())
        {
            QDateTime appearanceTime = d->cameraAppearanceTimes.value(action->data().toString());

            if (latest.isNull() || appearanceTime > latest)
            {
                primaryAction = action;
                latest        = appearanceTime;
            }
        }

        if (!primaryAction)
        {
            primaryAction = d->quickImportMenu->actions().first();
        }

        connect(d->quickImportMenu->menuAction(), SIGNAL(triggered()),
                primaryAction, SLOT(trigger()));

        d->quickImportMenu->setEnabled(true);
    }
}

void DigikamApp::slotImportAddImages()
{
    QString startingPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QUrl url             = DFileDialog::getExistingDirectoryUrl(this, i18n("Select folder to parse"),
                                                                QUrl::fromLocalFile(startingPath));

    if (url.isEmpty() || !url.isLocalFile())
    {
        return;
    }

    // The folder contents will be parsed by Camera interface in "Directory Browse" mode.
    downloadFrom(url.toLocalFile());
}

void DigikamApp::slotImportAddFolders()
{
    // NOTE: QFileDialog don't have an option to permit multiple selection of directories.
    // This work around is inspired from http://www.qtcentre.org/threads/34226-QFileDialog-select-multiple-directories
    // Check Later Qt 5.4 if a new native Qt way have been introduced.

    QPointer<DFileDialog> dlg = new DFileDialog(this);
    dlg->setWindowTitle(i18n("Select folders to import into album"));
    dlg->setFileMode(QFileDialog::Directory);
    dlg->setOptions(QFileDialog::ShowDirsOnly);

    QListView* const l = dlg->findChild<QListView*>(QLatin1String("listView"));

    if (l)
    {
        l->setSelectionMode(QAbstractItemView::MultiSelection);
    }

    QTreeView* const t = dlg->findChild<QTreeView*>();

    if (t)
    {
        t->setSelectionMode(QAbstractItemView::MultiSelection);
    }

    if (dlg->exec() != QDialog::Accepted)
    {
        delete dlg;
        return;
    }

    QList<QUrl> urls = dlg->selectedUrls();
    delete dlg;

    if (urls.isEmpty())
    {
        return;
    }

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* album            = 0;

    if (!albumList.isEmpty())
    {
        album = albumList.first();
    }

    if (album && album->type() != Album::PHYSICAL)
    {
        album = 0;
    }

    QString header(i18n("<p>Please select the destination album from the digiKam library to "
                        "import folders into.</p>"));

    album = AlbumSelectDialog::selectAlbum(this, (PAlbum*)album, header);

    if (!album)
    {
        return;
    }

    PAlbum* const pAlbum = dynamic_cast<PAlbum*>(album);

    if (!pAlbum)
    {
        return;
    }

    DIO::copy(urls, pAlbum);
}

void DigikamApp::slotImportFromScanner()
{
#ifdef HAVE_KSANE
    m_ksaneAction->activate(scannerTargetPlace(), configGroupName());
#endif
}

void DigikamApp::slotImportTool()
{
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_importGphotoAction)
    {
        QPointer<GSWindow> w = new GSWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                                            this, QLatin1String("googlephotoimport"));
        w->exec();
        delete w;
    }
    else if (tool == m_importSmugmugAction)
    {
        QPointer<SmugWindow> w = new SmugWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                                                this, true);
        w->exec();
        delete w;
    }

#ifdef HAVE_KIO
    else if (tool == m_importFileTransferAction)
    {
        QPointer<FTImportWindow> w = new FTImportWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
#endif
}

} // namespace Digikam
