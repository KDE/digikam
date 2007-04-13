/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2006-06-13
 * Description : a widget to display an image preview
 *
 * Copyright 2006-2007 Gilles Caulier
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

// Qt includes.

#include <qscrollview.h>

// Local includes

#include "digikam_export.h"

class QPixmap;
class QColor;

namespace Digikam
{

class PreviewWidgetPriv;

class DIGIKAM_EXPORT PreviewWidget : public QScrollView
{
Q_OBJECT

public:

    PreviewWidget(QWidget *parent=0);
    ~PreviewWidget();

    void setZoomFactor(double z);
    void setBackgroundColor(const QColor& color);
    void fitToWindow();
    bool isFitToWindow();
    void toggleFitToWindow();

    int  zoomWidth();
    int  zoomHeight();

    bool maxZoom();
    bool minZoom();

    double zoomFactor();
    double zoomMax();
    double zoomMin();
    void   setZoomMax(double z);
    void   setZoomMin(double z);

    void reset();

signals:

    void signalRightButtonClicked();
    void signalLeftButtonClicked();    
    void signalShowNextImage();
    void signalShowPrevImage();
    void signalZoomFactorChanged(double);

public slots:

    void slotIncreaseZoom();
    void slotDecreaseZoom();

protected:
    
    void   resizeEvent(QResizeEvent *);
    void   viewportPaintEvent(QPaintEvent *);
    void   contentsMousePressEvent(QMouseEvent *);
    void   contentsMouseMoveEvent(QMouseEvent *);
    void   contentsMouseReleaseEvent(QMouseEvent *);
    void   contentsWheelEvent(QWheelEvent *);
    double calcAutoZoomFactor();
    int    tileSize();
    void   updateAutoZoom();
    void   updateContentsSize();

    virtual int  previewWidth()=0;
    virtual int  previewHeight()=0;
    virtual bool previewIsNull()=0;
    virtual void resetPreview()=0;
    virtual void paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh)=0;
   
private:

    PreviewWidgetPriv* d;
};

}  // NameSpace Digikam

#endif /* PREVIEWWIDGET_H */
