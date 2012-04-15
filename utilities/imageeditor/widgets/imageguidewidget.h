/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-20
 * Description : a widget to display an image with guides
 *
 * Copyright (C) 2004-2011 Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtCore/QPoint>
#include <QtCore/QEvent>
#include <QtCore/QTimerEvent>

// Local includes

#include "dcolor.h"
#include "digikam_export.h"

namespace Digikam
{

class DColor;
class ImageIface;

class DIGIKAM_EXPORT ImageGuideWidget : public QWidget
{
    Q_OBJECT

public:

    enum GuideToolMode
    {
        HVGuideMode = 0,
        PickColorMode
    };

    enum ColorPointSrc
    {
        OriginalImage = 0,
        PreviewImage,
        TargetPreviewImage
    };

public:

    explicit ImageGuideWidget(QWidget* parent = 0,
                              bool spotVisible = true, int guideMode = PickColorMode,
                              const QColor& guideColor = Qt::red, int guideSize = 1,
                              bool blink = false, bool useImageSelection = false);
    ~ImageGuideWidget();

    ImageIface* imageIface() const;

    QPoint getSpotPosition() const;
    DColor getSpotColor(int getColorFrom) const;
    void   setSpotVisible(bool spotVisible, bool blink = false);
    int    previewMode() const;
    void   resetSpotPosition();
    void   updatePreview();
    void   setPoints(const QPolygon& p, bool drawLine = false);
    void   resetPoints();

    void   setPaintColor(const QColor& color);
    void   setEraseMode(bool erase);
    void   setMaskEnabled(bool enabled);
    void   setMaskPenSize(int size);
    void   setMaskCursor();

    QImage getMask() const;

    void   setBackgroundColor(const QColor&);
    void   ICCSettingsChanged();
    void   exposureSettingsChanged();

public Q_SLOTS:

    void slotChangeGuideColor(const QColor& color);
    void slotChangeGuideSize(int size);
    void slotPreviewModeChanged(int mode);

Q_SIGNALS:

    void spotPositionChangedFromOriginal(const Digikam::DColor& color, const QPoint& position);
    void spotPositionChangedFromTarget(const Digikam::DColor& color, const QPoint& position);
    void signalResized();

private:

    void   paintEvent(QPaintEvent*);
    void   resizeEvent(QResizeEvent*);
    void   timerEvent(QTimerEvent*);
    void   mousePressEvent(QMouseEvent*);
    void   mouseReleaseEvent(QMouseEvent*);
    void   mouseMoveEvent(QMouseEvent*);
    void   enterEvent(QEvent*);
    void   leaveEvent(QEvent*);
    void   updatePixmap();
    void   drawLineTo(const QPoint& endPoint);
    void   drawLineTo(int width, bool erase, const QColor& color, const QPoint& start, const QPoint& end);
    QPoint translatePointPosition(QPoint& point) const;
    void   drawText(QPainter* p, const QPoint& corner, const QString& text);
    void   updateMaskCursor();

private:

    class ImageGuideWidgetPriv;
    ImageGuideWidgetPriv* const d;
};

}  // namespace Digikam

#endif /* IMAGEGUIDEWIDGET_H */
