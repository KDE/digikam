/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "collectionpage.h"

// Qt includes

#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QLabel>
#include <QDir>
#include <QUrl>
#include <QStandardPaths>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QTemporaryFile>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dwidgetutils.h"
#include "digikam_debug.h"
#include "databaseparameters.h"
#include "digikam_version.h"

namespace Digikam
{

class CollectionPage::Private
{
public:

    Private() :
        dbPathEdited(false),
        rootAlbumPathRequester(0),
        dbPathRequester(0)
    {
    }

    bool           dbPathEdited;

    QString        rootAlbum;
    QString        dbPath;

    DFileSelector* rootAlbumPathRequester;
    DFileSelector* dbPathRequester;
};

CollectionPage::CollectionPage(AssistantDlg* const dlg)
    : AssistantDlgPage(dlg, i18n("<b>Configure where you keep your images and you will store database</b>")),
      d(new Private)
{
    QWidget* const widget      = new QWidget(this);
    QVBoxLayout* const vlayout = new QVBoxLayout(widget);

    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    qCDebug(DIGIKAM_GENERAL_LOG) << picturesPath;

    if (picturesPath.isEmpty())
    {
        picturesPath = QDir::homePath() + i18nc("This is a path name so you should "
                                                "include the slash in the translation", "/Pictures");
    }

    QLabel* const textLabel1 = new QLabel(widget);
    textLabel1->setWordWrap(true);

    QString message = i18n("<p>Please enter a location where you keep your images.</p> "
                           "<p>You can choose any local folder, even one that already contains images."
                           "<br/> "
                           "More folders can be added later under the <i>Settings</i> menu. "
                           "</p> ");

#ifndef _WIN32
    message.append(i18n("<p><i>Note:</i> removable media (such as USB drives or DVDs) and remote file systems "
                        "(such as NFS, or Samba mounted with cifs/smbfs) are supported.</p>"));
#endif

    textLabel1->setText(message);

    d->rootAlbumPathRequester = new DFileSelector(widget);
    d->rootAlbumPathRequester->setFileDlgMode(QFileDialog::Directory);
    d->rootAlbumPathRequester->setFileDlgOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog);
    d->rootAlbumPathRequester->lineEdit()->setText(picturesPath);

    QLabel* const textLabel3 = new QLabel(widget);
    textLabel3->setWordWrap(true);
    textLabel3->setText(i18n("<p>digiKam stores information and metadata about your images in a database file. "
                             "Please set the location of this file or accept the default.</p>"
                             "<p><i>Note:</i> you need to have write access to the folder used here, "
                             "and you cannot use a remote location on a networked server, "
                             "using NFS or Samba.</p>"));

    d->dbPathRequester = new DFileSelector(widget);
    d->dbPathRequester->setFileDlgMode(QFileDialog::Directory);
    d->dbPathRequester->setFileDlgOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog);
    d->dbPathRequester->lineEdit()->setText(picturesPath);

    vlayout->addWidget(textLabel1);
    vlayout->addWidget(d->rootAlbumPathRequester);
    vlayout->addWidget(textLabel3);
    vlayout->addWidget(d->dbPathRequester);
    vlayout->setMargin(0);
    vlayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    setPageWidget(widget);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("server-database")));

    connect(d->rootAlbumPathRequester, SIGNAL(signalUrlSelected(QUrl)),
            this, SLOT(slotAlbumRootChanged(QUrl)));

    connect(d->dbPathRequester, SIGNAL(signalUrlSelected(QUrl)),
            this, SLOT(slotDbPathChanged(QUrl)));
}

CollectionPage::~CollectionPage()
{
    delete d;
}

QString CollectionPage::firstAlbumPath() const
{
    return d->rootAlbum;
}

QString CollectionPage::databasePath() const
{
    return d->dbPath;
}

void CollectionPage::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("General Settings");
    group.writeEntry("Version", digikam_version);

    DatabaseParameters params = DatabaseParameters::parametersForSQLiteDefaultFile(d->dbPath);
    params.writeToConfig(config);

    config->sync();
}

bool CollectionPage::checkSettings()
{
    QString rootAlbumFolder;

    if (!checkRootAlbum(rootAlbumFolder))
    {
        return false;
    }

    QString dbFolder;

    if (!checkDatabase(dbFolder))
    {
        return false;
    }

    d->rootAlbum = rootAlbumFolder;
    d->dbPath    = dbFolder;

    return true;
}

