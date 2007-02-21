/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date  : 2003-02-10
 * Description : camera setup tab.
 * 
 * Copyright 2003-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006 by Gilles Caulier 
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

// Qt includes.

#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qdatetime.h>

// KDE includes.

#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurllabel.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes.

#include "setupcamera.h"
#include "cameraselection.h"
#include "cameralist.h"
#include "cameratype.h"
#include "gpiface.h"

namespace Digikam
{
class SetupCameraPriv
{
public:

    SetupCameraPriv()
    {
        listView         = 0;
        addButton        = 0;
        removeButton     = 0;        
        editButton       = 0;
        autoDetectButton = 0;
    }

    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *editButton;
    QPushButton *autoDetectButton;
    
    KListView   *listView;
};

SetupCamera::SetupCamera( QWidget* parent )
           : QWidget( parent )
{
    d = new SetupCameraPriv;
    QVBoxLayout *mainLayout = new QVBoxLayout(parent);

    QGridLayout* groupBoxLayout = new QGridLayout( this, 2, 5, 0, KDialog::spacingHint() );
    groupBoxLayout->setAlignment( Qt::AlignTop );

    d->listView = new KListView( this );
    d->listView->addColumn( i18n("Title") );
    d->listView->addColumn( i18n("Model") );
    d->listView->addColumn( i18n("Port") );
    d->listView->addColumn( i18n("Path") );
    d->listView->addColumn( "Last Access Date", 0 ); // No i18n here. Hidden column with the last access date.
    d->listView->setAllColumnsShowFocus(true);
    groupBoxLayout->addMultiCellWidget( d->listView, 0, 5, 0, 0 );
    QWhatsThis::add( d->listView, i18n("<p>Here you can see the digital camera list used by digiKam "
                                       "via the Gphoto interface."));

    // -------------------------------------------------------------

    d->addButton = new QPushButton( this );
    groupBoxLayout->addWidget( d->addButton, 0, 1 );

    d->removeButton = new QPushButton( this );
    groupBoxLayout->addWidget( d->removeButton, 1, 1 );

    d->editButton = new QPushButton( this );
    groupBoxLayout->addWidget( d->editButton, 2, 1 );

    d->autoDetectButton = new QPushButton( this );
    groupBoxLayout->addWidget( d->autoDetectButton, 3, 1 );

    d->addButton->setText( i18n( "&Add..." ) );
    d->removeButton->setText( i18n( "&Remove" ) );
    d->editButton->setText( i18n( "&Edit..." ) );
    d->autoDetectButton->setText( i18n( "Auto-&Detect" ) );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    groupBoxLayout->addItem( spacer, 4, 1 );

    KURLLabel *gphotoLogoLabel = new KURLLabel(this);
    gphotoLogoLabel->setText(QString());
    gphotoLogoLabel->setURL("http://www.gphoto.org");
    KGlobal::dirs()->addResourceType("logo-gphoto", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("logo-gphoto", "logo-gphoto.png");
    gphotoLogoLabel->setPixmap( QPixmap( directory + "logo-gphoto.png" ) );
    QToolTip::add(gphotoLogoLabel, i18n("Visit Gphoto project website"));
    groupBoxLayout->addWidget( gphotoLogoLabel, 5, 1 );

    adjustSize();
    mainLayout->addWidget(this);

   // Initialize buttons

    d->removeButton->setEnabled(false);
    d->editButton->setEnabled(false);

    // -------------------------------------------------------------

    connect(gphotoLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processGphotoURL(const QString&)));

    connect(d->listView, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddCamera()));

    connect(d->removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveCamera()));

    connect(d->editButton, SIGNAL(clicked()),
            this, SLOT(slotEditCamera()));

    connect(d->autoDetectButton, SIGNAL(clicked()),
            this, SLOT(slotAutoDetectCamera()));

    // Add cameras --------------------------------------

    CameraList* clist = CameraList::instance();
    
    if (clist) 
    {
        QPtrList<CameraType>* cl = clist->cameraList();

        for (CameraType *ctype = cl->first(); ctype;
             ctype = cl->next()) 
        {
            new KListViewItem(d->listView, ctype->title(), ctype->model(),
                              ctype->port(), ctype->path(), 
                              ctype->lastAccess().toString(Qt::ISODate));
        }
    }
}

