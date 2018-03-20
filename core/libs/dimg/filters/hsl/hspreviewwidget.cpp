/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-08
 * Description : Hue/Saturation preview widget
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "hspreviewwidget.h"

// Qt includes

#include <QStyle>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QResizeEvent>

// Local includes

#include "dimg.h"
#include "hslfilter.h"

namespace Digikam
{

class HSPreviewWidget::Private
{

public:

    Private() :
        xBorder(0),
        hue(0.0),
        sat(0.0)
    {
    }

    int     xBorder;

    double  hue;
    double  sat;

    QPixmap pixmap;
};

HSPreviewWidget::HSPreviewWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    d->xBorder = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setAttribute(Qt::WA_DeleteOnClose);
}

HSPreviewWidget::~HSPreviewWidget()
{
    delete d;
}

void HSPreviewWidget::setHS(double hue, double sat)
{
    d->hue = hue;
    d->sat = sat;
    updatePixmap();
    update();
}

void HSPreviewWidget::resizeEvent(QResizeEvent*)
{
    updatePixmap();
}

void HSPreviewWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(d->xBorder, 0, d->pixmap);
    p.end();
}

void HSPreviewWidget::updatePixmap()
{
    int xSize = width() - 2 * d->xBorder;
    int ySize = height();

    DImg   image(xSize, ySize, false, false, 0, false);
    QColor col;
    uint*  p  = 0;

    for (int s = ySize - 1 ; s >= 0 ; --s)
    {
        p = reinterpret_cast<uint*>(image.scanLine(ySize - s - 1));

        for (int h = 0 ; h < xSize ; ++h)
        {
            col.setHsv(359 * h / (xSize - 1), 255, 192);
            *p = col.rgb();
            ++p;
        }
    }

    HSLContainer settings;
    settings.hue        = d->hue;
    settings.saturation = d->sat;
    settings.lightness  = 0.0;
    HSLFilter hsl(&image, 0L, settings);
    hsl.startFilterDirectly();
    image.putImageData(hsl.getTargetImage().bits());

    d->pixmap = image.convertToPixmap();
}

}  // namespace Digikam
