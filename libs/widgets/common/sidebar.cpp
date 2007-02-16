/* ============================================================
 * Authors: Joern Ahrens <joern.ahrens@kdemail.net>
 *          Caulier Gilles <caulier dot gilles at gmail dot com>
 * Date   : 2005-03-22
 * Description : a widget to manage sidebar in gui.
 *
 * Copyright 2005-2006 by Joern Ahrens
 * Copyright 2006-2007 by Gilles Caulier  
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

#include <qsplitter.h>
#include <qwidgetstack.h>
#include <qdatastream.h>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kmultitabbar.h>
#include <kiconloader.h>

// Local includes.

#include "sidebar.h"
#include "sidebar.moc"

namespace Digikam
{
    
class SidebarPriv
{
public:

    SidebarPriv()
    {
        stack     = 0;
        splitter  = 0;
        tabs      = 0;
        activeTab = -1;
        minimized = false;
    }

    bool           minimizedDefault;
    bool           minimized;
    bool           isMinimized;      // Backup of minimized status (used with Fullscreen)

    int            tabs;
    int            activeTab;
    int            minSize;
    int            maxSize;
        
    QWidgetStack  *stack;
    QSplitter     *splitter;
    QSize          bigSize;

    Sidebar::Side  side;
};

Sidebar::Sidebar(QWidget *parent, const char *name, Side side, bool minimizedDefault)
       : KMultiTabBar(KMultiTabBar::Vertical, parent, name)
{
    d = new SidebarPriv;
    d->minimizedDefault = minimizedDefault;
    d->side             = side;
}

Sidebar::~Sidebar()
{
    saveViewState();
    delete d;
}

void Sidebar::setSplitter(QSplitter *sp)
{
#if KDE_IS_VERSION(3,3,0)
    setStyle(KMultiTabBar::KDEV3ICON);
#else
    setStyle(KMultiTabBar::KDEV3);
#endif
    showActiveTabTexts(true);
    d->stack = new QWidgetStack(sp);
            
    if(d->side == Left)
        setPosition(KMultiTabBar::Left);
    else
        setPosition(KMultiTabBar::Right);
}

void Sidebar::loadViewState()
{
    int tab;
    int minimized;
    
    KConfig *config = kapp->config();
    config->setGroup(QString("%1").arg(name()));
   
    tab = config->readNumEntry("ActiveTab", 0);
    minimized = config->readBoolEntry("Minimized", d->minimizedDefault);
        
    if(minimized)
    {
        d->activeTab = tab;
        setTab(d->activeTab, true);
        d->stack->raiseWidget(d->activeTab);

        emit signalChangedTab(d->stack->visibleWidget());
    }
    else
        d->activeTab = -1;
    
    clicked(tab);
}

void Sidebar::saveViewState()
{
    KConfig *config = kapp->config();
    config->setGroup(QString("%1").arg(name()));
    config->writeEntry("ActiveTab", d->activeTab);
    config->writeEntry("Minimized", d->minimized);
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
    w->reparent(d->stack, QPoint(0, 0));
    KMultiTabBar::appendTab(pic, d->tabs, title);
    d->stack->addWidget(w, d->tabs);

    connect(tab(d->tabs), SIGNAL(clicked(int)),
            this, SLOT(clicked(int)));
    
    d->tabs++;
}

void Sidebar::deleteTab(QWidget *w)
{
    int tab = d->stack->id(w);
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
        d->stack->raiseWidget(d->activeTab);
        
        if(d->minimized)
            expand();

        emit signalChangedTab(d->stack->visibleWidget());
    }
}

void Sidebar::setActiveTab(QWidget *w)
{
    int tab = d->stack->id(w);
    if(tab < 0)
        return;
    
    if(d->activeTab >= 0)
        setTab(d->activeTab, false);
    
    d->activeTab = tab;
    setTab(d->activeTab, true);
    d->stack->raiseWidget(d->activeTab);
        
    if(d->minimized)
        expand();    

    emit signalChangedTab(d->stack->visibleWidget());
}

QWidget* Sidebar::getActiveTab()
{
    return d->stack->visibleWidget();
}

void Sidebar::shrink()
{
    d->minimized = true;
    d->bigSize   = size();
    d->minSize   = minimumWidth();
    d->maxSize   = maximumWidth();
            
    d->stack->hide();

    KMultiTabBarTab* tab = tabs()->first();
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
