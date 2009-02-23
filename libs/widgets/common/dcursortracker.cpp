/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-23-03
 * Description : A tool tip widget which follows cursor movements.
 *               Tool tip content is displayed without delay.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>
#include <QToolTip>

// KDE includes.

#include <kdebug.h>

namespace Digikam
{

DCursorTracker::DCursorTracker(const QString& txt, QWidget *parent)
              : QLabel(txt, 0, Qt::ToolTip)
{
    m_parent = parent;
    m_parent->setMouseTracking(true);
    m_parent->installEventFilter(this);
    setEnable(true);

    m_keepOpen = false;

    m_autoHideTimer = new QTimer(this);
    m_autoHideTimer->setSingleShot(true);

    connect(m_autoHideTimer, SIGNAL(timeout()),
            this, SLOT(slotAutoHide()));
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
    m_enable = b;
    if (b)
        moveToParent(m_parent);
}

void DCursorTracker::setKeepOpen(bool b)
{
    m_keepOpen = b;
}

void DCursorTracker::triggerAutoShow(int timeout)
{
    if (m_enable)
    {
        show();
        moveToParent(m_parent);
        m_autoHideTimer->start(timeout);
    }
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
            if (m_enable && (widget->rect().contains(event->pos()) ||
                            (event->buttons() & Qt::LeftButton)))
            {
                show();
                moveToParent(widget);
            }
            else if (!m_keepOpen)
            {
                hide();
            }
            break;
        }

        case QEvent::Leave:
        {
            if (!m_keepOpen)
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
    QPoint p = parent->mapToGlobal(QPoint(parent->width()/2, 0));
    move(p.x()-width()/2, p.y()-height());
}


DTipTracker::DTipTracker(const QString& txt, QWidget *parent)
           : DCursorTracker(txt, parent)
{
    setPalette(QToolTip::palette());
    setFrameStyle(QFrame::Plain | QFrame::Box);
    setLineWidth(1);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

} // namespace Digikam
