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

// Local includes

#include "htmlalbumselectorpage.h"
#include "htmlwizard.h"
#include "galleryinfo.h"
#include "albumselecttabs.h"

namespace Digikam
{

class HTMLAlbumSelectorPage::Private
{
public:

    Private()
      : collectionSelector(0)
    {
    }

    AlbumSelectTabs* collectionSelector;
};

HTMLAlbumSelectorPage::HTMLAlbumSelectorPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    setObjectName(QLatin1String("AlbumSelectorPage"));

    d->collectionSelector = new AlbumSelectTabs(this);
    setPageWidget(d->collectionSelector);

    connect(d->collectionSelector, SIGNAL(signalAlbumSelectionChanged()),
            this, SIGNAL(completeChanged()));
}

HTMLAlbumSelectorPage::~HTMLAlbumSelectorPage()
{
    delete d;
}

bool HTMLAlbumSelectorPage::validatePage()
{
    if (d->collectionSelector->selectedAlbums().empty())
        return false;

    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());
    GalleryInfo* const info  = wizard->galleryInfo();
    info->mCollectionList    = d->collectionSelector->selectedAlbums();

    return true;
}

} // namespace Digikam
