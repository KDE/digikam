/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-08
 * Description : Hue/Saturation preview widget
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "hspreviewwidget.moc"

// Qt includes.

#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QResizeEvent>

// KDE includes.

#include <kapplication.h>
#include <klocale.h>

// Local includes.

#include "dimg.h"
#include "hslmodifier.h"

namespace DigikamImagesPluginCore
{

class HSPreviewWidgetPrivate
{

public:

    HSPreviewWidgetPrivate()
    {
        hue = 0.0;
        sat = 0.0;
    }

    int     xBorder;

    double  hue;
    double  sat;

    QPixmap pixmap;
};

HSPreviewWidget::HSPreviewWidget(QWidget *parent, int xBorder)
               : QWidget(parent), d(new HSPreviewWidgetPrivate)
{
    d->xBorder = xBorder;
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

void HSPreviewWidget::resizeEvent(QResizeEvent *)
{
    updatePixmap();
}

void HSPreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(d->xBorder, 0, d->pixmap);
    p.end();
}

void HSPreviewWidget::updatePixmap()
{
    int xSize = width()-2*d->xBorder;
    int ySize = height();

    Digikam::DImg image(xSize, ySize, false, false, 0, false);
    QColor  col;
    uint   *p;

    for ( int s = ySize-1 ; s >= 0 ; s-- )
    {
        p = (uint *)image.scanLine(ySize - s - 1);

        for( int h = 0 ; h < xSize ; h++ )
        {
            col.setHsv( 359*h/(xSize-1), 255, 192 );
            *p = col.rgb();
            p++;
        }
    }

    Digikam::HSLModifier cmod;
    cmod.setHue(d->hue);
    cmod.setSaturation(d->sat);
    cmod.setLightness(0.0);
    cmod.applyHSL(image);

    d->pixmap = image.convertToPixmap();
}

}  // namespace DigikamImagesPluginCore
