/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-20
 * Description : Managing visibility state with animations
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "digikam_debug.h"

namespace Digikam
{

ItemVisibilityControllerPropertyObject::ItemVisibilityControllerPropertyObject(QObject* const parent)
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

// ---------------------------------------------------------------------------------

AnimatedVisibility::AnimatedVisibility(QObject* const parent)
    : ItemVisibilityControllerPropertyObject(parent)
{
    m_controller = new ItemVisibilityController(this);
    m_controller->addItem(this);
}

ItemVisibilityController* AnimatedVisibility::controller() const
{
    return m_controller;
}

// ---------------------------------------------------------------------------------

HidingStateChanger::HidingStateChanger(QObject* const parent)
    : ItemVisibilityController(parent), m_object(0)
{
    connect(this, SIGNAL(propertiesAssigned(bool)),
            this, SLOT(slotPropertiesAssigned(bool)));
}

HidingStateChanger::HidingStateChanger(QObject* const target, const QByteArray& property, QObject* const parent)
    : ItemVisibilityController(parent)
{
    connect(this, SIGNAL(propertiesAssigned(bool)),
            this, SLOT(slotPropertiesAssigned(bool)));

    setTargetObject(target);
    setPropertyName(property);

    // here, we assume to start with a visible item
    setVisible(true);
}

void HidingStateChanger::setTargetObject(QObject* const object)
{
    m_object = object;
}

void HidingStateChanger::setPropertyName(const QByteArray& propertyName)
{
    m_property = propertyName;
}

void HidingStateChanger::changeValue(const QVariant& value)
{
    m_value = value;

    if (!hasVisibleItems())
    {
        // shortcut
        slotPropertiesAssigned(false);
        slotPropertiesAssigned(true);
    }
    else
    {
        hide();
    }
}

void HidingStateChanger::slotPropertiesAssigned(bool visible)
{
    if (!visible)
    {
        if (m_object)
        {
            m_object->setProperty(m_property.constData(), m_value);
        }

        emit stateChanged();
        show();
    }
    else
    {
        emit finished();
    }
}

// ---------------------------------------------------------------------------------

class AnimationControl
{
public:

    enum Situation
    {
        MainControl,
        IndependentControl,
        RemovingControl
    };

public:

    explicit AnimationControl(ItemVisibilityController* const q);
    AnimationControl(AnimationControl* const other, QObject* const item);
    ~AnimationControl();

    void clear();

    void addItem(QAbstractAnimation* const animation, QObject* const item);
    QAbstractAnimation* takeItem(QObject* const item);
    void moveTo(AnimationControl* const other, QObject* const item);
    void moveAllTo(AnimationControl* const other);

    bool hasItem(QObject* const o) const;
    bool hasVisibleItems(ItemVisibilityController::IncludeFadingOutMode mode) const;

    void transitionToVisible(bool show, bool immediately = false);
    void animationFinished();

    void syncProperties(QObject* const o);
    void connect(QObject* const item);
    void disconnect(QObject* const item);

    void setEasingCurve(const QEasingCurve& easing);
    void setAnimationDuration(int msecs);

public:

    QList<QObject*>                 items;
    QAbstractAnimation*             animation;
    ItemVisibilityController::State state;
    Situation                       situation;

private:

    void setVisibleProperty(bool value);
    void connect(QAbstractAnimation* const anim);
    void disconnect(QAbstractAnimation* const anim);
    void moveToGroup();

private:

