/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-08-15
 * Description : a widget to draw stars rating
 * 
 * Copyright (C) 2005 by Owen Hirst <n8rider@sbcglobal.net>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qpainter.h>
#include <qpalette.h>
#include <qpixmap.h>

// KDE includes.

#include <kglobal.h>
#include <kstandarddirs.h>

// Local includes.

#include "constants.h"
#include "themeengine.h"
#include "ratingwidget.h"
#include "ratingwidget.moc"

namespace Digikam
{

class RatingWidgetPriv
{
public:

    RatingWidgetPriv()
    {
        rating = 0;
    }

    int     rating;
    
    QString ratingPixPath;

    QPixmap disPixmap;
    QPixmap selPixmap;
    QPixmap regPixmap;
};

RatingWidget::RatingWidget(QWidget* parent)
            : QWidget(parent)
{
    d = new RatingWidgetPriv;

    KGlobal::dirs()->addResourceType("digikam_rating", 
                     KGlobal::dirs()->kde_default("data") + "digikam/data");
    d->ratingPixPath = KGlobal::dirs()->findResourceDir("digikam_rating", "rating.png");
    d->ratingPixPath.append("/rating.png");

    slotThemeChanged();

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

RatingWidget::~RatingWidget()
{
    delete d;
}

void RatingWidget::setRating(int val)
{
    if (val < RatingMin || val > RatingMax) return;

    d->rating = val;
    emit signalRatingChanged(d->rating);
    update();
}

int RatingWidget::rating() const
{
    return d->rating;
}

int RatingWidget::regPixmapWidth() const
{
    return d->regPixmap.width();
}

void RatingWidget::mouseMoveEvent(QMouseEvent* e)
{
    int pos = e->x() / d->regPixmap.width() +1;

    if (d->rating != pos)
    {
        if (pos >  RatingMax)       // B.K.O.: # 151357
            pos = RatingMax;
        if (pos < RatingMin)
            pos = RatingMin;
        d->rating = pos;
        emit signalRatingChanged(d->rating);
        update();
    }
}

void RatingWidget::mousePressEvent(QMouseEvent* e)
{
    int pos = e->x() / d->regPixmap.width() +1;

    if (d->rating == pos)
    {
        d->rating--;
    }
    else
    {
        d->rating = pos;
    }

    emit signalRatingChanged(d->rating);

    update();
}

void RatingWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    int x = 0;
    
    // Widget is disable : drawing grayed frame.
    if (!isEnabled())
    {
        for (int i=0; i<RatingMax; i++)
        {
            p.drawPixmap(x, 0, d->disPixmap);
            x += d->disPixmap.width();
        }
    }
    else
    {
        for (int i=0; i<d->rating; i++)
        {
            p.drawPixmap(x, 0, d->selPixmap);
            x += d->selPixmap.width();
        }
    
        for (int i=d->rating; i<RatingMax; i++)
        {
            p.drawPixmap(x, 0, d->regPixmap);
            x += d->regPixmap.width();
        }
    }

    p.end();
}

void RatingWidget::slotThemeChanged()
{
    d->regPixmap = QPixmap(d->ratingPixPath);
    d->selPixmap = d->regPixmap;
    d->disPixmap = d->regPixmap;

    QPainter painter(&d->regPixmap);
    painter.fillRect(0, 0, d->regPixmap.width(), d->regPixmap.height(),
                     colorGroup().dark());
    painter.end();

    QPainter painter2(&d->selPixmap);
    painter2.fillRect(0, 0, d->selPixmap.width(), d->selPixmap.height(),
                      ThemeEngine::instance()->textSpecialRegColor());
    painter2.end();
    
    QPainter painter3(&d->disPixmap);
    painter3.fillRect(0, 0, d->disPixmap.width(), d->disPixmap.height(),
                      palette().disabled().foreground());
    painter3.end();

    setFixedSize(QSize(d->regPixmap.width()*5, d->regPixmap.height()));
    update();
}

}  // namespace Digikam
