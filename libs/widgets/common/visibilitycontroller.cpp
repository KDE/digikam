/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "visibilitycontroller.h"

namespace Digikam
{

class VisibilityController::Private
{
public:

    Private() :
        status(VisibilityController::Unknown),
        containerWidget(0)
    {
    }

    VisibilityController::Status status;
    QList<VisibilityObject*>     objects;
    QWidget*                     containerWidget;
};

class VisibilityWidgetWrapper : public QObject, public VisibilityObject
{
public:

    VisibilityWidgetWrapper(VisibilityController* const parent, QWidget* const widget)
        : QObject(parent), m_widget(widget)
    {
    }

    virtual void setVisible(bool visible)
    {
        return m_widget->setVisible(visible);
    }

    virtual bool isVisible()
    {
        return m_widget->isVisible();
    }

    QWidget* m_widget;
};

// -------------------------------------------------------------------------------------

VisibilityController::VisibilityController(QObject* const parent)
    : QObject(parent), d(new Private)
{
}

VisibilityController::~VisibilityController()
{
    delete d;
}

void VisibilityController::addObject(VisibilityObject* const object)
{
    d->objects << object;

    // create clean state
    if (d->status == Unknown)
    {
        if (object->isVisible())
        {
            d->status = Shown;
        }
        else
        {
            d->status = Hidden;
        }
    }

    // set state on object
    if (d->status == Shown || d->status == Showing)
    {
        object->setVisible(true);
    }
    else
    {
        object->setVisible(false);
    }
}

void VisibilityController::addWidget(QWidget* const widget)
{
    addObject(new VisibilityWidgetWrapper(this, widget));
}

void VisibilityController::setContainerWidget(QWidget* const widget)
{
    d->containerWidget = widget;
}

void VisibilityController::setVisible(bool shallBeVisible)
{
    if (shallBeVisible)
    {
        if (d->status == Shown || d->status == Showing)
        {
            return;
        }

        d->status = Showing;
        beginStatusChange();
    }
    else
    {
        if (d->status == Hidden || d->status == Hiding)
        {
            return;
        }

        d->status = Hiding;
        beginStatusChange();
    }
}

void VisibilityController::show()
{
    setVisible(true);
}

void VisibilityController::hide()
{
    setVisible(false);
}

void VisibilityController::triggerVisibility()
{
    if (d->status == Shown || d->status == Showing || d->status == Unknown)
    {
        setVisible(false);
    }
    else
    {
        setVisible(true);
    }
}

bool VisibilityController::isVisible() const
{
    if (d->status == Shown || d->status == Showing)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void VisibilityController::beginStatusChange()
{
    allSteps();
}

void VisibilityController::step()
{
    if (d->status == Showing)
    {
        foreach(VisibilityObject* const o, d->objects)
        {
            if (!o->isVisible())
            {
                o->setVisible(true);
                return;
            }
        }
    }
    else if (d->status == Hiding)
    {
        foreach(VisibilityObject* const o, d->objects)
        {
            if (o->isVisible())
            {
                o->setVisible(false);
                return;
            }
        }
    }
}

void VisibilityController::allSteps()
{
    if (d->status == Showing)
    {
        if (d->containerWidget)
        {
            d->containerWidget->setUpdatesEnabled(false);
        }

        foreach(VisibilityObject* const o, d->objects)
        {
            o->setVisible(true);
        }

        if (d->containerWidget)
        {
            d->containerWidget->setUpdatesEnabled(true);
        }
    }
    else if (d->status == Hiding)
    {
        if (d->containerWidget)
        {
            d->containerWidget->setUpdatesEnabled(false);
        }

        foreach(VisibilityObject* const o, d->objects)
        {
            o->setVisible(false);
        }

        if (d->containerWidget)
        {
            d->containerWidget->setUpdatesEnabled(true);
        }
    }
}

} // namespace Digikam
