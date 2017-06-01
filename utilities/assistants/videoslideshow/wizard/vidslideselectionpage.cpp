/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "vidslideselectionpage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QStackedWidget>

// Local includes

#include "vidslidewizard.h"
#include "dimageslist.h"

namespace Digikam
{

class VidSlideSelectionPage::Private
{
public:

    Private(QWizard* const dialog)
      : albumSupport(false),
        albumSelector(0),
        imageList(0),
        stack(0),
        wizard(0),
        iface(0)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    bool             albumSupport;
    QWidget*         albumSelector;
    DImagesList*     imageList;
    QStackedWidget*  stack;
    VidSlideWizard*  wizard;
    DInfoInterface*  iface;
};

VidSlideSelectionPage::VidSlideSelectionPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("AlbumSelectorPage"));

    d->stack              = new QStackedWidget(this);
    d->albumSupport       = (d->iface && d->iface->supportAlbums());

    if (d->albumSupport)
    {
        d->albumSelector = d->iface->albumChooser(this);
    }
    else
    {
        d->albumSelector = new QWidget(this);
    }

    d->stack->insertWidget(VidSlideSettings::ALBUMS, d->albumSelector);

    d->imageList         = new DImagesList(this);
    d->imageList->setControlButtonsPlacement(DImagesList::ControlButtonsBelow);
    d->stack->insertWidget(VidSlideSettings::IMAGES, d->imageList);

    setPageWidget(d->stack);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-pictures")));

    if (d->albumSupport)
    {
        connect(d->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
                this, SIGNAL(completeChanged()));
    }

    connect(d->imageList, SIGNAL(signalImageListChanged()),
            this, SIGNAL(completeChanged()));
}

VidSlideSelectionPage::~VidSlideSelectionPage()
{
    delete d;
}

void VidSlideSelectionPage::setItemsList(const QList<QUrl>& urls)
{
    d->imageList->slotAddImages(urls);
}

void VidSlideSelectionPage::initializePage()
{
    d->imageList->setIface(d->iface);

    if (d->wizard->settings()->selMode == VidSlideSettings::IMAGES)
    {
        d->imageList->loadImagesFromCurrentSelection();
    }

    d->stack->setCurrentIndex(d->wizard->settings()->selMode);
}

bool VidSlideSelectionPage::validatePage()
{
    if (d->stack->currentIndex() == VidSlideSettings::ALBUMS)
    {
        if (d->albumSupport)
        {
            if (d->iface->albumChooserItems().empty())
                return false;

            // update image list with album contents.
            foreach(const QUrl& url, d->iface->albumsItems(d->iface->albumChooserItems()))
            {
                d->wizard->settings()->inputImages << url;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        if (d->imageList->imageUrls().empty())
            return false;

        d->wizard->settings()->inputImages = d->imageList->imageUrls();
    }

    return true;
}

bool VidSlideSelectionPage::isComplete() const
{
    if (d->stack->currentIndex() == VidSlideSettings::ALBUMS)
    {
        if (!d->albumSupport)
            return false;

        return (!d->iface->albumChooserItems().empty());
    }

    return (!d->imageList->imageUrls().empty());
}

} // namespace Digikam
