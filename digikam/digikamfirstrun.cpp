/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : dialog displayed at the first digiKam run
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikamfirstrun.h"
#include "digikamfirstrun.moc"

// Qt includes

#include <QLabel>
#include <QString>
#include <QDir>
#include <QDesktopServices>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QGridLayout>

// KDE includes

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kseparator.h>

// Local includes

#include "version.h"

namespace Digikam
{

class DigikamFirstRunPriv
{
public:

    DigikamFirstRunPriv()
    {
        rootAlbumPathRequester = 0;
        dbPathRequester        = 0;
        dbPathEdited           = false;
    }

    bool           dbPathEdited;

    QString        rootAlbum;
    QString        dbPath;

    KUrlRequester *rootAlbumPathRequester;
    KUrlRequester *dbPathRequester;
};

DigikamFirstRun::DigikamFirstRun(QWidget* parent)
               : KDialog(parent), d(new DigikamFirstRunPriv)
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
    textLabel2->setText(i18n( "<b>First-run Wizard: Configure where images and meta-data are stored</b>"));

    KSeparator *line1    = new KSeparator(Qt::Horizontal, widget);
    QGridLayout *grid    = new QGridLayout();
    QLabel *pixLabel     = new QLabel(widget);
    pixLabel->setAlignment(Qt::AlignTop);
    pixLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                        .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QString picturesPath;
#if KDE_IS_VERSION(4,1,61)
    picturesPath = KGlobalSettings::picturesPath();
#else
#if QT_VERSION >= 0x040400
    picturesPath = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
#endif
#endif
    kDebug() << picturesPath;
    if (picturesPath.isEmpty())
    {
        picturesPath = QDir::homePath() + i18nc("This is a path name so you should "
                                                "include the slash in the translation", "/Pictures");
    }

    QLabel *textLabel1 = new QLabel(widget);
    textLabel1->setWordWrap(true);
#ifndef _WIN32
    textLabel1->setText(i18n("<p>Please enter a location where you want to store your images.</p> "
                             "<p>You can choose any local folder, even one that already contains images."
                             "<br/> "
                             "More folders can be added later under the <i>Settings</i> menu. "
                             "</p> "
                             "<p>Note: Removable media (such as USB drives or DVDs) and remote file systems "
                             "(such as NFS, or Samba mounted with cifs/smbfs) are supported.</p>") );
#else
    textLabel1->setText(i18n("<p>Please enter a location where you want to store your images.</p> "
                             "<p>You can choose any local folder, even one that already contains images."
                             "<br/> "
                             "More folders can be added later under the <i>Settings</i> menu. "
                             "</p> ") );
#endif

    d->rootAlbumPathRequester = new KUrlRequester(widget);
    d->rootAlbumPathRequester->setMode(KFile::Directory | KFile::LocalOnly);
    d->rootAlbumPathRequester->setUrl(picturesPath);

    QLabel *textLabel3 = new QLabel(widget);
    textLabel3->setWordWrap(true);
    textLabel3->setText(i18n("<p>digiKam stores information and meta-data about your images in a database file. "
                             "Please set the location of this file or accept the default.</p>"
                             "<p><i>Note:</i> You need to have write access to the folder used here, "
                             "and you cannot use a remote location on a networked server, using NFS or Samba.</p>"));

    d->dbPathRequester = new KUrlRequester(widget);
    d->dbPathRequester->setMode(KFile::Directory | KFile::LocalOnly);
    d->dbPathRequester->setUrl(picturesPath);

    grid->addWidget(pixLabel,                  0, 0, 2, 1);
    grid->addWidget(textLabel1,                0, 1, 1, 1);
    grid->addWidget(d->rootAlbumPathRequester, 1, 1, 1, 1);
    grid->addWidget(textLabel3,                2, 1, 1, 1);
    grid->addWidget(d->dbPathRequester,        3, 1, 1, 1);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    vlayout->addWidget(textLabel2);
    vlayout->addWidget(line1);
    vlayout->addLayout(grid);
    vlayout->setStretchFactor(grid, 10);
    vlayout->setMargin(0);
    vlayout->setSpacing(KDialog::spacingHint());

    widget->setMinimumSize(450, sizeHint().height());

    connect(d->rootAlbumPathRequester, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotAlbumRootChanged(const KUrl &)));
    connect(d->dbPathRequester, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotDbPathChanged(const KUrl &)));
}

DigikamFirstRun::~DigikamFirstRun()
{
    delete d;
}

QString DigikamFirstRun::firstAlbumPath() const
{
    return d->rootAlbum;
}

QString DigikamFirstRun::databasePath() const
{
    return d->dbPath;
}

