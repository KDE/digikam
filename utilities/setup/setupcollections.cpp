/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2003-02-01
 * Description : collections setup tab
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// QT includes.

#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kpagedialog.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

// // Local includes.

#include "albumsettings.h"
#include "setupcollections.h"
#include "setupcollections.moc"

namespace Digikam
{

class SetupCollectionsPriv
{
public:

    SetupCollectionsPriv()
    {
        albumPathEdit    = 0;
        databasePathEdit = 0;
        mainDialog       = 0;
    }

    KUrlRequester *albumPathEdit;
    KUrlRequester *databasePathEdit;

    KPageDialog   *mainDialog;
};

SetupCollections::SetupCollections(KPageDialog* dialog, QWidget* parent)
                : QWidget(parent)
{
    d = new SetupCollectionsPriv;
    d->mainDialog = dialog;

    QVBoxLayout *layout = new QVBoxLayout( this );

    // --------------------------------------------------------

    QGroupBox *albumPathBox = new QGroupBox(i18n("Album &Library"), this);
    QVBoxLayout *gLayout1   = new QVBoxLayout(albumPathBox);

    QLabel *rootsPathLabel  = new QLabel(i18n("Root album path:"), albumPathBox);

    d->albumPathEdit = new KUrlRequester(albumPathBox);
    d->albumPathEdit->setMode(KFile::Directory | KFile::LocalOnly | KFile::ExistingOnly);    
    d->albumPathEdit->setToolTip(i18n("<p>Here you can set the main path to the digiKam album "
                                      "library in your computer."
                                      "<p>Write access is required for this path."));

    connect(d->albumPathEdit, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotChangeAlbumPath(const KUrl &)));

    connect(d->albumPathEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotAlbumPathEdited(const QString&)) );

    QLabel *databasePathLabel = new QLabel(i18n("Path to store database:"), albumPathBox);
    d->databasePathEdit       = new KUrlRequester(albumPathBox);
    d->databasePathEdit->setMode(KFile::Directory | KFile::LocalOnly);    

    d->databasePathEdit->setToolTip(i18n("<p>Here you can set the path use to host to the digiKam database "
                                         "in your computer. The database file is common for all roots album path. "
                                         "<p>Write access is required for this path."
                                         "<p>Do not use a remote path here, like an NFS mounted file system."));

    connect(d->databasePathEdit, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotChangeDatabsePath(const KUrl &)));

    connect(d->databasePathEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotDatabasePathEdited(const QString&)) );

    gLayout1->addWidget(rootsPathLabel);
    gLayout1->addWidget(d->albumPathEdit);
    gLayout1->addWidget(databasePathLabel);
    gLayout1->addWidget(d->databasePathEdit);
    gLayout1->setSpacing(0);
    gLayout1->setMargin(KDialog::spacingHint());
   
    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(albumPathBox);
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupCollections::~SetupCollections()
{
    delete d;
}

void SetupCollections::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setAlbumLibraryPath(d->albumPathEdit->url().path());
    settings->setDatabaseFilePath(d->databasePathEdit->url().path());
    settings->saveSettings();
}

void SetupCollections::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->albumPathEdit->setUrl(settings->getAlbumLibraryPath());
    d->databasePathEdit->setUrl(settings->getDatabaseFilePath());
}

void SetupCollections::slotChangeAlbumPath(const KUrl &result)
{
    if (KUrl(result).equals(KUrl(QDir::homePath()), KUrl::CompareWithoutTrailingSlash)) 
    {
        KMessageBox::sorry(0, i18n("Sorry; cannot use home directory as album library."));
        return;
    }

    QFileInfo targetPath(result.path());

    if (!result.isEmpty() && !targetPath.isWritable()) 
    {
        KMessageBox::information(0, i18n("No write access for this root album path.\n"
                                         "Warning: image and metadata editing will not work."));
    }
}

void SetupCollections::slotChangeDatabasePath(const KUrl &result)
{
    QFileInfo targetPath(result.path());

    if (!result.isEmpty() && !targetPath.isWritable()) 
    {
        KMessageBox::information(0, i18n("No write access for this path to store database.\n"
                                         "Warning: the caption and tag features will not work."));
    }
}

void SetupCollections::slotAlbumPathEdited(const QString& newPath)
{
    if (newPath.isEmpty()) 
    {
        d->mainDialog->enableButtonOk(false);
        return;
    }

    if (!newPath.startsWith("/")) 
    {
        d->albumPathEdit->setUrl(QDir::homePath() + '/' + newPath);
    }

    checkforOkButton();
}

void SetupCollections::slotDatabasePathEdited(const QString& newPath)
{
    if (newPath.isEmpty()) 
    {
        d->mainDialog->enableButtonOk(false);
        return;
    }

    if (!newPath.startsWith("/")) 
    {
        d->databasePathEdit->setUrl(QDir::homePath() + '/' + newPath);
    }

    checkforOkButton();
}

void SetupCollections::checkforOkButton()
{
    QFileInfo albumPath(d->albumPathEdit->url().path());
    QDir albumDir(d->albumPathEdit->url().path());
    bool albumOk = albumDir.exists() && (albumDir.path() != QDir::homePath());
    
    QDir dbDir(d->databasePathEdit->url().path());
    bool dbOk = dbDir.exists();

    d->mainDialog->enableButtonOk(dbOk && albumOk);
}

}  // namespace Digikam
