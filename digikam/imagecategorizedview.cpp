/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-22
 * Description : Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imagecategorizedview.h"
#include "imagecategorizedview.moc"

// Qt includes

// KDE includes

// Local includes

#include "imagealbumfiltermodel.h"
#include "imagethumbnailmodel.h"
#include "imagedelegate.h"
#include "imagecategorydrawer.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class ImageCategorizedViewPriv
{
public:

    ImageCategorizedViewPriv()
    {
    }

    ImageAlbumModel         *model;
    ImageAlbumFilterModel   *filterModel;

    ImageDelegate           *delegate;
    ImageCategoryDrawer     *categoryDrawer;

    ThumbnailSize            thumbnailSize;
};

ImageCategorizedView::ImageCategorizedView(QWidget *parent)
    : KCategorizedView(parent), d(new ImageCategorizedViewPriv)
{
    d->model = new ImageAlbumModel(this);
    d->model->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());
    d->filterModel = new ImageAlbumFilterModel(this);
    d->filterModel->setSourceModel(d->model);

    d->delegate = new ImageDelegate(this);
    setItemDelegate(d->delegate);
    setCategoryDrawer(d->delegate->categoryDrawer());

    setModel(d->filterModel);
}

ImageCategorizedView::~ImageCategorizedView()
{
    delete d;
}

ImageAlbumModel *ImageCategorizedView::imageModel() const
{
    return d->model;
}

ImageAlbumFilterModel *ImageCategorizedView::imageFilterModel() const
{
    return d->filterModel;
}

ImageDelegate *delegate() const
{
    return d->delegate;
}

ThumbnailSize ImageCategorizedView::thumbnailSize() const
{
    return d->model->thumbnailSize();
}

void ImageCategorizedView::setThumbnailSize(const ThumbnailSize &size)
{
    d->model->setThumbnailSize(size);
    d->delegate->setThumbnailSize(size);
    viewport()->update();
}

void ImageCategorizedView::slotThemeChanged()
{
    viewport()->update();
}

void ImageCategorizedView::slotSetupChanged()
{
    viewport()->update();
}

void ImageCategorizedView::changeEvent(QEvent * event);
{
    if (event->type == QEvent::FontChange)
        d->delegate->setDefaultViewOptions(viewOptions());
}

bool ImageCategorizedView::viewportEvent(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::FontChange:
            d->delegate->setDefaultViewOptions(viewOptions());
            break;
        case QEvent::helpEvent:
        default:
            break;
    }
    return KCategorizedView::viewportEvent(event);
}

}