/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : album type setup tab.
 * 
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDir>
#include <QGridLayout>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kpagedialog.h>
#include <kurl.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <klistwidget.h>

// // Local includes.

#include "thumbnailsize.h"
#include "albumsettings.h"
#include "setupalbumtype.h"
#include "setupalbumtype.moc"

namespace Digikam
{

class SetupAlbumTypePriv
{
public:

    SetupAlbumTypePriv()
    {
        collectionEdit      = 0;
        albumCollectionBox  = 0;
        addCollectionButton = 0;
        delCollectionButton = 0;
        repCollectionButton = 0;
    }

    QPushButton *addCollectionButton;
    QPushButton *delCollectionButton;
    QPushButton *repCollectionButton;

    KListWidget *albumCollectionBox;

    KLineEdit   *collectionEdit;
};

SetupAlbumType::SetupAlbumType(QWidget* parent )
              : QWidget(parent)
{
    d = new SetupAlbumTypePriv;

    QGridLayout *grid = new QGridLayout(this);

    // --------------------------------------------------------
    
    d->collectionEdit = new KLineEdit(this);
    d->collectionEdit->setClearButtonShown(true);

    d->albumCollectionBox = new KListWidget(this);
    d->albumCollectionBox->setWhatsThis(i18n("<p>You can add or remove Album "
                                             "collection types here to improve how "
                                             "your Albums are sorted in digiKam."));
    
    d->albumCollectionBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    
    d->addCollectionButton = new QPushButton(i18n("&Add..."), this);
    d->delCollectionButton = new QPushButton(i18n("&Delete"), this);
    d->repCollectionButton = new QPushButton(i18n("&Replace"), this);

    d->addCollectionButton->setIcon(SmallIcon("edit-add"));
    d->delCollectionButton->setIcon(SmallIcon("edit-delete"));
    d->repCollectionButton->setIcon(SmallIcon("view-refresh"));
    d->delCollectionButton->setEnabled(false);
    d->repCollectionButton->setEnabled(false);

    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(d->collectionEdit, 0, 0, 1, 1);
    grid->addWidget(d->albumCollectionBox, 1, 0, 5, 1);
    grid->addWidget(d->addCollectionButton, 1, 1, 1, 1);
    grid->addWidget(d->delCollectionButton, 2, 1, 1, 1);
    grid->addWidget(d->repCollectionButton, 3, 1, 1, 1);
    grid->setRowStretch(4, 10);      
    grid->setColumnStretch(0, 10);       
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->albumCollectionBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotCollectionSelectionChanged()));
    
    connect(d->addCollectionButton, SIGNAL(clicked()),
            this, SLOT(slotAddCollection()));
    
    connect(d->delCollectionButton, SIGNAL(clicked()),
            this, SLOT(slotDelCollection()));
    
    connect(d->repCollectionButton, SIGNAL(clicked()),
            this, SLOT(slotRepCollection()));

    // --------------------------------------------------------
    
    readSettings();
    adjustSize();
}

SetupAlbumType::~SetupAlbumType()
{
    delete d;
}

void SetupAlbumType::slotDelCollection()
{
    QListWidgetItem *item = d->albumCollectionBox->currentItem();
    if (!item) return;
    d->albumCollectionBox->takeItem(d->albumCollectionBox->row(item));
    delete item;
}

void SetupAlbumType::slotRepCollection()
{
    QString newCollection = d->collectionEdit->text();
    if (newCollection.isEmpty()) return;

    if (!d->albumCollectionBox->selectedItems().isEmpty())
    {
        d->albumCollectionBox->selectedItems()[0]->setText(newCollection);
        d->collectionEdit->clear();
    }
}

void SetupAlbumType::slotCollectionSelectionChanged()
{
    if (!d->albumCollectionBox->selectedItems().isEmpty())
    {
        d->collectionEdit->setText(d->albumCollectionBox->selectedItems()[0]->text());
        d->delCollectionButton->setEnabled(true);
        d->repCollectionButton->setEnabled(true);
    }
    else
    {
        d->delCollectionButton->setEnabled(false);
        d->repCollectionButton->setEnabled(false);
    }
}

void SetupAlbumType::slotAddCollection()
{
    QString newCollection = d->collectionEdit->text();
    if (newCollection.isEmpty()) return;

    bool found = false;
    for (int i = 0 ; i < d->albumCollectionBox->count(); i++)
    {
        QListWidgetItem *item = d->albumCollectionBox->item(i);
        if (newCollection == item->text()) 
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->albumCollectionBox->insertItem(d->albumCollectionBox->count(), newCollection);
        d->collectionEdit->clear();
    }
}

void SetupAlbumType::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    QStringList collectionList;

    for (int i = 0 ; i < d->albumCollectionBox->count(); i++)
    {
        QListWidgetItem *item = d->albumCollectionBox->item(i);
        collectionList.append(item->text());
    }

    settings->setAlbumCollectionNames(collectionList);
    settings->saveSettings();
}

void SetupAlbumType::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->albumCollectionBox->insertItems(0, settings->getAlbumCollectionNames());
}

}  // namespace Digikam
