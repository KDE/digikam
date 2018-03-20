/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-10
 * Description : meta-filter to apply FilterAction
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_export.h"
#include "dimgbuiltinfilter.h"
#include "dimgfiltermanager.h"
#include "filteraction.h"

namespace Digikam
{

class FilterActionFilter::Private
{
public:

    Private()
    {
        continueOnError = false;
    }

    bool                continueOnError;

    QList<FilterAction> actions;
    QList<FilterAction> appliedActions;

    QString             errorMessage;
};

FilterActionFilter::FilterActionFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

FilterActionFilter::~FilterActionFilter()
{
    delete d;
}

void FilterActionFilter::setContinueOnError(bool cont)
{
    d->continueOnError = cont;
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
    foreach(const FilterAction& action, d->actions)
    {
        if (!action.isNull() && action.category() != FilterAction::ReproducibleFilter)
        {
            return false;
        }
    }
    return true;
}

bool FilterActionFilter::isComplexAction() const
{
    foreach(const FilterAction& action, d->actions)
    {
        if (!action.isNull() && action.category() != FilterAction::ReproducibleFilter &&
            action.category() != FilterAction::ComplexFilter)
        {
            return false;
        }
    }

    return true;
}

bool FilterActionFilter::isSupported() const
{
    foreach(const FilterAction& action, d->actions)
    {
        if (!action.isNull() && !DImgFilterManager::instance()->isSupported(action.identifier(), action.version()))
        {
            return false;
        }
    }

    return true;
}

bool FilterActionFilter::completelyApplied() const
{
    return d->appliedActions.size() == d->actions.size();
}

QList<FilterAction> FilterActionFilter::appliedFilterActions() const
{
    return d->appliedActions;
}

FilterAction FilterActionFilter::failedAction() const
{
    if (d->appliedActions.size() >= d->actions.size())
    {
        return FilterAction();
    }

    return d->actions.at(d->appliedActions.size());
}

int FilterActionFilter::failedActionIndex() const
{
    return d->appliedActions.size();
}

QString FilterActionFilter::failedActionMessage() const
{
    return d->errorMessage;
}

void FilterActionFilter::filterImage()
{
    d->appliedActions.clear();
    d->errorMessage.clear();
    const float progressIncrement = 1.0 / qMax(1, d->actions.size());
    float progress                = 0;

    postProgress(0);

    DImg img = m_orgImage;

    foreach(const FilterAction& action, d->actions)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Replaying action" << action.identifier();

        if (action.isNull())
        {
            continue;
        }

        if (DImgBuiltinFilter::isSupported(action.identifier()))
        {
            DImgBuiltinFilter filter(action);

            if (!filter.isValid())
            {
                d->errorMessage = i18n("Built-in transformation not supported");

                if (d->continueOnError)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }

            filter.apply(img);
            d->appliedActions << filter.filterAction();
        }
        else
        {
            QScopedPointer<DImgThreadedFilter> filter
            (DImgFilterManager::instance()->createFilter(action.identifier(), action.version()));

            if (!filter)
            {
                d->errorMessage = i18n("Filter identifier or version is not supported");

                if (d->continueOnError)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }

            filter->readParameters(action);

            if (!filter->parametersSuccessfullyRead())
            {
                d->errorMessage = filter->readParametersError(action);

                if (d->continueOnError)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }

            // compute
            filter->setupAndStartDirectly(img, this, (int)progress, (int)(progress + progressIncrement));
            img = filter->getTargetImage();
            d->appliedActions << filter->filterAction();
        }

        progress += progressIncrement;
        postProgress((int)progress);
    }

    m_destImage = img;
}

} // namespace Digikam
