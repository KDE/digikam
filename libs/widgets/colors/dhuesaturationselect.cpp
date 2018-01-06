/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-02-20
 * Description : color chooser widgets
 *
 * Copyright (C)      1997 by Martin Jones (mjones at kde dot org)
 * Copyright (C) 2015-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QStyle>
#include <QPainter>
#include <QStyleOptionFrame>
#include <QMouseEvent>

// Local includes

#include "digikam_debug.h"
#include "dcolorchoosermode_p.h"

namespace Digikam
{

class DPointSelect::Private
{
public:

    Private(DPointSelect* const q):
        q(q),
        px(0),
        py(0),
        xPos(0),
        yPos(0),
        minX(0),
        maxX(100),
        minY(0),
        maxY(100),
        m_markerColor(Qt::white)
    {
    }

    void setValues(int _xPos, int _yPos);

public:

    DPointSelect* q;
    int           px;
    int           py;
    int           xPos;
    int           yPos;
    int           minX;
    int           maxX;
    int           minY;
    int           maxY;
    QColor        m_markerColor;
};

void DPointSelect::Private::setValues(int _xPos, int _yPos)
{
    int w = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    xPos  = _xPos;
    yPos  = _yPos;

    if ( xPos > maxX )
        xPos = maxX;
    else if ( xPos < minX )
        xPos = minX;

    if ( yPos > maxY )
        yPos = maxY;
    else if ( yPos < minY )
        yPos = minY;

    Q_ASSERT(maxX != minX);
    int xp = w + (q->width() - 2 * w) * xPos / (maxX - minX);

    Q_ASSERT(maxY != minY);
    int yp = q->height() - w - (q->height() - 2 * w) * yPos / (maxY - minY);

    q->setPosition( xp, yp );
}

DPointSelect::DPointSelect(QWidget* const parent)
    : QWidget(parent),
      d(new Private(this))
{
}

DPointSelect::~DPointSelect()
{
    delete d;
}

int DPointSelect::xValue() const
{
    return d->xPos;
}

int DPointSelect::yValue() const
{
    return d->yPos;
}

void DPointSelect::setRange(int _minX, int _minY, int _maxX, int _maxY)
{
    if (_maxX == _minX)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "DPointSelect::setRange invalid range: " << _maxX << " == " << _minX << " (for X) ";
        return;
    }

    if (_maxY == _minY)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "DPointSelect::setRange invalid range: " << _maxY << " == " << _minY << " (for Y) ";
        return;
    }

    int w   = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    d->px   = w;
    d->py   = w;
    d->minX = _minX;
    d->minY = _minY;
    d->maxX = _maxX;
    d->maxY = _maxY;
}

void DPointSelect::setXValue(int _xPos)
{
    setValues(_xPos, d->yPos);
}

void DPointSelect::setYValue(int _yPos)
{
    setValues(d->xPos, _yPos);
}

void DPointSelect::setValues(int _xPos, int _yPos)
{
    d->setValues(_xPos, _yPos);
}

void DPointSelect::setMarkerColor( const QColor &col )
{
    d->m_markerColor =  col;
}

QRect DPointSelect::contentsRect() const
{
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    return rect().adjusted(w, w, -w, -w);
}

QSize DPointSelect::minimumSizeHint() const
{
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    return QSize( 2 * w, 2 * w );
}

void DPointSelect::paintEvent( QPaintEvent * /* ev */ )
{
    QStyleOptionFrame opt;
    opt.initFrom(this);

    QPainter painter;
    painter.begin(this);

    drawContents(&painter);
    drawMarker(&painter, d->px, d->py);

    style()->drawPrimitive(QStyle::PE_Frame, &opt, &painter, this);

    painter.end();
}

void DPointSelect::mousePressEvent(QMouseEvent* e)
{
    mouseMoveEvent(e);
}

void DPointSelect::mouseMoveEvent(QMouseEvent* e)
{
    int xVal, yVal;
    int w = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
    valuesFromPosition( e->pos().x() - w, e->pos().y() - w, xVal, yVal );
    setValues( xVal, yVal );

    emit valueChanged( d->xPos, d->yPos );
}

void DPointSelect::wheelEvent(QWheelEvent* e)
{
    if ( e->orientation() == Qt::Horizontal )
        setValues( xValue() + e->delta()/120, yValue() );
    else
        setValues( xValue(), yValue() + e->delta()/120 );

    emit valueChanged( d->xPos, d->yPos );
}

void DPointSelect::valuesFromPosition(int x, int y, int& xVal, int& yVal) const
{
    int w = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
    xVal  = ( ( d->maxX - d->minX ) * ( x - w ) ) / ( width() - 2 * w );
    yVal  = d->maxY - ( ( ( d->maxY - d->minY ) * (y - w) ) / ( height() - 2 * w ) );

    if ( xVal > d->maxX )
        xVal = d->maxX;
    else if ( xVal < d->minX )
        xVal = d->minX;

    if ( yVal > d->maxY )
        yVal = d->maxY;
    else if ( yVal < d->minY )
        yVal = d->minY;
}

void DPointSelect::setPosition(int xp, int yp)
{
    int w = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );

    if ( xp < w )
        xp = w;
    else if ( xp > width() - w )
        xp = width() - w;

    if ( yp < w )
        yp = w;
    else if ( yp > height() - w )
        yp = height() - w;

    d->px = xp;
    d->py = yp;

    update();
}

void DPointSelect::drawMarker(QPainter* p, int xp, int yp)
{
    QPen pen(d->m_markerColor);
    p->setPen(pen);
    p->drawEllipse(xp - 4, yp - 4, 8, 8);
}

// --------------------------------------------------------------------------------------------------------

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
