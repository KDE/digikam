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

#include "vidslidealbumspage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QStackedWidget>

// Local includes

#include "vidslidewizard.h"
#include "dimageslist.h"

namespace Digikam
{

class VidSlideAlbumsPage::Private
{
public:

    Private(QWizard* const dialog)
      : albumSupport(false),
        albumSelector(0),
        imageList(0),
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
    VidSlideWizard*  wizard;
    DInfoInterface*  iface;
};

VidSlideAlbumsPage::VidSlideAlbumsPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("AlbumsSelectorPage"));

    d->albumSelector = d->iface->albumChooser(this);

    setPageWidget(d->albumSelector);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-pictures")));

    connect(d->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
            this, SIGNAL(completeChanged()));
}

VidSlideAlbumsPage::~VidSlideAlbumsPage()
{
    delete d;
}

bool VidSlideAlbumsPage::validatePage()
{
    if (d->iface->albumChooserItems().empty())
        return false;

    d->wizard->settings()->inputImages.clear();

    // update image list with album contents.
    foreach(const QUrl& url, d->iface->albumsItems(d->iface->albumChooserItems()))
    {
        d->wizard->settings()->inputImages << url;
    }

    return true;
}

bool VidSlideAlbumsPage::isComplete() const
{
    return (!d->iface->albumChooserItems().empty());
}

} // namespace Digikam
