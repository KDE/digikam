/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-01
 * Description : a widget to draw a control panel image tool.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepanelwidget.moc"

// Qt includes

#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>

// Local includes

#include "thumbnailsize.h"
#include "imageregionwidget.h"

namespace Digikam
{

class ImagePanelWidgetPriv
{
public:

    ImagePanelWidgetPriv()
    {
        imageRegionWidget = 0;
    }

    QString            settingsSection;

    ImageRegionWidget* imageRegionWidget;
};

ImagePanelWidget::ImagePanelWidget(uint w, uint h, const QString& settingsSection, QWidget* parent)
                : QWidget(parent), d(new ImagePanelWidgetPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    d->settingsSection = settingsSection;

    // -------------------------------------------------------------

    QVBoxLayout* vlay    = new QVBoxLayout(this);
    d->imageRegionWidget = new ImageRegionWidget(w, h, this);
    vlay->addWidget(d->imageRegionWidget, 0);

    // -------------------------------------------------------------

    connect(d->imageRegionWidget, SIGNAL(signalResized()),
            this, SIGNAL(signalResized()));

    connect(d->imageRegionWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SIGNAL(signalOriginalClipFocusChanged()));
}

ImagePanelWidget::~ImagePanelWidget()
{
    writeSettings();
    delete d;
}

ImageRegionWidget* ImagePanelWidget::previewWidget() const
{
    return d->imageRegionWidget;
}

void ImagePanelWidget::readSettings()
{
/*    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->settingsSection);
    int mode                  = group.readEntry("Separate View", (int)ImageRegionWidget::SeparateViewDuplicateVert);
    mode                      = qMax((int)ImageRegionWidget::SeparateViewHorizontal, mode);
    mode                      = qMin((int)ImageRegionWidget::SeparateViewDuplicateHorz, mode);

    d->imageRegionWidget->blockSignals(true);
    d->separateView->blockSignals(true);
    d->imageRegionWidget->slotPreviewModeChanged(mode);
    d->separateView->button(mode)->setChecked(true);
    d->imageRegionWidget->blockSignals(false);
    d->separateView->blockSignals(false);*/
}

void ImagePanelWidget::writeSettings()
{
/*    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->settingsSection);
    group.writeEntry("Separate View", d->separateView->checkedId());
    config->sync();*/
}

void ImagePanelWidget::setHighLightPoints(const QPolygon& pt)
{
    d->imageRegionWidget->setHighLightPoints(pt);
}

QRect ImagePanelWidget::getOriginalImageRegion()
{
    return ( d->imageRegionWidget->getOriginalImageRegion() );
}

QRect ImagePanelWidget::getOriginalImageRegionToRender()
{
    return ( d->imageRegionWidget->getOriginalImageRegionToRender() );
}

DImg ImagePanelWidget::getOriginalRegionImage()
{
    return ( d->imageRegionWidget->getOriginalRegionImage() );
}

void ImagePanelWidget::setPreviewImage(const DImg &img)
{
    d->imageRegionWidget->setPreviewImage(img);
}

void ImagePanelWidget::setCenterImageRegionPosition()
{
    d->imageRegionWidget->setCenterImageRegionPosition();
}

void ImagePanelWidget::ICCSettingsChanged()
{
    d->imageRegionWidget->ICCSettingsChanged();
}

void ImagePanelWidget::exposureSettingsChanged()
{
    d->imageRegionWidget->exposureSettingsChanged();
}

}  // namespace Digikam
