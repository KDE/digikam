/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : a full screen manager for digiKam XML GUI windows
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QString>
#include <QList>

// KDE includes

#include <kxmlguiwindow.h>
#include <ktogglefullscreenaction.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <ktoolbar.h>

// Local includes

#include "fullscreenmngr.h"

namespace Digikam
{

class FullScreenMngr::Private
{
public:

    Private()
    {
        win                    = 0;
        removeFullScreenButton = false;
    }

    /** Used by switchWindowToFullScreen() to switch tool-bar visibility in managed window
     */
    void hideToolBars();
    void showToolBars();

public:

    /** Windo instance to manage */
    KXmlGuiWindow*           win;

    /** Used by switchWindowToFullScreen() to manage state of full-screen button on managed window
     */
    bool                     removeFullScreenButton;

    static const QString     configFullScreenEntry;
    static const QString     configFullScreenHideThumbBarEntry;
    static const QString     configFullScreenHideToolBarEntry;
};

const QString FullScreenMngr::Private::configFullScreenEntry("FullScreen");
const QString FullScreenMngr::Private::configFullScreenHideThumbBarEntry("FullScreen Hide ThumbBar");
const QString FullScreenMngr::Private::configFullScreenHideToolBarEntry("FullScreen Hide ToolBar");

void FullScreenMngr::Private::hideToolBars()
{
    QList<KToolBar*> toolbars = win->toolBars();

    foreach(KToolBar* const toolbar, toolbars)
    {
        toolbar->hide();
    }
}

void FullScreenMngr::Private::showToolBars()
{
    QList<KToolBar*> toolbars = win->toolBars();

    foreach(KToolBar* const toolbar, toolbars)
    {
        toolbar->show();
    }
}

// --------------------------------------------------------------------------------------------------------

FullScreenMngr::FullScreenMngr()
    : d(new Private)
{
    m_fullScreenHideToolBar  = false;
    m_fullScreenHideThumbBar = true;
    m_fullScreenAction       = 0;
}

FullScreenMngr::~FullScreenMngr()
{
    delete d;
}

void FullScreenMngr::setManagedWindow(KXmlGuiWindow* const win)
{
    d->win = win;
}

void FullScreenMngr::readSettings(const KConfigGroup& group)
{
    m_fullScreenHideToolBar  = group.readEntry(d->configFullScreenHideToolBarEntry,  false);
    m_fullScreenHideThumbBar = group.readEntry(d->configFullScreenHideThumbBarEntry, true);

    if (group.readEntry(d->configFullScreenEntry, false))
    {
        if (m_fullScreenAction)
            m_fullScreenAction->activate(QAction::Trigger);
    }
}

void FullScreenMngr::saveSettings(KConfigGroup& group)
{
    group.writeEntry(d->configFullScreenHideToolBarEntry,  m_fullScreenHideToolBar);
    group.writeEntry(d->configFullScreenHideThumbBarEntry, m_fullScreenHideThumbBar);

    if (m_fullScreenAction)
        group.writeEntry(d->configFullScreenEntry, m_fullScreenAction->isChecked());
}

void FullScreenMngr::switchWindowToFullScreen(bool set)
{
    KToggleFullScreenAction::setFullScreen(d->win, set);

    if (!set)
    {
        kDebug() << "TURN OFF fullscreen";

        d->win->menuBar()->show();
        d->win->statusBar()->show();
        d->showToolBars();

        if (d->removeFullScreenButton)
        {
            QList<KToolBar*> toolbars = d->win->toolBars();

            foreach(KToolBar* const toolbar, toolbars)
            {
                // NOTE: name must be configured properly in ui.rc XML file of managed window

                if (toolbar->objectName() == "mainToolBar")
                {
                    toolbar->removeAction(m_fullScreenAction);
                    break;
                }
            }
        }
    }
    else
    {
        kDebug() << "TURN ON fullscreen";

        // hide menubar and statusbar
        d->win->menuBar()->hide();
        d->win->statusBar()->hide();

        if (m_fullScreenHideToolBar)
        {
            d->hideToolBars();
        }
        else
        {
            d->showToolBars();

            QList<KToolBar*> toolbars = d->win->toolBars();
            KToolBar* mainToolbar     = 0;

            foreach(KToolBar* const toolbar, toolbars)
            {
                if (toolbar->objectName() == "mainToolBar")
                {
                    mainToolbar = toolbar;
                    break;
                }
            }

            kDebug() << mainToolbar;

            // add fullscreen action if necessary in toolbar

            if (mainToolbar && !mainToolbar->actions().contains(m_fullScreenAction))
            {
                if (mainToolbar->actions().isEmpty())
                {
                    mainToolbar->addAction(m_fullScreenAction);
                }
                else
                {
                    mainToolbar->insertAction(mainToolbar->actions().first(), m_fullScreenAction);
                }

                d->removeFullScreenButton = true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->removeFullScreenButton = false;
            }
        }
    }
}

void FullScreenMngr::escapePressed()
{
    if (m_fullScreenAction->isChecked())
    {
        m_fullScreenAction->activate(QAction::Trigger);
    }
}

} // namespace Digikam
