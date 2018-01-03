/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-22
 * Description : a generic widget to display a panel to choose
 *               a rectangular image area.
 *
 * Copyright (C) 1997      by Tim D. Gilman (tdgilman at best dot org)
 * Copyright (C) 1998-2001 by Mirko Boehm (mirko at kde dot org)
 * Copyright (C) 2007      by John Layt <john at layt dot net>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PANICONWIDGET_H
#define PANICONWIDGET_H

// Qt includes

#include <QWidget>
#include <QRect>
#include <QImage>
#include <QPixmap>
#include <QHideEvent>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QPaintEvent>
#include <QFrame>

// Local includes

#include "digikam_export.h"
#include "dimg.h"

class QToolButton;

namespace Digikam
{

/**
 * Frame with popup menu behavior to host PanIconWidget.
 */
class DIGIKAM_EXPORT PanIconFrame : public QFrame
{
    Q_OBJECT

public:

    PanIconFrame(QWidget* const parent=0);
    ~PanIconFrame();

    /**
     * Set the main widget. You cannot set the main widget from the constructor,
     * since it must be a child of the frame itselfes.
     * Be careful: the size is set to the main widgets size. It is up to you to
     * set the main widgets correct size before setting it as the main
     * widget.
     */
    void setMainWidget(QWidget* const main);

    /**
     * Open the popup window at position pos.
     */
    void popup(const QPoint & pos);

    /**
     * Execute the popup window.
     */
    int exec(const QPoint& pos);

    /**
     * Execute the popup window.
     */
    int exec(int x, int y);

    /**
     * The resize event. Simply resizes the main widget to the whole
     * widgets client size.
     */
    virtual void resizeEvent(QResizeEvent* resize);

Q_SIGNALS:

    void leaveModality();

protected:
    /**
     * Catch key press events.
     */
    virtual void keyPressEvent(QKeyEvent* e);

public Q_SLOTS:

    /**
     * Close the popup window. This is called from the main widget, usually.
     * @p r is the result returned from exec().
     */
    void close(int r);

private:

    class Private;
    friend class Private;
    Private * const d;

    Q_DISABLE_COPY(PanIconFrame)
};

// ---------------------------------------------------------------------------------

class DIGIKAM_EXPORT PanIconWidget : public QWidget
{
    Q_OBJECT

public:

    explicit PanIconWidget(QWidget* const parent=0);
    ~PanIconWidget();

    static QToolButton* button();

    void setImage(int previewWidth, int previewHeight, const QImage& fullOriginalImage);
    void setImage(int previewWidth, int previewHeight, const DImg& fullOriginalImage);
    void setImage(const QImage& scaledPreviewImage, const QSize& fullImageSize);

    void  setRegionSelection(const QRect& regionSelection);
    QRect getRegionSelection() const;
    void  setCenterSelection();

    void  setCursorToLocalRegionSelectionCenter();
    void  setMouseFocus();

Q_SIGNALS:

    /**
     * Emitted when selection have been moved with mouse.
     * 'targetDone' boolean value is used for indicate if the mouse have been released.
     */
    void signalSelectionMoved(const QRect& rect, bool targetDone);

    void signalSelectionTakeFocus();

    void signalHidden();

public Q_SLOTS:

    void slotZoomFactorChanged(double);

protected:

    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

    /**
     * Recalculate the target selection position and emit 'signalSelectionMoved'.
     */
    void regionSelectionMoved(bool targetDone);

protected Q_SLOTS:

    void slotFlickerTimer();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PANICONWIDGET_H
