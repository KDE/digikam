/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-04
 * Description : RAW postProcessedImg widget.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QString>
#include <QPainter>
#include <QPixmap>
#include <QFileInfo>
#include <QResizeEvent>
#include <QFontMetrics>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "managedloadsavethread.h"
#include "loadingdescription.h"
#include "editorcore.h"
#include "previewlayout.h"
#include "imagepreviewitem.h"

namespace Digikam
{

class RawPreview::Private
{
public:

    Private() :
        currentFitWindowZoom(0.0),
        thread(0),
        item(0)
    {
    }

    double                 currentFitWindowZoom;

    QUrl                   url;

    DImg                   demosaicedImg;

    DRawDecoding           settings;
    ManagedLoadSaveThread* thread;
    LoadingDescription     loadingDesc;
    ImagePreviewItem*      item;
};

RawPreview::RawPreview(const QUrl& url, QWidget* const parent)
    : GraphicsDImgView(parent),
      d(new Private)
{
    d->item   = new ImagePreviewItem();
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

    connect(d->thread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotImageLoaded(LoadingDescription,DImg)));

    connect(d->thread, SIGNAL(signalLoadingProgress(LoadingDescription,float)),
            this, SLOT(slotLoadingProgress(LoadingDescription,float)));
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
    d->settings                     = settings;

    // All post processing settings will be used after demosaicing.
    DRawDecoding demosaisedSettings = settings;
    demosaisedSettings.resetPostProcessingSettings();

    d->loadingDesc                  = LoadingDescription(d->url.toLocalFile(), demosaisedSettings);
    d->thread->load(d->loadingDesc, ManagedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
    emit signalLoadingStarted();
}

void RawPreview::exposureSettingsChanged()
{
    viewport()->update();
}

void RawPreview::ICCSettingsChanged()
{
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
        QString msg    = i18n("Cannot decode RAW image\n\"%1\"",
                              QFileInfo(d->loadingDesc.filePath).fileName());
        QFontMetrics fontMt(font());
        QRect fontRect = fontMt.boundingRect(0, 0, width(), height(), 0, msg);
        QPixmap pix(fontRect.size());
        pix.fill(qApp->palette().color(QPalette::Base));
        QPainter p(&pix);
        p.setPen(QPen(qApp->palette().color(QPalette::Text)));
        p.drawText(0, 0, pix.width(), pix.height(), Qt::AlignCenter | Qt::TextWordWrap, msg);
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

int RawPreview::previewWidth() const
{
    return d->item->image().width();
}

int RawPreview::previewHeight() const
{
    return d->item->image().height();
}

bool RawPreview::previewIsNull() const
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

} // namespace Digikam
