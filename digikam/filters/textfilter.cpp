/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-23
 * Description : a widget to filter album contents by text query
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "textfilter.moc"

// Qt includes

#include <QGridLayout>

// KDE includes

#include <klocale.h>

// Local includes

#include "searchtextbar.h"

namespace Digikam
{

class TextFilter::TextFilterPriv
{
public:

    TextFilterPriv()
    {
        searchTextBar = 0;
    }

    SearchTextBar* searchTextBar;
};

TextFilter::TextFilter(QWidget* parent)
    : KHBox(parent), d(new TextFilterPriv)
{
    d->searchTextBar = new SearchTextBar(this, "AlbumIconViewFilterSearchTextBar");
    d->searchTextBar->setTextQueryCompletion(true);
    d->searchTextBar->setToolTip(i18n("Text quick filter (search)"));
    d->searchTextBar->setWhatsThis(i18n("Enter search patterns to quickly filter this view on "
                                     "file names, captions (comments), and tags"));

    setMargin(0);
    setSpacing(0);
}

TextFilter::~TextFilter()
{
    delete d;
}

SearchTextBar* TextFilter::searchTextBar() const
{
    return d->searchTextBar;
}

}  // namespace Digikam
