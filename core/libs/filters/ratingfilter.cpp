/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-09
 * Description : a widget to filter album contents by rating
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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

// Qt includes

#include <QLayout>
#include <QToolButton>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dcursortracker.h"
#include "thememanager.h"

namespace Digikam
{

class RatingFilterWidget::Private
{
public:

    Private()
    {
        dirty          = false;
        ratingTracker  = 0;
        filterCond     = ImageFilterSettings::GreaterEqualCondition;
        excludeUnrated = 0;
    }

    bool                                 dirty;

    DCursorTracker*                      ratingTracker;

    ImageFilterSettings::RatingCondition filterCond;
    bool                                 excludeUnrated;
};

RatingFilterWidget::RatingFilterWidget(QWidget* const parent)
    : RatingWidget(parent),
      d(new Private)
{
    d->ratingTracker = new DCursorTracker(QLatin1String(""), this);
    updateRatingTooltip();
    setMouseTracking(true);

    setWhatsThis(i18n("Select the rating value used to filter "
                      "albums' contents. Use the context pop-up menu to "
                      "set rating filter conditions."));

    // To dispatch signal from parent widget with filter condition.
    connect(this, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotRatingChanged()));
}

RatingFilterWidget::~RatingFilterWidget()
{
    delete d;
}

void RatingFilterWidget::slotRatingChanged()
{
    emit signalRatingFilterChanged(rating(), d->filterCond, d->excludeUnrated);
}

void RatingFilterWidget::setRatingFilterCondition(ImageFilterSettings::RatingCondition cond)
{
    d->filterCond = cond;
    updateRatingTooltip();
    slotRatingChanged();
}

ImageFilterSettings::RatingCondition RatingFilterWidget::ratingFilterCondition()
{
    return d->filterCond;
}

void RatingFilterWidget::setExcludeUnratedItems(bool excluded)
{
    d->excludeUnrated = excluded;
    slotRatingChanged();
}

bool RatingFilterWidget::isUnratedItemsExcluded()
{
    return d->excludeUnrated;
}

void RatingFilterWidget::mouseMoveEvent(QMouseEvent* e)
{
    // This method have been re-implemented to display and update the famous TipTracker contents.

    if ( d->dirty )
    {
        int pos = e->x() / regPixmapWidth() +1;

        if (rating() != pos)
        {
            setRating(pos);
        }

        updateRatingTooltip();
    }
}

void RatingFilterWidget::mousePressEvent(QMouseEvent* e)
{
    // This method must be re-implemented to handle which mouse button is pressed
    // and show the rating filter settings pop-up menu with right mouse button.
    // NOTE: Left and Middle Mouse buttons continue to change rating filter value.

    d->dirty = false;

    if ( e->button() == Qt::LeftButton || e->button() == Qt::MidButton )
    {
        d->dirty = true;
        int pos  = e->x() / regPixmapWidth() +1;

        if (rating() == pos)
        {
            setRating(rating()-1);
        }
        else
        {
            setRating(pos);
        }

        updateRatingTooltip();
    }
}

void RatingFilterWidget::mouseReleaseEvent(QMouseEvent*)
{
    d->dirty = false;
}

