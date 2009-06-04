/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-20
 * Description : a widget to display an image with guides
 *
 * Copyright (C) 2004-2009 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEGUIDEWIDGET_H
#define IMAGEGUIDEWIDGET_H

// Qt includes

#include <QtGui/QWidget>
#include <QtCore/QPoint>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtGui/QResizeEvent>
#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtCore/QTimerEvent>
#include <QtGui/QPaintEvent>

// Local includes

#include "dcolor.h"
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class DColor;
class ImageIface;
class ImageGuideWidgetPriv;

class DIGIKAM_EXPORT ImageGuideWidget : public QWidget
{
Q_OBJECT

public:

    enum GuideToolMode
    {
        HVGuideMode=0,
        PickColorMode
    };

    enum RenderingPreviewMode
    {
        PreviewOriginalImage=0,     // Original image only.
        PreviewBothImagesHorz,      // Horizontal with original and target duplicated.
        PreviewBothImagesVert,      // Vertical with original and target duplicated.
        PreviewBothImagesHorzCont,  // Horizontal with original and target in contiguous.
        PreviewBothImagesVertCont,  // Vertical with original and target in contiguous.
        PreviewTargetImage,         // Target image only.
        PreviewToggleOnMouseOver,   // Original image if mouse is over image area, else target image.
        NoPreviewMode               // Target image only without information displayed.
    };

    enum ColorPointSrc
    {
        OriginalImage=0,
        PreviewImage,
        TargetPreviewImage
    };

public:

    ImageGuideWidget(int w, int h, QWidget *parent=0,
                     bool spotVisible=true, int guideMode=HVGuideMode,
                     const QColor& guideColor=Qt::red, int guideSize=1,
                     bool blink=false, bool useImageSelection=false);
    ~ImageGuideWidget();

    ImageIface* imageIface();

    QPoint getSpotPosition();
    DColor getSpotColor(int getColorFrom);
    void   setSpotVisible(bool spotVisible, bool blink=false);
    int    getRenderingPreviewMode();
    void   setRenderingPreviewMode(int mode);
    void   resetSpotPosition();
    void   updatePreview();
    void   setPoints(const QPolygon& p, bool drawLine=false);
    void   resetPoints();

    void   setPaintColor(const QColor& color);
    void   setEraseMode(bool erase);
    void   setMaskEnabled(bool enabled);
    void   setMaskPenSize(int size);

    QImage getMask() const;

public Q_SLOTS:

    void slotChangeGuideColor(const QColor& color);
    void slotChangeGuideSize(int size);
    void slotChangeRenderingPreviewMode(int mode);
    void slotToggleUnderExposure(bool);
    void slotToggleOverExposure(bool);

Q_SIGNALS:

    void spotPositionChangedFromOriginal(const Digikam::DColor& color, const QPoint& position);
    void spotPositionChangedFromTarget(const Digikam::DColor& color, const QPoint& position);
    void signalResized();

protected:

    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void timerEvent(QTimerEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);

private:

    void   updatePixmap();
    void   drawLineTo(const QPoint& endPoint);
    void   drawLineTo(int width, bool erase, const QColor& color, const QPoint& start, const QPoint& end);
    QPoint translatePointPosition(QPoint& point);

private:

    ImageGuideWidgetPriv* const d;
};

}  // namespace Digikam

#endif /* IMAGEGUIDEWIDGET_H */
