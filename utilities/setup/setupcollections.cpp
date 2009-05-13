/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-01
 * Description : collections setup tab
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupcollections.h"
#include "setupcollections.moc"

// Qt includes

#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QList>
#include <QFileInfo>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <klineedit.h>
#include <kpagedialog.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

// Local includes

#include "albumsettings.h"
#include "setupcollectionview.h"

namespace Digikam
{

class SetupCollectionsPriv
{
public:

    SetupCollectionsPriv()
    {
        mainDialog       = 0;
        collectionView   = 0;
        collectionModel  = 0;
        databasePathEdit = 0;
        rootsPathChanged = false;
    }

    bool                     rootsPathChanged;

    SetupCollectionTreeView *collectionView;
    SetupCollectionModel    *collectionModel;

    KUrlRequester           *databasePathEdit;
    QString                  originalDbPath;

    KPageDialog             *mainDialog;
};

SetupCollections::SetupCollections(KPageDialog* dialog, QWidget* parent)
                : QScrollArea(parent), d(new SetupCollectionsPriv)
{
    d->mainDialog = dialog;

    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QVBoxLayout *layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox *albumPathBox = new QGroupBox(i18n("Root Album Folders"), panel);

#ifndef _WIN32
    QLabel *albumPathLabel  = new QLabel(i18n("Below are the locations of your root albums used to store "
                                              "your images. Write access is necessary to be able "
                                              "to edit images in these albums.\n"
                                              "Note: Removable media (such as USB drives or DVDs) and remote file systems "
                                              "(such as NFS, or Samba mounted with cifs/smbfs) are supported."),
                                         albumPathBox);
#else
    QLabel *albumPathLabel  = new QLabel(i18n("Below are the locations of your root albums used to store "
                                              "your images. Write access is necessary to be able "
                                              "to edit images in these albums.\n"),
                                         albumPathBox);
#endif
    albumPathLabel->setWordWrap(true);

    d->collectionView  = new SetupCollectionTreeView(albumPathBox);
    d->collectionModel = new SetupCollectionModel(panel);
    d->collectionView->setModel(d->collectionModel);

    QVBoxLayout *albumPathBoxLayout = new QVBoxLayout;
    albumPathBoxLayout->addWidget(albumPathLabel);
    albumPathBoxLayout->addWidget(d->collectionView);
    albumPathBox->setLayout(albumPathBoxLayout);
    albumPathBoxLayout->setSpacing(0);
    albumPathBoxLayout->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    QGroupBox *dbPathBox      = new QGroupBox(i18n("Database File Path"), panel);
    QVBoxLayout *vlay         = new QVBoxLayout(dbPathBox);
    QLabel *databasePathLabel = new QLabel(i18n("The location "
                                                "where the database file will be stored on your system. "
                                                "There is one common database file for all root albums.\n"
                                                "Write access is required to be able to edit image properties.\n"
                                                "Note: a remote file system, such as NFS, cannot be used here."),
                                           dbPathBox);
    databasePathLabel->setWordWrap(true);

    d->databasePathEdit = new KUrlRequester(dbPathBox);
    d->databasePathEdit->setMode(KFile::Directory | KFile::LocalOnly);

    vlay->addWidget(databasePathLabel);
    vlay->addWidget(d->databasePathEdit);
    vlay->setSpacing(0);
    vlay->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(albumPathBox);
    layout->addWidget(dbPathBox);
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();
    adjustSize();

    // --------------------------------------------------------

    connect(d->databasePathEdit, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotChangeDatabasePath(const KUrl &)));

    connect(d->databasePathEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotDatabasePathEdited(const QString&)));
}

SetupCollections::~SetupCollections()
{
    delete d;
}

void SetupCollections::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    QString newPath = d->databasePathEdit->url().path();
    QDir oldDir(d->originalDbPath);
    QDir newDir(newPath);
    if (oldDir != newDir)
    {
        settings->setDatabaseFilePath(newPath);
        settings->saveSettings();
    }
    else
    {
        d->collectionModel->apply();
    }
}

void SetupCollections::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->originalDbPath = settings->getDatabaseFilePath();
    d->databasePathEdit->setUrl(settings->getDatabaseFilePath());
    d->collectionModel->loadCollections();
}

void SetupCollections::slotChangeDatabasePath(const KUrl& result)
{
    QFileInfo targetPath(result.path());

    if (!result.isEmpty() && !targetPath.isWritable())
    {
        KMessageBox::information(0, i18n("You do not seem to have write access to this database folder.\n"
                                         "Without this access, the caption and tag features will not work."));
    }

    checkDBPath();
}

void SetupCollections::slotDatabasePathEdited(const QString& newPath)
{
    if (!newPath.isEmpty() && !newPath.startsWith('/'))
    {
        d->databasePathEdit->setUrl(QDir::homePath() + '/' + newPath);
    }

    checkDBPath();
}

void SetupCollections::checkDBPath()
{
    bool dbOk = false;
    bool pathUnchanged = true;
    QString newPath = d->databasePathEdit->url().path();
    if (!d->databasePathEdit->url().path().isEmpty())
    {
        QDir dbDir(newPath);
        QDir oldDir(d->originalDbPath);
        dbOk = dbDir.exists();
        pathUnchanged = (dbDir == oldDir);
    }

    d->collectionView->setEnabled(pathUnchanged);

    d->mainDialog->enableButtonOk(dbOk);
}

}  // namespace Digikam
