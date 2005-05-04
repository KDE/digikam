//////////////////////////////////////////////////////////////////////////////
//
//    SETUPGENERAL.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
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

// Qt includes.

#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kmessagebox.h>

// Local includes.

#include "setupcamera.h"
#include "cameraselection.h"
#include "cameralist.h"
#include "cameratype.h"
#include "gpiface.h"


SetupCamera::SetupCamera( QWidget* parent )
           : QWidget( parent )
{
    QVBoxLayout *mainLayout = new QVBoxLayout(parent);

    QGridLayout* groupBoxLayout = new QGridLayout( this, 2, 5, 0, KDialog::spacingHint() );
    groupBoxLayout->setAlignment( Qt::AlignTop );

    listView_ = new QListView( this );
    listView_->addColumn( i18n("Title") );
    listView_->addColumn( i18n("Model") );
    listView_->addColumn( i18n("Port") );
    listView_->addColumn( i18n("Path") );
    listView_->setAllColumnsShowFocus(true);
    groupBoxLayout->addMultiCellWidget( listView_, 0, 4, 0, 0 );
    QWhatsThis::add( listView_, i18n("<p>You can see here the digital camera list used by digiKam "
                                     "via the Gphoto interface."));

    addButton_ = new QPushButton( this );
    groupBoxLayout->addWidget( addButton_, 0, 1 );

    removeButton_ = new QPushButton( this );
    groupBoxLayout->addWidget( removeButton_, 1, 1 );

    editButton_ = new QPushButton( this );
    groupBoxLayout->addWidget( editButton_, 2, 1 );

    autoDetectButton_ = new QPushButton( this );
    groupBoxLayout->addWidget( autoDetectButton_, 3, 1 );

    addButton_->setText( i18n( "&Add..." ) );
    removeButton_->setText( i18n( "&Remove" ) );
    editButton_->setText( i18n( "&Edit..." ) );
    autoDetectButton_->setText( i18n( "Auto-&Detect" ) );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum,
                                           QSizePolicy::Expanding );
    groupBoxLayout->addItem( spacer, 4, 1 );

    adjustSize();
    mainLayout->addWidget(this);

   // Initialize buttons

    removeButton_->setEnabled(false);
    editButton_->setEnabled(false);

    // connections

    connect(listView_, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(addButton_, SIGNAL(clicked()),
            this, SLOT(slotAddCamera()));

    connect(removeButton_, SIGNAL(clicked()),
            this, SLOT(slotRemoveCamera()));

    connect(editButton_, SIGNAL(clicked()),
            this, SLOT(slotEditCamera()));

    connect(autoDetectButton_, SIGNAL(clicked()),
            this, SLOT(slotAutoDetectCamera()));

    // Add cameras --------------------------------------

    CameraList* clist = CameraList::instance();
    if (clist) {

        QPtrList<CameraType>* cl = clist->cameraList();
        for (CameraType *ctype = cl->first(); ctype;
             ctype = cl->next()) {
            new QListViewItem(listView_, ctype->title(), ctype->model(),
                              ctype->port(), ctype->path());
        }
    }
}

SetupCamera::~SetupCamera()
{
}

void SetupCamera::slotSelectionChanged()
{
    QListViewItem *item = listView_->selectedItem();

    if (!item) {
        removeButton_->setEnabled(false);
        editButton_->setEnabled(false);
        return;
    }

    removeButton_->setEnabled(true);
    editButton_->setEnabled(true);
}

void SetupCamera::slotAddCamera()
{
    CameraSelection *select = new CameraSelection;

    connect(select, SIGNAL(signalOkClicked(const QString&,
                                           const QString&,
                                           const QString&,
                                           const QString&)),
            this,   SLOT(slotAddedCamera(const QString&,
                                         const QString&,
                                         const QString&,
                                         const QString&)));
    select->show();
}

void SetupCamera::slotRemoveCamera()
{
    QListViewItem *item = listView_->currentItem();
    if (!item) return;

    delete item;
}

void SetupCamera::slotEditCamera()
{
    QListViewItem *item = listView_->currentItem();
    if (!item) return;

    CameraSelection *select = new CameraSelection;
    select->setCamera(item->text(0), item->text(1),
                      item->text(2), item->text(3));
    connect(select, SIGNAL(signalOkClicked(const QString&,
                                           const QString&,
                                           const QString&,
                                           const QString&)),
            this,   SLOT(slotEditedCamera(const QString&,
                                          const QString&,
                                          const QString&,
                                          const QString&)));
    select->show();
}

void SetupCamera::slotAutoDetectCamera()
{
    QString model, port;

    if (GPIface::autoDetect(model, port) != 0) {
        KMessageBox::error(this,i18n("Failed to auto-detect camera.\n"
                                     "Please check if your camera is turned on and retry or try setting it manually."));
        return;
    }

    // NOTE: See note in digikam/digikam/cameralist.cpp
    port = "usb:";
    
    if (listView_->findItem(model,1))
    {
       KMessageBox::information(this, i18n("Camera '%1' (%2) is already in list.").arg(model).arg(port));
    }
    else {
       KMessageBox::information(this, i18n("Found camera '%1' (%2) and added it to the list.").arg(model).arg(port));
        new QListViewItem(listView_, model, model, port, "/");
    }
}



void SetupCamera::slotAddedCamera(const QString& title,
                                  const QString& model,
                                  const QString& port,
                                  const QString& path)
{
    new QListViewItem(listView_, title, model, port, path);
}

void SetupCamera::slotEditedCamera(const QString& title,
                                  const QString& model,
                                  const QString& port,
                                  const QString& path)
{
    QListViewItem *item = listView_->currentItem();
    if (!item) return;

    item->setText(0, title);
    item->setText(1, model);
    item->setText(2, port);
    item->setText(3, path);
}

void SetupCamera::applySettings()
{
    CameraList* clist = CameraList::instance();
    if (clist) {
        clist->clear();

        QListViewItemIterator it(listView_);
        for ( ; it.current(); ++it ) {
            QListViewItem *item = it.current();
            CameraType *ctype = new CameraType(item->text(0),
                                               item->text(1),
                                               item->text(2),
                                               item->text(3));
            clist->insert(ctype);
        }
    }
}

#include "setupcamera.moc"
