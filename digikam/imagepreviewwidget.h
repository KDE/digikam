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

#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H

// Qt includes.

#include <qstring.h>
#include <qimage.h>

// Local includes

#include "digikam_export.h"
#include "canvas.h"
#include "loadingdescription.h"

class QPixmap;

namespace Digikam
{

class ImagePreviewWidgetPriv;

class DIGIKAM_EXPORT ImagePreviewWidget : public QScrollView
{
Q_OBJECT

public:

    ImagePreviewWidget(QWidget *parent=0);
    ~ImagePreviewWidget();

    void setImage(const QImage& image);
    void setZoomFactor(double z);
    bool fitToWindow();
    void toggleFitToWindow();
    bool maxZoom();
    bool minZoom();

signals:

    void signalRightButtonClicked();
    void signalLeftButtonClicked();    
    void signalShowNextImage();
    void signalShowPrevImage();

public slots:

    void slotIncreaseZoom();
    void slotDecreaseZoom();

protected:
    
    void resizeEvent(QResizeEvent* e);
    void viewportPaintEvent(QPaintEvent *e);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsWheelEvent(QWheelEvent *e);
   
private slots:

    void slotThemeChanged();

private:

    //void updatePixmap(void);

    void   resetImage();
    double calcAutoZoomFactor();
    void   updateAutoZoom();
    void   updateContentsSize();

private:

    ImagePreviewWidgetPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEPREVIEWWIDGET_H */
