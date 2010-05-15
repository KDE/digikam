/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View items for DImg
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dimgpreviewitem.moc"
#include "dimgitemspriv.h"

// Qt includes

#include <QApplication>
#include <QDesktopWidget>

// KDE includes

// Local includes

#include "imageinfo.h"
#include "loadingcacheinterface.h"
#include "loadingdescription.h"
#include "previewloadthread.h"


namespace Digikam
{

DImgPreviewItemPrivate::DImgPreviewItemPrivate()
{
    state             = DImgPreviewItem::NoImage;
    exifRotate        = true;
    previewSize       = 1024;
    loadFullImageSize = false;
    previewThread     = 0;
}

DImgPreviewItem::DImgPreviewItem(QGraphicsItem *parent)
    : GraphicsDImgItem(*new DImgPreviewItemPrivate, parent)
{
    Q_D(DImgPreviewItem);
    d->init(this);
}

DImgPreviewItem::DImgPreviewItem(DImgPreviewItemPrivate &dd, QGraphicsItem *parent)
    : GraphicsDImgItem(dd, parent)
{
    Q_D(DImgPreviewItem);
    d->init(this);
}

void DImgPreviewItemPrivate::init(DImgPreviewItem *q)
{
    previewThread = new PreviewLoadThread;
    preloadThread = new PreviewLoadThread;

    QObject::connect(previewThread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
                     q, SLOT(slotGotImagePreview(const LoadingDescription&, const DImg&)));

    QObject::connect(preloadThread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
                     q, SLOT(preloadNext()));

    // get preview size from screen size, but limit from VGA to WQXGA
    previewSize = qMax(QApplication::desktop()->height(),
                       QApplication::desktop()->width());
    if (previewSize < 640)
        previewSize = 640;
    if (previewSize > 2560)
        previewSize = 2560;

    LoadingCacheInterface::connectToSignalFileChanged(q,
            SLOT(slotFileChanged(const QString&)));
}

DImgPreviewItem::~DImgPreviewItem()
{
    Q_D(DImgPreviewItem);
    delete d->previewThread;
}

void DImgPreviewItem::setDisplayingWidget(QWidget *widget)
{
    Q_D(DImgPreviewItem);
    d->previewThread->setDisplayingWidget(widget);
}

void DImgPreviewItem::setLoadFullImageSize(bool b)
{
    Q_D(DImgPreviewItem);
    if (d->loadFullImageSize == b)
        return;
    d->loadFullImageSize = b;
    reload();
}

void DImgPreviewItem::setExifRotate(bool b)
{
    Q_D(DImgPreviewItem);
    d->exifRotate = b;
}

QString DImgPreviewItem::path() const
{
    Q_D(const DImgPreviewItem);
    return d->path;
}

void DImgPreviewItem::setPath(const QString& path)
{
    Q_D(DImgPreviewItem);
    if (path == d->path)
        return;

    d->path = path;

    if (d->path.isNull())
    {
        d->state = NoImage;
    }
    else
    {
        d->state = Loading;
        if (d->loadFullImageSize)
            d->previewThread->loadHighQuality(d->path, d->exifRotate);
        else
            d->previewThread->load(d->path, d->previewSize, d->exifRotate);
    }
}

void DImgPreviewItem::setPreloadPaths(const QStringList& pathsToPreload)
{
    Q_D(DImgPreviewItem);
    d->pathsToPreload = pathsToPreload;
    preloadNext();
}

void DImgPreviewItem::reload()
{
    Q_D(DImgPreviewItem);
    QString path = d->path;
    d->path = QString();
    setPath(path);
}

DImgPreviewItem::State DImgPreviewItem::state() const
{
    Q_D(const DImgPreviewItem);
    return d->state;
}

bool DImgPreviewItem::isLoaded() const
{
    Q_D(const DImgPreviewItem);
    return d->state == ImageLoaded;
}

void DImgPreviewItem::slotGotImagePreview(const LoadingDescription& description, const DImg& image)
{
    Q_D(DImgPreviewItem);
    if (description.filePath != d->path || description.isThumbnail())
        return;

    setImage(image);
    if (image.isNull())
    {
        d->state = ImageLoadingFailed;
        emit loadingFailed();
    }
    else
    {
        d->state = ImageLoaded;
        emit loaded();
    }
}

void DImgPreviewItem::preloadNext()
{
    Q_D(DImgPreviewItem);
    if (!isLoaded() || d->pathsToPreload.isEmpty())
        return;

    QString preloadPath = d->pathsToPreload.takeFirst();
    if (d->loadFullImageSize)
        d->preloadThread->loadHighQuality(preloadPath, d->exifRotate);
    else
        d->preloadThread->load(preloadPath, d->previewSize, d->exifRotate);
}

void DImgPreviewItem::slotFileChanged(const QString& path)
{
    Q_D(DImgPreviewItem);
    if (d->path == path)
        reload();
}

}

