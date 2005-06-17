/* ============================================================
 * File  : sidebar.h
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

/** @file sidebar.h */

#ifndef _SIDEBAR_H_
#define _SIDEBAR_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kmultitabbar.h>

class KMultiTabBar;
class QWidgetStack;
class QDataStream;
class QSplitter;

/**
 * This class handles a sidebar view
 */
class Sidebar : public KMultiTabBar
{
    Q_OBJECT
public:

    /**
     * The side where the bar should be visible
     */
    enum Side {
        Left,
        Right
    };    
    
    Sidebar(QWidget *parent, Side side=Left);
    virtual ~Sidebar();

    void setSplitter(QSplitter *sp);
    
    /**
     * Appends a new tab to the sidebar
     */
    void appendTab(QWidget *w, const QPixmap &pic, const QString &title);
    
    /**
     * Deletes a tab from the tabbar
     */
    void deleteTab(QWidget *w);
    
    /**
     * Activates a tab
     */
    void setActiveTab(QWidget *w);
    
    /**
     * Returns the currently activated tab, or 0 if no tab is active
    */
    QWidget* getActiveTab();
    
    /**
     * Hides the sidebar (display only the activation buttons)
     */
    void shrink();
    
    /**
     * redisplays the whole sidebar
     */
    void expand();
                
    /**
     * load the last view state from disk
     */
    void loadViewState(QDataStream &stream);
    
    /**
     * save the view state to disk
     */
    void saveViewState(QDataStream &stream);
    
private:
    Side            m_side;
    QWidgetStack    *m_stack;
    QSplitter       *m_splitter;
    QSize            m_bigSize;
    int              m_minSize;
    int              m_maxSize;
    bool             m_minimized;
    int              m_tabs;
    int              m_activeTab;
    
private slots:
    
    /**
     * Activates a tab
     */
    void clicked(int tab);
    
signals:
    void            signalChangedTab(QWidget *w);
};

#endif // _SIDEBAR_H_
