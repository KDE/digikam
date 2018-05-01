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

#ifndef DIGIKAM_VISIBILITY_CONTROLLER_H
#define DIGIKAM_VISIBILITY_CONTROLLER_H

// Qt includes

#include <QWidget>
#include <QList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT VisibilityObject
{
public:

    virtual ~VisibilityObject()
    {
    }

    virtual void setVisible(bool visible) = 0;
    virtual bool isVisible()              = 0;
};

// -----------------------------------------------------------------------------------

class DIGIKAM_EXPORT VisibilityController : public QObject
{
    Q_OBJECT

public:

    enum Status
    {
        Unknown,
        Hidden,
        Showing,
        Shown,
        Hiding
    };

public:

    explicit VisibilityController(QObject* const parent);
    ~VisibilityController();

    /** Set the widget containing the widgets added to this controller */
    void setContainerWidget(QWidget* const widget);

    /**
     * Add a widget to this controller.
     */
    void addWidget(QWidget* const widget);

    /**
     * Add an object implementing the VisibilityObject interface.
     * You can use this if you have your widgets grouped in intermediate objects.
     */
    void addObject(VisibilityObject* const object);

    /**
     * Returns true if the contained objects are visible or becoming visible.
     */
    bool isVisible() const;

public Q_SLOTS:

    /// Shows/hides all added objects
    void setVisible(bool visible);
    void show();
    void hide();

    /// Shows if hidden and hides if visible.
    void triggerVisibility();

protected:

    void step();
    void allSteps();

    virtual void beginStatusChange();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_VISIBILITY_CONTROLLER_H
