/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "digikamimageview.h"
#include "digikamimageview.moc"

// Qt includes

// KDE includes

#include <kdebug.h>

// Local includes

#include "imagealbummodel.h"
#include "imagealbumfiltermodel.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class DigikamImageViewPriv
{
public:
    DigikamImageViewPriv()
    {
    }
};

DigikamImageView::DigikamImageView(QWidget *parent)
               : ImageCategorizedView(parent), d(new DigikamImageViewPriv)
{
    imageFilterModel()->setCategorizationMode(ImageFilterModel::CategoryByAlbum);
    imageModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());
}

DigikamImageView::~DigikamImageView()
{
    delete d;
}

void DigikamImageView::activated(const ImageInfo &info)
{
}

void DigikamImageView::showContextMenu(QContextMenuEvent *event, const ImageInfo &info)
{
}

void DigikamImageView::showContextMenu(QContextMenuEvent *event)
{
}

void DigikamImageView::copy()
{
}

void DigikamImageView::paste()
{
}


} // namespace Digikam
