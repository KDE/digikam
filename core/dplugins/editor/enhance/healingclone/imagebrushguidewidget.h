/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-15
 * Description : a brush for use with tool to replace part of the image using another
 *
 * Copyright (C) 2017      by Shaza Ismail Kaoud <shaza dot ismail dot k at gmail dot com>
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

#ifndef DIGIKAM_IMAGE_BRUSH_GUIDE_WIDGET_H
#define DIGIKAM_IMAGE_BRUSH_GUIDE_WIDGET_H

// Local includes

#include "imageguidewidget.h"
#include "previewtoolbar.h"
#include <QPainter>

using namespace Digikam;

namespace DigikamEditorHealingCloneToolPlugin
{

class ImageBrushGuideWidget : public ImageGuideWidget
{
    Q_OBJECT

public:

    /**
     * Using the parent's constructor
     * Should be changed to get rid of the inheritance
     */
    using ImageGuideWidget::ImageGuideWidget;
    void setDefaults();
    void zoomImage(int zoomPercent);
    void zoomPlus();
    void zoomMinus();
    double getScaleRatio();

    explicit ImageBrushGuideWidget(QWidget* const parent = nullptr,
                              bool spotVisible = true,
                              int guideMode = PickColorMode,
                              const QColor& guideColor = Qt::red,
                              int guideSize = 1,
                              bool blink = false,
                              ImageIface::PreviewType type= ImageIface::FullImage);

public Q_SLOTS:

    /**
     * @brief slotSrcSet toggles the fixing of the brush source center
     */
    void slotSetSourcePoint();

Q_SIGNALS:

    /**
     * @brief signalClone emitted when the src is set and the user initiated a brush click
     * and keeps emitting with motion
     */
    void signalClone(const QPoint& currentSrc, const QPoint& currentDst);
    void signalReclone();
    void signalLasso(const QPoint& dst);
    void signalResetLassoPoint();

protected:

    void mouseReleaseEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent *event) ;
    void keyReleaseEvent(QKeyEvent *event) ;
    void resizeEvent(QResizeEvent *) override;
    void wheelEvent(QWheelEvent *event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void focusInEvent(QFocusEvent * event) override;
    void undoSlotSetSourcePoint();
    void changeCursorShape(QColor color);
    void resetPixels();




private:

    bool   srcSet = true;
    QPoint src;
    QPoint dst;
    QPoint oldPos;
    bool isMPressed= false;
    bool isSPressed = false;
    bool isPPressed = false;
    double default_w;
    double default_h;
    double float_w;
    double float_h;
    bool first_time = true;
    bool amIFocused = false;
};

} // namespace DigikamEditorHealingCloneToolPlugin

#endif // DIGIKAM_IMAGE_BRUSH_GUIDE_WIDGET_H
