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
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "sidebar.moc"

// Qt includes

#include <QDataStream>
#include <QDragEnterEvent>
#include <QEvent>
#include <QPixmap>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QHash>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>

namespace Digikam
{

class SidebarState
{

public:

    SidebarState() : activeWidget(0), size(0) {}
    SidebarState(QWidget* const w, int size) : activeWidget(w), size(size) {}

    QWidget* activeWidget;
    int      size;
};

class Sidebar::Private
{

public:

    Private() :
        minimizedDefault(false),
        minimized(false),
        isMinimized(false),
        tabs(0),
        activeTab(-1),
        dragSwitchId(-1),
        restoreSize(0),
        stack(0),
        splitter(0),
        dragSwitchTimer(0),
        appendedTabsStateCache(),
        optionActiveTabEntry("ActiveTab"),
        optionMinimizedEntry("Minimized"),
        optionRestoreSizeEntry("RestoreSize")
    {
    }

    bool                          minimizedDefault;
    bool                          minimized;
    bool                          isMinimized;      // Backup of shrinked status before backup(), restored by restore()
                                                    // NOTE: when sidebar is hidden, only icon bar is affected. If sidebar view is 
                                                    // visible, this one must be shrink and restored accordingly.

    int                           tabs;
    int                           activeTab;
    int                           dragSwitchId;
    int                           restoreSize;

    QStackedWidget*               stack;
    SidebarSplitter*              splitter;
    QTimer*                       dragSwitchTimer;

    QHash<QWidget*, SidebarState> appendedTabsStateCache;

    const QString                 optionActiveTabEntry;
    const QString                 optionMinimizedEntry;
    const QString                 optionRestoreSizeEntry;
};

class SidebarSplitter::Private
{
public:

    QList<Sidebar*> sidebars;
};

// -------------------------------------------------------------------------------------

Sidebar::Sidebar(QWidget* const parent, SidebarSplitter* const sp, KMultiTabBarPosition side, bool minimizedDefault)
    : KMultiTabBar(side, parent), StateSavingObject(this), d(new Private)
{
    d->splitter         = sp;
    d->minimizedDefault = minimizedDefault;
    d->stack            = new QStackedWidget(d->splitter);
    d->dragSwitchTimer  = new QTimer(this);

    connect(d->dragSwitchTimer, SIGNAL(timeout()),
            this, SLOT(slotDragSwitchTimer()));

    d->splitter->d->sidebars << this;

    setStyle(KMultiTabBar::VSNET);
}

Sidebar::~Sidebar()
{
    saveState();

    if (d->splitter)
    {
        d->splitter->d->sidebars.removeAll(this);
    }

    delete d;
}

SidebarSplitter* Sidebar::splitter() const
{
    return d->splitter;
}

void Sidebar::doLoadState()
{
    KConfigGroup group        = getConfigGroup();
    int tab                   = group.readEntry(entryName(d->optionActiveTabEntry),   0);
    bool minimized            = group.readEntry(entryName(d->optionMinimizedEntry),   d->minimizedDefault);
    d->restoreSize            = group.readEntry(entryName(d->optionRestoreSizeEntry), -1);

    // validate
    if (tab >= d->tabs || tab < 0)
    {
        tab = 0;
    }

    if (minimized)
    {
        d->activeTab = tab;
        setTab(d->activeTab, false);
        d->stack->setCurrentIndex(d->activeTab);
        shrink();
        emit signalChangedTab(d->stack->currentWidget());
        return;
    }

    d->activeTab = -1;
    clicked(tab);
}

void Sidebar::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    group.writeEntry(entryName(d->optionActiveTabEntry),   d->activeTab);
    group.writeEntry(entryName(d->optionMinimizedEntry),   d->minimized);
    group.writeEntry(entryName(d->optionRestoreSizeEntry), d->minimized ? d->restoreSize : -1);
}

void Sidebar::backup()
{
    // backup preview state of sidebar view (shrink or not)
    d->isMinimized = d->minimized;

    // In all case, shrink sidebar view 
    shrink();

    KMultiTabBar::hide();
}

void Sidebar::backup(const QList<QWidget*> thirdWidgetsToBackup, QList<int>* const sizes)
{
    sizes->clear();

    foreach(QWidget* const widget, thirdWidgetsToBackup)
    {
        *sizes << d->splitter->size(widget);
    }

    backup();
}

void Sidebar::restore()
{
    KMultiTabBar::show();

    // restore preview state of sidebar view, stored in backup()
    if (!d->isMinimized)
    {
        expand();
    }
}

void Sidebar::restore(const QList<QWidget*> thirdWidgetsToRestore, const QList<int>& sizes)
{
    restore();

    if (thirdWidgetsToRestore.size() == sizes.size())
    {
        for (int i=0; i<thirdWidgetsToRestore.size(); ++i)
        {
            d->splitter->setSize(thirdWidgetsToRestore.at(i), sizes.at(i));
        }
    }
}

