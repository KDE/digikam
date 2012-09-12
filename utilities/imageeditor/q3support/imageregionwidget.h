/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-17
 * Description : a widget to draw an image clip region.
 *
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEREGIONWIDGET_H
#define IMAGEREGIONWIDGET_H

// Qt includes

#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QEvent>
#include <QtGui/QPolygon>
#include <QtGui/QPixmap>
#include <QtGui/QResizeEvent>
#include <QtGui/QWheelEvent>

// Local includes

#include "dimg.h"
#include "previewwidget.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageRegionWidget : public PreviewWidget
{
    Q_OBJECT

public:

    ImageRegionWidget(QWidget* const parent = 0);
    ~ImageRegionWidget();

    /** To get target image region area to render.
     */
    QRect  getOriginalImageRegionToRender() const;

    /** To get target image region image to use for render operations
        If the bool parameter is true a downscaled version of the image
        region at screen resolution will be sent.
        Should be use to increase preview speed for the effects whose
        behaviour is a function of each pixel.
     */
    DImg   getOriginalRegionImage(bool useDownscaledImage = false) const;

    void   setPreviewImage(const DImg& img);

    void   setCapturePointMode(bool b);
    bool   capturePointMode() const;

    void   setHighLightPoints(const QPolygon& pointsList);
    void   setCenterImageRegionPosition();

    void   ICCSettingsChanged();
    void   exposureSettingsChanged();

Q_SIGNALS:

    void signalOriginalClipFocusChanged();
    void signalCapturedPointFromOriginal(const Digikam::DColor&, const QPoint&);

public Q_SLOTS:

    void slotPreviewModeChanged(int mode);
    void slotOriginalImageRegionChanged(bool targetDone);

private Q_SLOTS:

    void slotZoomFactorChanged();
    void slotPanIconSelectionMoved(const QRect& rect, bool targetDone);
    void slotContentTakeFocus();
    void slotContentLeaveFocus();

private:

    void   setContentsPosition(int x, int y, bool targetDone);

    /** To get image region including original or/and target area depending of separate view mode.
        The region is given using not scaled image unit.
     */
    QRect  getOriginalImageRegion()      const;

    QRect  getLocalImageRegionToRender() const;

    void   backupPixmapRegion();
    void   restorePixmapRegion();

    void   enterEvent(QEvent*);
    void   leaveEvent(QEvent*);
    void   contentsMousePressEvent(QMouseEvent*);
    void   contentsMouseReleaseEvent(QMouseEvent*);

    int    previewWidth()  const;
    int    previewHeight() const;
    bool   previewIsNull() const;
    void   resetPreview();
    QImage previewToQImage() const;

    void   viewportPaintExtraData();
    void   emitCapturedPointFromOriginal(const QPoint& pt);

    inline void paintPreview(QPixmap* const pix, int sx, int sy, int sw, int sh);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* IMAGEREGIONWIDGET_H */
