/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-29
 * Description : Pressable Button class using QGraphicsItem
 *
 * Copyright (C) 2010      by Frederico Duarte
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "button.h"

// Qt includes

#include <QGraphicsScene>

namespace Digikam
{

class Button::Private
{
public:

    Private()
    {
        isPressed = false;
    }

    bool    isPressed;
    QPixmap normal;
    QPixmap pressed;
};

Button::Button(QGraphicsItem* const parent)
      : QGraphicsItem(parent), d(new Private)
{
}

Button::Button(const QString& normal, const QString& pressed, QGraphicsItem* const parent)
      : QGraphicsItem(parent), d(new Private)
{
    setPixmap(normal, pressed);
}

Button::Button(const QPixmap& normal, const QPixmap& pressed, QGraphicsItem* const parent)
      : QGraphicsItem(parent), d(new Private)
{
    setPixmap(normal, pressed);
}

Button::~Button()
{
    delete d;
}

QRectF Button::boundingRect() const
{
    return d->normal.rect();
}

void Button::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (d->isPressed)
        painter->drawPixmap(0, 0, d->pressed);
    else
        painter->drawPixmap(0, 0, d->normal);
}

void Button::setPixmap(const QString& normal, const QString& pressed)
{
    if (! d->normal.isNull())
    {
        d->normal.detach();
        d->pressed.detach();
    }

    d->normal.load(normal);
    d->pressed.load(pressed);
}

void Button::setPixmap(const QPixmap& normal, const QPixmap& pressed)
{
    d->normal  = normal;
    d->pressed = pressed;
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (contains(event->pos()))
            d->isPressed = true;

        update();
    }
}

void Button::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (contains(event->pos()))
        {
            d->isPressed = true;
        }
        else
        {
            d->isPressed = false;
        }

        update();
    }
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        d->isPressed = false;

        update();

        if (contains(event->pos()))
        {
            emit clicked();
        }
    }
}

} // namespace Digikam
