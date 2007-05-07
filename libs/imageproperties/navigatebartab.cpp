/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-04
 * Description : A parent tab class with a navigation bar
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

#include <qlayout.h>
#include <qwidgetstack.h>
#include <qlabel.h>

// Local includes.

#include "statusnavigatebar.h"
#include "navigatebarwidget.h"
#include "navigatebartab.h"
#include "navigatebartab.moc"

namespace Digikam
{

class NavigateBarTabPriv
{
public:

    NavigateBarTabPriv()
    {
        stack       = 0;
        navigateBar = 0;
        label       = 0;
    }

    QWidgetStack      *stack;

    QLabel            *label;

    NavigateBarWidget *navigateBar;
};

NavigateBarTab::NavigateBarTab(QWidget* parent)
              : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new NavigateBarTabPriv;
    m_navigateBarLayout = 0;
}

NavigateBarTab::~NavigateBarTab()
{
    delete d;
}

void NavigateBarTab::setupNavigateBar(bool withBar)
{
    m_navigateBarLayout = new QVBoxLayout(this);

    if (withBar)
    {
        d->stack = new QWidgetStack(this);
        m_navigateBarLayout->addWidget(d->stack);

        d->navigateBar  = new NavigateBarWidget(d->stack, withBar);
        d->stack->addWidget(d->navigateBar);

        connect(d->navigateBar, SIGNAL(signalFirstItem()),
                this, SIGNAL(signalFirstItem()));

        connect(d->navigateBar, SIGNAL(signalPrevItem()),
                this, SIGNAL(signalPrevItem()));

        connect(d->navigateBar, SIGNAL(signalNextItem()),
                this, SIGNAL(signalNextItem()));

        connect(d->navigateBar, SIGNAL(signalLastItem()),
                this, SIGNAL(signalLastItem()));

        d->label = new QLabel(d->stack);
        d->label->setAlignment(Qt::AlignCenter);
        d->stack->addWidget(d->label);
    }
}

void NavigateBarTab::setNavigateBarState(bool hasPrevious, bool hasNext)
{
    if (!d->navigateBar)
        return;

    d->stack->raiseWidget(d->navigateBar);

    if (hasPrevious && hasNext)
        d->navigateBar->setButtonsState(StatusNavigateBar::ItemCurrent);
    else if (!hasPrevious && hasNext)
        d->navigateBar->setButtonsState(StatusNavigateBar::ItemFirst);
    else if (hasPrevious && !hasNext)
        d->navigateBar->setButtonsState(StatusNavigateBar::ItemLast);
    else
        d->navigateBar->setButtonsState(StatusNavigateBar::NoNavigation);
}

void NavigateBarTab::setNavigateBarState(int itemType)
{
    if (!d->navigateBar)
        return;

    d->stack->raiseWidget(d->navigateBar);
    d->navigateBar->setButtonsState(itemType);
}

void NavigateBarTab::setNavigateBarFileName(const QString &name)
{
    if (!d->navigateBar)
        return;

    d->stack->raiseWidget(d->navigateBar);
    d->navigateBar->setFileName(name);
}

void NavigateBarTab::setLabelText(const QString &text)
{
    if (!d->label)
        return;

    d->stack->raiseWidget(d->label);
    d->label->setText(text);
}

}  // NameSpace Digikam

