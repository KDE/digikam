/* ============================================================
 * File  : canvas.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-09
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef CANVAS_H
#define CANVAS_H

#include <qscrollview.h>

class QString;
class QPixmap;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;
class QKeyEvent;
class CanvasPrivate;

class Canvas : public QScrollView {

    Q_OBJECT
    
public:

    Canvas(QWidget *parent=0);
    ~Canvas();

    void load(const QString& filename);

    bool maxZoom();
    bool minZoom();
    int  imageWidth();
    int  imageHeight();

    void resizeImage(int w, int h);
    
protected:
    
    void resizeEvent(QResizeEvent* e);
    void viewportPaintEvent(QPaintEvent *e);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsWheelEvent(QWheelEvent *e);
    
private:

    void updateAutoZoom();
    void updateContentsSize();
    void paintViewportRect(const QRect& vr, bool aa);

    CanvasPrivate *d;

public slots:

    void slotIncreaseZoom();
    void slotDecreaseZoom();
    void slotSetAutoZoom(bool val);
    void slotToggleAutoZoom();

    // image modifiers
    void slotRotate90();
    void slotRotate180();
    void slotRotate270();

    void slotFlipHoriz();
    void slotFlipVert();
    
    void slotCrop();
    
    void slotRestore();

private slots:

    void slotSelected();
    void slotPaintSmooth();
    void slotRequestUpdate();
    
signals:

    void signalZoomChanged(float zoom);
    void signalMaxZoom();
    void signalMinZoom();
    void signalChanged(bool);
    void signalSelected(bool);
    void signalRightButtonClicked();
    void signalShowNextImage();
    void signalShowPrevImage();
};
    
#endif /* CANVAS_H */

