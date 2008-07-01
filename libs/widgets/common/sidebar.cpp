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

// Qt includes.

#include <QSplitter>
#include <QStackedWidget>
#include <QDataStream>
#include <QPixmap>
#include <QTimer>
#include <QEvent>
#include <QDragEnterEvent>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kdeversion.h>
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
        dragSwitchId     = -1;

        stack            = 0;
        dragSwitchTimer  = 0;
    }

    bool            minimizedDefault;
    bool            minimized;
    bool            isMinimized;      // Backup of minimized status (used with Fullscreen)

    int             tabs;
    int             activeTab;
    int             minSize;
    int             maxSize;
    int             dragSwitchId;

    QStackedWidget *stack;
    QSize           bigSize;
    QTimer         *dragSwitchTimer; 
};

Sidebar::Sidebar(QWidget *parent, KMultiTabBarPosition side, bool minimizedDefault)
       : KMultiTabBar(side, parent)
{
    d = new SidebarPriv;
    d->minimizedDefault = minimizedDefault;
    d->dragSwitchTimer  = new QTimer(this);

    connect(d->dragSwitchTimer, SIGNAL(timeout()),
            this, SLOT(slotDragSwitchTimer()));

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
}

void Sidebar::loadViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("%1").arg(objectName()));
    int tab                   = group.readEntry("ActiveTab", 0);
    bool minimized            = group.readEntry("Minimized", d->minimizedDefault);

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
    KConfigGroup group        = config->group(QString("%1").arg(objectName()));
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

    tab(d->tabs)->setAcceptDrops(true);
    tab(d->tabs)->installEventFilter(this);

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

bool Sidebar::eventFilter(QObject *obj, QEvent *ev)
{
    for (int i = 0 ; i < d->tabs; i++)
    {
        if ( obj == tab(i) )
        {
            if ( ev->type() == QEvent::DragEnter)
            {
                QDragEnterEvent *e = static_cast<QDragEnterEvent *>(ev);
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
                QDragLeaveEvent *e = static_cast<QDragLeaveEvent *>(ev);
                leaveEvent(e);
                return false;
            }
            else if (ev->type() == QEvent::Drop)
            {
                d->dragSwitchTimer->stop();
                QDropEvent *e = static_cast<QDropEvent *>(ev);
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

}  // namespace Digikam
