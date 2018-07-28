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

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "wswizard.h"
#include "dimageslist.h"
#include "dlayoutbox.h"

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
    WSWizard*           wizard;
    DInfoInterface*     iface;
    WSAuthentication*   wsAuth;
};

WSImagesPage::WSImagesPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{    
    DHBox* const hbox  = new DHBox(this);
    
    DVBox* const vboxImage  = new DVBox(hbox);
    QLabel* const descImage = new QLabel(vboxImage);
    descImage->setText(i18n("<p>This view lists all items to export.</p>"));

    DVBox* const vboxAlbum  = new DVBox(hbox);
    QLabel* const descAlbum = new QLabel(vboxAlbum);
    descAlbum->setText(i18n("<p>This view lists all albums.</p>"));    
    
    d->imageList       = new DImagesList(vboxImage);
    d->imageList->setControlButtonsPlacement(DImagesList::ControlButtonsBelow);
    
    d->albumView       = new QTreeWidget(vboxAlbum);

    hbox->setStretchFactor(vboxImage,   2);
    hbox->setStretchFactor(vboxAlbum,   1);

    setPageWidget(hbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("image-stack")));

    connect(d->imageList, SIGNAL(signalImageListChanged()),
            this, SIGNAL(completeChanged()));
    connect(d->wsAuth, SIGNAL(signalListAlbumsDone(const QMap<QString, AlbumSimplified>&, const QStringList&)),
            this, SLOT(slotListAlbumsDone(const QMap<QString, AlbumSimplified>&, const QStringList&)));
}

WSImagesPage::~WSImagesPage()
{
    delete d;
}

void WSImagesPage::setItemsList(const QList<QUrl>& urls)
{
    d->imageList->slotAddImages(urls);
}

void WSImagesPage::initializePage()
{
    // Hide back button because we don't want to go back to a blank authentication page
    d->wizard->button(QWizard::BackButton)->hide();
    
    d->imageList->setIface(d->iface);
    d->imageList->listView()->clear();
    
    // List current albums in user account
    d->wsAuth->listAlbums();
    
/*
    if (d->wizard->settings()->selMode == WSSettings::IMAGES)
    {
        d->imageList->loadImagesFromCurrentSelection();
    }
    else
    {
        setItemsList(d->wizard->settings()->inputImages);
    }
*/
}

bool WSImagesPage::validatePage()
{
    d->wizard->button(QWizard::BackButton)->show();
    
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
        QTreeWidgetItem* item = new QTreeWidgetItem(parent);
        item->setText(0, albumTree[albumId].title);
        
        addChildToTreeView(item, albumTree, albumTree[albumId].childrenIDs);
    }
}

void WSImagesPage::slotListAlbumsDone(const QMap<QString, AlbumSimplified>& albumTree, 
                                      const QStringList& rootAlbums)
{
    foreach(const QString& albumId, rootAlbums)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(d->albumView);
        item->setText(0, albumTree[albumId].title);
        
        addChildToTreeView(item, albumTree, albumTree[albumId].childrenIDs);
    }
}

} // namespace Digikam
