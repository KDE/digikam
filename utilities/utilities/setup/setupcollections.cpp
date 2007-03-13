/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date  : 2004-01-02
 * Description : collection setup tab.
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qlistbox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(3,2,0)
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

// // Local includes.

#include "thumbnailsize.h"
#include "albumsettings.h"
#include "setupcollections.h"

namespace Digikam
{

class SetupCollectionsPriv
{
public:

    SetupCollectionsPriv()
    {
        albumCollectionBox  = 0;
        addCollectionButton = 0;
        delCollectionButton = 0;
    }
   
    QListBox     *albumCollectionBox;

    QPushButton  *addCollectionButton;
    QPushButton  *delCollectionButton;
};

SetupCollections::SetupCollections(QWidget* parent )
                : QWidget(parent)
{
    d = new SetupCollectionsPriv;
    QVBoxLayout *mainLayout = new QVBoxLayout(parent);

    QGridLayout *collectionGroupLayout = new QGridLayout( this, 2, 5, 0, KDialog::spacingHint() );
    collectionGroupLayout->setAlignment( Qt::AlignTop );

   // --------------------------------------------------------

   d->albumCollectionBox = new QListBox(this);
   QWhatsThis::add( d->albumCollectionBox, i18n("<p>You can add or remove Album "
                                              "collection types here to improve how "
                                              "your Albums are sorted in digiKam."));

   d->albumCollectionBox->setVScrollBarMode(QScrollView::AlwaysOn);

   collectionGroupLayout->addMultiCellWidget( d->albumCollectionBox,
                                              0, 4, 0, 0 );

   d->addCollectionButton = new QPushButton( i18n("&Add..."), this);
   collectionGroupLayout->addWidget( d->addCollectionButton, 0, 1);

   d->delCollectionButton = new QPushButton( i18n("&Delete"), this);
   collectionGroupLayout->addWidget( d->delCollectionButton, 1, 1);
   d->delCollectionButton->setEnabled(false);

   connect(d->albumCollectionBox, SIGNAL(selectionChanged()),
           this, SLOT(slotCollectionSelectionChanged()));

   connect(d->addCollectionButton, SIGNAL(clicked()),
           this, SLOT(slotAddCollection()));

   connect(d->delCollectionButton, SIGNAL(clicked()),
           this, SLOT(slotDelCollection()));

   QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum,
                                          QSizePolicy::Expanding );
   collectionGroupLayout->addItem( spacer, 4, 1 );

   // --------------------------------------------------------

   readSettings();
   adjustSize();
   mainLayout->addWidget(this);
}

SetupCollections::~SetupCollections()
{
    delete d;
}

void SetupCollections::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    QStringList collectionList;

    for (QListBoxItem *item = d->albumCollectionBox->firstItem();
         item; item = item->next())
    {
        collectionList.append(item->text());
    }

    settings->setAlbumCollectionNames(collectionList);

    settings->saveSettings();
}

void SetupCollections::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->albumCollectionBox->insertStringList(settings->getAlbumCollectionNames());
}

void SetupCollections::slotCollectionSelectionChanged()
{
    if (d->albumCollectionBox->currentItem() != -1)
        d->delCollectionButton->setEnabled(true);
    else
        d->delCollectionButton->setEnabled(false);
}

void SetupCollections::slotAddCollection()
{
    bool ok;

#if KDE_IS_VERSION(3,2,0)
    QString newCollection =
        KInputDialog::getText(i18n("New Collection Name"),
                              i18n("Enter new collection name:"),
                              QString(), &ok, this);
#else
    QString newCollection =
        KLineEditDlg::getText(i18n("New Collection Name"),
                              i18n("Enter new collection name:"),
                              QString(), &ok, this);
#endif

    if (!ok) return;

    bool found = false;
    for (QListBoxItem *item = d->albumCollectionBox->firstItem();
         item; item = item->next()) 
    {
        if (newCollection == item->text()) 
        {
            found = true;
            break;
        }
    }

    if (!found)
        d->albumCollectionBox->insertItem(newCollection);
}

void SetupCollections::slotDelCollection()
{
    int index = d->albumCollectionBox->currentItem();
    if (index == -1)
        return;

    QListBoxItem* item = d->albumCollectionBox->item(index);
    if (!item) return;
    delete item;
}

}  // namespace Digikam

#include "setupcollections.moc"
