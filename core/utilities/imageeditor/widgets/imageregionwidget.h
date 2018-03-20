/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-15
 * Description : a widget to draw an image clip region.
 *
 * Copyright (C) 2013-2014 by Yiou Wang <geow812 at gmail dot com>
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QPoint>
#include <QRect>
#include <QEvent>
#include <QPolygon>
#include <QPixmap>
#include <QResizeEvent>
#include <QWheelEvent>

// Local includes

#include "dimg.h"
#include "graphicsdimgview.h"
#include "imagezoomsettings.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageRegionWidget : public GraphicsDImgView
{
    Q_OBJECT

public:

    explicit ImageRegionWidget(QWidget* const parent = 0);
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

    DImg   getOriginalImage() const;

    void   setPreviewImage(const DImg& img);

    void   setCapturePointMode(bool b);
    bool   capturePointMode() const;

    void   setHighLightPoints(const QPolygon& pointsList);

    void   ICCSettingsChanged();
    void   exposureSettingsChanged();

Q_SIGNALS:

    void signalOriginalClipFocusChanged();
    void signalCapturedPointFromOriginal(const Digikam::DColor&, const QPoint&);

public Q_SLOTS:

    void slotPreviewModeChanged(int mode);
    void slotOriginalImageRegionChanged(bool targetDone=true);

protected:

    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

private:

    void emitCapturedPointFromOriginal(const QPointF&);

private Q_SLOTS:

    void slotOriginalImageRegionChangedDelayed();

private:

    class Private;
    Private* const d_ptr;
};

}  // namespace Digikam

#endif /* IMAGEREGIONWIDGET_H */
