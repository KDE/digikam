/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : action categorized view
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "actioncategorizedview.h"

// Qt includes

#include <QString>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QSignalMapper>
#include <QApplication>
#include <QStyle>

// Local includes

#include "digikam_debug.h"
#include "dcategorydrawer.h"

namespace Digikam
{

ActionCategorizedView::ActionCategorizedView(QWidget* const parent, bool autoScroll)
    : DCategorizedView(parent)
{
    m_autoScroll = autoScroll;
    m_horizontalScrollAnimation = new QPropertyAnimation(horizontalScrollBar(), "value", this);
    m_verticalScrollAnimation   = new QPropertyAnimation(verticalScrollBar(),   "value", this);
}

ActionCategorizedView::~ActionCategorizedView()
{
}

void ActionCategorizedView::setupIconMode()
{
    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setCategoryDrawer(new DCategoryDrawer(this)); // deprecated, but needed for KDE 4.4 compatibility
    setSelectionMode(QAbstractItemView::SingleSelection);

    setMouseTracking(true);
    viewport()->setAttribute(Qt::WA_Hover);

    setFrameShape(QFrame::NoFrame);
}

void ActionCategorizedView::adjustGridSize()
{
    // Find a suitable grid size. The delegate's size hint does never word-wrap.
    // To keep a suitable width, we want to word wrap.
    setWordWrap(true);
    int maxSize = viewOptions().decorationSize.width() * 4;
    QFontMetrics fm(viewOptions().font);
    QSize grid;

    for (int i = 0; i < model()->rowCount(); ++i)
    {
        const QModelIndex index = model()->index(i, 0);
        const QSize size        = sizeHintForIndex(index);

        if (size.width() > maxSize)
        {
            QString text        = index.data(Qt::DisplayRole).toString();
            QRect unwrappedRect = fm.boundingRect(QRect(0, 0, size.width(), size.height()), Qt::AlignLeft, text);
            QRect wrappedRect   = fm.boundingRect(QRect(0, 0, maxSize, maxSize), Qt::AlignLeft | Qt::TextWordWrap, text);
            grid                = grid.expandedTo(QSize(maxSize, size.height() + wrappedRect.height() - unwrappedRect.height()));
        }
        else
        {
            grid = grid.expandedTo(size);
        }
    }

    //grid += QSize(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing), QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    setGridSize(grid);
}

int ActionCategorizedView::autoScrollDuration(float relativeDifference, QPropertyAnimation* animation)
{
    const int minimumTime       = 1000;
    const int maxPixelPerSecond = 1000;

    int pixelToScroll           = qAbs(animation->startValue().toInt() - animation->endValue().toInt());
    int factor                  = qMax(1.0f, relativeDifference * 100); // in [1;15]

    int duration                = 1000 * pixelToScroll / maxPixelPerSecond;
    duration                    *= factor;

    return qMax(minimumTime, duration);
}

void ActionCategorizedView::autoScroll(float relativePos, QScrollBar* scrollBar, QPropertyAnimation* animation)
{

    if (scrollBar->minimum() != scrollBar->maximum())
    {
        const float lowerPart = 0.15F;
        const float upperPart = 0.85F;

        if (relativePos > upperPart && scrollBar->value() !=  scrollBar->maximum())
        {
            animation->stop();
            animation->setStartValue(scrollBar->value());
            animation->setEndValue(scrollBar->maximum());
            animation->setDuration(autoScrollDuration(1 - relativePos, animation));
            animation->start();
        }
        else if (relativePos < lowerPart && scrollBar->value() !=  scrollBar->minimum())
        {
            animation->stop();
            animation->setStartValue(scrollBar->value());
            animation->setEndValue(scrollBar->minimum());
            animation->setDuration(autoScrollDuration(relativePos, animation));
            animation->start();
        }
        else
        {
            animation->stop();
        }
    }
}

void ActionCategorizedView::mouseMoveEvent(QMouseEvent* e)
{
    DCategorizedView::mouseMoveEvent(e);

    if (m_autoScroll)
    {
        autoScroll(float(e->pos().x()) / viewport()->width(),  horizontalScrollBar(), m_horizontalScrollAnimation);
        autoScroll(float(e->pos().y()) / viewport()->height(), verticalScrollBar(),   m_verticalScrollAnimation);
    }
}

void ActionCategorizedView::leaveEvent(QEvent* e)
{
    DCategorizedView::leaveEvent(e);
    m_horizontalScrollAnimation->stop();
    m_verticalScrollAnimation->stop();
}

}  // namespace Digikam
