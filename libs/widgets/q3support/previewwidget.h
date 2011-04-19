/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-13
 * Description : a widget to display an image preview
 *
 * Copyright (C) 2006-2011 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

// Qt includes

#include <Qt3Support/Q3ScrollView>
#include <QtGui/QPixmap>
#include <QtGui/QResizeEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT PreviewWidget : public Q3ScrollView
{
    Q_OBJECT

public:

    PreviewWidget(QWidget* parent=0);
    ~PreviewWidget();

    void   setZoomFactor(double z);
    void   setZoomFactor(double z, bool centerView);
    void   setZoomFactorSnapped(double z);
    void   fitToWindow();
    bool   isFitToWindow();
    void   toggleFitToWindow();
    void   toggleFitToWindowOr100();

    int    zoomWidth();
    int    zoomHeight();
    bool   maxZoom();
    bool   minZoom();
    double snapZoom(double zoom);

    double zoomFactor();
    double zoomMax();
    double zoomMin();
    void   setZoomMax(double z);
    void   setZoomMin(double z);

    void   setBackgroundColor(const QColor& color);

Q_SIGNALS:

    void signalRightButtonClicked();
    void signalLeftButtonClicked();
    void signalLeftButtonDoubleClicked();
    void signalActivated();
    void signalShowNextImage();
    void signalShowPrevImage();
    void signalZoomFactorChanged(double);
    void signalContentsMovedEvent(bool);
    void signalContentTakeFocus();
    void signalResized();
    void signalToggleOffFitToWindow();

public Q_SLOTS:

    void slotIncreaseZoom();
    void slotDecreaseZoom();
    void slotReset();

protected:

    bool m_movingInProgress;

protected Q_SLOTS:

    void slotCornerButtonPressed();
    void slotPanIconHiden();
    virtual void slotPanIconSelectionMoved(const QRect&, bool);
    virtual void slotContentTakeFocus();
    virtual void slotContentLeaveFocus();

protected:

    enum AutoZoomMode
    {
        ZoomInOrOut,
        ZoomInOnly
    };

    double calcAutoZoomFactor(AutoZoomMode mode = ZoomInOrOut);
    int    tileSize();
    void   updateAutoZoom(AutoZoomMode mode = ZoomInOrOut);
    void   updateContentsSize();
    void   updateZoomAndSize(bool alwaysFitToWindow);
    void   clearCache();
    QRect  previewRect();
    void   drawText(QPainter* p, const QPoint& corner, const QString& text);
    void   startPanning(const QPoint& pos);
    void   continuePanning(const QPoint& pos);
    void   finishPanning();

    virtual void setContentsSize();
    virtual void viewportPaintExtraData() {};
    virtual int  previewWidth()=0;
    virtual int  previewHeight()=0;
    virtual bool previewIsNull()=0;
    virtual void resetPreview()=0;
    virtual QImage previewToQImage() const =0;
    virtual void paintPreview(QPixmap* pix, int sx, int sy, int sw, int sh)=0;
    virtual void zoomFactorChanged(double zoom);

    // Re-implemented from parent class.
    virtual void resizeEvent(QResizeEvent*);
    virtual void viewportPaintEvent(QPaintEvent*);
    virtual void contentsMouseDoubleClickEvent(QMouseEvent*);
    virtual void contentsMousePressEvent(QMouseEvent*);
    virtual void contentsMouseMoveEvent(QMouseEvent*);
    virtual void contentsMouseReleaseEvent(QMouseEvent*);
    virtual void contentsWheelEvent(QWheelEvent*);
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);

private:

    class PreviewWidgetPriv;
    PreviewWidgetPriv* const d;
};

}  // namespace Digikam

#endif /* PREVIEWWIDGET_H */
