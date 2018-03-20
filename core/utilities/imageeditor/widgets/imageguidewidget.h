/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-20
 * Description : a widget to display an image with guides
 *
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

#ifndef IMAGEGUIDEWIDGET_H
#define IMAGEGUIDEWIDGET_H

// Qt includes

#include <QWidget>
#include <QColor>
#include <QPixmap>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QEvent>
#include <QTimerEvent>

// Local includes

#include "imageiface.h"
#include "dcolor.h"
#include "digikam_export.h"

namespace Digikam
{

class DColor;

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

    explicit ImageGuideWidget(QWidget* const parent = 0,
                              bool spotVisible = true,
                              int guideMode = PickColorMode,
                              const QColor& guideColor = Qt::red,
                              int guideSize = 1,
                              bool blink = false,
                              ImageIface::PreviewType type= ImageIface::FullImage);
    ~ImageGuideWidget();

    ImageIface* imageIface()                   const;
    QImage      getMask()                      const;
    QPoint      getSpotPosition()              const;
    DColor      getSpotColor(int getColorFrom) const;
    int         previewMode()                  const;

    void   setSpotVisible(bool spotVisible, bool blink = false);
    void   resetSpotPosition();
    void   updatePreview();
    void   setPoints(const QPolygon& p, bool drawLine = false);
    void   resetPoints();

    void   setPaintColor(const QColor& color);
    void   setEraseMode(bool erase);
    void   setMaskEnabled(bool enabled);
    void   setMaskPenSize(int size);
    void   setMaskCursor();

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

protected:

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
    QPoint translatePointPosition(const QPoint& point) const;
    void   drawText(QPainter* const p, const QPoint& corner, const QString& text);
    void   updateMaskCursor();
    void   setSpotPosition(QPoint& point);
    void   updateSpotPosition(int x, int y);
    QPoint translateImagePosition(const QPoint& point, bool src) const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // IMAGEGUIDEWIDGET_H
