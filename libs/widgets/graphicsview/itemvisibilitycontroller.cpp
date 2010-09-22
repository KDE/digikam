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

#include "itemvisibilitycontroller.h"

// Qt includes

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

// KDE includes

#include <kdebug.h>

// Local includes

namespace Digikam
{

ItemVisibilityControllerPropertyObject::ItemVisibilityControllerPropertyObject(QObject *parent)
    : QObject(parent)
{
    m_opacity = 0;
    m_visible = false;
}

qreal ItemVisibilityControllerPropertyObject::opacity() const
{
    return m_opacity;
}

void ItemVisibilityControllerPropertyObject::setOpacity(qreal opacity)
{
    m_opacity = opacity;
    emit opacityChanged();
}

bool ItemVisibilityControllerPropertyObject::isVisible() const
{
    return m_visible;
}

void ItemVisibilityControllerPropertyObject::setVisible(bool visible)
{
    m_visible = visible;
    visibleChanged();
}

// ---

AnimatedVisibility::AnimatedVisibility(QObject *parent)
    : ItemVisibilityControllerPropertyObject(parent)
{
    m_controller = new ItemVisibilityController(ItemVisibilityController::SingleItem, this);
    m_controller->setItem(this);
}

ItemVisibilityController *AnimatedVisibility::controller() const
{
    return m_controller;
}

// ---

class ItemVisibilityController::ItemVisibilityControllerPriv
{
public:

    ItemVisibilityControllerPriv(ItemVisibilityController *q) : q(q)
    {
        animation      = 0;
        state          = ItemVisibilityController::Hidden;
        visible        = false;
        shallBeShown   = true;
    }

    ItemVisibilityController::ItemType type;
    QAbstractAnimation*                animation;
    ItemVisibilityController::State    state;
    bool                               visible;
    bool                               shallBeShown;
    QList<QObject*>                    items;

    void                       transitionToVisible(bool show);
    void                       setVisibleProperty(bool value);
    void                       syncProperties(QObject *o);
    QPropertyAnimation        *createAnimation();

    QPropertyAnimation        *propertyAnimation() const { return static_cast<QPropertyAnimation*>(animation); }
    QParallelAnimationGroup   *animationGroup() const { return static_cast<QParallelAnimationGroup*>(animation); }

    ItemVisibilityController* const q;
};


ItemVisibilityController::ItemVisibilityController(ItemType type, QObject *parent)
    : QObject(parent), d(new ItemVisibilityControllerPriv(this))
{
    d->type = type;
}

ItemVisibilityController::~ItemVisibilityController()
{
    delete d->animation;
    delete d;
}

void ItemVisibilityController::setItem(QObject *object)
{
    if (d->type == ItemGroup)
    {
        clear();
        addItem(object);
        return;
    }

    if (!object && !d->items.isEmpty())
    {
        disconnect(d->items.first(), SIGNAL(destroyed(QObject*)),
                   this, SLOT(objectDestroyed(QObject*)));
        clear();
        return;
    }

    if (!d->propertyAnimation())
    {
        d->animation = d->createAnimation();
    }

    d->propertyAnimation()->setTargetObject(object);

    connect(object, SIGNAL(destroyed(QObject*)),
            this, SLOT(objectDestroyed(QObject*)));
    d->items << object;

    d->syncProperties(object);
}

void ItemVisibilityController::addItem(QObject *object)
{
    if (d->type == SingleItem)
    {
        setItem(object);
        return;
    }

    if (!object)
        return;

    if (!d->animationGroup())
    {
        d->animation = new  QParallelAnimationGroup(this);

        connect(d->animationGroup(), SIGNAL(finished()),
                this, SLOT(animationFinished()));
    }

    QPropertyAnimation* anim = d->createAnimation();
    anim->setTargetObject(object);

    connect(object, SIGNAL(destroyed(QObject*)),
            this, SLOT(objectDestroyed(QObject*)));
    d->items << object;
    d->animationGroup()->addAnimation(anim);

    d->syncProperties(object);
}

void ItemVisibilityController::removeItem(QObject *object)
{
    if (d->type == SingleItem && !d->items.isEmpty() && d->items.first() == object)
    {
        return;
        setItem(0);
    }

    if (!object)
        return;

    int index = d->items.indexOf(object);
    if (index == -1)
        return;
    d->items.removeAt(index);
    delete d->animationGroup()->takeAnimation(index);
    disconnect(object, SIGNAL(destroyed(QObject*)),
               this, SLOT(objectDestroyed(QObject*)));
}

void ItemVisibilityController::clear()
{
    if (d->type == ItemGroup)
    {
        if (d->animationGroup())
            d->animationGroup()->clear();
    }
    else
    {
        delete d->animation;
        d->animation = 0;
    }
    d->items.clear();
    d->state   = Hidden;
    d->visible = false;
}

bool ItemVisibilityController::shallBeShown() const
{
    return d->shallBeShown;
}

bool ItemVisibilityController::isVisible() const
{
    return d->visible;
}

ItemVisibilityController::State ItemVisibilityController::state() const
{
    return d->state;
}

QAbstractAnimation *ItemVisibilityController::animation() const
{
    return d->animation;
}

void ItemVisibilityController::setShallBeShown(bool shallBeShown)
{
    d->shallBeShown = shallBeShown;
    d->transitionToVisible(d->shallBeShown && d->visible);
}

void ItemVisibilityController::show()
{
    setVisible(true);
}

void ItemVisibilityController::hide()
{
    setVisible(false);
}

void ItemVisibilityController::setVisible(bool visible)
{
    d->visible = visible;
    d->transitionToVisible(d->shallBeShown && d->visible);
}

QPropertyAnimation *ItemVisibilityController::ItemVisibilityControllerPriv::createAnimation()
{
    QPropertyAnimation *anim = new QPropertyAnimation(q);
    anim->setPropertyName("opacity");
    anim->setStartValue(0);
    anim->setEndValue(1.0);
    anim->setDuration(200);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    return anim;
}

void ItemVisibilityController::ItemVisibilityControllerPriv::setVisibleProperty(bool value)
{
    foreach (QObject *o, items)
    {
        o->setProperty("visible", value);
    }
}

void ItemVisibilityController::ItemVisibilityControllerPriv::syncProperties(QObject *o)
{
    if (state == Visible || state == FadingIn)
    {
        o->setProperty("visible", true);
        o->setProperty("opacity", 1.0);
    }
    else
    {
        o->setProperty("visible", false);
        o->setProperty("opacity", 0);
    }
}

void ItemVisibilityController::ItemVisibilityControllerPriv::transitionToVisible(bool show)
{
    //kDebug() << "state" << state << "show" << show << items.size();

    if (show)
    {
        if (state == Visible || state == FadingIn)
            return;
        if (state == Hidden)
            setVisibleProperty(true);
        state = FadingIn;
    }
    else
    {
        if (state == Hidden || state == FadingOut)
            return;
        state = FadingOut;
    }

    QAbstractAnimation::Direction direction = show ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
    animation->setDirection(direction);
    animation->start();
}

void ItemVisibilityController::animationFinished()
{
    if (d->state == FadingOut)
    {
        d->setVisibleProperty(false);
        d->state = Hidden;
    }
    else
    {
        d->state = Visible;
    }
}

void ItemVisibilityController::objectDestroyed(QObject* item)
{
    removeItem(item);
}


} // namespace Digikam


