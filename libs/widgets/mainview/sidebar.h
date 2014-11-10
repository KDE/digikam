/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in GUI.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SIDEBAR_H
#define SIDEBAR_H

// Qt includes

#include <QtGui/QPixmap>
#include <QtGui/QSplitter>

// KDE includes

#include <kconfiggroup.h>
#include <kmultitabbar.h>

// Local includes

#include "digikam_export.h"
#include "statesavingobject.h"

class QSplitter;

namespace Digikam
{

class SidebarSplitter;

/**
 * This class handles a sidebar view
 *
 * Since this class derives from StateSavingObject, you can call
 * StateSavingObject#loadState() and StateSavingObject#saveState()
 * for loading/saving of settings. However, if you use multiple
 * sidebar instances in your program, you have to remember to either
 * call QObject#setObjectName(), StateSavingObject#setEntryPrefix() or
 * StateSavingObject#setConfigGroup() first.
 */
class DIGIKAM_EXPORT Sidebar : public KMultiTabBar, public StateSavingObject
{
    Q_OBJECT

public:

    /**
     * Creates a new sidebar
     * @param parent sidebar's parent
     * @param sp sets the splitter, which should handle the width. The splitter normally
     *           is part of the main view. Internally, the width of the widget stack can
     *           be changed by a QSplitter.
     * @param side where the sidebar should be displayed. At the left or right border.
                   Use KMultiTabBar::Left or KMultiTabBar::Right.
     * @param minimizedDefault hide the sidebar when the program is started the first time.
     */
    Sidebar(QWidget* const parent, SidebarSplitter* const sp, KMultiTabBarPosition side=KMultiTabBar::Left,
            bool minimizedDefault=false);

    virtual ~Sidebar();

    SidebarSplitter* splitter() const;

    /**
     * Appends a new tab to the sidebar
     * @param w widget which is activated by this tab
     * @param pic icon which is shown in this tab
     * @param title text which is shown it this tab
     */
    void appendTab(QWidget* const w, const QPixmap& pic, const QString& title);

    /**
     * Deletes a tab from the tabbar
     */
    void deleteTab(QWidget* const w);

    /**
     * Activates a tab
     */
    void setActiveTab(QWidget* const w);

    /**
     * Activates a next tab from current one. If current one is last, first one is actived.
     */
    void activeNextTab();

    /**
     * Activates a previous tab from current one. If current one is first, last one is actived.
     */
    void activePreviousTab();

    /**
     * Returns the currently activated tab, or 0 if no tab is active
    */
    QWidget* getActiveTab() const;

    /**
     * Hides the sidebar (display only the activation buttons)
     */
    void shrink();

    /**
     * redisplays the whole sidebar
     */
    void expand();

    /**
     * hide sidebar and backup minimized state.
     */
    void backup();

    /**
     * Hide sidebar and backup minimized state.
     * If there are other widgets in this splitter, stores
     * their sizes in the provided list.
     */
    void backup(const QList<QWidget*> thirdWidgetsToBackup, QList<int>* const sizes);

    /**
     * show sidebar and restore minimized state.
     */
    void restore();

    /**
     * Show sidebar and restore minimized state.
     * Restores other widgets' sizes in splitter.
     */
    void restore(const QList<QWidget*> thirdWidgetsToRestore, const QList<int>& sizes);

    /**
     * return the visible status of current sidebar tab.
     */
    bool isExpanded() const;

protected:

    /**
     * load the last view state from disk - called by StateSavingObject#loadState()
     */
    void doLoadState();

    /**
     * save the view state to disk - called by StateSavingObject#saveState()
     */
    void doSaveState();

private:

    bool eventFilter(QObject* o, QEvent* e);
    void switchTabAndStackToTab(int tab);

private Q_SLOTS:

    /**
     * Activates a tab
     */
    void clicked(int tab);

    void slotDragSwitchTimer();

    void slotSplitterBtnClicked();

Q_SIGNALS:

    /**
     * is emitted, when another tab is activated
     */
    void signalChangedTab(QWidget* w);

    /**
     * is emitted, when tab is shrink or expanded
     */
    void signalViewChanged();

private:

    friend class SidebarSplitter;

    class Private;
    Private* const d;
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT SidebarSplitter : public QSplitter
{
    Q_OBJECT

public:

    const static QString DEFAULT_CONFIG_KEY;

    /**
     *  This is a QSplitter with better support for storing its state
     *  in config files, especially if Sidebars are contained in the splitter.
     */
    explicit SidebarSplitter(QWidget* const parent = 0);
    explicit SidebarSplitter(Qt::Orientation orientation, QWidget* const parent = 0);

    ~SidebarSplitter();

    /**
     * Saves the splitter state to group, handling minimized sidebars correctly.
     * DEFAULT_CONFIG_KEY is used for storing the state.
     */
    void saveState(KConfigGroup& group);
    /**
     * Saves the splitter state to group, handling minimized sidebars correctly.
     * This version uses a specified key in the config group.
     */
    void saveState(KConfigGroup& group, const QString& key);
    /**
     * Restores the splitter state from group, handling minimized sidebars correctly.
     * DEFAULT_CONFIG_KEY is used for restoring the state.
     */
    void restoreState(KConfigGroup& group);
    /**
     * Restores the splitter state from group, handling minimized sidebars correctly.
     * This version uses a specified key in the config group.
     */
    void restoreState(KConfigGroup& group, const QString& key);

    /**
     * Returns the value of sizes() that corresponds to the given Sidebar or splitter child widget.
     */
    int size(Sidebar* const bar) const;
    int size(QWidget* const widget) const;
    /**
     * Sets the splitter size for the given sidebar or splitter child widget to size.
     * Special value -1: Sets the minimum size hint of the widget.
     */
    void setSize(Sidebar* const bar, int size);
    void setSize(QWidget* const widget, int size);

    void addSplitterCollapserButton(QWidget* const widget);

private Q_SLOTS:

    void slotSplitterMoved(int pos, int index);

private:

    friend class Sidebar;

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SIDEBAR_H
