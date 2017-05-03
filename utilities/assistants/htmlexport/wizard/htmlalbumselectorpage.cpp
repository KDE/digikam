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

#include "htmlalbumselectorpage.h"

namespace Digikam
{

HTMLAlbumSelectorPage::HTMLAlbumSelectorPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title)
{
    setObjectName(QLatin1String("AlbumSelectorPage"));

    mCollectionSelector = new AlbumSelectTabs(this);
    setPageWidget(mCollectionSelector);

    connect(mCollectionSelector, SIGNAL(signalAlbumSelectionChanged()),
            this, SIGNAL(completeChanged()));
}

HTMLAlbumSelectorPage::~HTMLAlbumSelectorPage()
{
}

bool HTMLAlbumSelectorPage::validatePage()
{
    return (!mCollectionSelector->selectedAlbums().empty());
}

} // namespace Digikam
