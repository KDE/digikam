/* ============================================================
 * Authors: Owen Hirst <n8rider@sbcglobal.net>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2005-08-15
 * Description : a widget to draw stars rating
 * 
 * Copyright 2005 by Owen Hirst
 * Copyright 2006 by Gilles Caulier
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
        rating    = 0;
    }

    int     rating;
    
    QString ratingPixPath;

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
    d->rating = val;
    emit signalRatingChanged(d->rating);
    update();
}

int RatingWidget::rating() const
{
    return d->rating;
}

void RatingWidget::mouseMoveEvent(QMouseEvent* e)
{
    int pos = e->x() / d->regPixmap.width() +1;

    if (d->rating != pos)
    {
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
    for (int i=0; i<d->rating; i++)
    {
        p.drawPixmap(x, 0, d->selPixmap);
        x += d->selPixmap.width();
    }

    for (int i=d->rating; i<5; i++)
    {
        p.drawPixmap(x, 0, d->regPixmap);
        x += d->regPixmap.width();
    }

    p.end();
}

void RatingWidget::slotThemeChanged()
{
    d->regPixmap = QPixmap(d->ratingPixPath);
    d->selPixmap = d->regPixmap;

    QPainter painter(&d->regPixmap);
    painter.fillRect(0, 0, d->regPixmap.width(), d->regPixmap.height(),
                     colorGroup().dark());
    painter.end();

    QPainter painter2(&d->selPixmap);
    painter2.fillRect(0, 0, d->selPixmap.width(), d->selPixmap.height(),
                      ThemeEngine::instance()->textSpecialRegColor());
    painter2.end();
    
    setFixedSize(QSize(d->regPixmap.width()*5, d->regPixmap.height()));
    update();
}

}  // namespace Digikam

