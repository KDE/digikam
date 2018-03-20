/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-23-03
 * Description : A tool tip widget which follows cursor movements.
 *               Tool tip content is displayed without delay.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
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

// Qt includes

#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>
#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QPointer>
#include <QTimer>
#include <QToolTip>

namespace Digikam
{

class DCursorTracker::Private
{

public:

    Private()
    {
        keepOpen      = false;
        enable        = true;
        autoHideTimer = 0;
        parent        = 0;
    }

    Qt::Alignment     alignment;
    bool              enable;
    bool              keepOpen;
    QTimer*           autoHideTimer;
    QPointer<QWidget> parent;
};

DCursorTracker::DCursorTracker(const QString& txt, QWidget* const parent, Qt::Alignment align)
    : QLabel(txt, parent, Qt::ToolTip | Qt::BypassGraphicsProxyWidget),
      d(new Private)
{
    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    setPalette(QToolTip::palette());
    ensurePolished();
    const int fwidth = 1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this);
    setContentsMargins(fwidth, fwidth, fwidth, fwidth);
    setFrameStyle(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setIndent(1);
    setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);

    d->alignment = align;
    d->parent    = parent;
    d->parent->setMouseTracking(true);
    d->parent->installEventFilter(this);

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
}

void DCursorTracker::setKeepOpen(bool b)
{
    d->keepOpen = b;
}

void DCursorTracker::setTrackerAlignment(Qt::Alignment alignment)
{
    d->alignment = alignment;
}

void DCursorTracker::triggerAutoShow(int timeout)
{
    if (canBeDisplayed())
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

bool DCursorTracker::eventFilter(QObject* object, QEvent* e)
{
    QWidget* const widget = static_cast<QWidget*>(object);

    switch (e->type())
    {
        case QEvent::MouseMove:
        {
            QMouseEvent* event = static_cast<QMouseEvent*>(e);

            if (canBeDisplayed() && (widget->rect().contains(event->pos()) ||
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

void DCursorTracker::moveToParent(QWidget* const parent)
{
    if (!parent)
    {
        return;
    }

    switch (d->alignment)
    {
        case Qt::AlignLeft:
        {
            QPoint p = parent->mapToGlobal(QPoint(0, 0));
            int y    = p.y() - height();
            move(p.x(), (y < 0) ? (p.y() + parent->height()) : y);
            break;
        }

        case Qt::AlignRight:
        {
            QPoint p = parent->mapToGlobal(QPoint(parent->width(), 0));
            int y    = p.y() - height();
            move(p.x()-width(), (y < 0) ? (p.y() + parent->height()) : y);
            break;
        }

        case Qt::AlignCenter:
        default:
        {
            QPoint p = parent->mapToGlobal(QPoint(parent->width()/2, 0));
            int y    = p.y() - height();
            move(p.x()-width()/2, (y < 0) ? (p.y() + parent->height()) : y);
            break;
        }
    }
}

void DCursorTracker::paintEvent(QPaintEvent* e)
{
    QStylePainter p(this);
    QStyleOptionFrame opt;
    opt.init(this);
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    p.end();

    QLabel::paintEvent(e);
}

bool DCursorTracker::canBeDisplayed()
{
    return d->enable && d->parent->isVisible();
}

} // namespace Digikam
