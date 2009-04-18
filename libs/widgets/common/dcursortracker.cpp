/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-23-03
 * Description : A tool tip widget which follows cursor movements.
 *               Tool tip content is displayed without delay.
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "dcursortracker.h"
#include "dcursortracker.moc"

// Qt includes

#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>
#include <QToolTip>

// KDE includes

#include <kdebug.h>

namespace Digikam
{

class DCursorTrackerPriv
{

public:

    DCursorTrackerPriv()
    {
        keepOpen      = false;
        enable        = true;
        autoHideTimer = 0;
        parent        = 0;
    }

    Qt::Alignment alignment;
    bool          enable;
    bool          keepOpen;
    QTimer*       autoHideTimer;
    QWidget*      parent;
};

DCursorTracker::DCursorTracker(const QString& txt, QWidget *parent, Qt::Alignment align)
              : QLabel(txt, 0, Qt::ToolTip), d(new DCursorTrackerPriv)
{
    d->parent = parent;
    d->parent->setMouseTracking(true);
    d->parent->installEventFilter(this);

    d->alignment = align;

    d->autoHideTimer = new QTimer(this);
    d->autoHideTimer->setSingleShot(true);

    connect(d->autoHideTimer, SIGNAL(timeout()),
            this, SLOT(slotAutoHide()));
}

DCursorTracker::~DCursorTracker()
{
    delete d;
}

/**
 * Overload to make sure the widget size is correct
 */
void DCursorTracker::setText(const QString& txt)
{
    QLabel::setText(txt);
    adjustSize();
}

void DCursorTracker::setEnable(bool b)
{
    d->enable = b;
    moveToParent(d->parent);
}

void DCursorTracker::setKeepOpen(bool b)
{
    d->keepOpen = b;
}

void DCursorTracker::triggerAutoShow(int timeout)
{
    if (d->enable)
    {
        show();
        moveToParent(d->parent);
        d->autoHideTimer->start(timeout);
    }
}

void DCursorTracker::refresh()
{
    moveToParent(d->parent);
}

void DCursorTracker::slotAutoHide()
{
    hide();
}

bool DCursorTracker::eventFilter(QObject *object, QEvent *e)
{
    QWidget *widget = static_cast<QWidget*>(object);

    switch (e->type())
    {
        case QEvent::MouseMove:
        {
            QMouseEvent *event = static_cast<QMouseEvent*>(e);
            if (d->enable && (widget->rect().contains(event->pos()) ||
                             (event->buttons() & Qt::LeftButton)))
            {
                show();
                moveToParent(widget);
            }
            else if (!d->keepOpen)
            {
                hide();
            }
            break;
        }

        case QEvent::Leave:
        {
            if (!d->keepOpen)
                hide();
            break;
        }

        default:
            break;
    }

    return false;
}

void DCursorTracker::moveToParent(QWidget* parent)
{
    switch (d->alignment)
    {
        case Qt::AlignLeft:
        {
            QPoint p = parent->mapToGlobal(QPoint(0, 0));
            move(p.x(), p.y()-height());
            break;
        }
        case Qt::AlignRight:
        {
            QPoint p = parent->mapToGlobal(QPoint(parent->width(), 0));
            move(p.x()-width(), p.y()-height());
            break;
        }
        case Qt::AlignCenter:
        default:
        {
            QPoint p = parent->mapToGlobal(QPoint(parent->width()/2, 0));
            move(p.x()-width()/2, p.y()-height());
            break;
        }
    }
}


DTipTracker::DTipTracker(const QString& txt, QWidget *parent, Qt::Alignment align)
           : DCursorTracker(txt, parent, align)
{
    setPalette(QToolTip::palette());
    setFrameStyle(QFrame::Plain | QFrame::Box);
    setLineWidth(1);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

} // namespace Digikam
