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

#include <klocale.h>
#include <kurl.h>
#include <klineeditdlg.h>

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qdatetimeedit.h>
#include <qlistview.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qheader.h>

#include <interfaces/albuminfo.h>

#include "albumsettings.h"
#include "albumpropsedit.h"

AlbumPropsEdit::AlbumPropsEdit(const Digikam::AlbumInfo* albumInfo)
    : KDialogBase( Plain, QString::null, Ok|Cancel, Ok,
                   0, 0, true, true )
{
    albumInfo_ = albumInfo;
    
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint() );

    QLabel *topLabel = new QLabel( plainPage() );
    topLabel->setText( i18n( "Edit '%1' Album Properties").arg(albumInfo->getTitle()));
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
        new QGridLayout( titleBox->layout(), spacingHint() );
                                                   
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

    dateEdit_ = new QDateEdit( dateBox );
    dateBoxLayout->addWidget( dateEdit_ );

    topLayout->addWidget( dateBox );

    // ------------------------------------------------------

    QGroupBox *collectionBox = new QGroupBox( plainPage() );
    collectionBox->setTitle( i18n( "Change Album Collection" ) );
    collectionBox->setColumnLayout(0, Qt::Horizontal );
    QGridLayout *collectionBoxLayout =
        new QGridLayout( collectionBox->layout(), spacingHint() );

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

    titleEdit_->setText( albumInfo->getTitle() );
    commentsEdit_->setText( albumInfo->getComments() );
    dateEdit_->setDate( albumInfo->getDate() );

    QCheckListItem *checkItem =
        (QCheckListItem*) collectionEdit_->findItem(albumInfo->getCollection(), 0);
    if (checkItem) checkItem->setOn(true);

    // Connections -------------------------------------------

    connect(addCollectionBtn, SIGNAL(clicked()),
            this, SLOT(slotAddCollection()));

    connect(delCollectionBtn, SIGNAL(clicked()),
            this, SLOT(slotDelCollection()));
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
    return dateEdit_->date();
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
    QString newCollection = KLineEditDlg::getText(i18n("Enter New Collection Name: "), "", &ok, this);
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

bool AlbumPropsEdit::editProps(const Digikam::AlbumInfo *albumInfo, QString& title,
                               QString& comments, QDate& date, QString& collection,
                               QStringList& albumCollections)
{
    AlbumPropsEdit dlg(albumInfo);

    bool ok = dlg.exec() == QDialog::Accepted;

    title            = dlg.title();
    comments         = dlg.comments();
    date             = dlg.date();
    collection       = dlg.collection();
    albumCollections = dlg.albumCollections();

    return ok;
}