void Sidebar::appendTab(QWidget* const w, const QPixmap& pic, const QString& title)
{
    // Store state (but not on initialization)
    if (isVisible())
    {
        d->appendedTabsStateCache[w] = SidebarState(d->stack->currentWidget(), d->splitter->size(this));
    }

    // Add tab
    w->setParent(d->stack);
    KMultiTabBar::appendTab(pic, d->tabs, title);
    d->stack->insertWidget(d->tabs, w);

    tab(d->tabs)->setAcceptDrops(true);
    tab(d->tabs)->installEventFilter(this);

    connect(tab(d->tabs), SIGNAL(clicked(int)),
            this, SLOT(clicked(int)));

    d->tabs++;
}

void Sidebar::deleteTab(QWidget* const w)
{
    int tab = d->stack->indexOf(w);

    if (tab < 0)
    {
        return;
    }

    bool removingActiveTab = (tab == d->activeTab);

    if (removingActiveTab)
    {
        d->activeTab = -1;
    }

    d->stack->removeWidget(d->stack->widget(tab));
    // delete widget
    removeTab(tab);
    d->tabs--;

    // restore or reset active tab and width
    if (!d->minimized)
    {
        // restore to state before adding tab
        // using a hash is simple, but does not handle well multiple add/removal operations at a time
        SidebarState state = d->appendedTabsStateCache.take(w);

        if (state.activeWidget)
        {
            int tab = d->stack->indexOf(state.activeWidget);

            if (tab != -1)
            {
                switchTabAndStackToTab(tab);
                emit signalChangedTab(d->stack->currentWidget());

                if (state.size == 0)
                {
                    d->minimized = true;
                    setTab(d->activeTab, false);
                }

                d->splitter->setSize(this, state.size);
            }
        }
        else
        {
            if (removingActiveTab)
            {
                clicked(d->tabs - 1);
            }

            d->splitter->setSize(this, -1);
        }
    }
    else
    {
        d->restoreSize = -1;
    }
}

void Sidebar::clicked(int tab)
{
    if (tab >= d->tabs || tab < 0)
    {
        return;
    }

    if (tab == d->activeTab)
    {
        d->stack->isHidden() ? expand() : shrink();
    }
    else
    {
        switchTabAndStackToTab(tab);

        if (d->minimized)
        {
            expand();
        }

        emit signalChangedTab(d->stack->currentWidget());
    }
}

void Sidebar::setActiveTab(QWidget* const w)
{
    int tab = d->stack->indexOf(w);

    if (tab < 0)
    {
        return;
    }

    switchTabAndStackToTab(tab);

    if (d->minimized)
    {
        expand();
    }

    emit signalChangedTab(d->stack->currentWidget());
}

void Sidebar::activePreviousTab()
{
    int tab = d->stack->indexOf(d->stack->currentWidget());

    if (tab == 0)
        tab = d->tabs-1;
    else
        tab--;

    setActiveTab(d->stack->widget(tab));
}

void Sidebar::activeNextTab()
{
    int tab = d->stack->indexOf(d->stack->currentWidget());

    if (tab== d->tabs-1)
        tab = 0;
    else
        tab++;

    setActiveTab(d->stack->widget(tab));
}

void Sidebar::switchTabAndStackToTab(int tab)
{
    if (d->activeTab >= 0)
    {
        setTab(d->activeTab, false);
    }

    d->activeTab = tab;
    setTab(d->activeTab, true);
    d->stack->setCurrentIndex(d->activeTab);
}

QWidget* Sidebar::getActiveTab() const
{
    if (d->splitter)
    {
        return d->stack->currentWidget();
    }
    else
    {
        return 0;
    }
}

void Sidebar::shrink()
{
    d->minimized = true;

    // store the size that we had. We may later need it when we restore to visible.
    int currentSize = d->splitter->size(this);

    if (currentSize)
    {
        d->restoreSize = currentSize;
    }

    d->stack->hide();
    emit signalViewChanged();
}

void Sidebar::expand()
{
    d->minimized = false;
    d->stack->show();

    // Do not expand to size 0 (only splitter handle visible)
    // but either to previous size, or the minimum size hint
    if (d->splitter->size(this) == 0)
    {
        setTab(d->activeTab, true);
        d->splitter->setSize(this, d->restoreSize ? d->restoreSize : -1);
    }

    emit signalViewChanged();
}

bool Sidebar::isExpanded() const
{
    return !d->minimized;
}

