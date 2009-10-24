/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-09
 * Description : a widget to filter album contents by rating
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007 by Arnd Baecker <arnd dot baecker at web dot de>
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

#include "ratingfilter.h"
#include "ratingfilter.moc"

// Qt includes

#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QCursor>

// KDE includes

#include <kmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// Local includes

#include "debug.h"
#include "dcursortracker.h"
#include "themeengine.h"

namespace Digikam
{

class RatingFilterPriv
{
public:

    RatingFilterPriv()
    {
        dirty         = false;
        ratingTracker = 0;
        filterCond    = ImageFilterSettings::GreaterEqualCondition;
    }

    bool                          dirty;

    DTipTracker                  *ratingTracker;

    ImageFilterSettings::
        RatingCondition           filterCond;
};

RatingFilter::RatingFilter(QWidget* parent)
            : RatingWidget(parent), d(new RatingFilterPriv)
{
    d->ratingTracker = new DTipTracker("", this);
    updateRatingTooltip();
    setMouseTracking(true);

    setWhatsThis(i18n("Select the rating value used to filter "
                      "albums contents. Use the context pop-up menu to "
                      "set rating filter conditions."));

    // To dispatch signal from parent widget with filter condition.
    connect(this, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotRatingChanged()));
}

RatingFilter::~RatingFilter()
{
    delete d;
}

void RatingFilter::slotRatingChanged()
{
    emit signalRatingFilterChanged(rating(), d->filterCond);
}

void RatingFilter::setRatingFilterCondition(ImageFilterSettings::RatingCondition cond)
{
    d->filterCond = cond;
    updateRatingTooltip();
    slotRatingChanged();
}

ImageFilterSettings::RatingCondition RatingFilter::ratingFilterCondition()
{
    return d->filterCond;
}

void RatingFilter::mouseMoveEvent(QMouseEvent* e)
{
    // This method have been re-implemented to display and update the famous TipTracker contents.

    if ( d->dirty )
    {
        int pos = e->x() / regPixmapWidth() +1;

        if (rating() != pos)
            setRating(pos);

        updateRatingTooltip();
    }
}

void RatingFilter::mousePressEvent(QMouseEvent* e)
{
    // This method must be re-implemented to handle which mouse button is pressed
    // and show the rating filter settings pop-up menu with right mouse button.
    // NOTE: Left and Middle Mouse buttons continue to change rating filter value.

    d->dirty = false;

    if ( e->button() == Qt::LeftButton || e->button() == Qt::MidButton )
    {
        d->dirty   = true;
        int pos = e->x() / regPixmapWidth() +1;

        if (rating() == pos)
            setRating(rating()-1);
        else
            setRating(pos);
        updateRatingTooltip();
    }
    else if (e->button() == Qt::RightButton)
    {
        // Show pop-up menu about Rating Filter condition settings

        KMenu popmenu(this);
        popmenu.addTitle(SmallIcon("digikam"), i18n("Rating Filter"));
        QAction *geCondAction = popmenu.addAction(i18n("Greater Than or Equals Condition"));
        geCondAction->setCheckable(true);
        QAction *eqCondAction = popmenu.addAction(i18n("Equals Condition"));
        eqCondAction->setCheckable(true);
        QAction *leCondAction = popmenu.addAction(i18n("Less Than or Equals Condition"));
        leCondAction->setCheckable(true);

        switch(d->filterCond)
        {
            case ImageFilterSettings::GreaterEqualCondition:
                geCondAction->setChecked(true);
                break;
            case ImageFilterSettings::EqualCondition:
                eqCondAction->setChecked(true);
                break;
            case ImageFilterSettings::LessEqualCondition:
                leCondAction->setChecked(true);
                break;
        }

        QAction *choice = popmenu.exec(QCursor::pos());
        if (choice)
        {
            if (choice == geCondAction)
            {
                setRatingFilterCondition(ImageFilterSettings::GreaterEqualCondition);
            }
            else if (choice == eqCondAction)
            {
                setRatingFilterCondition(ImageFilterSettings::EqualCondition);
            }
            else if (choice == leCondAction)
            {
                setRatingFilterCondition(ImageFilterSettings::LessEqualCondition);
            }
        }
    }
}

void RatingFilter::mouseReleaseEvent(QMouseEvent*)
{
    d->dirty = false;
}

void RatingFilter::updateRatingTooltip()
{
    // Adapt tip message with rating filter condition settings.

    switch(d->filterCond)
    {
        case ImageFilterSettings::GreaterEqualCondition:
        {
            d->ratingTracker->setText(i18n("Rating greater than or equal to %1.", rating()));
            break;
        }
        case ImageFilterSettings::EqualCondition:
        {
            d->ratingTracker->setText(i18n("Rating equal to %1.", rating()));
            break;
        }
        case ImageFilterSettings::LessEqualCondition:
        {
            d->ratingTracker->setText( i18n("Rating less than or equal to %1.", rating()));
            break;
        }
        default:
            break;
    }
}

}  // namespace Digikam
