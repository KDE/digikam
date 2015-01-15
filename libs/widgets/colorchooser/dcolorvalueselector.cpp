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

#include "dcolorvalueselector.h"

// Qt includes

#include <QPainter>

// Local includes

#include "dcolorchoosermode_p.h"

namespace Digikam
{

class DColorValueSelector::Private
{
public:

    Private(DColorValueSelector *q): q(q), _hue(0), _sat(0), _colorValue(0), _mode(ChooserClassic) {}

    DColorValueSelector *q;
    int _hue;
    int _sat;
    int _colorValue;
    DColorChooserMode _mode;
    QPixmap pixmap;
};

DColorValueSelector::DColorValueSelector( QWidget *parent )
        : KSelector( Qt::Vertical, parent ), d( new Private( this ) )
{
    setRange( 0, 255 );
}

DColorValueSelector::DColorValueSelector( Qt::Orientation o, QWidget *parent )
        : KSelector( o, parent ), d( new Private( this ) )
{
    setRange( 0, 255 );
}

DColorValueSelector::~DColorValueSelector()
{
    delete d;
}

int DColorValueSelector::hue() const
{
    return d->_hue;
}

void DColorValueSelector::setHue( int hue )
{
    d->_hue = hue;
}

int DColorValueSelector::saturation() const
{
    return d->_sat;
}

void DColorValueSelector::setSaturation( int saturation )
{
    d->_sat = saturation;
}

int DColorValueSelector::colorValue () const
{
    return d->_colorValue;
}

void DColorValueSelector::setColorValue ( int colorValue )
{
    d->_colorValue = colorValue;
}

void DColorValueSelector::updateContents()
{
    drawPalette( &d->pixmap );
}

void DColorValueSelector::resizeEvent( QResizeEvent * )
{
    updateContents();
}

void DColorValueSelector::drawContents( QPainter *painter )
{
    painter->drawPixmap( contentsRect().x(), contentsRect().y(), d->pixmap );
}

void DColorValueSelector::setChooserMode( DColorChooserMode c )
{
    if ( c == ChooserHue ) {
        setRange( 0, 360 );
    } else {
        setRange( 0, 255 );
    }
    d->_mode = c;

    //really needed?
    //emit modeChanged();
}

DColorChooserMode DColorValueSelector::chooserMode () const
{
    return d->_mode;
}

void DColorValueSelector::drawPalette( QPixmap *pixmap )
{
    QColor color;
    if (chooserMode() == ChooserHue) {
        color.setHsv(hue(), 255, 255);
    } else {
        color.setHsv(hue(), saturation(), colorValue());
    }

    QLinearGradient gradient;
    if (orientation() == Qt::Vertical) {
        gradient.setStart(0, contentsRect().height());
        gradient.setFinalStop(0, 0);
    } else {
        gradient.setStart(0, 0);
        gradient.setFinalStop(contentsRect().width(), 0);
    }

    const int steps = componentValueSteps(chooserMode());
    for (int v = 0; v <= steps; ++v) {
        setComponentValue(color, chooserMode(), v * (1.0 / steps));
        gradient.setColorAt(v * (1.0 / steps), color);
    }

    *pixmap = QPixmap(contentsRect().size());
    QPainter painter(pixmap);
    painter.fillRect(pixmap->rect(), gradient);
}

}  // namespace Digikam
