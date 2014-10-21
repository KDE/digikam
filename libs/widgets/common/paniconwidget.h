/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-22
 * Description : a generic widget to display a panel to choose
 *               a rectangular image area.
 *
 * Copyright (C) 2004-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QtGui/QWidget>
#include <QtCore/QRect>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QHideEvent>
#include <QtGui/QMouseEvent>
#include <QtCore/QTimerEvent>
#include <QtGui/QPaintEvent>

// Local includes

#include "digikam_export.h"
#include "dimg.h"

class QToolButton;

namespace Digikam
{

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

#endif /* PANICONWIDGET_H */