bool Sidebar::eventFilter(QObject* obj, QEvent* ev)
{
    for (int i = 0 ; i < d->tabs; ++i)
    {
        if ( obj == tab(i) )
        {
            if ( ev->type() == QEvent::DragEnter)
            {
                QDragEnterEvent* const e = static_cast<QDragEnterEvent*>(ev);
                enterEvent(e);
                e->accept();
                return false;
            }
            else if (ev->type() == QEvent::DragMove)
            {
                if (!d->dragSwitchTimer->isActive())
                {
                    d->dragSwitchTimer->setSingleShot(true);
                    d->dragSwitchTimer->start(800);
                    d->dragSwitchId = i;
                }

                return false;
            }
            else if (ev->type() == QEvent::DragLeave)
            {
                d->dragSwitchTimer->stop();
                QDragLeaveEvent* const e = static_cast<QDragLeaveEvent*>(ev);
                leaveEvent(e);
                return false;
            }
            else if (ev->type() == QEvent::Drop)
            {
                d->dragSwitchTimer->stop();
                QDropEvent* const e = static_cast<QDropEvent*>(ev);
                leaveEvent(e);
                return false;
            }
            else
            {
                return false;
            }
        }
    }

    // Else, pass the event on to the parent class
    return KMultiTabBar::eventFilter(obj, ev);
}

void Sidebar::slotDragSwitchTimer()
{
    clicked(d->dragSwitchId);
}

void Sidebar::slotSplitterBtnClicked()
{
    clicked(d->activeTab);
}

// -----------------------------------------------------------------------------

const QString SidebarSplitter::DEFAULT_CONFIG_KEY = "SplitterState";

SidebarSplitter::SidebarSplitter(QWidget* const parent)
    : QSplitter(parent), d(new Private)
{
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(slotSplitterMoved(int,int)));
}

SidebarSplitter::SidebarSplitter(Qt::Orientation orientation, QWidget* const parent)
    : QSplitter(orientation, parent), d(new Private)
{
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(slotSplitterMoved(int,int)));
}

SidebarSplitter::~SidebarSplitter()
{
    // retreat cautiously from sidebars that live longer
    foreach(Sidebar* const sidebar, d->sidebars)
    {
        sidebar->d->splitter = 0;
    }

    delete d;
}

void SidebarSplitter::restoreState(KConfigGroup& group)
{
    restoreState(group, DEFAULT_CONFIG_KEY);
}

void SidebarSplitter::restoreState(KConfigGroup& group, const QString& key)
{
    if (group.hasKey(key))
    {
        QByteArray state;
        state = group.readEntry(key, state);
        QSplitter::restoreState(QByteArray::fromBase64(state));
    }
}

void SidebarSplitter::saveState(KConfigGroup& group)
{
    saveState(group, DEFAULT_CONFIG_KEY);
}

void SidebarSplitter::saveState(KConfigGroup& group, const QString& key)
{
    group.writeEntry(key, QSplitter::saveState().toBase64());
}

int SidebarSplitter::size(Sidebar* const bar) const
{
    return size(bar->d->stack);
}

int SidebarSplitter::size(QWidget* const widget) const
{
    int index = indexOf(widget);

    if (index == -1)
    {
        return -1;
    }

    return sizes().at(index);
}

void SidebarSplitter::setSize(Sidebar* const bar, int size)
{
    setSize(bar->d->stack, size);
}

void SidebarSplitter::setSize(QWidget* const widget, int size)
{
    int index = indexOf(widget);

    if (index == -1)
    {
        return;
    }

    // special case: Use minimum size hint
    if (size == -1)
    {
        if (orientation() == Qt::Horizontal)
        {
            size = widget->minimumSizeHint().width();
        }

        if (orientation() == Qt::Vertical)
        {
            size = widget->minimumSizeHint().height();
        }
    }

    QList<int> sizeList = sizes();
    sizeList[index] = size;
    setSizes(sizeList);
}

void SidebarSplitter::slotSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos);

    // When the user moves the splitter so that size is 0 (collapsed),
    // it may be difficult to restore the sidebar as clicking the buttons
    // has no effect (only hides/shows the splitter handle)
    // So we want to transform this size-0-sidebar
    // to a sidebar that is shrunk (d->stack hidden)
    // and can be restored by clicking a tab bar button

    // We need to look at the widget between index-1 and index
    // and the one between index and index+1

    QList<int> sizeList = sizes();

    // Is there a sidebar with size 0 before index ?

    if (index > 0 && sizeList.at(index-1) == 0)
    {
        QWidget* const w = widget(index-1);

        foreach(Sidebar* const sidebar, d->sidebars)
        {
            if (w == sidebar->d->stack)
            {
                if (!sidebar->d->minimized)
                {
                    sidebar->setTab(sidebar->d->activeTab, false);
                    sidebar->shrink();
                }

                break;
            }
        }
    }

    // Is there a sidebar with size 0 after index ?

    if (sizeList.at(index) == 0)
    {
        QWidget* const w = widget(index);

        foreach(Sidebar* sidebar, d->sidebars)
        {
            if (w == sidebar->d->stack)
            {
                if (!sidebar->d->minimized)
                {
                    sidebar->setTab(sidebar->d->activeTab, false);
                    sidebar->shrink();
                }

                break;
            }
        }
    }
}

}  // namespace Digikam
