/* ============================================================
 * File  : albumpropsedit.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-03-09
 * Description :
 *
 * Copyright 2003 by Renchi Raju
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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
 
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qheader.h>

// KDE includes.

#include <klocale.h>
#include <kurl.h>
#include <kdatepicker.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(3,2,0)
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

// Local includes.

#include "album.h"
#include "albumsettings.h"
#include "albumpropsedit.h"

AlbumPropsEdit::AlbumPropsEdit(PAlbum* album)
              : KDialogBase( Plain, i18n("Edit Album"), Help|Ok|Cancel, Ok,
                             0, 0, true, true )
{
    setHelp("albumpropsedit.anchor", "digikam");
    album_ = album;
    
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint() );

    QLabel *topLabel = new QLabel( plainPage() );
    topLabel->setText( i18n( "Edit '%1' Album Properties").arg(album->getTitle()));
    topLayout->addWidget( topLabel  );

    // --------------------------------------------------------

    QFrame *topLine = new QFrame( plainPage() );
    topLine->setFrameShape( QFrame::HLine );
    topLine->setFrameShadow( QFrame::Sunken );
    topLayout->addWidget( topLine );

    // --------------------------------------------------------
    
    QGroupBox *titleBox = new QGroupBox( plainPage() );
    titleBox->setTitle( i18n( "Edit Album Description" ) );
    titleBox->setColumnLayout( 0, Qt::Horizontal );
    QGridLayout *titleBoxLayout =
        new QGridLayout( titleBox->layout(), 2, 2, spacingHint() );
                                                   
    QLabel *titleLabel = new QLabel( titleBox );
    titleLabel->setText( i18n( "Title: " ) );
    titleBoxLayout->addWidget( titleLabel, 0, 0 );

    titleEdit_ = new QLineEdit( titleBox );
    titleBoxLayout->addWidget( titleEdit_, 0, 1 );

    QLabel *commentsLabel = new QLabel( titleBox );
    commentsLabel->setText( i18n( "Comments: " ) );
    titleBoxLayout->addWidget( commentsLabel, 2, 0 );

    commentsEdit_ = new QLineEdit( titleBox );
    titleBoxLayout->addWidget( commentsEdit_, 2, 1 );
    
    topLayout->addWidget( titleBox );

    // ------------------------------------------------------

    QGroupBox *dateBox = new QGroupBox( plainPage() );
    dateBox->setTitle( i18n( "Change Album Date" ) );
    dateBox->setColumnLayout( 0, Qt::Horizontal );
    QVBoxLayout *dateBoxLayout =
        new QVBoxLayout( dateBox->layout(), spacingHint() );

    datePicker_ = new KDatePicker( dateBox );
    dateBoxLayout->addWidget( datePicker_ );

    topLayout->addWidget( dateBox );

    // ------------------------------------------------------

    QGroupBox *collectionBox = new QGroupBox( plainPage() );
    collectionBox->setTitle( i18n( "Change Album Collection" ) );
    collectionBox->setColumnLayout(0, Qt::Horizontal );
    QGridLayout *collectionBoxLayout =
        new QGridLayout( collectionBox->layout(), 2, 2, spacingHint() );

    collectionEdit_ = new QListView( collectionBox );
    collectionEdit_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                               QSizePolicy::MinimumExpanding));
    collectionBoxLayout->addMultiCellWidget( collectionEdit_,
                                             0, 2, 0, 0 );

    QPushButton *addCollectionBtn = new QPushButton( i18n("Add"),
                                                     collectionBox );
    collectionBoxLayout->addWidget( addCollectionBtn, 0, 1);
    
    QPushButton *delCollectionBtn = new QPushButton( i18n("Delete"),
                                                     collectionBox );
    collectionBoxLayout->addWidget( delCollectionBtn, 1, 1);

    topLayout->addWidget( collectionBox );
        
    // Initialize ---------------------------------------------

    populateCollections();

    titleEdit_->setText( album->getTitle() );
    commentsEdit_->setText( album->getCaption() );
    datePicker_->setDate( album->getDate() );

    QCheckListItem *checkItem =
        (QCheckListItem*) collectionEdit_->findItem(album->getCollection(), 0);
    if (checkItem) checkItem->setOn(true);

    // Connections -------------------------------------------

    connect(addCollectionBtn, SIGNAL(clicked()),
            this, SLOT(slotAddCollection()));

    connect(delCollectionBtn, SIGNAL(clicked()),
            this, SLOT(slotDelCollection()));
    
    //resize(500, 400);
    adjustSize();
}

AlbumPropsEdit::~AlbumPropsEdit()
{
}

void AlbumPropsEdit::populateCollections()
{
    AlbumSettings *settings = AlbumSettings::instance();
    if (!settings) return;

    collectionEdit_->addColumn("Collections");
    collectionEdit_->header()->hide();
    collectionEdit_->setColumnWidthMode(0, QListView::Maximum);

    rootCollectionItem_ =
        new QCheckListItem(collectionEdit_, i18n("Collections"));
    rootCollectionItem_->setSelectable(false);
    rootCollectionItem_->setOpen(true);
    
    albumCollections_ = settings->getAlbumCollectionNames();
    for (QStringList::const_iterator it = albumCollections_.begin();
         it != albumCollections_.end(); ++it ) {
        new QCheckListItem(rootCollectionItem_, *it,
                           QCheckListItem::RadioButton);
    }
    
}

QString AlbumPropsEdit::title() const
{
    return titleEdit_->text();    
}

QString AlbumPropsEdit::comments() const
{
    return commentsEdit_->text();
}

QDate AlbumPropsEdit::date() const
{
    return datePicker_->date();
}

QString AlbumPropsEdit::collection() const
{
    QString name;

    QListViewItemIterator it(collectionEdit_);
    for ( ; it.current(); ++it) {
        QCheckListItem *item =
            (QCheckListItem*)(it.current());
        if (item->type() == QCheckListItem::RadioButton &&
            item->isOn()) {
            name = item->text();
        }
    }

    if (name.isNull()) name = i18n( "Unknown" );
    
    return name;
}

QStringList AlbumPropsEdit::albumCollections() const
{
    return albumCollections_;    
}

void AlbumPropsEdit::slotAddCollection()
{
    bool ok;

#if KDE_IS_VERSION(3,2,0)
    QString newCollection = KInputDialog::getText(i18n("New Collection Name"),
                                                  i18n("Enter New Collection Name: "),
                                                  QString::null, &ok, this);
#else
    QString newCollection = KLineEditDlg::getText(i18n("New Collection Name"),
                                                  i18n("Enter New Collection Name: "),
                                                  QString::null, &ok, this);
#endif
    if (!ok) return;

    if (!albumCollections_.contains(newCollection)) {
        new QCheckListItem(rootCollectionItem_, newCollection,
                           QCheckListItem::RadioButton);
        albumCollections_.append(newCollection);
    }
}

void AlbumPropsEdit::slotDelCollection()
{
    QListViewItem *item = collectionEdit_->selectedItem();
    if (!item) return;    

    QCheckListItem *checkItem = (QCheckListItem*) item;
    if (!checkItem || checkItem == rootCollectionItem_)
        return;

    albumCollections_.remove(checkItem->text(0));
    delete checkItem;
}

bool AlbumPropsEdit::editProps(PAlbum *album, QString& title,
                               QString& comments, QDate& date, QString& collection,
                               QStringList& albumCollections)
{
    AlbumPropsEdit dlg(album);

    bool ok = dlg.exec() == QDialog::Accepted;

    title            = dlg.title();
    comments         = dlg.comments();
    date             = dlg.date();
    collection       = dlg.collection();
    albumCollections = dlg.albumCollections();

    return ok;
}

#include "albumpropsedit.moc"
