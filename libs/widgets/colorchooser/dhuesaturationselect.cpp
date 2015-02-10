/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-02-20
 * Description : color chooser widgets
 *
 * Copyright (C) 1997 by Martin Jones (mjones at kde dot org)
 * Copyright (C) 2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dhuesaturationselect.h"

// Qt includes

#include <QPainter>

// Local includes

#include "dcolorchoosermode_p.h"

namespace Digikam
{

class DHueSaturationSelector::Private
{
public:

    Private(DHueSaturationSelector* const q)
        : q(q),
          mode(ChooserClassic),
          hue(0),
          saturation(0),
          color(0)
    {
    }

    DHueSaturationSelector* q;
    QPixmap                 pixmap;

    /**
     * Stores the chooser mode
     */
    DColorChooserMode       mode;

    /**
     * Stores the values for hue, saturation and lumniousity
     */
    int                     hue;
    int                     saturation;
    int                     color;
};

DHueSaturationSelector::DHueSaturationSelector(QWidget* const parent)
    : DPointSelect(parent),
      d(new Private(this))
{
    setChooserMode(ChooserClassic);
}

DHueSaturationSelector::~DHueSaturationSelector()
{
    delete d;
}

DColorChooserMode DHueSaturationSelector::chooserMode() const
{
    return d->mode;
}

void DHueSaturationSelector::setChooserMode(DColorChooserMode chooserMode)
{
    int x = 0;
    int y = 255;

    switch (chooserMode)
    {
        case ChooserSaturation:
        case ChooserValue:
            x = 359;
            break;
        default:
            x = 255;
            break;
    }

    setRange(0, 0, x, y);
    d->mode = chooserMode;
}

int DHueSaturationSelector::hue() const
{
    return d->hue;
}

void DHueSaturationSelector::setHue(int hue)
{
    d->hue = hue;
}

int DHueSaturationSelector::saturation() const
{
    return d->saturation;
}

void DHueSaturationSelector::setSaturation(int saturation)
{
    d->saturation = saturation;
}

int DHueSaturationSelector::colorValue() const
{
    return d->color;
}

void DHueSaturationSelector::setColorValue(int color)
{
    d->color = color;
}

void DHueSaturationSelector::updateContents()
{
    drawPalette(&d->pixmap);
}

void DHueSaturationSelector::resizeEvent(QResizeEvent*)
{
    updateContents();
}

void DHueSaturationSelector::drawContents(QPainter* painter)
{
    painter->drawPixmap(contentsRect().x(), contentsRect().y(), d->pixmap);
}

void DHueSaturationSelector::drawPalette(QPixmap* pixmap)
{
    int xSteps = componentXSteps(chooserMode());
    int ySteps = componentYSteps(chooserMode());

    QColor color;
    color.setHsv(hue(), saturation(), chooserMode() == ChooserClassic ? 192 : colorValue());

    QImage image(QSize(xSteps + 1, ySteps + 1), QImage::Format_RGB32);

    for (int y = 0; y <= ySteps; ++y)
    {
        setComponentY(color, chooserMode(), y * (1.0 / ySteps));

        for (int x = 0; x <= xSteps; ++x)
        {
            setComponentX(color, chooserMode(), x * (1.0 / xSteps));
            image.setPixel(x, ySteps - y, color.rgb());
        }
    }

    QPixmap pix(contentsRect().size());
    QPainter painter(&pix);
    // Bilinear filtering
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    QRectF srcRect(0.5, 0.5, xSteps, ySteps);
    QRectF destRect(QPointF(0, 0), contentsRect().size());
    painter.drawImage(destRect, image, srcRect);
    painter.end();

    *pixmap = pix;
}

}  // namespace Digikam
