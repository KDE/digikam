/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-10
 * Description : meta-filter to apply FilterAction
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "filteractionfilter.h"

// Qt includes

#include <QScopedPointer>

// Local includes

#include "digikam_export.h"
#include "dimgbuiltinfilter.h"
#include "dimgfiltermanager.h"
#include "filteraction.h"

namespace Digikam
{

class FilterActionFilter::FilterActionFilterPriv
{
public:

    FilterActionFilterPriv()
    {
        appliedActions = 0;
    }

    QList<FilterAction> actions;

    int     appliedActions;
    QString errorMessage;
};


FilterActionFilter::FilterActionFilter(QObject *parent)
            : DImgThreadedFilter(parent),
              d(new FilterActionFilterPriv)
{
    initFilter();
}

FilterActionFilter::~FilterActionFilter()
{
    delete d;
}

void FilterActionFilter::setFilterActions(const QList<FilterAction>& actions)
{
    d->actions = actions;
}

void FilterActionFilter::addFilterActions(const QList<FilterAction>& actions)
{
    d->actions += actions;
}

void FilterActionFilter::setFilterAction(const FilterAction& action)
{
    d->actions.clear();
    d->actions << action;
}

void FilterActionFilter::addFilterAction(const FilterAction& action)
{
    d->actions << action;
}

QList<FilterAction> FilterActionFilter::filterActions() const
{
    return d->actions;
}

bool FilterActionFilter::isReproducible() const
{
    foreach (const FilterAction& action, d->actions)
        if (action.category() != FilterAction::ReproducibleFilter)
            return false;
    return true;
}

bool FilterActionFilter::isComplexAction() const
{
    foreach (const FilterAction& action, d->actions)
        if (action.category() != FilterAction::ReproducibleFilter
            && action.category() != FilterAction::ComplexFilter)
            return false;
    return true;
}

bool FilterActionFilter::isSupported() const
{
    foreach (const FilterAction& action, d->actions)
        if (!DImgFilterManager::instance()->isSupported(action.identifier(), action.version()))
            return false;
    return true;
}

bool FilterActionFilter::completelyApplied() const
{
    return d->appliedActions == d->actions.size();
}

QList<FilterAction> FilterActionFilter::appliedActions()
{
    if (completelyApplied())
        return filterActions();

    return d->actions.mid(0, d->appliedActions);
}

FilterAction FilterActionFilter::failedAction() const
{
    if (completelyApplied() || d->appliedActions >= d->actions.size())
        return FilterAction();
    return d->actions[d->appliedActions];
}

int FilterActionFilter::failedActionIndex() const
{
    return d->appliedActions;
}

QString FilterActionFilter::failedActionMessage() const
{
    return d->errorMessage;
}

void FilterActionFilter::filterImage()
{
    d->appliedActions = 0;
    d->errorMessage   = QString();
    const float progressIncrement = 1.0 / qMax(1,d->actions.size());
    float progress = 0;

    postProgress(0);

    m_destImage = m_orgImage;

    foreach (const FilterAction& action, d->actions)
    {
        if (DImgBuiltinFilter::isSupported(action.identifier()))
        {
            DImgBuiltinFilter filter(action);
            if (!filter.isValid())
            {
                d->errorMessage = i18n("Built-in transformation not supported");
                break;
            }
            filter.apply(m_destImage);
        }
        else
        {
            QScopedPointer<DImgThreadedFilter> filter
                (DImgFilterManager::instance()->createFilter(action.identifier(), action.version()));

            if (!filter)
            {
                d->errorMessage = i18n("Filter identifier or version is not supported");
                break;
            }
            filter->initSlave(this, (int)progress, (int)(progress+progressIncrement));
            filter->readParameters(action);
            if (!filter->parametersSuccessfullyRead())
            {
                d->errorMessage = filter->readParametersError(action);
                break;
            }

            // compute
            filter->setupFilter(m_destImage);

            m_destImage = filter->getTargetImage();
        }

        d->appliedActions++;
        progress += progressIncrement;
        postProgress((int)progress);
    }
}

} // namespace Digikam


