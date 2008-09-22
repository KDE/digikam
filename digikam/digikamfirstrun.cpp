/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : dialog displayed at the first digiKam run
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QLabel>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QGridLayout>

// KDE includes.

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kseparator.h>

// Local includes.

#include "version.h"
#include "digikamfirstrun.h"
#include "digikamfirstrun.moc"

namespace Digikam
{

DigikamFirstRun::DigikamFirstRun(QWidget* parent)
               : KDialog(parent)
{
    setModal(true);
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setCaption(i18n("First Run"));
    setHelp("firstrundialog.anchor", "digikam");

    QWidget *widget = new QWidget(this);
    setMainWidget(widget);

    QVBoxLayout *vlayout = new QVBoxLayout(widget);
    QLabel *textLabel2   = new QLabel(widget);
    textLabel2->setText(i18n( "<b>Configure First Local Root Album and Database File Paths</b>"));

    KSeparator *line1    = new KSeparator(Qt::Horizontal, widget);
    QGridLayout *grid    = new QGridLayout();
    QLabel *pixLabel     = new QLabel(widget);
    pixLabel->setAlignment(Qt::AlignTop);
    pixLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                        .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel *textLabel1 = new QLabel(widget);
    textLabel1->setWordWrap(true);
    textLabel1->setText(i18n("<p>digiKam use root album paths to store your photo albums "
                             "created in <b>My Albums</b> view from left side-bar. "
                             "Below, please select which folder you would like "
                             "digiKam to use as first root album path from your local "
                             "file system.</p>"
                             "<p><b>Note: you can set other root album paths later using "
                             "digiKam settings panel. Removable medias and shared files system are "
                             "supported.</b></p>") );

    m_rootAlbumPath = new KUrlRequester(widget);
    m_rootAlbumPath->setMode(KFile::Directory | KFile::LocalOnly);
    m_rootAlbumPath->setUrl(QDir::homePath() + i18nc("This is a path name so you should "
                            "include the slash in the translation", "/Pictures"));

    QLabel *textLabel3 = new QLabel(widget);
    textLabel3->setWordWrap(true);
    textLabel3->setText(i18n("Below, set the location on your computer "
                             "where the digiKam database file will be stored. "
                             "Write access is required to be able to edit image "
                             "properties.\nPlease note that you cannot use a "
                             "remote file system here, such as NFS."));

    m_dbPath = new KUrlRequester(widget);
    m_dbPath->setMode(KFile::Directory | KFile::LocalOnly);
    m_dbPath->setUrl(QDir::homePath() + i18nc("This is a path name so you should "
                     "include the slash in the translation", "/Pictures"));

    grid->addWidget(pixLabel,        0, 0, 2, 1);
    grid->addWidget(textLabel1,      0, 1, 1, 1);
    grid->addWidget(m_rootAlbumPath, 1, 1, 1, 1);
    grid->addWidget(textLabel3,      2, 1, 1, 1);
    grid->addWidget(m_dbPath,        3, 1, 1, 1);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    vlayout->addWidget(textLabel2);
    vlayout->addWidget(line1);
    vlayout->addLayout(grid);
    vlayout->setStretchFactor(grid, 10);
    vlayout->setMargin(0);
    vlayout->setSpacing(KDialog::spacingHint());

    widget->setMinimumSize(450, sizeHint().height());

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOk()));
}

DigikamFirstRun::~DigikamFirstRun()
{
}

void DigikamFirstRun::saveSettings(const QString& rootAlbumFolder, const QString& dbFolder)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("General Settings");
    group.writeEntry("Version", digikam_version);

    group = config->group("Album Settings");
    group.writeEntry("Album Path", rootAlbumFolder);
    group.writeEntry("Database File Path", dbFolder);

    config->sync();
}

