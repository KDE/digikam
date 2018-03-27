/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to print images
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

#include "advprintalbumspage.h"

// Qt includes

#include <QIcon>

// Local includes

#include "advprintwizard.h"

namespace Digikam
{

class AdvPrintAlbumsPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : albumSupport(false),
        albumSelector(0),
        wizard(0),
        iface(0)
    {
        wizard = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    bool             albumSupport;
    QWidget*         albumSelector;
    AdvPrintWizard*  wizard;
    DInfoInterface*  iface;
};

AdvPrintAlbumsPage::AdvPrintAlbumsPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    if (d->iface)
    {
        d->albumSelector = d->iface->albumChooser(this);

        connect(d->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
                this, SIGNAL(completeChanged()));
    }
    else
    {
        d->albumSelector = new QWidget(this);
    }

    setPageWidget(d->albumSelector);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-mail")));
}

AdvPrintAlbumsPage::~AdvPrintAlbumsPage()
{
    delete d;
}

bool AdvPrintAlbumsPage::validatePage()
{
    if (!d->iface)
        return false;

    if (d->iface->albumChooserItems().isEmpty())
        return false;

    d->wizard->settings()->inputImages.clear();

    // update image list with album contents.
    foreach(const QUrl& url, d->iface->albumsItems(d->iface->albumChooserItems()))
    {
        d->wizard->settings()->inputImages << url;
    }

    return true;
}

bool AdvPrintAlbumsPage::isComplete() const
{
    if (!d->iface)
        return false;

    return (!d->iface->albumChooserItems().isEmpty());
}

} // namespace Digikam
