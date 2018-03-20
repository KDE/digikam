/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : A DItemToolTip prepared for use in QAbstractItemViews
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include <QToolTip>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class ItemViewToolTip::Private
{
public:

    Private()
    {
        view            = 0;
        filterInstalled = false;
    }

    QAbstractItemView* view;
    QModelIndex        index;
    QRect              rect;
    QString            text;
    bool               filterInstalled;
};

ItemViewToolTip::ItemViewToolTip(QAbstractItemView* view)
    : DItemToolTip(view), d(new Private)
{
    d->view = view;

    setBackgroundRole(QPalette::ToolTipBase);
    setPalette(QToolTip::palette());
    setMouseTracking(true);
}

ItemViewToolTip::~ItemViewToolTip()
{
    delete d;
}

QAbstractItemView* ItemViewToolTip::view() const
{
    return d->view;
}

QAbstractItemModel* ItemViewToolTip::model() const
{
    return d->view ? d->view->model() : 0;
}

QModelIndex ItemViewToolTip::currentIndex() const
{
    return d->index;
}

void ItemViewToolTip::show(const QStyleOptionViewItem& option, const QModelIndex& index)
{
    d->index = index;
    d->rect  = option.rect;
    d->rect.moveTopLeft(d->view->viewport()->mapToGlobal(d->rect.topLeft()));
    updateToolTip();
    reposition();

    if (isHidden() && !toolTipIsEmpty())
    {
        if (!d->filterInstalled)
        {
            qApp->installEventFilter(this);
            d->filterInstalled = true;
        }

        DItemToolTip::show();
    }
}

void ItemViewToolTip::setTipContents(const QString& tipContents)
{
    d->text = tipContents;
    updateToolTip();
}

QString ItemViewToolTip::tipContents()
{
    return d->text;
}

QRect ItemViewToolTip::repositionRect()
{
    return d->rect;
}

void ItemViewToolTip::hideEvent(QHideEvent*)
{
    d->rect  = QRect();
    d->index = QModelIndex();

    if (d->filterInstalled)
    {
        d->filterInstalled = false;
        qApp->removeEventFilter(this);
    }
}

// The following code is inspired by qtooltip.cpp,
// Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).

bool ItemViewToolTip::eventFilter(QObject* o, QEvent* e)
{
    switch (e->type())
    {

#ifdef Q_OS_OSX
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        {
            int key                    = static_cast<QKeyEvent*>(e)->key();
            Qt::KeyboardModifiers mody = static_cast<QKeyEvent*>(e)->modifiers();

            if (!(mody & Qt::KeyboardModifierMask) &&
                key != Qt::Key_Shift               &&
                key != Qt::Key_Control             &&
                key != Qt::Key_Alt                 &&
                key != Qt::Key_Meta)
            {
                hide();
            }

            break;
        }
#endif // Q_OS_OSX

        case QEvent::Leave:
            hide(); // could add a 300ms timer here, like Qt
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
        {
            // needs mouse tracking, obviously
            if (o == d->view->viewport() &&
                !d->rect.isNull()        &&
                !d->rect.contains(static_cast<QMouseEvent*>(e)->globalPos()))
            {
                hide();
            }
            break;
        }
        default:
            break;
    }

    return false;
}

void ItemViewToolTip::mouseMoveEvent(QMouseEvent* e)
{
    if (d->rect.isNull())
    {
        return;
    }

    QPoint pos = e->globalPos();
    pos        = d->view->viewport()->mapFromGlobal(pos);

    if (!d->rect.contains(pos))
    {
        hide();
    }

    DItemToolTip::mouseMoveEvent(e);
}

} // namespace Digikam
