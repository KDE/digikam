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

#include "dxmlguiwindow.moc"

// Qt includes

#include <QString>
#include <QList>
#include <QMap>
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

namespace Digikam
{

class DXmlGuiWindow::Private
{
public:

    Private()
    {
        fsOptions              = FS_NONE;
        fullScreenAction       = 0;
        fullScreenBtn          = 0;
        dirtyMainToolBar       = false;
        fullScreenHideToolBars = false;
        fullScreenHideThumbBar = true;
        fullScreenHideSideBars = false;
        thumbbarVisibility     = true;
        menubarVisibility      = true;
        statusbarVisibility    = true;
    }

public:

    /** Settings taken from managed window configuration to handle toolbar visibility  in full-screen mode
     */
    bool                     fullScreenHideToolBars;

    /** Settings taken from managed window configuration to handle thumbbar visibility in full-screen mode
     */
    bool                     fullScreenHideThumbBar;

    /** Settings taken from managed window configuration to handle toolbar visibility  in full-screen mode
     */
    bool                     fullScreenHideSideBars;

    /** Full-Screen options. See FullScreenOptions enum and setFullScreenOptions() for details.
     */
    int                      fsOptions;

    /** Action plug in managed window to switch fullscreen state */
    KToggleFullScreenAction* fullScreenAction;

    /** Show only if toolbar is hidden */
    QToolButton*             fullScreenBtn;

    /** Used by switchWindowToFullScreen() to manage state of full-screen button on managed window
     */
    bool                     dirtyMainToolBar;

    /** Store previous visibility of toolbars before ful-screen mode.
     */
    QMap<KToolBar*, bool>    toolbarsVisibility;

    /** Store previous visibility of thumbbar before ful-screen mode.
     */
    bool                     thumbbarVisibility;

    /** Store previous visibility of menubar before ful-screen mode.
     */
    bool                     menubarVisibility;

