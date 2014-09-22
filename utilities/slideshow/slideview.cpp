/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow image widget
 *
 * Copyright (C) 2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slideview.moc"

// Qt includes

#include <QPainter>

// KDE includes

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kapplication.h>

// Local includes

#include "dimg.h"
#include "previewloadthread.h"

namespace Digikam
{

class SlideView::Private
{

public:

    Private() :
        useFullSizePreviews(false),
        previewThread(0),
        previewPreloadThread(0)
    {
    }

    bool                useFullSizePreviews;

    QSize               deskSize;
    QPixmap             pixmap;

    KUrl                currentImage;

    DImg                preview;
    PreviewLoadThread*  previewThread;
    PreviewLoadThread*  previewPreloadThread;
};

SlideView::SlideView(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setWindowFlags(Qt::FramelessWindowHint);
    setMouseTracking(true);

    d->deskSize             = KGlobalSettings::desktopGeometry(kapp->activeWindow()).size();
    d->previewThread        = new PreviewLoadThread();
    d->previewPreloadThread = new PreviewLoadThread();

    connect(d->previewThread, SIGNAL(signalImageLoaded(LoadingDescription, DImg)),
            this, SLOT(slotGotImagePreview(LoadingDescription, DImg)));
}

SlideView::~SlideView()
{
    delete d->previewThread;
    delete d->previewPreloadThread;
    delete d;
}

void SlideView::setLoadFullImageSize(bool b)
{
    d->useFullSizePreviews = b;
}

void SlideView::setLoadUrl(const KUrl& url)
{
    d->currentImage = url;

    if (d->useFullSizePreviews)
    {
        d->previewThread->loadHighQuality(url.toLocalFile());
    }
    else
    {
        d->previewThread->load(url.toLocalFile(), qMax(d->deskSize.width(), d->deskSize.height()));
    }
}

void SlideView::setPreloadUrl(const KUrl& url)
{
    if (d->useFullSizePreviews)
    {
        d->previewPreloadThread->loadHighQuality(url.toLocalFile());
    }
    else
    {
        d->previewPreloadThread->load(url.toLocalFile(), qMax(d->deskSize.width(), d->deskSize.height()));
    }
}

void SlideView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, width(), height(), d->pixmap,
                 0, 0, d->pixmap.width(), d->pixmap.height());
    p.end();
}

void SlideView::slotGotImagePreview(const LoadingDescription& desc, const DImg& preview)
{
    if (desc.filePath != d->currentImage.toLocalFile() || desc.isThumbnail())
    {
        return;
    }

    d->preview = preview;

    if (!d->preview.isNull())
    {
        updatePixmap();
        update();

        emit signalImageLoaded(true);

        return;
    }

    emit signalImageLoaded(false);
}

void SlideView::updatePixmap()
{
    /* For high resolution ("retina") displays, Mac OS X / Qt
        report only half of the physical resolution in terms of
        pixels, i.e. every logical pixels corresponds to 2x2
        physical pixels. However, UI elements and fonts are
        nevertheless rendered at full resolution, and pixmaps
        as well, provided their resolution is high enough (that
        is, higher than the reported, logical resolution).

        To work around this, we render the photos not a logical
        resolution, but with the photo's full resolution, but
        at the screen's aspect ratio. When we later draw this
        high resolution bitmap, it is up to Qt to scale the
        photo to the true physical resolution.  The ratio
        computed below is the ratio between the photo and
        screen resolutions, or equivalently the factor by which
        we need to increase the pixel size of the rendered
        pixmap.
    */
#ifdef USE_QT_SCALING
    double xratio  = double(d->preview.width())  / width();
    double yratio  = double(d->preview.height()) / height();
    double ratio   = qMax(qMin(xratio, yratio), 1.0);
#else
    double ratio   = 1.0;
#endif
    QSize fullSize = QSizeF(ratio*width(), ratio*height()).toSize();
    d->pixmap      = QPixmap(fullSize);
    d->pixmap.fill(Qt::black);
    QPainter p(&(d->pixmap));

    QPixmap pix(d->preview.smoothScale(d->pixmap.width(), d->pixmap.height(), Qt::KeepAspectRatio).convertToPixmap());
    p.drawPixmap((d->pixmap.width()  - pix.width())  / 2,
                 (d->pixmap.height() - pix.height()) / 2, pix,
                 0, 0, pix.width(), pix.height());
}

}  // namespace Digikam
