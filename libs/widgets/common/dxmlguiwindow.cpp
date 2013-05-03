/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window
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
#include <QToolButton>
#include <QEvent>
#include <QHoverEvent>

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

#include "dxmlguiwindow.h"

namespace Digikam
{

class DXmlGuiWindow::Private
{
public:

    Private()
    {
        options                = FS_DEFAULT;
        fullScreenAction       = 0;
        fullScreenBtn          = 0;
        removeFullScreenButton = false;
    }

public:

    int                      options;

    /** Action plug in managed window to switch fullscreen state */
    KToggleFullScreenAction* fullScreenAction;

    /** Show only if toolbar is hidden */
    QToolButton*             fullScreenBtn;

    /** Used by switchWindowToFullScreen() to manage state of full-screen button on managed window
     */
    bool                     removeFullScreenButton;
};

// --------------------------------------------------------------------------------------------------------

DXmlGuiWindow::DXmlGuiWindow(QWidget* const parent, Qt::WindowFlags f)
    : KXmlGuiWindow(parent, f), d(new Private)
{
    m_fullScreenHideToolBar  = false;
    m_fullScreenHideThumbBar = true;
    installEventFilter(this);
}

DXmlGuiWindow::~DXmlGuiWindow()
{
    delete d;
}

void DXmlGuiWindow::setFullScreenOptions(int options)
{
    d->options = options;
}

void DXmlGuiWindow::createFullScreenAction(const QString& name)
{
    d->fullScreenAction = KStandardAction::fullScreen(0, 0, this, this);
    actionCollection()->addAction(name, d->fullScreenAction);
    d->fullScreenBtn    = new QToolButton(this);
    d->fullScreenBtn->setDefaultAction(d->fullScreenAction);
    d->fullScreenBtn->hide();

    connect(d->fullScreenAction, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleFullScreen(bool)));
}

void DXmlGuiWindow::readFullScreenSettings(const KConfigGroup& group)
{
    if (d->options & FS_TOOLBAR)
        m_fullScreenHideToolBar  = group.readEntry(s_configFullScreenHideToolBarEntry,  false);

    if (d->options & FS_THUMBBAR)
        m_fullScreenHideThumbBar = group.readEntry(s_configFullScreenHideThumbBarEntry, true);

    if (group.readEntry(s_configRestoreFullScreenModeEntry, false))
    {
        if (d->fullScreenAction)
            d->fullScreenAction->activate(QAction::Trigger);
    }
}

void DXmlGuiWindow::saveFullScreenSettings(KConfigGroup& group)
{
    if (d->fullScreenAction)
        group.writeEntry(s_configRestoreFullScreenModeEntry, d->fullScreenAction->isChecked());
}

void DXmlGuiWindow::slotToggleFullScreen(bool set)
{
    KToggleFullScreenAction::setFullScreen(this, set);

    if (!set)
    {
        kDebug() << "TURN OFF fullscreen";

        // restore menubar, statusbar, and sidebar

        menuBar()->show();
        statusBar()->show();
        showSideBar(true);

        // restore toolbar

        showToolBars();
        d->fullScreenBtn->hide();

        if (d->removeFullScreenButton)
        {
            QList<KToolBar*> toolbars = toolBars();

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

        // hide menubar, statusbar, and sidebar

        menuBar()->hide();
        statusBar()->hide();
        showSideBar(false);

        // hide toolbar

        if ((d->options & FS_TOOLBAR) && m_fullScreenHideToolBar)
        {
            hideToolBars();
        }
        else
        {
            showToolBars();

            QList<KToolBar*> toolbars = toolBars();
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

bool DXmlGuiWindow::fullScreenIsActive() const
{
    if (d->fullScreenAction)
        return d->fullScreenAction->isChecked();

    kDebug() << "FullScreenAction is not initialized";
    return false;
}

bool DXmlGuiWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (this && (obj == this))
    {
        if (ev && (ev->type() == QEvent::HoverMove))
        {
            // We will handle a stand alone FullScreen button action on top/right corner of screen
            // only if managed window tool bar is hidden, and if we switched already in Full Screen mode.

            if ((d->options & FS_TOOLBAR) && m_fullScreenHideToolBar && fullScreenIsActive())
            {
                QHoverEvent* const mev = dynamic_cast<QHoverEvent*>(ev);

                if (mev)
                {
                    QPoint pos(mev->pos());
                    QRect  desktopRect = KGlobalSettings::desktopGeometry(this);

                    QRect sizeRect(QPoint(0, 0), d->fullScreenBtn->size());
                    QRect topLeft, topRight;
                    QRect topRightLarger;

                    desktopRect       = QRect(desktopRect.y(), desktopRect.y(), desktopRect.width(), desktopRect.height());
                    topLeft           = sizeRect;
                    topRight          = sizeRect;

                    topLeft.moveTo(desktopRect.x(), desktopRect.y());
                    topRight.moveTo(desktopRect.x() + desktopRect.width() - sizeRect.width() - 1, topLeft.y());

                    topRightLarger    = topRight.adjusted(-25, 0, 0, 10);

                    if (topRightLarger.contains(pos))
                    {
                        d->fullScreenBtn->move(topRight.topLeft());
                        d->fullScreenBtn->show();
                    }
                    else
                    {
                        d->fullScreenBtn->hide();
                    }

                    return false;
                }
            }
        }
    }

    // pass the event on to the parent class
    return QObject::eventFilter(obj, ev);
}

void DXmlGuiWindow::hideToolBars()
{
    QList<KToolBar*> toolbars = toolBars();

    foreach(KToolBar* const toolbar, toolbars)
    {
        toolbar->hide();
    }
}

void DXmlGuiWindow::showToolBars()
{
    QList<KToolBar*> toolbars = toolBars();

    foreach(KToolBar* const toolbar, toolbars)
    {
        toolbar->show();
    }
}

void DXmlGuiWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        if (fullScreenIsActive())
        {
            d->fullScreenAction->activate(QAction::Trigger);
        }
    }
}

void DXmlGuiWindow::showSideBar(bool visible)
{
    Q_UNUSED(visible);
}

} // namespace Digikam
