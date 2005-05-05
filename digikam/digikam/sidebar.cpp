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
#include <qlayout.h>
#include <qsize.h>

#include <kmultitabbar.h>
#include <kiconloader.h>
#include <kdebug.h>

Sidebar::Sidebar(QWidget *parent, Side side)
    : QFrame(parent, "sidebar")
{
    m_tabs = 0;
    m_activeTab = -1;
    m_minimized = false;
    
    QHBoxLayout *box = new QHBoxLayout(this);
    
    m_tabBar = new KMultiTabBar(KMultiTabBar::Vertical, this);
    m_tabBar->setStyle(KMultiTabBar::KDEV3ICON);
    m_tabBar->showActiveTabTexts(true);
    m_stack = new QWidgetStack(this);
            
    if(side == Left)
    {
        box->add(m_tabBar);
        box->add(m_stack);
        m_tabBar->setPosition(KMultiTabBar::Left);
    }
    else
    {
        box->add(m_stack);
        box->add(m_tabBar);
        m_tabBar->setPosition(KMultiTabBar::Right);
    }
 
    setMinimumWidth(m_tabBar->width());
}

Sidebar::~Sidebar()
{
}

void Sidebar::appendTab(QWidget *w, const QPixmap &pic, const QString &title)
{
    m_tabBar->appendTab(pic, m_tabs, title);
    m_stack->addWidget(w, m_tabs);

    connect(m_tabBar->tab(m_tabs), SIGNAL(clicked(int)),
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
    
    m_tabBar->removeTab(tab);
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
            m_tabBar->setTab(m_activeTab, false);
    
        m_activeTab = tab;    
        m_tabBar->setTab(m_activeTab, true);
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
    
    clicked(tab);
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
    setFixedWidth(m_tabBar->width());
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
