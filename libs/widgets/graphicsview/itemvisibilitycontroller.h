/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-20
 * Description : Managing visibility state with animations
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ITEMVISIBILITYCONTROLLER_H
#define ITEMVISIBILITYCONTROLLER_H

// Qt includes

#include <QAbstractAnimation>

// KDE includes

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ItemVisibilityController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool  shallBeShown READ shallBeShown WRITE setShallBeShown)
    Q_PROPERTY(bool  visible READ isVisible WRITE setVisible)
    Q_ENUMS(State)

public:

    /**
     *  This class handles complex visibility situations for items.
     *  There is a 3-tiered approach:
     *  1) shallBeShown determines if the items shall at any time be shown.
     *     If it is false, items will never be shown.
     *     Default is true, so you can ignore this setting.
     *  2) visible determines if the items shall be shown now.
     *     Only takes effect if shallBeShown is true.
     *     Default is false: Initially, controlled items are hidden.
     *  3) Opacitiy and individual item visibility:
     *     When showing, items are first set to individually visible,
     *     then their opacity is increased from 0 to 1.
     *     When hiding, opacity is first decreased from 1 to 0,
     *     then they are set individually to hidden.
     *  Different types of items can be handled:
     *  - a group of items with an "opacity" and "visible" property
     *  - a single item with an "opacity" and "visible" property
     *  - a proxy object with these properties (see above)
     */

    enum State
    {
        Hidden,
        FadingIn,
        Visible,
        FadingOut
    };

    enum ItemType
    {
        SingleItem,
        ItemGroup
    };

    ItemVisibilityController(ItemType type = SingleItem, QObject *parent = 0);
    ~ItemVisibilityController();

    /**
     * Set or add objects. The given objects shall provide
     * an "opacity" and a "visible" property.
     * You can, for convenience, use a ItemVisibilityControllerPropertyObject
     * as a value container, if your items do not provide these properties directly.
     * No ownership is taken, so the objects should live as long as this object
     * is used.
     */
    /// Call this for SingleItem animations
    void setItem(QObject *object);
    /// Call this for ItemGroup animations
    void addItem(QObject *object);
    void removeItem(QObject *object);

    bool  shallBeShown() const;
    bool  isVisible() const;
    State state() const;

    /// Remove all animations
    void clear();

    /**
     * Returns the property used for the animation.
     * If the item is a SingleItem, it will be a QPropertyAnimation.
     * If it's an ItemGroup, it will be a QParallelAnimationGroup.
     * If animations are switched off globally, this can be 0.
     */
    QAbstractAnimation *animation() const;

public Q_SLOTS:

    void setShallBeShown(bool shallBeShown);
    void show();
    void hide();
    void setVisible(bool visible);

protected Q_SLOTS:

    void animationFinished();
    void objectDestroyed(QObject *);

public: // internal


private:

    class ItemVisibilityControllerPriv;
    ItemVisibilityControllerPriv* const d;
};

class DIGIKAM_EXPORT ItemVisibilityControllerPropertyObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(bool  visible READ isVisible WRITE setVisible NOTIFY visibleChanged)

public:

    /**
     *  You can use this object as a container providing the properties
     *  set by ItemVisibilityController.
     *  Connect to the signals accordingly, e.g. to trigger a repaint.
     */

    ItemVisibilityControllerPropertyObject(QObject *parent = 0);

    qreal opacity() const;
    void setOpacity(qreal opacity);

    bool isVisible() const;
    void setVisible(bool visible);

Q_SIGNALS:

    void opacityChanged();
    void visibleChanged();

protected:

    qreal m_opacity;
    qreal m_visible;
};

class DIGIKAM_EXPORT AnimatedVisibility : public ItemVisibilityControllerPropertyObject
{
    Q_OBJECT

public:

    /** A convenience class:
     *  The property object brings its own controller.
     *  Ready to use: Just construct an object and connect to the signals.
     *  Please note the difference between controller()->setVisible() and setVisible():
     *  You want to call the controller's method!
     */

    AnimatedVisibility(QObject *parent = 0);

    ItemVisibilityController *controller() const;

protected:

    ItemVisibilityController *m_controller;
};

} // namespace Digikam

#endif // ITEMVISIBILITYCONTROLLER_H

