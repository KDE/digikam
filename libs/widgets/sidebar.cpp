/* ============================================================
 * File  : sidebar.cpp
 * Author: Jörn Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-03-22
 * Copyright 2005 by Jörn Ahrens
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
 * ============================================================ */

/** @file sidebar.cpp */

#include "sidebar.h"

#include <qsplitter.h>
#include <qwidgetstack.h>
#include <qdatastream.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kmultitabbar.h>
#include <kiconloader.h>

using namespace Digikam;

Sidebar::Sidebar(QWidget *parent, Side side, bool minimizedDefault)
    : KMultiTabBar(KMultiTabBar::Vertical, parent, "Sidebar")
{
    m_tabs = 0;
    m_activeTab = -1;
    m_minimized = false;
    m_minimizedDefault = minimizedDefault;
    m_side = side;
}

Sidebar::~Sidebar()
{
    saveViewState();
}

void Sidebar::setSplitter(QSplitter *sp)
{
#if KDE_IS_VERSION(3,3,0)
    setStyle(KMultiTabBar::KDEV3ICON);
#else
    setStyle(KMultiTabBar::KDEV3);
#endif
    showActiveTabTexts(true);
    m_stack = new QWidgetStack(sp);
            
    if(m_side == Left)
        setPosition(KMultiTabBar::Left);
    else
        setPosition(KMultiTabBar::Right);
}

void Sidebar::loadViewState()
{
    int tab;
    int minimized;
    
    KConfig *config = kapp->config();
    config->setGroup(QString("%1-%2").arg(name()).arg(m_side));
   
    tab = config->readNumEntry("ActiveTab", 0);
    minimized = config->readNumEntry("Minimized", m_minimizedDefault);
        
    if(minimized)
    {
        m_activeTab = tab;
        setTab(m_activeTab, true);
        m_stack->raiseWidget(m_activeTab);

        emit signalChangedTab(m_stack->visibleWidget());        
    }
    else
        m_activeTab = -1;
    
    clicked(tab);
}

void Sidebar::saveViewState()
{
    KConfig *config = kapp->config();
    config->setGroup(QString("%1-%2").arg(name()).arg(m_side));
    
    config->writeEntry("ActiveTab", m_activeTab);
    config->writeEntry("Minimized", (int)m_minimized);
}

void Sidebar::appendTab(QWidget *w, const QPixmap &pic, const QString &title)
{
    w->reparent(m_stack, QPoint(0,0));
    KMultiTabBar::appendTab(pic, m_tabs, title);
    m_stack->addWidget(w, m_tabs);

    connect(tab(m_tabs), SIGNAL(clicked(int)),
            this, SLOT(clicked(int)));
    
    m_tabs++;
}

void Sidebar::deleteTab(QWidget *w)
{
    int tab = m_stack->id(w);
    if(tab < 0)
        return;
    
    if(tab == m_activeTab)
        m_activeTab = -1;
    
    removeTab(tab);
    //TODO show another widget
}
 
void Sidebar::clicked(int tab)
{
    if(tab >= m_tabs || tab < 0)
        return;
    
    if(tab == m_activeTab)
    {
        m_stack->isHidden() ? expand() : shrink();
    }
    else
    {
        if(m_activeTab >= 0)
            setTab(m_activeTab, false);
    
        m_activeTab = tab;    
        setTab(m_activeTab, true);
        m_stack->raiseWidget(m_activeTab);
        
        if(m_minimized)
            expand();

        emit signalChangedTab(m_stack->visibleWidget());
    }
}

void Sidebar::setActiveTab(QWidget *w)
{
    int tab = m_stack->id(w);
    if(tab < 0)
        return;
    
    if(m_activeTab >= 0)
        setTab(m_activeTab, false);
    
    m_activeTab = tab;    
    setTab(m_activeTab, true);
    m_stack->raiseWidget(m_activeTab);
        
    if(m_minimized)
        expand();    

    emit signalChangedTab(m_stack->visibleWidget());
}

QWidget* Sidebar::getActiveTab()
{
    return m_stack->visibleWidget();
}

void Sidebar::shrink()
{
    m_minimized = true;
    m_bigSize = size();
    m_minSize = minimumWidth();
    m_maxSize = maximumWidth();
            
    m_stack->hide();

    KMultiTabBarTab* tab = tabs()->first();
    if (tab)
    {
        setFixedWidth(tab->width());
    }
    else
    {
        setFixedWidth(width());
    }
}

void Sidebar::expand()
{
    m_minimized = false;
    m_stack->show();
    resize(m_bigSize);
    setMinimumWidth(m_minSize);
    setMaximumWidth(m_maxSize);
}

#include "sidebar.moc"