void DigikamFirstRun::saveSettings(const QString& rootAlbumFolder, const QString& dbFolder)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("General Settings");
    group.writeEntry("Version", digikam_version);

    group = config->group("Album Settings");
    group.writeEntry("Database File Path", dbFolder);

    d->rootAlbum = rootAlbumFolder;
    d->dbPath    = dbFolder;

    config->sync();
}

bool DigikamFirstRun::checkRootAlbum(QString& rootAlbumFolder)
{
    rootAlbumFolder = d->rootAlbumPathRequester->url().toLocalFile();

    if (rootAlbumFolder.isEmpty())
    {
        KMessageBox::sorry(this, i18n("You must select a folder for digiKam to "
                                      "use as the root album. All of your images will go there."));
        return false;
    }

#ifndef _WIN32
    if (!QDir::isAbsolutePath(rootAlbumFolder))
    {
        rootAlbumFolder.prepend(QDir::homePath());
    }
#endif

    /*
    if (KUrl(rootAlbumFolder).equals(KUrl(QDir::homePath()), KUrl::CompareWithoutFragment))
    {
        KMessageBox::sorry(this, i18n("digiKam will not use your home folder as the "
                                      "root album. Please select another location."));
        return false;
    }
    */

    QDir targetPath(rootAlbumFolder);

    if (!targetPath.exists())
    {
        int rc = KMessageBox::questionYesNo(this,
                                   i18n("The folder to use as the root album path does not exist: "
                                        "<p><b>%1</b></p>"
                                        "Would you like digiKam to create it for you?",
                                        rootAlbumFolder),
                                   i18n("Create Root Album Folder?"));

        if (rc == KMessageBox::No)
        {
            return false;
        }

        if (!targetPath.mkdir(rootAlbumFolder))
        {
            KMessageBox::sorry(this,
                               i18n("digiKam could not create the folder to use as the root album.\n"
                                    "Please select a different location."
                                    "<p><b>%1</b></p>", rootAlbumFolder),
                               i18n("Create Root Album Folder Failed"));
            return false;
        }
    }

    QFileInfo path(rootAlbumFolder);

    if (!path.isWritable())
    {
        KMessageBox::information(this, i18n("You do not seem to have write access for the folder selected to be the root album.\n"
                                            "Warning: Without write access, the comment and tag features "
                                            "will not work."));
    }

    return true;
}

bool DigikamFirstRun::checkDatabase(QString& dbFolder)
{
    dbFolder = d->dbPathRequester->url().toLocalFile();

    if (dbFolder.isEmpty())
    {
        KMessageBox::sorry(this, i18n("You must select a folder for digiKam to "
                                      "store information and meta-data in a database file."));
        return false;
    }

#ifndef _WIN32
    if (!QDir::isAbsolutePath(dbFolder))
    {
        dbFolder.prepend(QDir::homePath());
    }
#endif

    /*
    if (KUrl(dbFolder).equals(KUrl(QDir::homePath()), KUrl::CompareWithoutFragment))
    {
        KMessageBox::sorry(this, i18n("digiKam cannot use your home folder as "
                                      "database file path."));
        return false;
    }
    */

    QDir targetPath(dbFolder);

    if (!targetPath.exists())
    {
        int rc = KMessageBox::questionYesNo(this,
                                   i18n("The folder to put your database in does not seem to exist: "
                                        "<p><b>%1</b></p>"
                                        "Would you like digiKam to create it for you?",
                                        dbFolder),
                                   i18n("Create Database Folder?"));

        if (rc == KMessageBox::No)
        {
            return false;
        }

        if (!targetPath.mkdir(dbFolder))
        {
            KMessageBox::sorry(this,
                               i18n("digiKam could not create the folder to host your database file.\n"
                                    "Please select a different location."
                                    "<p><b>%1</b></p>", dbFolder),
                               i18n("Create Database Folder Failed"));
            return false;
        }
    }

    QFileInfo path(dbFolder);

    if (!path.isWritable())
    {
        KMessageBox::information(this, i18n("<p>You do not seem to have write access for the folder to host the database file.<br/>"
                                            "Please select a different location.</p>"
                                            "<p><b>%1</b></p>", dbFolder),
                                 i18n("No Database Write Access"));
        return false;
    }

    return true;
}

void DigikamFirstRun::slotAlbumRootChanged(const KUrl &url)
{
    if (!d->dbPathEdited)
        d->dbPathRequester->setUrl(url);
}

void DigikamFirstRun::slotDbPathChanged(const KUrl &)
{
    d->dbPathEdited = true;
}

void DigikamFirstRun::slotButtonClicked( int button )
{
    if (button == KDialog::Ok)
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
    else
    {
        KDialog::slotButtonClicked(button);
    }
}

}  // namespace Digikam