SetupCamera::~SetupCamera()
{
    delete d;
}

void SetupCamera::processGphotoURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void SetupCamera::slotSelectionChanged()
{
    QListViewItem *item = d->listView->selectedItem();

    if (!item) 
    {
        d->removeButton->setEnabled(false);
        d->editButton->setEnabled(false);
        return;
    }

    d->removeButton->setEnabled(true);
    d->editButton->setEnabled(true);
}

void SetupCamera::slotAddCamera()
{
    CameraSelection *select = new CameraSelection;

    connect(select, SIGNAL(signalOkClicked(const QString&, const QString&, 
                                           const QString&, const QString&)),
            this,   SLOT(slotAddedCamera(const QString&, const QString&, 
                                         const QString&, const QString&)));

    select->show();
}

void SetupCamera::slotRemoveCamera()
{
    QListViewItem *item = d->listView->currentItem();
    if (!item) return;

    delete item;
}

void SetupCamera::slotEditCamera()
{
    QListViewItem *item = d->listView->currentItem();
    if (!item) return;

    CameraSelection *select = new CameraSelection;
    select->setCamera(item->text(0), item->text(1), item->text(2), item->text(3));

    connect(select, SIGNAL(signalOkClicked(const QString&, const QString&, 
                                           const QString&, const QString&)),
            this,   SLOT(slotEditedCamera(const QString&, const QString&, 
                                          const QString&, const QString&)));

    select->show();
}

void SetupCamera::slotAutoDetectCamera()
{
    QString model, port;
    
    kapp->setOverrideCursor( KCursor::waitCursor() );
    int ret = GPIface::autoDetect(model, port);
    kapp->restoreOverrideCursor();
    
    if (ret != 0) 
    {
        KMessageBox::error(this,i18n("Failed to auto-detect camera.\n"
                                     "Please check if your camera is turned on "
                                     "and retry or try setting it manually."));
        return;
    }

    // NOTE: See note in digikam/digikam/cameralist.cpp
    if (port.startsWith("usb:"))
    port = "usb:";
    
    if (d->listView->findItem(model, 1))
    {
        KMessageBox::information(this, i18n("Camera '%1' (%2) is already in list.").arg(model).arg(port));
    }
    else 
    {
        KMessageBox::information(this, i18n("Found camera '%1' (%2) and added it to the list.")
                                 .arg(model).arg(port));
        new KListViewItem(d->listView, model, model, port, "/", 
                          QDateTime::currentDateTime().toString(Qt::ISODate));
    }
}

void SetupCamera::slotAddedCamera(const QString& title, const QString& model,
                                  const QString& port, const QString& path)
{
    new KListViewItem(d->listView, title, model, port, path, 
                      QDateTime::currentDateTime().toString(Qt::ISODate));
}

void SetupCamera::slotEditedCamera(const QString& title, const QString& model,
                                   const QString& port, const QString& path)
{
    QListViewItem *item = d->listView->currentItem();
    if (!item) return;

    item->setText(0, title);
    item->setText(1, model);
    item->setText(2, port);
    item->setText(3, path);
}

void SetupCamera::applySettings()
{
    CameraList* clist = CameraList::instance();

    if (clist) 
    {
        clist->clear();

        QListViewItemIterator it(d->listView);

        for ( ; it.current(); ++it ) 
        {
            QListViewItem *item  = it.current();
            QDateTime lastAccess = QDateTime::currentDateTime();

            if (!item->text(4).isEmpty())
                lastAccess = QDateTime::fromString(item->text(4), Qt::ISODate);
                            
            CameraType *ctype = new CameraType(item->text(0), item->text(1), item->text(2), 
                                               item->text(3), lastAccess);
            clist->insert(ctype);
        }

        clist->save();
    }
}

}  // namespace Digikam

#include "setupcamera.moc"