bool CollectionPage::checkRootAlbum(QString& rootAlbumFolder)
{
    rootAlbumFolder = d->rootAlbumPathRequester->lineEdit()->text();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Root album is : " << rootAlbumFolder;

    if (rootAlbumFolder.isEmpty())
    {
        QMessageBox::information(this, qApp->applicationName(), 
                                 i18n("You must select a folder for digiKam to "
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
    if (QUrl::fromLocalFile(rootAlbumFolder).equals(QUrl::fromLocalFile(QDir::homePath()), QUrl::CompareWithoutFragment))
    {
        QMessageBox::information(this, qApp->applicationName(), 
                                 i18n("digiKam will not use your home folder as the "
                                      "root album. Please select another location."));
        return false;
    }
*/

    QDir targetPath(rootAlbumFolder);

    if (!targetPath.exists())
    {
        int rc = QMessageBox::question(this, i18n("Create Root Album Folder?"),
                                       i18n("<p>The folder to use as the root album path does not exist:</p>"
                                                 "<p><b>%1</b></p>"
                                                 "Would you like digiKam to create it for you?",
                                                 rootAlbumFolder));

        if (rc == QMessageBox::No)
        {
            return false;
        }

        if (!targetPath.mkpath(rootAlbumFolder))
        {
            QMessageBox::information(this, i18n("Create Root Album Folder Failed"),
                                     i18n("<p>digiKam could not create the folder to use as the root album.\n"
                                          "Please select a different location.</p>"
                                          "<p><b>%1</b></p>", rootAlbumFolder));
            return false;
        }
    }

    QFileInfo path(rootAlbumFolder);

#ifdef _WIN32
    // Work around bug #189168
    QTemporaryFile temp;
    temp.setFileTemplate(rootAlbumFolder + QLatin1String("XXXXXX"));

    if (!temp.open())
#else
    if (!path.isWritable())
#endif
    {
        QMessageBox::information(this, qApp->applicationName(), 
                                 i18n("You do not seem to have write access for the folder "
                                      "selected to be the root album.\n"
                                      "Warning: Without write access, items cannot be edited."));
    }

    return true;
}

bool CollectionPage::checkDatabase(QString& dbFolder)
{
    dbFolder = d->dbPathRequester->lineEdit()->text();
    qCDebug(DIGIKAM_GENERAL_LOG) << "DB folder is : " << dbFolder;

    if (dbFolder.isEmpty())
    {
        QMessageBox::information(this, qApp->applicationName(), 
                                 i18n("You must select a folder for digiKam to "
                                      "store information and metadata in a database file."));
        return false;
    }

#ifndef _WIN32

    if (!QDir::isAbsolutePath(dbFolder))
    {
        dbFolder.prepend(QDir::homePath());
    }

#endif

/*
    if (QUrl::fromLocalFile(dbFolder).equals(QUrl::fromLocalFile(QDir::homePath()), QUrl::CompareWithoutFragment))
    {
        QMessageBox::information(this, qApp->applicationName(), 
                                 i18n("digiKam cannot use your home folder as "
                                      "database file path."));
        return false;
    }
*/

    QDir targetPath(dbFolder);

    if (!targetPath.exists())
    {
        int rc = QMessageBox::question(this, i18n("Create Database Folder?"),
                                            i18n("<p>The folder to put your database in does not seem to exist:</p>"
                                                 "<p><b>%1</b></p>"
                                                 "Would you like digiKam to create it for you?", dbFolder));

        if (rc == QMessageBox::No)
        {
            return false;
        }

        if (!targetPath.mkpath(dbFolder))
        {
            QMessageBox::information(this, i18n("Create Database Folder Failed"),
                                     i18n("<p>digiKam could not create the folder to host your database file.\n"
                                          "Please select a different location.</p>"
                                          "<p><b>%1</b></p>", dbFolder));
            return false;
        }
    }

    QFileInfo path(dbFolder);

#ifdef _WIN32
    // Work around bug #189168
    QTemporaryFile temp;
    temp.setFileTemplate(dbFolder + QLatin1String("XXXXXX"));

    if (!temp.open())
#else
    if (!path.isWritable())
#endif
    {
        QMessageBox::information(this, i18n("No Database Write Access"),
                                 i18n("<p>You do not seem to have write access "
                                      "for the folder to host the database file.<br/>"
                                      "Please select a different location.</p>"
                                      "<p><b>%1</b></p>", dbFolder));
        return false;
    }

    return true;
}

void CollectionPage::slotAlbumRootChanged(const QUrl& url)
{
    if (!d->dbPathEdited)
    {
        d->dbPathRequester->lineEdit()->setText(url.toLocalFile());
    }
}

void CollectionPage::slotDbPathChanged(const QUrl&)
{
    d->dbPathEdited = true;
}

}   // namespace Digikam
