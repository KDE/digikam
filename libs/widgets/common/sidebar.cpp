/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in gui.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>  
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

/** @file sidebar.cpp */

// QT includes.

#include <QSplitter>
#include <QStackedWidget>
#include <QDataStream>
#include <QPixmap>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kmultitabbar.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kconfiggroup.h>
// Local includes.

#include "ddebug.h"
#include "sidebar.h"
#include "sidebar.moc"

namespace Digikam
{

class SidebarPriv
{
public:

    SidebarPriv()
    {
        minimizedDefault = false;
        minimized        = false;
        isMinimized      = false;

        tabs             = 0;
        activeTab        = -1;
        minSize          = 0;
        maxSize          = 0;

        stack            = 0;
        splitter         = 0;
    }

    bool            minimizedDefault;
    bool            minimized;
    bool            isMinimized;      // Backup of minimized status (used with Fullscreen)

    int             tabs;
    int             activeTab;
    int             minSize;
    int             maxSize;

    QStackedWidget *stack;
    QSplitter      *splitter;
    QSize           bigSize;

    Sidebar::Side   side;
};

Sidebar::Sidebar(QWidget *parent, Side side, bool minimizedDefault)
       : KMultiTabBar(KMultiTabBar::Left, parent)
{
    d = new SidebarPriv;
    d->minimizedDefault = minimizedDefault;
    d->side             = side;
    setStyle(KMultiTabBar::VSNET);
}

Sidebar::~Sidebar()
{
    saveViewState();
    delete d;
}

void Sidebar::setSplitter(QSplitter *sp)
{
    d->stack = new QStackedWidget(sp);

    if(d->side == DockLeft)
        setPosition(KMultiTabBar::Left);
    else
        setPosition(KMultiTabBar::Right);
}

void Sidebar::loadViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("%1").arg(objectName()));

    int tab        = group.readEntry("ActiveTab", 0);
    bool minimized = group.readEntry("Minimized", d->minimizedDefault);

    if (minimized)
    {
        d->activeTab = tab;
        //setTab(d->activeTab, true);
        d->stack->setCurrentIndex(d->activeTab);
        emit signalChangedTab(d->stack->currentWidget());
    }
    else
        d->activeTab = -1;

    clicked(tab);
}

void Sidebar::saveViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("%1").arg(objectName()));
    group.writeEntry("ActiveTab", d->activeTab);
    group.writeEntry("Minimized", d->minimized);
    config->sync();
}

void Sidebar::backup()
{
    d->isMinimized = d->minimized;

    if (!d->isMinimized) 
        shrink();

    KMultiTabBar::hide();
}

void Sidebar::restore()
{
    if (!d->isMinimized) 
        expand();

    KMultiTabBar::show();
}

void Sidebar::appendTab(QWidget *w, const QPixmap &pic, const QString &title)
{
    w->setParent(d->stack);
    KMultiTabBar::appendTab(pic, d->tabs, title);
    d->stack->insertWidget(d->tabs, w);

    connect(tab(d->tabs), SIGNAL(clicked(int)),
            this, SLOT(clicked(int)));

    d->tabs++;
}

void Sidebar::deleteTab(QWidget *w)
{
    int tab = d->stack->indexOf(w);
    if(tab < 0)
        return;

    if(tab == d->activeTab)
        d->activeTab = -1;

    removeTab(tab);
    //TODO show another widget
}

void Sidebar::clicked(int tab)
{
    if(tab >= d->tabs || tab < 0)
        return;

    if(tab == d->activeTab)
    {
        d->stack->isHidden() ? expand() : shrink();
    }
    else
    {
        if(d->activeTab >= 0)
            setTab(d->activeTab, false);

        d->activeTab = tab;
        setTab(d->activeTab, true);
        d->stack->setCurrentIndex(d->activeTab);

        if(d->minimized)
            expand();

        emit signalChangedTab(d->stack->currentWidget());
    }
}

void Sidebar::setActiveTab(QWidget *w)
{
    int tab = d->stack->indexOf(w);
    if(tab < 0)
        return;

    if(d->activeTab >= 0)
        setTab(d->activeTab, false);

    d->activeTab = tab;
    setTab(d->activeTab, true);
    d->stack->setCurrentIndex(d->activeTab);

    if(d->minimized)
        expand();

    emit signalChangedTab(d->stack->currentWidget());
}

QWidget* Sidebar::getActiveTab()
{
    return d->stack->currentWidget();
}

void Sidebar::shrink()
{
    d->minimized = true;
    d->bigSize   = size();
    d->minSize   = minimumWidth();
    d->maxSize   = maximumWidth();

    d->stack->hide();

    KMultiTabBarTab* tab = this->tab(0);
    if (tab)
        setFixedWidth(tab->width());
    else
        setFixedWidth(width());

    emit signalViewChanged();
}

void Sidebar::expand()
{
    d->minimized = false;
    d->stack->show();
    resize(d->bigSize);
    setMinimumWidth(d->minSize);
    setMaximumWidth(d->maxSize);
    emit signalViewChanged();
}

bool Sidebar::isExpanded()
{
    return !d->minimized; 
}

}  // namespace Digikam
