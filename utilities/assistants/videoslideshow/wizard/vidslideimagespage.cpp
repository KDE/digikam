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

#include "vidslideimagespage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QStackedWidget>

// Local includes

#include "vidslidewizard.h"
#include "dimageslist.h"

namespace Digikam
{

class VidSlideImagesPage::Private
{
public:

    Private(QWizard* const dialog)
      : imageList(0),
        wizard(0),
        iface(0)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    DImagesList*     imageList;
    VidSlideWizard*  wizard;
    DInfoInterface*  iface;
};

VidSlideImagesPage::VidSlideImagesPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("ImagesSelectorPage"));

    d->imageList = new DImagesList(this);
    d->imageList->setControlButtonsPlacement(DImagesList::ControlButtonsBelow);

    setPageWidget(d->imageList);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("media-playlist-repeat")));

    connect(d->imageList, SIGNAL(signalImageListChanged()),
            this, SIGNAL(completeChanged()));
}

VidSlideImagesPage::~VidSlideImagesPage()
{
    delete d;
}

void VidSlideImagesPage::setItemsList(const QList<QUrl>& urls)
{
    d->imageList->slotAddImages(urls);
}

void VidSlideImagesPage::initializePage()
{
    d->imageList->setIface(d->iface);

    if (d->wizard->settings()->selMode == VidSlideSettings::IMAGES)
    {
        d->imageList->loadImagesFromCurrentSelection();
    }
    else
    {
        setItemsList(d->wizard->settings()->inputImages);
    }
}

bool VidSlideImagesPage::validatePage()
{
    if (d->imageList->imageUrls().empty())
        return false;

    d->wizard->settings()->inputImages = d->imageList->imageUrls();

    return true;
}

bool VidSlideImagesPage::isComplete() const
{
    return (!d->imageList->imageUrls().empty());
}

} // namespace Digikam
