/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in gui.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

/** @file sidebar.h */

#ifndef _SIDEBAR_H_
#define _SIDEBAR_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes.

#include <kmultitabbar.h>

// Local includes.

#include "digikam_export.h"

class QSplitter;

namespace Digikam
{

class SidebarPriv;

/**
 * This class handles a sidebar view
 */
class DIGIKAM_EXPORT Sidebar : public KMultiTabBar
{
    Q_OBJECT

public:

    /**
     * The side where the bar should be visible
     */
    enum Side
    {
        Left,
        Right
    };

    /**
     * Creates a new sidebar
     * @param parent sidebar's parent
     * @param name the name of the widget is used to store its state to config
     * @param side where the sidebar should be displayed. At the left or right border.
     * @param minimizedDefault hide the sidebar when the program is started the first time?
     */
    Sidebar(QWidget *parent, const char *name, Side side=Left, bool mimimizedDefault=false);
    virtual ~Sidebar();

    /**
     * The width of the widget stack can be changed by a QSplitter.
     * @param sp sets the splitter, which should handle the width. The splitter normally
     *           is part of the main view.
     */
    void setSplitter(QSplitter *sp);
    void setSplitterSizePolicy(QSizePolicy p);

    QSplitter* splitter() const;

    /**
     * Appends a new tab to the sidebar
     * @param w widget which is activated by this tab
     * @param pic icon which is shown in this tab
     * @param title text which is shown it this tab
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
    void loadViewState();

    /**
     * hide sidebar and backup minimized state.
     */ 
    void backup();

    /**
     * show sidebar and restore minimized state.
     */ 
    void restore();

    /**
     * return the visible status of current sidebar tab.
     */ 
    bool isExpanded();

private:

    /**
     * save the view state to disk
     */
    void saveViewState();
    bool eventFilter(QObject *o, QEvent *e);
    void updateMinimumWidth();

private slots:

    /**
     * Activates a tab
     */
    void clicked(int tab);

    void slotDragSwitchTimer();

signals:

    /**
     * is emitted, when another tab is activated
     */
    void signalChangedTab(QWidget *w);

    /**
     * is emitted, when tab is shrink or expanded
     */
    void signalViewChanged();

private:

    SidebarPriv* d;
};

}  // namespace Digikam

#endif // _SIDEBAR_H_
