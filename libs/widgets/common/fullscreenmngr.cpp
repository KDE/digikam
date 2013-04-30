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
#include <QVBoxLayout>
#include <QCheckBox>

// KDE includes

#include <kxmlguiwindow.h>
#include <ktogglefullscreenaction.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kdialog.h>
#include <klocale.h>

// Local includes

#include "fullscreenmngr.h"

namespace Digikam
{

class FullScreenMngr::Private
{
public:

    Private()
    {
        options                = FS_DEFAULT;
        win                    = 0;
        fullScreenAction       = 0;
        removeFullScreenButton = false;
    }

    /** Used by switchWindowToFullScreen() to switch tool-bar visibility in managed window
     */
    void hideToolBars();
    void showToolBars();

public:

    int                      options;

    /** Windo instance to manage */
    KXmlGuiWindow*           win;

    /** Action plug in managed window to switch fullscreen state*/
    KToggleFullScreenAction* fullScreenAction;

    /** Used by switchWindowToFullScreen() to manage state of full-screen button on managed window
     */
    bool                     removeFullScreenButton;

    static const QString     configRestoreFullScreenEntry;
    static const QString     configFullScreenHideThumbBarEntry;
    static const QString     configFullScreenHideToolBarEntry;
};

const QString FullScreenMngr::Private::configRestoreFullScreenEntry("RestoreFullScreen");
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

FullScreenMngr::FullScreenMngr(int options)
    : d(new Private)
{
    d->options               = options;
    m_fullScreenHideToolBar  = false;
    m_fullScreenHideThumbBar = true;
}

FullScreenMngr::~FullScreenMngr()
{
    delete d;
}

void FullScreenMngr::setManagedWindow(KXmlGuiWindow* const win)
{
    d->win = win;
}

QAction* FullScreenMngr::createFullScreenAction(const QString& name)
{
    d->fullScreenAction = KStandardAction::fullScreen(0, 0, d->win, d->win);
    d->win->actionCollection()->addAction(name, d->fullScreenAction);
    return d->fullScreenAction;
}

void FullScreenMngr::readSettings(const KConfigGroup& group)
{
    if (d->options & FS_TOOLBAR)
        m_fullScreenHideToolBar  = group.readEntry(d->configFullScreenHideToolBarEntry,  false);

    if (d->options & FS_THUMBBAR)
        m_fullScreenHideThumbBar = group.readEntry(d->configFullScreenHideThumbBarEntry, true);

    if (group.readEntry(d->configRestoreFullScreenEntry, false))
    {
        if (d->fullScreenAction)
            d->fullScreenAction->activate(QAction::Trigger);
    }
}

void FullScreenMngr::saveSettings(KConfigGroup& group)
{
    if (d->options & FS_TOOLBAR)
        group.writeEntry(d->configFullScreenHideToolBarEntry,  m_fullScreenHideToolBar);

    if (d->options & FS_THUMBBAR)
        group.writeEntry(d->configFullScreenHideThumbBarEntry, m_fullScreenHideThumbBar);

    if (d->fullScreenAction)
        group.writeEntry(d->configRestoreFullScreenEntry, d->fullScreenAction->isChecked());
}

void FullScreenMngr::switchWindowToFullScreen(bool set)
{
    KToggleFullScreenAction::setFullScreen(d->win, set);

    if (!set)
    {
        kDebug() << "TURN OFF fullscreen";

        // restore menubar and statusbar

        d->win->menuBar()->show();
        d->win->statusBar()->show();

        // restore toolbar

        d->showToolBars();

        if (d->removeFullScreenButton)
        {
            QList<KToolBar*> toolbars = d->win->toolBars();

            foreach(KToolBar* const toolbar, toolbars)
            {
                // NOTE: name must be configured properly in ui.rc XML file of managed window

                if (toolbar->objectName() == "mainToolBar")
                {
                    toolbar->removeAction(d->fullScreenAction);
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

        // hide toolbar

        if ((d->options & FS_TOOLBAR) && m_fullScreenHideToolBar)
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

            if (mainToolbar && !mainToolbar->actions().contains(d->fullScreenAction))
            {
                if (mainToolbar->actions().isEmpty())
                {
                    mainToolbar->addAction(d->fullScreenAction);
                }
                else
                {
                    mainToolbar->insertAction(mainToolbar->actions().first(), d->fullScreenAction);
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
    if (d->fullScreenAction->isChecked())
    {
        d->fullScreenAction->activate(QAction::Trigger);
    }
}

// -------------------------------------------------------------------------------------------------------------

class FullScreenSettings::Private
{
public:

    Private()
    {
        options      = FS_DEFAULT;
        hideToolBar  = 0;
        hideThumbBar = 0;
    }

    int        options;

    QCheckBox* hideToolBar;
    QCheckBox* hideThumbBar;
};

FullScreenSettings::FullScreenSettings(int options, QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    d->options              = options;
    QVBoxLayout* const vlay = new QVBoxLayout(this);
    d->hideToolBar          = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"),  this);
    d->hideThumbBar         = new QCheckBox(i18n("Hide &thumbbar in fullscreen mode"), this);

    if (!(options & FS_TOOLBAR))  d->hideToolBar->hide();
    if (!(options & FS_THUMBBAR)) d->hideThumbBar->hide();

    vlay->addWidget(d->hideToolBar);
    vlay->addWidget(d->hideThumbBar);
    vlay->setMargin(0);
    vlay->setSpacing(KDialog::spacingHint());
}

FullScreenSettings::~FullScreenSettings()
{
    delete d;
}

void FullScreenSettings::readSettings(const KConfigGroup& group)
{
    FullScreenMngr mngr(d->options);
    mngr.readSettings(group);

    d->hideToolBar->setChecked(mngr.m_fullScreenHideToolBar);
    d->hideThumbBar->setChecked(mngr.m_fullScreenHideThumbBar);
}

void FullScreenSettings::saveSettings(KConfigGroup& group)
{
    FullScreenMngr mngr(d->options);

    mngr.m_fullScreenHideToolBar  = d->hideToolBar->isChecked();
    mngr.m_fullScreenHideThumbBar = d->hideThumbBar->isChecked();

    mngr.saveSettings(group);
}

} // namespace Digikam
