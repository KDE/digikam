//////////////////////////////////////////////////////////////////////////////
//
//    SETUPCOLLECTIONS.CPP
//
//    Copyright (C) 2004 Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

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


SetupCollections::SetupCollections(QWidget* parent )
            : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(parent);

    QGridLayout *collectionGroupLayout = new QGridLayout( this, 2, 5, 0, KDialog::spacingHint() );
    collectionGroupLayout->setAlignment( Qt::AlignTop );

   // --------------------------------------------------------

   albumCollectionBox_ = new QListBox(this);
   QWhatsThis::add( albumCollectionBox_, i18n("<p>You can add or remove Album "
                                              "collection types here to improve how "
                                              "your Albums are sorted in digiKam."));

   albumCollectionBox_->setVScrollBarMode(QScrollView::AlwaysOn);

   collectionGroupLayout->addMultiCellWidget( albumCollectionBox_,
                                              0, 4, 0, 0 );

   addCollectionButton_ = new QPushButton( i18n("&Add..."), this);
   collectionGroupLayout->addWidget( addCollectionButton_, 0, 1);

   delCollectionButton_ = new QPushButton( i18n("&Delete"), this);
   collectionGroupLayout->addWidget( delCollectionButton_, 1, 1);
   delCollectionButton_->setEnabled(false);

   connect(albumCollectionBox_, SIGNAL(selectionChanged()),
           this, SLOT(slotCollectionSelectionChanged()));

   connect(addCollectionButton_, SIGNAL(clicked()),
           this, SLOT(slotAddCollection()));

   connect(delCollectionButton_, SIGNAL(clicked()),
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
}

void SetupCollections::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    QStringList collectionList;

    for (QListBoxItem *item = albumCollectionBox_->firstItem();
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

    albumCollectionBox_->insertStringList(settings->getAlbumCollectionNames());
}

void SetupCollections::slotCollectionSelectionChanged()
{
    if (albumCollectionBox_->currentItem() != -1)
        delCollectionButton_->setEnabled(true);
    else
        delCollectionButton_->setEnabled(false);
}

void SetupCollections::slotAddCollection()
{
    bool ok;

#if KDE_IS_VERSION(3,2,0)
    QString newCollection =
        KInputDialog::getText(i18n("New Collection Name"),
                              i18n("Enter new collection name:"),
                              QString::null, &ok, this);
#else
    QString newCollection =
        KLineEditDlg::getText(i18n("New Collection Name"),
                              i18n("Enter new collection name:"),
                              QString::null, &ok, this);
#endif

    if (!ok) return;

    bool found = false;
    for (QListBoxItem *item = albumCollectionBox_->firstItem();
         item; item = item->next()) {
        if (newCollection == item->text()) {
            found = true;
            break;
        }
    }

    if (!found)
        albumCollectionBox_->insertItem(newCollection);
}

void SetupCollections::slotDelCollection()
{
    int index = albumCollectionBox_->currentItem();
    if (index == -1)
        return;

    QListBoxItem* item = albumCollectionBox_->item(index);
    if (!item) return;
    delete item;
}

#include "setupcollections.moc"
