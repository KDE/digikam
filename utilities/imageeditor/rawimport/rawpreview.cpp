/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-04
 * Description : RAW postProcessedImg widget.
 *
 * Copyright (C) 2008 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rawpreview.h"
#include "rawpreview.moc"

// Qt includes

#include <QString>
#include <QPainter>
#include <QToolButton>
#include <QPixmap>
#include <QFileInfo>
#include <QResizeEvent>

// KDE includes

#include <kcursor.h>
#include <kdatetable.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "paniconwidget.h"
#include "managedloadsavethread.h"
#include "loadingdescription.h"
#include "themeengine.h"

namespace Digikam
{

class RawPreviewPriv
{
public:

    RawPreviewPriv()
    {
        panIconPopup         = 0;
        panIconWidget        = 0;
        cornerButton         = 0;
        thread               = 0;
        url                  = 0;
        currentFitWindowZoom = 0;
    }

    double                 currentFitWindowZoom;

    QToolButton*           cornerButton;

    KPopupFrame*           panIconPopup;
    KUrl                   url;

    PanIconWidget*         panIconWidget;
    DImg                   demosaicedImg;
    DImg                   postProcessedImg;
    DRawDecoding           settings;
    ManagedLoadSaveThread* thread;
    LoadingDescription     loadingDesc;
};

RawPreview::RawPreview(const KUrl& url, QWidget *parent)
          : PreviewWidget(parent), d(new RawPreviewPriv)
{
    d->thread = new ManagedLoadSaveThread;
    d->url    = url;

    setMinimumWidth(500);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->cornerButton = PanIconWidget::button();
    setCornerWidget(d->cornerButton);

    // ------------------------------------------------------------

    connect(d->thread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
            this, SLOT(slotImageLoaded(const LoadingDescription&, const DImg&)));

    connect(d->thread, SIGNAL(signalLoadingProgress(const LoadingDescription&, float)),
            this, SLOT(slotLoadingProgress(const LoadingDescription&, float)));

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));

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
    // Save post processing settings.
    d->settings = settings;

    // All post processing settings will be used after demosaicing.
    DRawDecoding demosaisedSettings = settings;
    demosaisedSettings.resetPostProcessingSettings();

    d->loadingDesc = LoadingDescription(d->url.toLocalFile(), demosaisedSettings);
    d->thread->load(d->loadingDesc, ManagedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
    emit signalLoadingStarted();
}

void RawPreview::cancelLoading()
{
    d->thread->stopLoading(d->loadingDesc);
}

void RawPreview::slotLoadingProgress(const LoadingDescription& description, float progress)
{
    if (description.filePath != d->loadingDesc.filePath)
        return;

    emit signalLoadingProgress(progress);
}

void RawPreview::slotImageLoaded(const LoadingDescription& description, const DImg& image)
{
    if (description.filePath != d->loadingDesc.filePath)
        return;

    if (image.isNull())
    {
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::instance()->baseColor());
        QPainter p(&pix);
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
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
        // NOTE: we will apply all Raw post processing corrections into RawImport class.
    }
}

void RawPreview::slotThemeChanged()
{
    QPalette plt(palette());
    plt.setColor(backgroundRole(), ThemeEngine::instance()->baseColor());
    setPalette(plt);
}

void RawPreview::slotCornerButtonPressed()
{
    if (d->panIconPopup)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
    }

    d->panIconPopup    = new KPopupFrame(this);
    PanIconWidget *pan = new PanIconWidget(d->panIconPopup);
    pan->setImage(180, 120, postProcessedImage());
    d->panIconPopup->setMainWidget(pan);

    QRect r((int)(contentsX()    / zoomFactor()), (int)(contentsY()     / zoomFactor()),
            (int)(visibleWidth() / zoomFactor()), (int)(visibleHeight() / zoomFactor()));
    pan->setRegionSelection(r);
    pan->setMouseFocus();

    connect(pan, SIGNAL(signalSelectionMoved(const QRect&, bool)),
            this, SLOT(slotPanIconSelectionMoved(const QRect&, bool)));

    connect(pan, SIGNAL(signalHidden()),
            this, SLOT(slotPanIconHiden()));

    QPoint g = mapToGlobal(viewport()->pos());
    g.setX(g.x()+ viewport()->size().width());
    g.setY(g.y()+ viewport()->size().height());
    d->panIconPopup->popup(QPoint(g.x() - d->panIconPopup->width(),
                                  g.y() - d->panIconPopup->height()));

    pan->setCursorToLocalRegionSelectionCenter();
}

void RawPreview::slotPanIconHiden()
{
    d->cornerButton->blockSignals(true);
    d->cornerButton->animateClick();
    d->cornerButton->blockSignals(false);
}

void RawPreview::slotPanIconSelectionMoved(const QRect& r, bool b)
{
    setContentsPos((int)(r.x()*zoomFactor()), (int)(r.y()*zoomFactor()));

    if (b)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
        slotPanIconHiden();
    }
}

void RawPreview::zoomFactorChanged(double zoom)
{
    updateScrollBars();

    if (horizontalScrollBar()->isVisible() || verticalScrollBar()->isVisible())
        d->cornerButton->show();
    else
        d->cornerButton->hide();

    PreviewWidget::zoomFactorChanged(zoom);
}

void RawPreview::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    Q3ScrollView::resizeEvent(e);

    if (!d->loadingDesc.filePath.isEmpty())
        d->cornerButton->hide();

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

void RawPreview::paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh)
{
    DImg img     = d->postProcessedImg.smoothScaleSection(sx, sy, sw, sh, tileSize(), tileSize());
    QPixmap pix2 = img.convertToPixmap();
    QPainter p(pix);
    p.drawPixmap(0, 0, pix2);
    p.end();
}

}  // namespace Digikam
