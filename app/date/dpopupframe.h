/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : Frame with popup menu behavior.
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 1997      by Tim D. Gilman <tdgilman at best dot org>
 * Copyright (C) 1998-2001 by Mirko Boehm <mirko at kde dot org>
 * Copyright (C) 2007      by John Layt <john at layt dot net>
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

#ifndef DPOPUPFRAME_H
#define DPOPUPFRAME_H

// Qt includes

#include <QFrame>

namespace Digikam
{

class DPopupFrame : public QFrame
{
    Q_OBJECT

public:

    /**
     * The contructor. Creates a dialog without buttons.
     */
    explicit DPopupFrame(QWidget* const parent = 0);

    /**
     * The destructor
     */
    ~DPopupFrame();

    /**
     * Set the main widget. You cannot set the main widget from the constructor,
     * since it must be a child of the frame itselfes.
     * Be careful: the size is set to the main widgets size. It is up to you to
     * set the main widgets correct size before setting it as the main
     * widget.
     */
    void setMainWidget(QWidget* const m);

    /**
     * The resize event. Simply resizes the main widget to the whole
     * widgets client size.
     */
    void resizeEvent(QResizeEvent* e) Q_DECL_OVERRIDE;

    /**
     * Open the popup window at position pos.
     */
    void popup(const QPoint& p);

    /**
     * Execute the popup window.
     */
    int exec(const QPoint& p);

    /**
     * Execute the popup window.
     */
    int exec(int x, int y);

Q_SIGNALS:

    void leaveModality();

protected:

    /**
     * Catch key press events.
     */
    void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;

    /**
     * Catch hide events.
     */
    void hideEvent(QHideEvent* e) Q_DECL_OVERRIDE;

public Q_SLOTS:

    /**
     * Close the popup window. This is called from the main widget, usually.
     * @p r is the result returned from exec().
     */
    void close(int r);

private:

    class Private;
    Private* const d;

    friend class Private;

    Q_DISABLE_COPY(DPopupFrame)
};

}  // namespace Digikam

#endif // DPOPUPFRAME_H
