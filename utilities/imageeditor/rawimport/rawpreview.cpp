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
#include <QFontMetrics>
#include <QStyleOptionGraphicsItem>

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
#include "dimginterface.h"
#include "graphicsdimgitem.h"
#include "dimgitemspriv.h"
#include "previewlayout.h"

namespace Digikam
{

class RawPreviewItem : public GraphicsDImgItem
{
public:

    RawPreviewItem()
    {
    }

private:

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_D(GraphicsDImgItem);

        QRect   drawRect = option->exposedRect.intersected(boundingRect()).toAlignedRect();
        QRect   pixSourceRect;
        QPixmap pix;
        QSize   completeSize = boundingRect().size().toSize();

        // scale "as if" scaling to whole image, but clip output to our exposed region
        DImg scaledImage     = d->image.smoothScaleClipped(completeSize.width(), completeSize.height(),
                               drawRect.x(), drawRect.y(), drawRect.width(), drawRect.height());

        if (d->cachedPixmaps.find(drawRect, &pix, &pixSourceRect))
        {
            if (pixSourceRect.isNull())
            {
                painter->drawPixmap(drawRect.topLeft(), pix);
            }
            else
            {
                painter->drawPixmap(drawRect.topLeft(), pix, pixSourceRect);
            }
        }
        else
        {
            // Apply CM settings.

            ICCSettingsContainer iccSettings = DImgInterface::defaultInterface()->getICCSettings();

            if (iccSettings.enableCM && iccSettings.useManagedView)
            {
                IccManager manager(scaledImage);
                IccTransform monitorICCtrans = manager.displayTransform(widget);
                pix                          = scaledImage.convertToPixmap(monitorICCtrans);
            }
            else
            {
                pix = scaledImage.convertToPixmap();
            }

            d->cachedPixmaps.insert(drawRect, pix);

            painter->drawPixmap(drawRect.topLeft(), pix);
        }

        // Show the Over/Under exposure pixels indicators

        ExposureSettingsContainer* expoSettings = DImgInterface::defaultInterface()->getExposureSettings();

        if (expoSettings)
        {
            if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
            {
                QImage pureColorMask = scaledImage.pureColorMask(expoSettings);
                QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
                painter->drawPixmap(drawRect.topLeft(), pixMask);
            }
        }
    }
};

// --------------------------------------------------------------------------

class RawPreview::RawPreviewPriv
{
public:

    RawPreviewPriv() :
        currentFitWindowZoom(0.0),
        thread(0),
        item(0)
    {
    }

    double                 currentFitWindowZoom;

    KUrl                   url;

    DImg                   demosaicedImg;

    DRawDecoding           settings;
    ManagedLoadSaveThread* thread;
    LoadingDescription     loadingDesc;
    RawPreviewItem*        item;
};

RawPreview::RawPreview(const KUrl& url, QWidget* parent)
    : GraphicsDImgView(parent), d(new RawPreviewPriv)
{
    d->item = new RawPreviewItem();
    setItem(d->item);

    d->url    = url;
    d->thread = new ManagedLoadSaveThread;
    d->thread->setLoadingPolicy(ManagedLoadSaveThread::LoadingPolicyFirstRemovePrevious);

    // ------------------------------------------------------------

    // set default zoom
    layout()->fitToWindow();

    installPanIcon();

    setMinimumWidth(500);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    // ------------------------------------------------------------

    connect(d->thread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
            this, SLOT(slotImageLoaded(const LoadingDescription&, const DImg&)));

    connect(d->thread, SIGNAL(signalLoadingProgress(const LoadingDescription&, float)),
            this, SLOT(slotLoadingProgress(const LoadingDescription&, float)));
}

RawPreview::~RawPreview()
{
    delete d->item;
    delete d;
}

void RawPreview::setPostProcessedImage(const DImg& image)
{
    d->item->setImage(image);
}

DImg RawPreview::postProcessedImage() const
{
    return d->item->image();
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
    update();
}

void RawPreview::ICCSettingsChanged()
{
    update();
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
        QString msg    = i18n("Cannot decode RAW image\n\"%1\"",
                         QFileInfo(d->loadingDesc.filePath).fileName());
        QFontMetrics fontMt(font());
        QRect fontRect = fontMt.boundingRect(0, 0, width(), height(), 0, msg);
        QPixmap pix(fontRect.size());
        pix.fill(kapp->palette().color(QPalette::Base));
        QPainter p(&pix);
        p.setPen(QPen(kapp->palette().color(QPalette::Text)));
        p.drawText(0, 0, pix.width(), pix.height(), Qt::AlignCenter|Qt::TextWordWrap, msg);
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

int RawPreview::previewWidth()
{
    return d->item->image().width();
}

int RawPreview::previewHeight()
{
    return d->item->image().height();
}

bool RawPreview::previewIsNull()
{
    return d->item->image().isNull();
}

void RawPreview::resetPreview()
{
    d->item->setImage(DImg());
    d->loadingDesc = LoadingDescription();
    update();
}

QImage RawPreview::previewToQImage() const
{
    return d->item->image().copyQImage();
}

}  // namespace Digikam
