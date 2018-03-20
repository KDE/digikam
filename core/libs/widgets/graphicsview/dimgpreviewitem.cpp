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

#include "dimgpreviewitem.h"
#include "dimgitemspriv.h"

// Qt includes

#include <QApplication>
#include <QDesktopWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "iccsettings.h"
#include "loadingcacheinterface.h"
#include "loadingdescription.h"
#include "previewloadthread.h"
#include "previewsettings.h"

namespace Digikam
{

DImgPreviewItem::DImgPreviewItem(QGraphicsItem* const parent)
    : GraphicsDImgItem(*new DImgPreviewItemPrivate, parent)
{
    Q_D(DImgPreviewItem);
    d->init(this);
}

DImgPreviewItem::DImgPreviewItem(DImgPreviewItemPrivate& dd, QGraphicsItem* const parent)
    : GraphicsDImgItem(dd, parent)
{
    Q_D(DImgPreviewItem);
    d->init(this);
}

DImgPreviewItem::DImgPreviewItemPrivate::DImgPreviewItemPrivate()
{
    state             = DImgPreviewItem::NoImage;
    previewSize       = 1024;
    exifRotate        = false;
    previewThread     = 0;
    preloadThread     = 0;
}

void DImgPreviewItem::DImgPreviewItemPrivate::init(DImgPreviewItem* const q)
{
    previewThread = new PreviewLoadThread;
    preloadThread = new PreviewLoadThread;
    preloadThread->setPriority(QThread::LowPriority);

    QObject::connect(previewThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
                     q, SLOT(slotGotImagePreview(LoadingDescription,DImg)));

    QObject::connect(preloadThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
                     q, SLOT(preloadNext()));

    // get preview size from screen size, but limit from VGA to WQXGA
    previewSize = qBound(640,
                         qMax(QApplication::desktop()->availableGeometry(-1).height(),
                              QApplication::desktop()->availableGeometry(-1).width()),
                         2560);

    LoadingCacheInterface::connectToSignalFileChanged(q, SLOT(slotFileChanged(QString)));

    QObject::connect(IccSettings::instance(), SIGNAL(settingsChanged(ICCSettingsContainer,ICCSettingsContainer)),
                     q, SLOT(iccSettingsChanged(ICCSettingsContainer,ICCSettingsContainer)));
}

DImgPreviewItem::~DImgPreviewItem()
{
    Q_D(DImgPreviewItem);
    delete d->previewThread;
    delete d->preloadThread;
}

void DImgPreviewItem::setDisplayingWidget(QWidget* const widget)
{
    Q_D(DImgPreviewItem);
    d->previewThread->setDisplayingWidget(widget);
}

void DImgPreviewItem::setPreviewSettings(const PreviewSettings& settings)
{
    Q_D(DImgPreviewItem);
    if (settings == d->previewSettings)
    {
        return;
    }
    d->previewSettings = settings;
    reload();
}

QString DImgPreviewItem::path() const
{
    Q_D(const DImgPreviewItem);
    return d->path;
}

void DImgPreviewItem::setPath(const QString& path, bool rePreview)
{
    Q_D(DImgPreviewItem);

    if (path == d->path && !rePreview)
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
        d->previewThread->load(d->path, d->previewSettings, d->previewSize);

        emit stateChanged(d->state);
    }

    d->preloadThread->stopLoading();
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
                if (d->image.attribute(QLatin1String("fromRawEmbeddedPreview")).toBool())
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
                if (approximates(d->image.originalSize(), d->image.size()))
                {
                    //return i18n("Full Size Preview");
                    return QString();
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
            break;
        }
    }

    return i18n("Failed to load image");
}

void DImgPreviewItem::reload()
{
    Q_D(DImgPreviewItem);
    QString path = d->path;
    d->path.clear();
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

    preloadNext();
}

void DImgPreviewItem::preloadNext()
{
    Q_D(DImgPreviewItem);

    if (!isLoaded() || d->pathsToPreload.isEmpty())
    {
        return;
    }

    QString preloadPath = d->pathsToPreload.takeFirst();
    d->preloadThread->load(preloadPath, d->previewSettings, d->previewSize);
}

void DImgPreviewItem::slotFileChanged(const QString& path)
{
    Q_D(DImgPreviewItem);

    if (d->path == path)
    {
        reload();
    }
}

void DImgPreviewItem::iccSettingsChanged(const ICCSettingsContainer& current, const ICCSettingsContainer& previous)
{
    if (current.enableCM != previous.enableCM                     ||
        current.useManagedPreviews != previous.useManagedPreviews ||
        current.monitorProfile != previous.monitorProfile)
    {
        reload();
    }
}

} // namespace Digikam