    /** Store previous visibility of statusbar before ful-screen mode.
     */
    bool                     statusbarVisibility;
};

// --------------------------------------------------------------------------------------------------------

DXmlGuiWindow::DXmlGuiWindow(QWidget* const parent, Qt::WindowFlags f)
    : KXmlGuiWindow(parent, f), d(new Private)
{
    installEventFilter(this);
}

DXmlGuiWindow::~DXmlGuiWindow()
{
    delete d;
}

void DXmlGuiWindow::setFullScreenOptions(int options)
{
    d->fsOptions = options;
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
    if (d->fsOptions & FS_TOOLBARS)
        d->fullScreenHideToolBars  = group.readEntry(s_configFullScreenHideToolBarsEntry,  false);

    if (d->fsOptions & FS_THUMBBAR)
        d->fullScreenHideThumbBar = group.readEntry(s_configFullScreenHideThumbBarEntry, true);

    if (d->fsOptions & FS_SIDEBARS)
        d->fullScreenHideSideBars  = group.readEntry(s_configFullScreenHideSideBarsEntry,  false);
}

void DXmlGuiWindow::slotToggleFullScreen(bool set)
{
    KToggleFullScreenAction::setFullScreen(this, set);

    showCustomizedView(!set);

    if (!set)
    {
        kDebug() << "TURN OFF fullscreen";

        // restore menubar

        if (d->menubarVisibility)
            menuBar()->show();

        // restore statusbar

        if (d->statusbarVisibility)
            statusBar()->show();

        // restore sidebars

        showSideBars(true);

        // restore thummbbar

        if ((d->fsOptions & FS_THUMBBAR) && d->fullScreenHideThumbBar)
            showThumbBar(d->thumbbarVisibility);

        // restore toolbars and manage full-screen button

        showToolBars(true);
        d->fullScreenBtn->hide();

        if (d->dirtyMainToolBar)
        {
            KToolBar* const mainbar = mainToolBar();

            if (mainbar)
            {
                mainbar->removeAction(d->fullScreenAction);
            }
        }
    }
    else
    {
        kDebug() << "TURN ON fullscreen";

        // hide menubar

        d->menubarVisibility = menuBar()->isVisible();
        menuBar()->hide();

        // hide statusbar

        d->statusbarVisibility = statusBar()->isVisible();
        statusBar()->hide();

        // hide sidebars

        if ((d->fsOptions & FS_SIDEBARS) && d->fullScreenHideSideBars)
            showSideBars(false);

        // hide thummbbar

        d->thumbbarVisibility = thumbbarVisibility();

        if ((d->fsOptions & FS_THUMBBAR) && d->fullScreenHideThumbBar)
            showThumbBar(false);

        // hide toolbars and manage full-screen button

        if ((d->fsOptions & FS_TOOLBARS) && d->fullScreenHideToolBars)
        {
            showToolBars(false);
        }
        else
        {
            showToolBars(true);

            // add fullscreen action if necessary in toolbar

            KToolBar* const mainbar = mainToolBar();

            if (mainbar && !mainbar->actions().contains(d->fullScreenAction))
            {
                if (mainbar->actions().isEmpty())
                {
                    mainbar->addAction(d->fullScreenAction);
                }
                else
                {
                    mainbar->insertAction(mainbar->actions().first(), d->fullScreenAction);
                }

                d->dirtyMainToolBar = true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->dirtyMainToolBar = false;
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
        if (ev && (ev->type() == QEvent::HoverMove) && fullScreenIsActive())
        {
            // We will handle a stand alone FullScreen button action on top/right corner of screen
            // only if managed window tool bar is hidden, and if we switched already in Full Screen mode.

            KToolBar* const mainbar = mainToolBar();

            if (mainbar)
            {
                if (((d->fsOptions & FS_TOOLBARS) && d->fullScreenHideToolBars) || !mainbar->isVisible())
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
    }

    // pass the event on to the parent class
    return QObject::eventFilter(obj, ev);
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

KToolBar* DXmlGuiWindow::mainToolBar() const
{
    QList<KToolBar*> toolbars = toolBars();
    KToolBar* mainToolbar     = 0;

    foreach(KToolBar* const toolbar, toolbars)
    {
        if (toolbar && (toolbar->objectName() == "mainToolBar"))
        {
            mainToolbar = toolbar;
            break;
        }
    }

    return mainToolbar;
}

void DXmlGuiWindow::showToolBars(bool visible)
{
    // We will hide toolbars: store previous state for future restoring.
    if (!visible)
    {
        d->toolbarsVisibility.clear();

        foreach(KToolBar* const toolbar, toolBars())
        {
            if (toolbar)
            {
                bool visibility = toolbar->isVisible();
                d->toolbarsVisibility.insert(toolbar, visibility);
            }
        }
    }

    // Switch toolbars visibility

    for (QMap<KToolBar*, bool>::const_iterator it = d->toolbarsVisibility.constBegin(); it != d->toolbarsVisibility.constEnd(); ++it)
    {
        KToolBar* const toolbar = it.key();
        bool visibility         = it.value();

        if (toolbar)
        {
            if (visible && visibility)
                toolbar->show();
            else
                toolbar->hide();
        }
    }

    // We will show toolbars: restore previous state.
    if (visible)
    {
        for (QMap<KToolBar*, bool>::const_iterator it = d->toolbarsVisibility.constBegin(); it != d->toolbarsVisibility.constEnd(); ++it)
        {
            KToolBar* const toolbar = it.key();
            bool visibility         = it.value();

            if (toolbar)
            {
                visibility ? toolbar->show() : toolbar->hide();
            }
        }
    }
}

void DXmlGuiWindow::showSideBars(bool visible)
{
    Q_UNUSED(visible);
}

void DXmlGuiWindow::showThumbBar(bool visible)
{
    Q_UNUSED(visible);
}

void DXmlGuiWindow::showCustomizedView(bool visible)
{
    Q_UNUSED(visible);
}

bool DXmlGuiWindow::thumbbarVisibility() const
{
    return true;
}

} // namespace Digikam
