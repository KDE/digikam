/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-15-08
 * Description : A floatable/dockable widget for thumbnail bars (ThumbBarView
 *               and its descendants), providing i drag handle similar to the
 *               one on toolbars and a standard Action to show/hide the
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

#ifndef THUMB_BAR_DOCK_H
#define THUMB_BAR_DOCK_H

// Qt includes

#include <QDockWidget>
#include <QPainter>
#include <QString>
#include <QStyle>
#include <QStyleOptionToolBar>
#include <QMainWindow>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/** An alternative handle for QDockWidget's that looks like a toolbar handle.
 */
class DragHandle : public QWidget
{
    Q_OBJECT

public:

    explicit DragHandle(QDockWidget* const);
    ~DragHandle();

    QSize sizeHint()        const;
    QSize minimumSizeHint() const;

protected:

    void paintEvent(QPaintEvent*);

private Q_SLOTS:

    void dockLocationChanged(Qt::DockWidgetArea);

private:

    class Private;
    Private* const d;
};

/** A dock widget specifically designed for thumbnail bars (class ThumbNailView
 *  or one of its descendants). It provides the same look as a toolbar.
 */
class DIGIKAM_EXPORT ThumbBarDock : public QDockWidget
{
    Q_OBJECT

public:

    enum Visibility
    {
        WAS_HIDDEN,
        WAS_SHOWN,
        SHOULD_BE_HIDDEN,
        SHOULD_BE_SHOWN
    };

public:

    explicit ThumbBarDock(QWidget* const parent = 0, Qt::WindowFlags flags = 0);
    ~ThumbBarDock();

    /** Measure the orientation and size of the widget and adjust the containing
     *  thumbnail bar accordingly. Normally not needed, but useful when the
     *  dock widget has changed location and/or size and the appropriate signals
     *  aren't emitted.
     */
    void reInitialize();

    /** Return an Action to show and hide the thumbnail bar.
     */
    QAction* getToggleAction(QObject* const parent, const QString& caption = i18n("Show Thumbbar")) const;

    /** The normal show() and hide() functions don't apply that well, because
     *  there are two orthogonal reasons to hide the thumbbar: the user doesn't
     *  want it, and the window with the thumbbar isn't shown.
     *  The restoreVisibility() function will set the visibility status to what
     *  it should be according to the user setting. The setShouldBeVisible()
     *  function can change this setting. showThumbBar() can be used to hide and
     *  show the thumbbar according to the user preference. shouldBeVisible()
     *  tells whether the thumbbar should be shown according to the user.
     */
    bool shouldBeVisible() const;
    void setShouldBeVisible(bool);
    void restoreVisibility();

    static QPixmap generateFuzzyRect(const QSize& size, const QColor& color, int radius, const QColor& fillColor = Qt::transparent);
    static QPixmap generateFuzzyRectForGroup(const QSize& size, const QColor& color, int radius);

public Q_SLOTS:

    void showThumbBar(bool);

private:

    Visibility m_visible;
};

} // namespace Digikam

#endif // THUMB_BAR_DOCK_H
