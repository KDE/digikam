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

#include "databasepage.h"

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
#include "dbengineparameters.h"

namespace Digikam
{

class DatabasePage::Private
{
public:

    Private() :
        dbPathEdited(false),
        dbPathRequester(0)
    {
    }

    bool           dbPathEdited;

    QString        dbPath;

    DFileSelector* dbPathRequester;
};

DatabasePage::DatabasePage(AssistantDlg* const dlg)
    : AssistantDlgPage(dlg, i18n("<b>Configure where you will store databases</b>")),
      d(new Private)
{
    QWidget* const widget      = new QWidget(this);
    QVBoxLayout* const vlayout = new QVBoxLayout(widget);

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

    vlayout->addWidget(textLabel3);
    vlayout->addWidget(d->dbPathRequester);
    vlayout->setContentsMargins(QMargins());
    vlayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    setPageWidget(widget);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("server-database")));

    connect(d->dbPathRequester, SIGNAL(signalUrlSelected(QUrl)),
            this, SLOT(slotDbPathChanged(QUrl)));
}

DatabasePage::~DatabasePage()
{
    delete d;
}

void DatabasePage::setDatabasePath(const QString& path)
{
    d->dbPath = path;
    d->dbPathRequester->lineEdit()->setText(d->dbPath);
}

QString DatabasePage::databasePath() const
{
    return d->dbPath;
}

void DatabasePage::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("General Settings");

    DbEngineParameters params = DbEngineParameters::parametersForSQLiteDefaultFile(d->dbPath);
    params.writeToConfig(config);

    config->sync();
}

bool DatabasePage::checkSettings()
{
    QString dbFolder;

    if (!checkDatabase(dbFolder))
    {
        return false;
    }

    d->dbPath    = dbFolder;

    return true;
}

bool DatabasePage::checkDatabase(QString& dbFolder)
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

void DatabasePage::slotDbPathChanged(const QUrl&)
{
    d->dbPathEdited = true;
}

}   // namespace Digikam
