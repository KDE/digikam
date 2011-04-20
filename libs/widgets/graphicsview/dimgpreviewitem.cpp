/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View items for DImg
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <klocale.h>

// Local includes

#include "loadingcacheinterface.h"
#include "loadingdescription.h"
#include "previewloadthread.h"

namespace Digikam
{

DImgPreviewItem::DImgPreviewItem(QGraphicsItem* parent)
    : GraphicsDImgItem(*new DImgPreviewItemPrivate, parent)
{
    Q_D(DImgPreviewItem);
    d->init(this);
}

DImgPreviewItem::DImgPreviewItem(DImgPreviewItemPrivate& dd, QGraphicsItem* parent)
    : GraphicsDImgItem(dd, parent)
{
    Q_D(DImgPreviewItem);
    d->init(this);
}

DImgPreviewItem::DImgPreviewItemPrivate::DImgPreviewItemPrivate()
{
    state             = DImgPreviewItem::NoImage;
    exifRotate        = true;
    previewSize       = 1024;
    loadFullImageSize = false;
    previewThread     = 0;
}

void DImgPreviewItem::DImgPreviewItemPrivate::init(DImgPreviewItem* q)
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
    {
        previewSize = 640;
    }

    if (previewSize > 2560)
    {
        previewSize = 2560;
    }

    LoadingCacheInterface::connectToSignalFileChanged(q, SLOT(slotFileChanged(const QString&)));
}

DImgPreviewItem::~DImgPreviewItem()
{
    Q_D(DImgPreviewItem);
    delete d->previewThread;
}

void DImgPreviewItem::setDisplayingWidget(QWidget* widget)
{
    Q_D(DImgPreviewItem);
    d->previewThread->setDisplayingWidget(widget);
}

void DImgPreviewItem::setLoadFullImageSize(bool b)
{
    Q_D(DImgPreviewItem);

    if (d->loadFullImageSize == b)
    {
        return;
    }

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
    {
        return;
    }

    d->path = path;

    if (d->path.isNull())
    {
        d->state = NoImage;
        emit stateChanged(d->state);
    }
    else
    {
        d->state = Loading;

        if (d->loadFullImageSize)
        {
            d->previewThread->loadHighQuality(d->path, d->exifRotate);
        }
        else
        {
            d->previewThread->load(d->path, d->previewSize, d->exifRotate);
        }

        emit stateChanged(d->state);
    }
}

void DImgPreviewItem::setPreloadPaths(const QStringList& pathsToPreload)
{
    Q_D(DImgPreviewItem);
    d->pathsToPreload = pathsToPreload;
    preloadNext();
}

static bool approximates(const QSizeF& s1, const QSizeF& s2)
{
    if (s1 == s2)
    {
        return true;
    }

    double widthRatio = s1.width() / s2.width();

    if (widthRatio < 0.98 || widthRatio > 1.02)
    {
        return false;
    }

    double heightRatio = s1.height() / s2.height();

    if (heightRatio < 0.98 || heightRatio > 1.02)
    {
        return false;
    }

    return true;
}

QString DImgPreviewItem::userLoadingHint() const
{
    Q_D(const DImgPreviewItem);

    switch (d->state)
    {
        case NoImage:
        {
            return QString();
        }
        case Loading:
        {
            return i18n("Loading...");
        }
        case ImageLoaded:
        {
            if (d->image.detectedFormat() == DImg::RAW)
            {
                if (d->image.attribute("fromRawEmbeddedPreview").toBool())
                {
                    return i18n("Embedded JPEG Preview");
                }
                else
                {
                    return i18n("Half Size Raw Preview");
                }
            }
            else
            {
                if (approximates(d->image.originalSize(),d->image.size()))
                {
                    return i18n("Full Size Preview");
                }
                else
                {
                    return i18n("Reduced Size Preview");
                }
            }

            return QString();   // To please compiler without warnings.
        }
        default: // ImageLoadingFailed:
        {
            return i18n("Failed to load image");
        }
    }
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
    return (d->state == ImageLoaded);
}

void DImgPreviewItem::slotGotImagePreview(const LoadingDescription& description, const DImg& image)
{
    Q_D(DImgPreviewItem);

    if (description.filePath != d->path || description.isThumbnail())
    {
        return;
    }

    setImage(image);

    if (image.isNull())
    {
        d->state = ImageLoadingFailed;
        emit stateChanged(d->state);
        emit loadingFailed();
    }
    else
    {
        d->state = ImageLoaded;
        emit stateChanged(d->state);
        emit loaded();
    }
}

void DImgPreviewItem::preloadNext()
{
    Q_D(DImgPreviewItem);

    if (!isLoaded() || d->pathsToPreload.isEmpty())
    {
        return;
    }

    QString preloadPath = d->pathsToPreload.takeFirst();

    if (d->loadFullImageSize)
    {
        d->preloadThread->loadHighQuality(preloadPath, d->exifRotate);
    }
    else
    {
        d->preloadThread->load(preloadPath, d->previewSize, d->exifRotate);
    }
}

void DImgPreviewItem::slotFileChanged(const QString& path)
{
    Q_D(DImgPreviewItem);

    if (d->path == path)
    {
        reload();
    }
}

} // namespace Digikam