bool DigikamFirstRun::checkRootAlbum(QString& rootAlbumFolder)
{
    rootAlbumFolder = m_rootAlbumPath->url().path();

    if (rootAlbumFolder.isEmpty())
    {
        KMessageBox::sorry(this, i18n("You must select a folder for digiKam to "
                                      "use as the root album path."));
        return false;
    }

    if (!rootAlbumFolder.startsWith('/'))
    {
        rootAlbumFolder.prepend(QDir::homePath());
    }

    if (KUrl(rootAlbumFolder).equals(KUrl(QDir::homePath()), KUrl::CompareWithoutFragment))
    {
        KMessageBox::sorry(this, i18n("digiKam cannot use your home folder as "
                                      "root album path."));
        return false;
    }

    QDir targetPath(rootAlbumFolder);

    if (!targetPath.exists())
    {
        int rc = KMessageBox::questionYesNo(this,
                                   i18n("<qt>The folder to use as root album path does not exist: "
                                        "<p><b>%1</b></p>"
                                        "Would you like digiKam to create it?</qt>",
                                        rootAlbumFolder),
                                   i18n("Create Root Album Folder?"));

        if (rc == KMessageBox::No)
        {
            return false;
        }

        if (!targetPath.mkdir(rootAlbumFolder))
        {
            KMessageBox::sorry(this,
                               i18n("<qt>digiKam could not create the folder to use as root album path.\n"
                                    "Please select a different location."
                                    "<p><b>%1</b></p></qt>", rootAlbumFolder),
                               i18n("Create Root Album Folder Failed"));
            return false;
        }
    }

    QFileInfo path(rootAlbumFolder);

    if (!path.isWritable())
    {
        KMessageBox::information(this, i18n("No write access for root album path.\n"
                                            "Warning: the comment and tag features "
                                            "will not work."));
    }

    return true;
}

bool DigikamFirstRun::checkDatabase(QString& dbFolder)
{
    dbFolder = m_dbPath->url().path();

    if (dbFolder.isEmpty())
    {
        KMessageBox::sorry(this, i18n("You must select a folder for digiKam to "
                                      "store database file."));
        return false;
    }

    if (!dbFolder.startsWith('/'))
    {
        dbFolder.prepend(QDir::homePath());
    }

    if (KUrl(dbFolder).equals(KUrl(QDir::homePath()), KUrl::CompareWithoutFragment))
    {
        KMessageBox::sorry(this, i18n("digiKam cannot use your home folder as "
                                      "database file path."));
        return false;
    }

    QDir targetPath(dbFolder);

    if (!targetPath.exists())
    {
        int rc = KMessageBox::questionYesNo(this,
                                   i18n("<qt>The folder to host database file does not exist: "
                                        "<p><b>%1</b></p>"
                                        "Would you like digiKam to create it?</qt>",
                                        dbFolder),
                                   i18n("Create Database Folder?"));

        if (rc == KMessageBox::No)
        {
            return false;
        }

        if (!targetPath.mkdir(dbFolder))
        {
            KMessageBox::sorry(this,
                               i18n("<qt>digiKam could not create the folder to host database file.\n"
                                    "Please select a different location."
                                    "<p><b>%1</b></p></qt>", dbFolder),
                               i18n("Create Database Folder Failed"));
            return false;
        }
    }

    QFileInfo path(dbFolder);

    if (!path.isWritable())
    {
        KMessageBox::information(this, i18n("<p>No write access for path used to host database file.<br/>"
                                            "Please select a different location.</p>"
                                            "<p><b>%1</b></p>", dbFolder),
                                 i18n("No Database Write Access"));
        return false;
    }

    return true;
}

void DigikamFirstRun::slotOk()
{
    QString rootAlbumFolder;
    if (!checkRootAlbum(rootAlbumFolder))
        return;

    QString dbFolder;
    if (!checkDatabase(dbFolder))
        return;

    saveSettings(rootAlbumFolder, dbFolder);
    KDialog::accept();
}

}  // namespace Digikam
