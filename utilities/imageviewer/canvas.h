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
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;
class CanvasPrivate;
class ImlibInterface;

class Canvas : public QScrollView {

    Q_OBJECT
    
public:

    Canvas(QWidget *parent=0);
    ~Canvas();

    void load(const QString& filename);
    void preload(const QString& filename);
    int  save(const QString& filename);
    void getBCGSettings(int& gamma, int& brightness,
                        int& contrast);
    bool autoZoomOn();

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

    bool maxZoom();
    bool minZoom();
    void setZoom(double zoom);

    void drawRubber(QPainter *p);
    
    CanvasPrivate *d;
    ImlibInterface *iface;
    int counter;

public slots:

    void slotIncreaseZoom();
    void slotDecreaseZoom();
    void slotSetZoom1();
    void slotSetAutoZoom(bool val);
    void slotToggleAutoZoom();

    // image modifiers
    void slotRotate90();
    void slotRotate180();
    void slotRotate270();

    void slotCrop();
    
    void slotGammaPlus();
    void slotGammaMinus();
    void slotBrightnessPlus();
    void slotBrightnessMinus();
    void slotContrastPlus();
    void slotContrastMinus();
    
    void slotRestore();

signals:

    void signalZoomChanged(double zoom);
    void signalMaxZoom();
    void signalMinZoom();
    void signalChanged(bool);
    void signalCropSelected(bool);
    void signalRightButtonClicked();
    void signalShowNextImage();
    void signalShowPrevImage();
    
};
    
#endif /* CANVAS_H */