void RatingFilterWidget::updateRatingTooltip()
{
    // Adapt tip message with rating filter condition settings.

    switch (d->filterCond)
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

// -----------------------------------------------------------------------------------------------

class RatingFilter::Private
{
public:

    Private()
    {
        ratingWidget   = 0;
        optionsBtn     = 0;
        optionsMenu    = 0;
        geCondAction   = 0;
        eqCondAction   = 0;
        leCondAction   = 0;
        excludeUnrated = 0;
    }

    QToolButton*        optionsBtn;

    QAction*            geCondAction;
    QAction*            eqCondAction;
    QAction*            leCondAction;
    QAction*            excludeUnrated;

    QMenu*              optionsMenu;

    RatingFilterWidget* ratingWidget;
};

RatingFilter::RatingFilter(QWidget* const parent)
    : DHBox(parent),
      d(new Private)
{
    d->ratingWidget = new RatingFilterWidget(this);

    d->optionsBtn   = new QToolButton(this);
    d->optionsBtn->setToolTip( i18n("Rating Filter Options"));
    d->optionsBtn->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->optionsBtn->setPopupMode(QToolButton::InstantPopup);

    d->optionsMenu  = new QMenu(d->optionsBtn);
    d->geCondAction = d->optionsMenu->addAction(i18n("Greater Than or Equals Condition"));
    d->geCondAction->setCheckable(true);
    d->eqCondAction = d->optionsMenu->addAction(i18n("Equals Condition"));
    d->eqCondAction->setCheckable(true);
    d->leCondAction = d->optionsMenu->addAction(i18n("Less Than or Equals Condition"));
    d->leCondAction->setCheckable(true);
    d->optionsMenu->addSeparator();
    d->excludeUnrated = d->optionsMenu->addAction(i18n("Exclude Items Without Rating"));
    d->excludeUnrated->setCheckable(true);
    d->optionsBtn->setMenu(d->optionsMenu);

    layout()->setAlignment(d->ratingWidget, Qt::AlignVCenter|Qt::AlignRight);
    setContentsMargins(QMargins());
    setSpacing(0);

    connect(d->optionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOptionsTriggered(QAction*)));

    connect(d->optionsMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotOptionsMenu()));

    connect(d->ratingWidget, SIGNAL(signalRatingFilterChanged(int,ImageFilterSettings::RatingCondition,bool)),
            this, SIGNAL(signalRatingFilterChanged(int,ImageFilterSettings::RatingCondition,bool)));
}

RatingFilter::~RatingFilter()
{
    delete d;
}

void RatingFilter::setRatingFilterCondition(ImageFilterSettings::RatingCondition cond)
{
    d->ratingWidget->setRatingFilterCondition(cond);
}

ImageFilterSettings::RatingCondition RatingFilter::ratingFilterCondition()
{
    return d->ratingWidget->ratingFilterCondition();
}

void RatingFilter::setExcludeUnratedItems(bool excluded)
{
    d->ratingWidget->setExcludeUnratedItems(excluded);
    d->excludeUnrated->setChecked(excluded);
}

bool RatingFilter::isUnratedItemsExcluded()
{
    return d->ratingWidget->isUnratedItemsExcluded();
}

void RatingFilter::slotOptionsMenu()
{
    d->geCondAction->setChecked(false);
    d->eqCondAction->setChecked(false);
    d->leCondAction->setChecked(false);

    switch (ratingFilterCondition())
    {
        case ImageFilterSettings::GreaterEqualCondition:
            d->geCondAction->setChecked(true);
            break;
        case ImageFilterSettings::EqualCondition:
            d->eqCondAction->setChecked(true);
            break;
        case ImageFilterSettings::LessEqualCondition:
            d->leCondAction->setChecked(true);
            break;
    }
}

void RatingFilter::slotOptionsTriggered(QAction* action)
{
    if (action)
    {
        if (action == d->geCondAction)
        {
            setRatingFilterCondition(ImageFilterSettings::GreaterEqualCondition);
        }
        else if (action == d->eqCondAction)
        {
            setRatingFilterCondition(ImageFilterSettings::EqualCondition);
        }
        else if (action == d->leCondAction)
        {
            setRatingFilterCondition(ImageFilterSettings::LessEqualCondition);
        }
        else if(action == d->excludeUnrated)
        {
            setExcludeUnratedItems(d->excludeUnrated->isChecked());
        }
    }
}

void RatingFilter::setRating(int val)
{
    d->ratingWidget->setRating(val);
}

int RatingFilter::rating() const
{
    return d->ratingWidget->rating();
}

}  // namespace Digikam
