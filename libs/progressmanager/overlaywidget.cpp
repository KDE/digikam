/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-13
 * Description : progress manager
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2004      by David Faure <faure at kde dot org>
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

#include "overlaywidget.h"

// KDE includes

#include <QResizeEvent>
#include <QEvent>

namespace Digikam
{

OverlayWidget::OverlayWidget(QWidget* const alignWidget, QWidget* const parent, const QString& name)
    : DHBox(parent),
      mAlignWidget(0)
{
    setObjectName(name);
    setAlignWidget(alignWidget);
}

OverlayWidget::~OverlayWidget()
{
}

void OverlayWidget::reposition()
{
    if ( !mAlignWidget )
        return;

    // p is in the alignWidget's coordinates
    QPoint p;
    // We are always above the alignWidget, right-aligned with it.
    p.setX(mAlignWidget->width() - width());
    p.setY(-height());
    // Position in the toplevelwidget's coordinates
    QPoint pTopLevel = mAlignWidget->mapTo(topLevelWidget(), p);
    // Position in the widget's parentWidget coordinates
    QPoint pParent   = parentWidget()->mapFrom(topLevelWidget(), pTopLevel);
    // Move 'this' to that position.
    move(pParent);
}

void OverlayWidget::setAlignWidget(QWidget* const w)
{
    if (w == mAlignWidget)
        return;

    if (mAlignWidget)
        mAlignWidget->removeEventFilter(this);

    mAlignWidget = w;

    if (mAlignWidget)
        mAlignWidget->installEventFilter(this);

    reposition();
}

bool OverlayWidget::eventFilter(QObject* o, QEvent* e)
{
    if (o == mAlignWidget && ( e->type() == QEvent::Move || e->type() == QEvent::Resize ))
    {
        reposition();
    }

    return QFrame::eventFilter(o, e);
}

void OverlayWidget::resizeEvent(QResizeEvent* ev)
{
    reposition();
    QFrame::resizeEvent(ev);
}

QWidget* OverlayWidget::alignWidget() const
{
    return mAlignWidget;
}

} // namespace Digikam
