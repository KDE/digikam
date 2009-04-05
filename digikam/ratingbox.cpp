/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-01-07
 * Description : a box to host rating widget in iocn item
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ratingbox.h"
#include "ratingbox.moc"

// Qt includes

#include <QLayout>

// Local includes

#include "ratingwidget.h"

namespace Digikam
{

class RatingBoxPriv
{
public:

    RatingBoxPriv()
    {
        ratingWidget = 0;
    }

    RatingWidget *ratingWidget;
};

RatingBox::RatingBox(QWidget* parent)
         : KHBox(parent), d(new RatingBoxPriv)
{
    d->ratingWidget = new RatingWidget(this);
    d->ratingWidget->setTracking(false);
    layout()->setAlignment(d->ratingWidget, Qt::AlignCenter);
    setMargin(1);
    setSpacing(0);
    setLineWidth(0);
//    setFrameStyle(QFrame::Box|QFrame::Plain);
    setFrameStyle(QFrame::NoFrame);
    hide();

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            this, SIGNAL(signalRatingChanged(int)));
}

RatingBox::~RatingBox()
{
    delete d;
}

void RatingBox::setRating(int val)
{
    d->ratingWidget->blockSignals(true);
    d->ratingWidget->setRating(val);
    d->ratingWidget->blockSignals(false);
}

int RatingBox::rating() const
{
    return d->ratingWidget->rating();
}

}  // namespace Digikam
