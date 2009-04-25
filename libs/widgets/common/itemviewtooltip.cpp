/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : A DItemToolTip prepared for use in QAbstractItemViews
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "itemviewtooltip.h"

// Qt includes

#include <QApplication>

// KDE includes

// Local includes

namespace Digikam
{

class ItemViewToolTipPriv
{
public:

    ItemViewToolTipPriv()
    {
        view = 0;
    }

    QAbstractItemView *view;
    QModelIndex        index;
    QRect              rect;
};

ItemViewToolTip::ItemViewToolTip(QAbstractItemView *view)
               : DItemToolTip(), d(new ItemViewToolTipPriv)
{
    d->view = view;

    setMouseTracking(true);
}

ItemViewToolTip::~ItemViewToolTip()
{
    delete d;
}

QAbstractItemView *ItemViewToolTip::view() const
{
    return d->view;
}

QModelIndex ItemViewToolTip::currentIndex() const
{
    return d->index;
}

void ItemViewToolTip::show(QHelpEvent *, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    d->index = index;
    d->rect  = option.rect;
    qApp->installEventFilter(this);
}

QRect ItemViewToolTip::repositionRect()
{
    return d->rect;
}

void ItemViewToolTip::hideEvent(QHideEvent *)
{
    d->rect  = QRect();
    d->index = QModelIndex();
    qApp->removeEventFilter(this);
}

// The following code is inspired by qtooltip.cpp,
// Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).

bool ItemViewToolTip::eventFilter(QObject *o, QEvent *e)
{
    if (!isVisible())
        return false;

    switch (e->type()) 
    {
        #ifdef Q_WS_MAC
        case QEvent::KeyPress:
        case QEvent::KeyRelease: 
	{
            int key = static_cast<QKeyEvent *>(e)->key();
            Qt::KeyboardModifiers mody = static_cast<QKeyEvent *>(e)->modifiers();
            if (!(mody & Qt::KeyboardModifierMask)
                && key != Qt::Key_Shift && key != Qt::Key_Control
                && key != Qt::Key_Alt && key != Qt::Key_Meta)
                hide();
            break;
        }
        #endif
        case QEvent::Leave:
            hide();
            break;
        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Wheel:
            hide();
            break;

        case QEvent::MouseMove:
            if (o == d->view && !d->rect.isNull() && !d->rect.contains(static_cast<QMouseEvent*>(e)->pos()))
                hide();
        default:
            break;
    }
    return false;
}

void ItemViewToolTip::mouseMoveEvent(QMouseEvent *e)
{
    if (d->rect.isNull())
        return;
    QPoint pos = e->globalPos();
    pos = d->view->mapFromGlobal(pos);
    if (!d->rect.contains(pos))
        hide();
    DItemToolTip::mouseMoveEvent(e);
}

} // namespace Digikam