    QParallelAnimationGroup*        animationGroup;
    ItemVisibilityController* const q;
};

AnimationControl::AnimationControl(ItemVisibilityController* const q)
    : animation(0), state(ItemVisibilityController::Hidden),
      situation(MainControl), animationGroup(0), q(q)
{
}

AnimationControl::AnimationControl(AnimationControl* const other, QObject* const object)
    : animation(0), state(other->state),
      situation(IndependentControl), animationGroup(0), q(other->q)
{
    other->moveTo(this, object);
}

AnimationControl::~AnimationControl()
{
    clear();
    delete animation;
}

void AnimationControl::clear()
{
    state = ItemVisibilityController::Hidden;

    if (animation)
    {
        disconnect(animation);
    }

    delete animation;
    animation      = 0;
    animationGroup = 0; // the same pointer as animation

    foreach(QObject* const item, items)
    {
        disconnect(item);
    }

    items.clear();
}

void AnimationControl::connect(QObject* const item)
{
    q->connect(item, SIGNAL(destroyed(QObject*)),
               q, SLOT(objectDestroyed(QObject*)));
}

void AnimationControl::disconnect(QObject* const item)
{
    q->disconnect(item, SIGNAL(destroyed(QObject*)),
                  q, SLOT(objectDestroyed(QObject*)));
}

void AnimationControl::connect(QAbstractAnimation* const anim)
{
    q->connect(anim, SIGNAL(finished()),
               q, SLOT(animationFinished()));
}

void AnimationControl::disconnect(QAbstractAnimation* const anim)
{
    q->disconnect(anim, SIGNAL(finished()),
                  q, SLOT(animationFinished()));
}

void AnimationControl::moveToGroup()
{
    if (!animationGroup)
    {
        animationGroup = new QParallelAnimationGroup;
        connect(animationGroup);

        if (animation)
        {
            disconnect(animation);
            animationGroup->addAnimation(animation);
        }

        animation = animationGroup;
    }
}

void AnimationControl::addItem(QAbstractAnimation* const anim, QObject* const item)
{
    // Either there is no group but now for the first time two items,
    // or the control got empty intermittently, but still has the group installed
    if (!items.isEmpty() || animationGroup)
    {
        moveToGroup();
        animationGroup->addAnimation(anim);
    }
    else
    {
        connect(anim);
        animation = anim;
    }

    items << item;
}

QAbstractAnimation* AnimationControl::takeItem(QObject* const item)
{
    int index = items.indexOf(item);

    if (index == -1)
    {
        return 0;
    }

    items.removeAt(index);

    if (animationGroup)
    {
        return animationGroup->takeAnimation(index);
    }
    else
    {
        QAbstractAnimation* const anim = animation;
        disconnect(animation);
        animation = 0;
        return anim;
    }
}

void AnimationControl::moveTo(AnimationControl* const other, QObject* const item)
{
    QAbstractAnimation* const anim = takeItem(item);

    if (anim)
    {
        other->addItem(anim, item);
    }
}

void AnimationControl::moveAllTo(AnimationControl* const other)
{
    foreach(QObject* const item, items)
    {
        moveTo(other, item);
    }
}

bool AnimationControl::hasItem(QObject* const o) const
{
    return items.contains(o);
}

bool AnimationControl::hasVisibleItems(ItemVisibilityController::IncludeFadingOutMode mode) const
{
    if (items.isEmpty())
    {
        return false;
    }

    if (mode == ItemVisibilityController::IncludeFadingOut)
    {
        return state != ItemVisibilityController::Hidden;
    }
    else
    {
        return state != ItemVisibilityController::Hidden && state != ItemVisibilityController::FadingOut;
    }
}

void AnimationControl::setVisibleProperty(bool value)
{
    foreach(QObject* const o, items)
    {
        o->setProperty("visible", value);
    }
}

void AnimationControl::syncProperties(QObject* const o)
{
    if (state == ItemVisibilityController::Visible || state == ItemVisibilityController::FadingIn)
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

void AnimationControl::transitionToVisible(bool show, bool immediately)
{
    //qCDebug(DIGIKAM_WIDGETS_LOG) << "state" << state << "show" << show << items.size();

    if (show)
    {
        if (state == ItemVisibilityController::Visible || state == ItemVisibilityController::FadingIn)
        {
            return;
        }

        if (state == ItemVisibilityController::Hidden)
        {
            setVisibleProperty(true);
        }

        state = ItemVisibilityController::FadingIn;
    }
    else
    {
        if (state == ItemVisibilityController::Hidden || state == ItemVisibilityController::FadingOut)
        {
            return;
        }

        state = ItemVisibilityController::FadingOut;
    }

    if (animation)
    {
        QAbstractAnimation::Direction direction = show ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
        animation->setDirection(direction);

        if (immediately)
        {
            animation->setCurrentTime(show ? animation->totalDuration() : 0);
        }

        animation->start();
    }
}

void AnimationControl::animationFinished()
{
    if (state == ItemVisibilityController::FadingOut)
    {
        setVisibleProperty(false);
        state = ItemVisibilityController::Hidden;
    }
    else if (state == ItemVisibilityController::FadingIn)
    {
        state = ItemVisibilityController::Visible;
    }
}

void AnimationControl::setEasingCurve(const QEasingCurve& easing)
{
    if (animationGroup)
    {
        for (int i=0; i<animationGroup->animationCount(); ++i)
        {
            QVariantAnimation* const anim = /*qobject*/static_cast<QVariantAnimation*>(animationGroup->animationAt(i));
            anim->setEasingCurve(easing);
        }
    }
    else if (animation)
    {
        QVariantAnimation* const anim = /*qobject*/static_cast<QVariantAnimation*>(animation);

        if (anim)
        {
            anim->setEasingCurve(easing);
        }
    }
}

void AnimationControl::setAnimationDuration(int msecs)
{
    if (animationGroup)
    {
        for (int i=0; i<animationGroup->animationCount(); ++i)
        {
            QVariantAnimation* const anim = /*qobject*/static_cast<QVariantAnimation*>(animationGroup->animationAt(i));
            anim->setDuration(msecs);
        }
    }
    else if (animation)
    {
        QVariantAnimation* const anim = /*qobject*/static_cast<QVariantAnimation*>(animation);

        if (anim)
        {
            anim->setDuration(msecs);
        }
    }
}

// ---------------------------------------------------------------------------------

class ItemVisibilityController::Private
{
public:

    explicit Private(ItemVisibilityController* const q)
        : q(q)
    {
        visible          = false;
        shallBeShown     = true;
        itemShallBeShown = 0;
        animationDuration= 75;
        easingCurve      = QEasingCurve::InOutQuad;
        control          = 0;
    }


public:

    void              setVisible(bool v, bool immediately);
    void              setItemVisible(QObject* const item, bool visible, bool immediately);

    AnimationControl* findInChildren(QObject* const item) const;
    AnimationControl* getChild(QObject* const item);
    void              cleanupChildren(QAbstractAnimation* const finishedAnimation);

public:

    bool                            visible;
    bool                            shallBeShown;
    QObject*                        itemShallBeShown;

    int                             animationDuration;
    QEasingCurve                    easingCurve;

    AnimationControl*               control;
    QList<AnimationControl*>        childControls;
    ItemVisibilityController* const q;
};

AnimationControl* ItemVisibilityController::Private::findInChildren(QObject* const item) const
{
    foreach(AnimationControl* const child, childControls)
    {
        if (child->hasItem(item))
        {
            return child;
        }
    }

    return 0;
}

AnimationControl* ItemVisibilityController::Private::getChild(QObject* const item)
{
    if (!control)
    {
        return 0;
    }

    if (control->hasItem(item))
    {
        AnimationControl* const child = new AnimationControl(control, item);
        childControls << child;
        return child;
    }
    else
    {
        return findInChildren(item);
    }
}

void ItemVisibilityController::Private::cleanupChildren(QAbstractAnimation* const finishedAnimation)
{
    QList<AnimationControl*>::iterator it;

    for (it = childControls.begin(); it != childControls.end(); )
    {
        AnimationControl* child = *it;

        if (child->state == control->state && child->situation == AnimationControl::IndependentControl)
        {
            // merge back to main control
            child->moveAllTo(control);
            delete child;
            it = childControls.erase(it);
        }
        else if (child->animation == finishedAnimation && child->situation == AnimationControl::RemovingControl)
        {
            foreach(QObject* const item, child->items)
            {
                emit (q->hiddenAndRemoved(item));
            }

            delete child;
            it = childControls.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ItemVisibilityController::Private::setVisible(bool v, bool immediately)
{
    // no check d->visible == visible
    visible = v;

    if (control)
    {
        control->transitionToVisible(shallBeShown && visible, immediately);
    }

    foreach(AnimationControl* const child, childControls)
    {
        if (child->situation == AnimationControl::IndependentControl)
        {
            child->transitionToVisible(shallBeShown && visible, immediately);
        }
    }

    if (itemShallBeShown)
    {
        setItemVisible(itemShallBeShown, visible, immediately);
    }
}

void ItemVisibilityController::Private::setItemVisible(QObject* const item, bool visible, bool immediately)
{
    AnimationControl* const child = getChild(item);

    if (child)
    {
        child->transitionToVisible(visible, immediately);
    }
}

// ---------------------------------------------------------------------------------

ItemVisibilityController::ItemVisibilityController(QObject* parent)
    : QObject(parent), d(new Private(this))
{
}

ItemVisibilityController::~ItemVisibilityController()
{
    clear();
    delete d->control;
    delete d;
}

QPropertyAnimation* ItemVisibilityController::createAnimation(QObject*)
{
    QPropertyAnimation* const anim = new QPropertyAnimation(this);
    anim->setPropertyName("opacity");
    anim->setStartValue(0);
    anim->setEndValue(1.0);
    anim->setDuration(d->animationDuration);
    anim->setEasingCurve(d->easingCurve);
    return anim;
}

void ItemVisibilityController::addItem(QObject* item)
{
    if (!item)
    {
        return;
    }

    if (!d->control)
    {
        // initialize main control
        d->control = new AnimationControl(this);
        d->control->transitionToVisible(d->shallBeShown && d->visible);
    }

    QPropertyAnimation* const anim = createAnimation(item);
    anim->setTargetObject(item);
    d->control->connect(item);
    d->control->syncProperties(item);
    d->control->addItem(anim, item);
}

void ItemVisibilityController::removeItem(QObject* item)
{
    if (!item || !d->control)
    {
        return;
    }

    if (d->control->hasItem(item))
    {
        d->control->disconnect(item);
        delete d->control->takeItem(item);
    }
    else
    {
        AnimationControl* child = d->findInChildren(item);

        if (child)
        {
            child->disconnect(item);
            d->childControls.removeOne(child);
            delete child;
        }
    }
}

void ItemVisibilityController::clear()
{
    if (d->control)
    {
        d->control->clear();
    }

    foreach(AnimationControl* const child, d->childControls)
    {
        child->clear();
    }

    d->childControls.clear();

    d->visible = false;
}

QList<QObject*> ItemVisibilityController::items() const
{
    QList<QObject*> items;

    if (d->control)
    {
        items = d->control->items;
    }

    foreach(AnimationControl* const child, d->childControls)
    {
        items += child->items;
    }

    return items;
}

QList<QObject*> ItemVisibilityController::visibleItems(IncludeFadingOutMode mode) const
{
    QList<QObject*> items;

    if (d->control && d->control->hasVisibleItems(mode))
    {
        items = d->control->items;
    }

    foreach(AnimationControl* const child, d->childControls)
    {
        if (child->hasVisibleItems(mode))
        {
            items += child->items;
        }
    }

    return items;
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
    return d->control ? d->control->state : Hidden;
}

bool ItemVisibilityController::hasVisibleItems(IncludeFadingOutMode mode) const
{
    if (d->control && d->control->hasVisibleItems(mode))
    {
        return true;
    }

    foreach(AnimationControl* const child, d->childControls)
    {
        if (child->hasVisibleItems(mode))
        {
            return true;
        }
    }

    return false;
}

void ItemVisibilityController::setEasingCurve(const QEasingCurve& easing)
{
    d->easingCurve = easing;

    if (d->control)
    {
        d->control->setEasingCurve(easing);
    }

    foreach(AnimationControl* const child, d->childControls)
    {
        child->setEasingCurve(easing);
    }
}

void ItemVisibilityController::setAnimationDuration(int msecs)
{
    d->animationDuration = msecs;

    if (d->control)
    {
        d->control->setAnimationDuration(msecs);
    }

    foreach(AnimationControl* const child, d->childControls)
    {
        child->setAnimationDuration(msecs);
    }
}

void ItemVisibilityController::setShallBeShown(bool shallBeShown)
{
    // no check d->shallBeShown == shallBeShown
    d->shallBeShown     = shallBeShown;
    d->itemShallBeShown = 0;

    // apply
    setVisible(d->visible);
}

void ItemVisibilityController::setShallBeShownDirectly(bool shallBeShown)
{
    // no check d->shallBeShown == shallBeShown
    d->shallBeShown     = shallBeShown;
    d->itemShallBeShown = 0;

    // apply
    setDirectlyVisible(d->visible);
}

void ItemVisibilityController::setItemThatShallBeShown(QObject* item)
{
    d->itemShallBeShown = item;
    d->shallBeShown     = false;
    setVisible(d->visible);
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
    d->setVisible(visible, false);
}

void ItemVisibilityController::setDirectlyVisible(bool visible)
{
    d->setVisible(visible, true);
}

void ItemVisibilityController::showItem(QObject* item)
{
    setItemVisible(item, true);
}

void ItemVisibilityController::hideItem(QObject* item)
{
    setItemVisible(item, false);
}

void ItemVisibilityController::setItemVisible(QObject* item, bool visible)
{
    d->setItemVisible(item, visible, false);
}

void ItemVisibilityController::setItemDirectlyVisible(QObject* item, bool visible)
{
    d->setItemVisible(item, visible, true);
}

void ItemVisibilityController::hideAndRemoveItem(QObject* item)
{
    AnimationControl* const child = d->getChild(item);

    if (child)
    {
        child->situation = AnimationControl::RemovingControl;
        child->transitionToVisible(false);
    }
}

void ItemVisibilityController::animationFinished()
{
    QAbstractAnimation* const animation = static_cast<QAbstractAnimation*>(sender());

    if (d->control && d->control->animation == animation)
    {
        d->control->animationFinished();
        emit propertiesAssigned(d->control->state == Visible);
    }

    foreach(AnimationControl* const child, d->childControls)
    {
        if (child->animation == animation)
        {
            child->animationFinished();

            foreach(QObject* const item, child->items)
            {
                emit propertiesAssigned(item, d->control->state == Visible);
            }
        }
    }

    // if a child is now in main state, move again to main control
    d->cleanupChildren(animation);
}

void ItemVisibilityController::objectDestroyed(QObject* item)
{
    removeItem(item);
}

} // namespace Digikam
