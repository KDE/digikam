/* ============================================================
 * Authors: Caulier Gilles <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2007-01-04
 * Description : A parent tab class with a navigation bar
 *
 * Copyright 2006 by Gilles Caulier
 * Copyright 2007 by Gilles Caulier, Marcel Wiesweg
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

// Local includes.

#include "navigatebarwidget.h"
#include "navigatebartab.h"
#include "navigatebartab.moc"

namespace Digikam
{

NavigateBarTab::NavigateBarTab(QWidget* parent)
              : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_navigateBarLayout = 0;
    m_navigateBar       = 0;
}

NavigateBarTab::~NavigateBarTab()
{
}

void NavigateBarTab::setupNavigateBar(bool navBar)
{
    m_navigateBarLayout = new QVBoxLayout(this);

    if (navBar)
    {
        m_navigateBar = new NavigateBarWidget(this, navBar);

        m_navigateBarLayout->addWidget(m_navigateBar);

        connect(m_navigateBar, SIGNAL(signalFirstItem()),
                this, SIGNAL(signalFirstItem()));

        connect(m_navigateBar, SIGNAL(signalPrevItem()),
                this, SIGNAL(signalPrevItem()));

        connect(m_navigateBar, SIGNAL(signalNextItem()),
                this, SIGNAL(signalNextItem()));

        connect(m_navigateBar, SIGNAL(signalLastItem()),
                this, SIGNAL(signalLastItem()));
    }
}

void NavigateBarTab::setNavigateBarState(bool hasPrevious, bool hasNext)
{
    if (!m_navigateBar)
        return;

    if (!hasPrevious)
        m_navigateBar->setButtonsState(NavigateBarWidget::ItemFirst);
    else if (!hasNext)
        m_navigateBar->setButtonsState(NavigateBarWidget::ItemLast);
    else
        m_navigateBar->setButtonsState(NavigateBarWidget::ItemCurrent);
}

void NavigateBarTab::setNavigateBarState(int itemType)
{
    if (!m_navigateBar)
        return;

    m_navigateBar->setButtonsState(itemType);
}

void NavigateBarTab::setNavigateBarFileName(const QString &name)
{
    if (!m_navigateBar)
        return;

    m_navigateBar->setFileName(name);
}

}  // NameSpace Digikam

