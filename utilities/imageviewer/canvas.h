//////////////////////////////////////////////////////////////////////////////
//
//    CANVAS.H
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef CANVAS_H
#define CANVAS_H

// Qt lib includes.

#include <qscrollview.h>

class QString;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;
class CanvasPrivate;
class ImlibInterface;

class Canvas : public QScrollView 
{
    Q_OBJECT
    
public:

    Canvas(QWidget *parent=0);
    ~Canvas();

    void load(const QString& filename);
    void preload(const QString& filename);
    int  save(const QString& filename);
    int  saveAs(const QString& filename);
    bool autoZoomOn();
       
    void ajustInit(void);
    void ajustRejected(void);
    void ajustAccepted(void);
    
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
    
    CanvasPrivate  *d;
    ImlibInterface *iface;
    int             counter;

public slots:

    void slotIncreaseZoom();
    void slotDecreaseZoom();
    void slotSetZoom1();
    void slotSetAutoZoom(bool val);
    void slotToggleAutoZoom();

    // Image modifiers
    
    void slotRotate90();
    void slotRotate180();
    void slotRotate270();
    
    void slotFlipHorizontal();
    void slotFlipVertical();

    void slotCrop();
    
    void slotGammaChanged(int val);
    void slotBrightnessChanged(int val);
    void slotContrastChanged(int val);
    void slotPreviewEnabled(bool on);
    
    void slotRestore();

signals:

    void signalZoomChanged(double zoom);
    void signalMaxZoom();
    void signalMinZoom();
    void signalChanged(bool);
    void signalRotatedOrFlipped();
    void signalCropSelected(bool);
    void signalRightButtonClicked();
    void signalShowNextImage();
    void signalShowPrevImage();
};
    
#endif // CANVAS_H 
