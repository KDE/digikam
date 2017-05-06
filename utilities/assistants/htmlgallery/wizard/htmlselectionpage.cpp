/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "htmlselectionpage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QStackedWidget>

// Local includes

#include "htmlwizard.h"
#include "galleryinfo.h"
#include "albumselecttabs.h"
#include "dimageslist.h"

namespace Digikam
{

class HTMLSelectionPage::Private
{
public:

    Private()
      : collectionSelector(0),
        imageList(0),
        stack(0)
    {
    }

    AlbumSelectTabs* collectionSelector;
    DImagesList*     imageList;
    QStackedWidget*  stack;
};

HTMLSelectionPage::HTMLSelectionPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    setObjectName(QLatin1String("AlbumSelectorPage"));

    d->stack              = new QStackedWidget(this);

    d->collectionSelector = new AlbumSelectTabs(this);
    d->stack->insertWidget(GalleryInfo::ALBUMS, d->collectionSelector);

    d->imageList          = new DImagesList(this);
    d->imageList->setControlButtonsPlacement(DImagesList::ControlButtonsBelow);
    d->stack->insertWidget(GalleryInfo::IMAGES, d->imageList);

    setPageWidget(d->stack);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-pictures")));

    connect(d->collectionSelector, SIGNAL(signalAlbumSelectionChanged()),
            this, SIGNAL(completeChanged()));

    connect(d->imageList, SIGNAL(signalImageListChanged()),
            this, SIGNAL(completeChanged()));
}

HTMLSelectionPage::~HTMLSelectionPage()
{
    delete d;
}

void HTMLSelectionPage::initializePage()
{
    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (wizard)
    {
        GalleryInfo* const info  = wizard->galleryInfo();
        d->imageList->setIface(info->mIface);
        d->imageList->loadImagesFromCurrentSelection();
        d->stack->setCurrentIndex(info->mGetOption);
    }
}

bool HTMLSelectionPage::validatePage()
{
    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
        return false;

    GalleryInfo* const info  = wizard->galleryInfo();

    if (d->stack->currentIndex() == GalleryInfo::ALBUMS)
    {
        if (d->collectionSelector->selectedAlbums().empty())
            return false;

        info->mCollectionList = d->collectionSelector->selectedAlbums();
    }
    else
    {
        if (d->imageList->imageUrls().empty())
            return false;

        info->mImageList = d->imageList->imageUrls();
    }

    return true;
}

bool HTMLSelectionPage::isComplete() const
{
    if (d->stack->currentIndex() == GalleryInfo::ALBUMS)
        return (!d->collectionSelector->selectedAlbums().empty());

    return (!d->imageList->imageUrls().empty());
}

} // namespace Digikam
