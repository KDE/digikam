/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dfileselector.h"
#include "digikam_debug.h"
#include "digikam_version.h"

namespace Digikam
{

class CollectionPage::Private
{
public:

    explicit Private()
      : rootAlbumPathRequester(0)
    {
    }

    QString        rootAlbum;

    DFileSelector* rootAlbumPathRequester;
};

CollectionPage::CollectionPage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Configure where you keep your images</b>")),
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

#ifndef Q_OS_WIN
    message.append(i18n("<p><i>Note:</i> removable media (such as USB drives or DVDs) and remote file systems "
                        "(such as NFS, or Samba mounted with cifs/smbfs) are supported.</p>"));
#endif

    textLabel1->setText(message);

    d->rootAlbumPathRequester = new DFileSelector(widget);
    d->rootAlbumPathRequester->setFileDlgMode(DFileDialog::Directory);
    d->rootAlbumPathRequester->setFileDlgOptions(DFileDialog::ShowDirsOnly);
    d->rootAlbumPathRequester->setFileDlgPath(picturesPath);

    vlayout->addWidget(textLabel1);
    vlayout->addWidget(d->rootAlbumPathRequester);
    vlayout->setContentsMargins(QMargins());
    vlayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    setPageWidget(widget);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-pictures")));
}

CollectionPage::~CollectionPage()
{
    delete d;
}

QString CollectionPage::firstAlbumPath() const
{
    return d->rootAlbum;
}

void CollectionPage::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("General Settings");
    group.writeEntry("Version", digikam_version);

    config->sync();
}

bool CollectionPage::checkSettings()
{
    QString rootAlbumFolder;

    if (!checkRootAlbum(rootAlbumFolder))
    {
        return false;
    }

    d->rootAlbum = rootAlbumFolder;

    return true;
}

bool CollectionPage::checkRootAlbum(QString& rootAlbumFolder)
{
    rootAlbumFolder = d->rootAlbumPathRequester->fileDlgPath();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Root album is : " << rootAlbumFolder;

    if (rootAlbumFolder.isEmpty())
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18n("You must select a folder for digiKam to "
                                      "use as the root album. All of your images will go there."));
        return false;
    }

#ifndef Q_OS_WIN

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
                                                 QDir::toNativeSeparators(rootAlbumFolder)));

        if (rc == QMessageBox::No)
        {
            return false;
        }

        if (!targetPath.mkpath(rootAlbumFolder))
        {
            QMessageBox::information(this, i18n("Create Root Album Folder Failed"),
                                     i18n("<p>digiKam could not create the folder to use as the root album.\n"
                                          "Please select a different location.</p>"
                                          "<p><b>%1</b></p>", QDir::toNativeSeparators(rootAlbumFolder)));
            return false;
        }
    }

    QFileInfo path(rootAlbumFolder);

#ifdef Q_OS_WIN
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

} // namespace Digikam
