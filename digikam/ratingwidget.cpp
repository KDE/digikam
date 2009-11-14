/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-08-15
 * Description : a widget to draw stars rating
 *
 * Copyright (C) 2005 by Owen Hirst <n8rider@sbcglobal.net>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ratingwidget.h"
#include "ratingwidget.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QTimeLine>
#include <QPolygon>

// KDE includes

#include <kdebug.h>

// Local includes

#include "globals.h"
#include "themeengine.h"

namespace Digikam
{

class RatingWidgetPriv
{
public:

    RatingWidgetPriv()
    {
        tracking       = true;
        isHovered      = false;
        fading         = false;
        rating         = 0;
        fadingTimeLine = 0;
        fadingValue    = 0;
        offset         = 0;
        duration       = 600;   // ms

        // Pre-computed star polygon for a 15x15 pixmap.
        starPolygon << QPoint(0,  6);
        starPolygon << QPoint(5,  5);
        starPolygon << QPoint(7,  0);
        starPolygon << QPoint(9,  5);
        starPolygon << QPoint(14, 6);
        starPolygon << QPoint(10, 9);
        starPolygon << QPoint(11, 14);
        starPolygon << QPoint(7,  11);
        starPolygon << QPoint(3,  14);
        starPolygon << QPoint(4,  9);
    }

    bool          tracking;
    bool          isHovered;
    bool          fading;

    int           rating;
    int           fadingValue;
    int           duration;
    int           offset;

    QTimeLine    *fadingTimeLine;

    QPolygon      starPolygon;

    QPixmap       selPixmap;      // Selected star.
    QPixmap       regPixmap;      // Regular star.
    QPixmap       disPixmap;      // Disable star.
};

RatingWidget::RatingWidget(QWidget* parent)
            : QWidget(parent), d(new RatingWidgetPriv)
{
    slotThemeChanged();

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

RatingWidget::~RatingWidget()
{
    delete d;
}

void RatingWidget::setupTimeLine()
{
    if (d->fadingTimeLine)
        delete d->fadingTimeLine;

    d->fadingTimeLine = new QTimeLine(d->duration, this);
    d->fadingTimeLine->setFrameRange(0, 255);

    connect(d->fadingTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFadingValue(int)));

    d->fadingTimeLine->start();
}

int RatingWidget::regPixmapWidth() const
{
    return d->regPixmap.width();
}

void RatingWidget::setRating(int val)
{
    if ((val < RatingMin || val > RatingMax) && val != NoRating) return;

    d->rating = val;
    if (d->tracking)
        emit signalRatingChanged(d->rating);
    update();
}

int RatingWidget::rating() const
{
    return d->rating;
}

void RatingWidget::setTracking(bool tracking)
{
    d->tracking = tracking;
}

bool RatingWidget::hasTracking() const
{
    return d->tracking;
}

void RatingWidget::setFading(bool fading)
{
    d->fading = fading;
}

bool RatingWidget::hasFading() const
{
    return d->fading;
}

void RatingWidget::setFadingValue(int value)
{
    d->fadingValue = value;
    if (d->fadingValue >= 255)
    {
        d->fadingTimeLine->stop();
    }
    update();
}

void RatingWidget::setVisible(bool visible)
{
    QWidget::setVisible(visible);

    if (visible)
        startFading();
    else
        stopFading();
}

void RatingWidget::startFading()
{
    if (!hasFading()) return;

    if (!d->isHovered)
    {
        d->isHovered   = true;
        d->fadingValue = 0;
        setupTimeLine();
    }
}

void RatingWidget::stopFading()
{
    if (!hasFading()) return;

    if (d->fadingTimeLine)
        d->fadingTimeLine->stop();

    d->isHovered   = false;
    d->fadingValue = 0;
    update();
}

QPixmap RatingWidget::starPixmapDisabled() const
{
    return d->disPixmap;
}

QPixmap RatingWidget::starPixmapFilled() const
{
    return d->selPixmap;
}

QPixmap RatingWidget::starPixmap() const
{
    return d->regPixmap;
}

void RatingWidget::regeneratePixmaps()
{
    slotThemeChanged();
}

void RatingWidget::mousePressEvent(QMouseEvent* e)
{
    if (hasFading() && d->fadingValue < 255) return;

    int pos = (e->x() - d->offset) / d->regPixmap.width() +1;

    if (d->rating == pos)
    {
        d->rating--;
    }
    else
    {
        d->rating = pos;
    }

    if (d->rating > RatingMax)
        d->rating = RatingMax;
    if (d->rating < RatingMin)
        d->rating = RatingMin;

    if (d->tracking)
        emit signalRatingChanged(d->rating);

    update();
}

void RatingWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (hasFading() && d->fadingValue < 255) return;

