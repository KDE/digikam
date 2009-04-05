/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-16
 * Description : a dialog to select a target album to download
 *               pictures from camera
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#include "albumselectdialog.h"
#include "albumselectdialog.moc"

// Qt includes

#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QCursor>
#include <QGridLayout>
#include <QPixmap>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>

// Local includes

#include "treefolderitem.h"
#include "album.h"
#include "albummanager.h"
#include "albumselectwidget.h"
#include "albumthumbnailloader.h"
#include "collectionmanager.h"

namespace Digikam
{

class AlbumSelectDialogPrivate
{

public:

    AlbumSelectDialogPrivate()
    {
        allowRootSelection = false;
        albumSel           = 0;
        searchBar          = 0;
    }

    bool               allowRootSelection;

    QString            newAlbumString;

    AlbumSelectWidget *albumSel;

    SearchTextBar     *searchBar;
};

AlbumSelectDialog::AlbumSelectDialog(QWidget* parent, PAlbum* albumToSelect,
                                     const QString& header,
                                     const QString& newAlbumString,
                                     bool allowRootSelection)
                 : KDialog(parent), d(new AlbumSelectDialogPrivate)
{
    d->allowRootSelection = allowRootSelection;
    d->newAlbumString     = newAlbumString;

    setCaption(i18n("Select Album"));
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setHelp("targetalbumdialog.anchor", "digikam");
    enableButtonOk(false);

    // -------------------------------------------------------------

    QWidget *page = new QWidget(this);
    setMainWidget(page);

    QGridLayout* grid       = new QGridLayout(page);
    QLabel *logo            = new QLabel(page);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                            .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel *message = new QLabel(page);
    message->setWordWrap(true);
    if (!header.isEmpty())
        message->setText(header);

    d->albumSel = new AlbumSelectWidget(page, albumToSelect);

    grid->addWidget(logo,        0, 0, 1, 1);
    grid->addWidget(message,     1, 0, 1, 1);
    grid->addWidget(d->albumSel, 0, 1, 3, 1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    connect(d->albumSel->albumView(), SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    // -------------------------------------------------------------

    resize(500, 500);
    slotSelectionChanged();
}

AlbumSelectDialog::~AlbumSelectDialog()
{
    delete d;
}

void AlbumSelectDialog::slotSelectionChanged()
{
    QTreeWidgetItem* selItem = d->albumSel->albumView()->currentItem();

    if ((!selItem || (selItem == d->albumSel->albumView()->topLevelItem(0))) &&
        !d->allowRootSelection)
    {
        enableButtonOk(false);
        return;
    }

    enableButtonOk(true);
}

PAlbum* AlbumSelectDialog::selectAlbum(QWidget* parent,
                                       PAlbum* albumToSelect,
                                       const QString& header,
                                       const QString& newAlbumString,
                                       bool allowRootSelection )
{
    AlbumSelectDialog dlg(parent, albumToSelect,
                          header, newAlbumString,
                          allowRootSelection);

    if (dlg.exec() != KDialog::Accepted)
        return 0;

    TreeAlbumItem* item = (TreeAlbumItem*) dlg.d->albumSel->albumView()->currentItem();
    if ((!item || (item == dlg.d->albumSel->albumView()->topLevelItem(0))) &&
        !allowRootSelection)
    {
        return 0;
    }

    return (dynamic_cast<PAlbum*>(item->album()));
}

}  // namespace Digikam
