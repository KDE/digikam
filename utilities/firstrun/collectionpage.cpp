/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "collectionpage.moc"

// Qt includes

#include <QLabel>
#include <QDir>
#include <QDesktopServices>
#include <QFileInfo>
#include <QVBoxLayout>

// KDE includes

#include <kdialog.h>
#include <kvbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kurlrequester.h>
#include <kglobalsettings.h>
#include <kurl.h>

namespace Digikam
{

class CollectionPagePriv
{
public:

    CollectionPagePriv()
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

CollectionPage::CollectionPage(KAssistantDialog* dlg)
              : AssistantDlgPage(dlg, i18n("<b>Configure where images and meta-data are stored</b>")), 
                d(new CollectionPagePriv)
{
    QWidget *widget      = new QWidget(this);
    QVBoxLayout *vlayout = new QVBoxLayout(widget);

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

    vlayout->addWidget(textLabel1);
    vlayout->addWidget(d->rootAlbumPathRequester);
    vlayout->addWidget(textLabel3);
    vlayout->addWidget(d->dbPathRequester);
    vlayout->setMargin(0);
    vlayout->setSpacing(KDialog::spacingHint());

    widget->setMinimumSize(450, sizeHint().height());
    setContentsWidget(widget);

    connect(d->rootAlbumPathRequester, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotAlbumRootChanged(const KUrl &)));

    connect(d->dbPathRequester, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotDbPathChanged(const KUrl &)));
}

CollectionPage::~CollectionPage()
{
    delete d;
}

}   // namespace Digikam
