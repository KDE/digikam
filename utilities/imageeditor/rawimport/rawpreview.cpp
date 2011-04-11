/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-04
 * Description : RAW postProcessedImg widget.
 *
 * Copyright (C) 2008-2011 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rawpreview.moc"

// Qt includes

#include <QString>
#include <QPainter>
#include <QPixmap>
#include <QFileInfo>
#include <QResizeEvent>

// KDE includes

#include <kdebug.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kapplication.h>

// Local includes

#include "managedloadsavethread.h"
#include "loadingdescription.h"
#include "exposurecontainer.h"
#include "iccmanager.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "themeengine.h"
#include "dimginterface.h"

namespace Digikam
{

class RawPreview::RawPreviewPriv
{
public:

    RawPreviewPriv() :
        currentFitWindowZoom(0.0),
        thread(0)
    {
    }

    double                 currentFitWindowZoom;

    KUrl                   url;

    DImg                   demosaicedImg;
    DImg                   postProcessedImg;
    DRawDecoding           settings;
    ManagedLoadSaveThread* thread;
    LoadingDescription     loadingDesc;
};

RawPreview::RawPreview(const KUrl& url, QWidget* parent)
    : PreviewWidget(parent), d(new RawPreviewPriv)
{
    d->thread = new ManagedLoadSaveThread;
    d->thread->setLoadingPolicy(ManagedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
    d->url    = url;

    setMinimumWidth(500);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ------------------------------------------------------------

    connect(d->thread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
            this, SLOT(slotImageLoaded(const LoadingDescription&, const DImg&)));

    connect(d->thread, SIGNAL(signalLoadingProgress(const LoadingDescription&, float)),
            this, SLOT(slotLoadingProgress(const LoadingDescription&, float)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // ------------------------------------------------------------

    slotReset();
}

RawPreview::~RawPreview()
{
    delete d;
}

void RawPreview::setPostProcessedImage(const DImg& image)
{
    d->postProcessedImg = image;

    updateZoomAndSize(false);

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

DImg& RawPreview::postProcessedImage() const
{
    return d->postProcessedImg;
}

DImg& RawPreview::demosaicedImage() const
{
    return d->demosaicedImg;
}

void RawPreview::setDecodingSettings(const DRawDecoding& settings)
{
    if (d->settings == settings && d->thread->isRunning())
    {
        return;
    }

    // Save post processing settings.
    d->settings = settings;

    // All post processing settings will be used after demosaicing.
    DRawDecoding demosaisedSettings = settings;
    demosaisedSettings.resetPostProcessingSettings();

    d->loadingDesc = LoadingDescription(d->url.toLocalFile(), demosaisedSettings);
    d->thread->load(d->loadingDesc, ManagedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
    emit signalLoadingStarted();
}

void RawPreview::exposureSettingsChanged()
{
    clearCache();
    viewport()->update();
}

void RawPreview::ICCSettingsChanged()
{
    clearCache();
    viewport()->update();
}

void RawPreview::cancelLoading()
{
    d->thread->stopLoading(d->loadingDesc);
}

void RawPreview::slotLoadingProgress(const LoadingDescription& description, float progress)
{
    if (description.filePath != d->loadingDesc.filePath)
    {
        return;
    }

    emit signalLoadingProgress(progress);
}

void RawPreview::slotImageLoaded(const LoadingDescription& description, const DImg& image)
{
    if (description.filePath != d->loadingDesc.filePath)
    {
        return;
    }

    if (image.isNull())
    {
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(kapp->palette().color(QPalette::Base));
        QPainter p(&pix);
        p.setPen(QPen(kapp->palette().color(QPalette::Text)));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap,
                   i18n("Cannot decode RAW image for\n\"%1\"",
                        QFileInfo(d->loadingDesc.filePath).fileName()));
        p.end();
        // three copies - but the image is small
        setPostProcessedImage(DImg(pix.toImage()));
        emit signalLoadingFailed();
    }
    else
    {
        d->demosaicedImg = image;
        emit signalDemosaicedImage();
        // NOTE: we will apply all Raw post processing corrections in RawImport class.
    }
}

void RawPreview::slotThemeChanged()
{
    QPalette plt(palette());
    plt.setColor(backgroundRole(), kapp->palette().color(QPalette::Base));
    setPalette(plt);
}

void RawPreview::resizeEvent(QResizeEvent* e)
{
    if (!e)
    {
        return;
    }

    PreviewWidget::resizeEvent(e);

    updateZoomAndSize(false);
}

void RawPreview::updateZoomAndSize(bool alwaysFitToWindow)
{
    // Set zoom for fit-in-window as minimum, but don't scale up images
    // that are smaller than the available space, only scale down.
    double zoom = calcAutoZoomFactor(ZoomInOnly);
    setZoomMin(zoom);
    setZoomMax(zoom*12.0);

    // Is currently the zoom factor set to fit to window? Then set it again to fit the new size.
    if (zoomFactor() < zoom || alwaysFitToWindow || zoomFactor() == d->currentFitWindowZoom)
    {
        setZoomFactor(zoom);
    }

    // store which zoom factor means it is fit to window
    d->currentFitWindowZoom = zoom;

    updateContentsSize();
}

int RawPreview::previewWidth()
{
    return d->postProcessedImg.width();
}

int RawPreview::previewHeight()
{
    return d->postProcessedImg.height();
}

bool RawPreview::previewIsNull()
{
    return d->postProcessedImg.isNull();
}

void RawPreview::resetPreview()
{
    d->postProcessedImg = DImg();
    d->loadingDesc      = LoadingDescription();

    updateZoomAndSize(false);
}

void RawPreview::paintPreview(QPixmap* pix, int sx, int sy, int sw, int sh)
{
    DImg img = d->postProcessedImg.smoothScaleSection(sx, sy, sw, sh, tileSize(), tileSize());

    QPixmap pixImage;

    ICCSettingsContainer iccSettings = DImgInterface::defaultInterface()->getICCSettings();

    if (iccSettings.enableCM && iccSettings.useManagedView)
    {
        IccManager manager(img);
        IccTransform monitorICCtrans = manager.displayTransform(this);
        pixImage                     = img.convertToPixmap(monitorICCtrans);
    }
    else
    {
        pixImage = img.convertToPixmap();
    }

    QPainter p(pix);
    p.drawPixmap(0, 0, pixImage);

    // Show the Over/Under exposure pixels indicators

    ExposureSettingsContainer* expoSettings = DImgInterface::defaultInterface()->getExposureSettings();

    if (expoSettings)
    {
        if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
        {
            QImage pureColorMask = img.pureColorMask(expoSettings);
            QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
            p.drawPixmap(0, 0, pixMask);
        }
    }

    p.end();
}

QImage RawPreview::previewToQImage() const
{
    return d->postProcessedImg.copyQImage();
}

}  // namespace Digikam
