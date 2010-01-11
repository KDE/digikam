/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-01
 * Description : a widget to display an image preview with some
 *               modes to compare effect results.
 *
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagewidget.moc"

// Qt includes

#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "previewtoolbar.h"

namespace Digikam
{

class ImageWidgetPriv
{
public:

    ImageWidgetPriv()
    {
        previewWidget  = 0;
    }

    QString             settingsSection;

    ImageGuideWidget*   previewWidget;
};

ImageWidget::ImageWidget(const QString& settingsSection, QWidget* parent,
                         const QString& previewWhatsThis, bool /*prevModeOptions*/,
                         int guideMode, bool guideVisible, bool useImageSelection)
           : QWidget(parent), d(new ImageWidgetPriv)
{
    d->settingsSection = settingsSection;

    QVBoxLayout* vlay = new QVBoxLayout(this);
    d->previewWidget  = new ImageGuideWidget(this, guideVisible,
                                             guideMode, Qt::red, 1, false,
                                             useImageSelection);
    d->previewWidget->setWhatsThis(previewWhatsThis);
    vlay->addWidget(d->previewWidget, 0);
    vlay->setMargin(5);
    vlay->setSpacing(0);
    
    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SIGNAL(signalResized()));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromOriginal(const Digikam::DColor&, const QPoint&)),
            this, SIGNAL(spotPositionChangedFromOriginal(const Digikam::DColor&, const QPoint&)));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint&)),
            this, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint&)));

    // -------------------------------------------------------------
}

ImageWidget::~ImageWidget()
{
    writeSettings();
    delete d;
}

ImageIface* ImageWidget::imageIface()
{
    return d->previewWidget->imageIface();
}

void ImageWidget::updatePreview()
{
    d->previewWidget->updatePreview();
}

void ImageWidget::slotChangeGuideColor(const QColor& color)
{
    d->previewWidget->slotChangeGuideColor(color);
}

void ImageWidget::slotChangeGuideSize(int size)
{
    d->previewWidget->slotChangeGuideSize(size);
}

void ImageWidget::resetSpotPosition()
{
    d->previewWidget->resetSpotPosition();
}

QPoint ImageWidget::getSpotPosition()
{
    return ( d->previewWidget->getSpotPosition() );
}

DColor ImageWidget::getSpotColor(int getColorFrom)
{
    return ( d->previewWidget->getSpotColor(getColorFrom) );
}

void ImageWidget::setSpotVisible(bool spotVisible, bool blink)
{
    d->previewWidget->setSpotVisible(spotVisible, blink);
}

int ImageWidget::getRenderingPreviewMode()
{
}

void ImageWidget::setRenderingPreviewMode(int)
{
}

void ImageWidget::readSettings()
{
}

void ImageWidget::writeSettings()
{
}

void ImageWidget::setPoints(const QPolygon& p, bool drawLine)
{
    d->previewWidget->setPoints(p, drawLine);
}

void ImageWidget::resetPoints()
{
    d->previewWidget->resetPoints();
}

void ImageWidget::setPaintColor(const QColor& color)
{
    d->previewWidget->setPaintColor(color);
}

void ImageWidget::setMaskEnabled(bool enabled)
{
    d->previewWidget->setMaskEnabled(enabled);
}

QImage ImageWidget::getMask() const
{
    return d->previewWidget->getMask();
}

void ImageWidget::setMaskPenSize(int size)
{
    d->previewWidget->setMaskPenSize(size);
}

void ImageWidget::setEraseMode(bool erase)
{
    d->previewWidget->setEraseMode(erase);
}

void ImageWidget::ICCSettingsChanged()
{
    d->previewWidget->ICCSettingsChanged();
}

void ImageWidget::exposureSettingsChanged()
{
    d->previewWidget->exposureSettingsChanged();
}

void ImageWidget::slotPreviewModeChanged(int button)
{
    d->previewWidget->slotPreviewModeChanged(button);
}

}  // namespace Digikam
