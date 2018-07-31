/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "wsimagespage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "dimageslist.h"
#include "wswizard.h"
#include "wsnewalbumdialog.h"

namespace Digikam
{

class WSImagesPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : imageList(0),
        wizard(0),
        iface(0),
        wsAuth(0)
    {
        wizard = dynamic_cast<WSWizard*>(dialog);

        if (wizard)
        {
            iface   = wizard->iface();
            wsAuth  = wizard->wsAuth();
        }
    }

    DImagesList*        imageList;

    QTreeWidget*        albumView;
    QString             currentAlbumId;
    QPushButton*        newAlbumBtn;
    QPushButton*        reloadAlbumsBtn;
    
    WSWizard*           wizard;
    DInfoInterface*     iface;
    WSAuthentication*   wsAuth;
};

WSImagesPage::WSImagesPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{    
    DHBox* const hbox       = new DHBox(this);

    /* --------------------
     * Widget for Images list
     */
    
    DVBox* const vboxImage  = new DVBox(hbox);
    QLabel* const descImage = new QLabel(vboxImage);
    descImage->setText(i18n("<p>This view lists all items to export.</p>"));

    d->imageList            = new DImagesList(vboxImage);
    d->imageList->setControlButtonsPlacement(DImagesList::ControlButtonsBelow);
    
    connect(d->imageList, SIGNAL(signalImageListChanged()),
            this, SIGNAL(completeChanged()));
    
    /* --------------------
     * User albums list
     */    

    DVBox* const vboxAlbum  = new DVBox(hbox);
    QLabel* const descAlbum = new QLabel(vboxAlbum);
    descAlbum->setText(i18n("<p>This view lists all albums.</p>"));    
    
    d->albumView            = new QTreeWidget(vboxAlbum);
    DHBox* const buttonBox  = new DHBox(vboxAlbum);
    d->newAlbumBtn          = new QPushButton(QString("New Album"), buttonBox);
    d->reloadAlbumsBtn      = new QPushButton(QString("Reload"), buttonBox);

    connect(d->newAlbumBtn, SIGNAL(clicked()),
            d->wsAuth, SLOT(slotNewAlbumRequest()));
    connect(d->reloadAlbumsBtn, SIGNAL(clicked()),
            this, SLOT(slotReloadListAlbums()));
    
    /* --------------------
     * General settings for imagespage
     */
    
    hbox->setStretchFactor(vboxImage,   2);
    hbox->setStretchFactor(vboxAlbum,   1);

    setPageWidget(hbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("image-stack")));
    
    connect(d->wsAuth, SIGNAL(signalListAlbumsDone(const QMap<QString, AlbumSimplified>&, const QStringList&, const QString&)),
            this, SLOT(slotListAlbumsDone(const QMap<QString, AlbumSimplified>&, const QStringList&, const QString&)));
}

WSImagesPage::~WSImagesPage()
{
    delete d;
}

void WSImagesPage::setItemsList(const QList<QUrl>& urls)
{
    d->imageList->slotAddImages(urls);
}

void WSImagesPage::setCurrentAlbumId(const QString& currentAlbumId)
{
    d->currentAlbumId = currentAlbumId;
    d->wizard->settings()->currentAlbumId = currentAlbumId;
}

void WSImagesPage::initializePage()
{    
    d->imageList->setIface(d->iface);
    d->imageList->listView()->clear();
    
    // List current albums in user account
    d->wsAuth->listAlbums();
}

bool WSImagesPage::validatePage()
{  
    setCurrentAlbumId(d->albumView->currentItem()->data(0, Qt::AccessibleDescriptionRole).toString());

    if (d->imageList->imageUrls().isEmpty())
        return false;

    d->wizard->settings()->inputImages = d->imageList->imageUrls();

    return true;
}

bool WSImagesPage::isComplete() const
{
    return (!d->imageList->imageUrls().isEmpty());
}

void WSImagesPage::addChildToTreeView(QTreeWidgetItem* const parent,
                                      const QMap<QString, AlbumSimplified>& albumTree, 
                                      const QStringList& childrenAlbums)
{
    if(childrenAlbums.isEmpty())
    {
        return;
    }
    
    foreach(const QString& albumId, childrenAlbums)
    {
        QTreeWidgetItem* item   = new QTreeWidgetItem(parent);
        item->setText(0, albumTree[albumId].title);
        item->setData(0, Qt::AccessibleDescriptionRole, albumId); // set ID hidden, so that we can use it for uploading photos to album
        if(albumTree[albumId].uploadable)
        {
            item->setWhatsThis(0, QLatin1String("Albums that can be uploaded."));
        }
        else
        {
//             item->setDisabled(true);
            item->setWhatsThis(0, QLatin1String("Albums that cannot be uploaded."));
        }
        
        if(albumId == d->currentAlbumId)
        {
            d->albumView->setCurrentItem(item);
        }
        
        addChildToTreeView(item, albumTree, albumTree[albumId].childrenIDs);
    }
}

void WSImagesPage::slotListAlbumsDone(const QMap<QString, AlbumSimplified>& albumTree, 
                                      const QStringList& rootAlbums,
                                      const QString& currentAlbumId)
{
    d->albumView->clear();
    
    if(currentAlbumId.isEmpty())
    {
        d->currentAlbumId = rootAlbums.first();
    }
    else
    {
        d->currentAlbumId = currentAlbumId;
    }
    
    foreach(const QString& albumId, rootAlbums)
    {
        QTreeWidgetItem* item   = new QTreeWidgetItem(d->albumView);
        item->setText(0, albumTree[albumId].title);
        item->setData(0, Qt::AccessibleDescriptionRole, albumId); // set ID hidden, so that we can use it for uploading photos to album
        if(albumTree[albumId].uploadable)
        {
            item->setWhatsThis(0, QLatin1String("Albums that can be uploaded."));
        }
        else
        {
//             item->setDisabled(true);
            item->setWhatsThis(0, QLatin1String("Albums that cannot be uploaded."));
        }
        
        if(d->currentAlbumId == albumId)
        {
            d->albumView->setCurrentItem(item);
        }
        
        addChildToTreeView(item, albumTree, albumTree[albumId].childrenIDs);
    }
}

void WSImagesPage::slotReloadListAlbums()
{
    d->wsAuth->listAlbums();
}

} // namespace Digikam
