/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow image widget
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slideimage.h"

// Qt includes

#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "previewloadthread.h"

namespace Digikam
{

class SlideImage::Private
{

public:

    Private() :
        deskSize(1024),
        previewThread(0),
        previewPreloadThread(0)
    {
    }

    PreviewSettings     previewSettings;

    int                 deskSize;
    QPixmap             pixmap;

    QUrl                currentImage;

    DImg                preview;
    PreviewLoadThread*  previewThread;
    PreviewLoadThread*  previewPreloadThread;
};

SlideImage::SlideImage(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setWindowFlags(Qt::FramelessWindowHint);
    setMouseTracking(true);

    d->previewThread        = new PreviewLoadThread();
    d->previewPreloadThread = new PreviewLoadThread();

    connect(d->previewThread, SIGNAL(signalImageLoaded(LoadingDescription, DImg)),
            this, SLOT(slotGotImagePreview(LoadingDescription, DImg)));
}

SlideImage::~SlideImage()
{
    delete d->previewThread;
    delete d->previewPreloadThread;
    delete d;
}

void SlideImage::setPreviewSettings(const PreviewSettings& settings)
{
    d->previewSettings = settings;
    // calculate preview size which is used for fast previews
    QSize desktopSize  = QApplication::desktop()->screenGeometry(parentWidget()).size();
    d->deskSize        = qMax(640, qMax(desktopSize.height(), desktopSize.width()));
}

void SlideImage::setLoadUrl(const QUrl& url)
{
    d->currentImage = url;
    d->previewThread->load(url.toLocalFile(), d->previewSettings, d->deskSize);
}

void SlideImage::setPreloadUrl(const QUrl& url)
{
    d->previewPreloadThread->load(url.toLocalFile(), d->previewSettings, d->deskSize);
}

void SlideImage::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, width(), height(), d->pixmap,
                 0, 0, d->pixmap.width(), d->pixmap.height());
    p.end();
}

void SlideImage::slotGotImagePreview(const LoadingDescription& desc, const DImg& preview)
{
    if (desc.filePath != d->currentImage.toLocalFile() || desc.isThumbnail())
    {
        return;
    }

    d->preview.reset();

    if (!DImg::isAnimatedImage(desc.filePath))      // Special case for animated images as GIF or NMG
    {
        d->preview = preview;
    }

    if (!d->preview.isNull())
    {
        updatePixmap();
        update();

        emit signalImageLoaded(true);

        return;
    }

    emit signalImageLoaded(false);
}

void SlideImage::updatePixmap()
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
