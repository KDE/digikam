/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-07
 * Description : Rubber band for Q3ScrollView
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "drubberband.h"

// Qt includes

#include <Qt3Support/Q3ScrollView>

namespace Digikam
{

class DRubberBand::Private
{
public:

    Private()
    {
        scrollView = 0;
        active     = false;
    }

    Q3ScrollView* scrollView;
    bool          active;
    QPoint        firstPoint;
    QPoint        secondPoint;
    QRect         restriction;
};

DRubberBand::DRubberBand(Q3ScrollView* const scrollView)
    : QRubberBand(QRubberBand::Rectangle, scrollView->viewport()),
      d(new Private)
{
    d->scrollView = scrollView;
    hide();
}

DRubberBand::~DRubberBand()
{
    delete d;
}

QRect DRubberBand::rubberBandAreaOnContents() const
{
    QRect rubber = QRect(d->firstPoint, d->secondPoint);
    rubber       = rubber.normalized();

    if (!d->restriction.isNull())
    {
        rubber = rubber.intersected(d->restriction);
    }

    return rubber;
}

bool DRubberBand::isActive() const
{
    return d->active;
}

void DRubberBand::setActive(bool active)
{
    d->active = active;

    if (d->active)
    {
        show();
    }
    else
    {
        hide();
    }
}

void DRubberBand::setFirstPointOnViewport(const QPoint& p)
{
    d->firstPoint = p;
    d->active     = true;
}

void DRubberBand::setFirstPointOnContents(const QPoint& p)
{
    setFirstPointOnViewport(d->scrollView->contentsToViewport(p));
}

void DRubberBand::setSecondPointOnViewport(const QPoint& p)
{
    d->secondPoint = p;

    updateForContentsPosition(d->scrollView->contentsX(), d->scrollView->contentsY());

    if (d->active)
    {
        show();
    }
}

void DRubberBand::setSecondPointOnContents(const QPoint& p)
{
    setSecondPointOnViewport(d->scrollView->contentsToViewport(p));
}

void DRubberBand::setRestrictionOnContents(const QRect& rect)
{
    d->restriction = rect;
}

void DRubberBand::setRectOnContents(const QRect& rect)
{
    setFirstPointOnContents(rect.topLeft());
    setSecondPointOnContents(rect.bottomRight());
}

void DRubberBand::setRectOnViewport(const QRect& rect)
{
    setFirstPointOnViewport(rect.topLeft());
    setSecondPointOnViewport(rect.bottomRight());
}

void DRubberBand::updateForContentsPosition(int contentsX, int contentsY)
{
    QRect rubber = rubberBandAreaOnContents();
    rubber.translate( - contentsX, - contentsY);

    move(rubber.x(), rubber.y());
    resize(rubber.width(), rubber.height());
}

} // namespace Digikam
