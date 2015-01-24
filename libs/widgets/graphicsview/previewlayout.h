/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Layout for an item on image preview
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef PREVIEWLAYOUT_H
#define PREVIEWLAYOUT_H

// Qt includes

#include <QFlags>
#include <QObject>
#include <QPointF>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class GraphicsDImgItem;
class GraphicsDImgView;

class DIGIKAM_EXPORT SinglePhotoPreviewLayout : public QObject
{
    Q_OBJECT

public:

    enum SetZoomFlag
    {
        JustSetFactor  = 0,
        CenterView     = 1 << 0,
        SnapZoomFactor = 1 << 1
    };
    Q_DECLARE_FLAGS(SetZoomFlags, SetZoomFlag)

public:

    explicit SinglePhotoPreviewLayout(QObject* const parent = 0);
    ~SinglePhotoPreviewLayout();

    /** Set the graphics view, and associated scene, to operate on. */
    void setGraphicsView(GraphicsDImgView* view);

    /** Set the item to layout. For a SinglePhoto layout, typically,
     *  you can add only one item. */
    void addItem(GraphicsDImgItem* item);

    bool   isFitToWindow() const;
    double zoomFactor() const;

    /** The zoom range for incrementing and decrementing. */
    double maxZoomFactor() const;
    double minZoomFactor() const;
    void   setMaxZoomFactor(double z);
    void   setMinZoomFactor(double z);
    bool   atMaxZoom() const;
    bool   atMinZoom() const;

Q_SIGNALS:

    void fitToWindowToggled(bool fitToWindow);
    void zoomFactorChanged(double);

public Q_SLOTS:

    void increaseZoom(const QPoint& viewportAnchor = QPoint());
    void decreaseZoom(const QPoint& viewportAnchor = QPoint());
    void setZoomFactor(double z, const QPoint& viewportAnchor = QPoint(), SetZoomFlags flags = JustSetFactor);
    void setZoomFactor(double z, SetZoomFlags flags);
    void setZoomFactorSnapped(double z);
    void fitToWindow();
    /// Toggle between fitToWindow and previous zoom factor
    void toggleFitToWindow();
    /// Toggle between fitToWindow and zoom factor 1
    void toggleFitToWindowOr100();

    /// Update settings when size of image or view changed
    void updateZoomAndSize();

protected:

    void updateLayout();

private:

    class Private;
    Private* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SinglePhotoPreviewLayout::SetZoomFlags)

} // namespace Digikam

#endif // PREVIEWLAYOUT_H