    int pos = (e->x() - d->offset) / d->regPixmap.width() +1;

    if (d->rating != pos)
    {
        if (pos > RatingMax)       // B.K.O.: # 151357
            pos = RatingMax;
        if (pos < RatingMin)
            pos = RatingMin;

        d->rating = pos;

        if (d->tracking)
            emit signalRatingChanged(d->rating);

        update();
    }
}

void RatingWidget::mouseReleaseEvent(QMouseEvent*)
{
    if (hasFading() && d->fadingValue < 255) return;

    emit signalRatingChanged(d->rating);
}

void RatingWidget::slotThemeChanged()
{
    d->regPixmap = QPixmap(15, 15);
    d->regPixmap.fill(Qt::transparent);
    d->selPixmap = QPixmap(15, 15);
    d->selPixmap.fill(Qt::transparent);
    d->disPixmap = QPixmap(15, 15);
    d->disPixmap.fill(Qt::transparent);

    QPainter p1(&d->regPixmap);
    p1.setRenderHint(QPainter::Antialiasing, true);
    p1.setBrush(palette().color(QPalette::Active, backgroundRole()));
    p1.setPen(palette().color(QPalette::Active, foregroundRole()));
    p1.drawPolygon(d->starPolygon, Qt::WindingFill);
    p1.end();

    QPainter p2(&d->selPixmap);
    p2.setRenderHint(QPainter::Antialiasing, true);
    p2.setBrush(ThemeEngine::instance()->textSpecialRegColor());
    p2.setPen(palette().color(QPalette::Active, foregroundRole()));
    p2.drawPolygon(d->starPolygon, Qt::WindingFill);
    p2.end();

    QPainter p3(&d->disPixmap);
    p3.setRenderHint(QPainter::Antialiasing, true);
    p3.setBrush(palette().color(QPalette::Disabled, backgroundRole()));
    p3.setPen(palette().color(QPalette::Disabled, foregroundRole()));
    p3.drawPolygon(d->starPolygon, Qt::WindingFill);
    p3.end();

    setMinimumSize(QSize((d->regPixmap.width()+1)*RatingMax, d->regPixmap.height()));
    update();
}

void RatingWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    d->offset = (width() - RatingMax * (d->disPixmap.width()+1)) / 2;

    // Widget is disable : drawing grayed frame.
    if (!isEnabled())
    {
        int x = d->offset;
        for (int i = 0; i < RatingMax; ++i)
        {
            p.drawPixmap(x, 0, d->disPixmap);
            x += d->disPixmap.width()+1;
        }
    }
    else
    {
        int x       = d->offset;
        int rate    = d->rating != NoRating ? d->rating : 0;
        QPixmap sel = d->selPixmap;
        applyFading(sel);
        for (int i = 0; i < rate; ++i)
        {
            p.drawPixmap(x, 0, sel);
            x += sel.width()+1;
        }

        QPixmap reg = d->regPixmap;
        applyFading(reg);
        for (int i = rate; i < RatingMax; ++i)
        {
            p.drawPixmap(x, 0, reg);
            x += reg.width()+1;
        }
    }
    p.end();
}

void RatingWidget::applyFading(QPixmap& pix)
{
    if (hasFading() && d->fadingValue < 255)
    {
        QPixmap alphaMask(pix.width(), pix.height());
        const QColor color(d->fadingValue, d->fadingValue, d->fadingValue);
        alphaMask.fill(color);
        pix.setAlphaChannel(alphaMask);
    }
}

}  // namespace Digikam
