/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-15-08
 * Description : A floatable/dockable widget for thumbnail bars (ThumbBarView
 *               and its descendants), providing i drag handle similar to the
 *               one on toolbars and a standard KToggleAction to show/hide the
 *               thumbnail bar. It inherits QDockWidget and can be used in
 *               the dock area's of a QMainWindow.
 *
 * Copyright (C) 2009 by Pieter Edelman <p dot edelman at gmx dot net>
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

#ifndef THUMBBARDOCK_H_
#define THUMBBARDOCK_H_

// Qt includes

#include <QDockWidget>
#include <QPainter>
#include <QString>
#include <QStyle>
#include <QStyleOptionToolBar>
#include <QMainWindow>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <ktoggleaction.h>

// local includes

#include "thumbbar.h"

namespace Digikam
{

/* An alternative handle for QDockWidget's that looks like a toolbar handle. */
class DragHandle : public QWidget
{
    Q_OBJECT

public:

    DragHandle(QDockWidget *);
    ~DragHandle();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:

    void paintEvent(QPaintEvent *);

private Q_SLOTS:

    void dockLocationChanged(Qt::DockWidgetArea);

private:

    QDockWidget *m_parent;
};

/* A dock widget specifically designed for thumbnail bars (class ThumbNailView
 * or one of its descendants). It provides the same look as a toolbar.
 */
class DIGIKAM_EXPORT ThumbBarDock : public QDockWidget
{
    Q_OBJECT

    enum Visibility
    {
        WAS_HIDDEN,
        WAS_SHOWN,
        SHOULD_BE_HIDDEN,
        SHOULD_BE_SHOWN
    };

public:

    ThumbBarDock(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~ThumbBarDock();

    /* Measure the orientation and size of the widget and adjust the containing
     * thumbnail bar accordingly. Normally not needed, but useful when the
     * dock widget has changed location and/or size and the appropriate signals
     * aren't emitted.
     */
    void reInitialize();

    /* Return a KToggleAction to show and hide the thumbnail bar. */
    KToggleAction *getToggleAction(QObject *parent, QString caption = i18n("Show Thumbbar"));

    /* The first two functions can be used to hide a (floating) thumbbar and
     * show it again, if it was visible. They can be thought of as the state
     * respecting counterparts of hide and show.
     * The third and fourth function specify if the thumbbar should be visible
     * when restoreVisibility is called. The first time the window is shown
     * after calling these last two functions, the "should be" state is reset to
     * whatever the current state is.
     * NOTE: The setVisible() (or show() and hide()) functions are still
     * available as low-level functions, but they don't keep track of the state.
     * To do that, the showThumbBar() function is available.
     */
    void makeInvisible();
    void restoreVisibility();
    bool shouldBeVisible();
    void setShouldBeVisible(bool);

public Q_SLOTS:

	void showThumbBar(bool);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

private:

    Visibility m_visible;
};

} // namespace Digikam

#endif /* THUMBBARDOCK_H_ */
