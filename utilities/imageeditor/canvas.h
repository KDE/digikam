/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-01-09
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju, Gilles Caulier
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

#ifndef CANVAS_H
#define CANVAS_H

// Qt includes.

#include <qscrollview.h>

class QString;
class QPixmap;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;
class QKeyEvent;
class QColor;

class CanvasPrivate;

class Canvas : public QScrollView {

    Q_OBJECT
    
public:

    Canvas(QWidget *parent=0);
    ~Canvas();

    void load(const QString& filename);
    void preload(const QString& filename);
    int  save(const QString& filename, int JPEGcompression, 
              int PNGcompression, bool TIFFcompression);
    int  saveAs(const QString& filename, int JPEGcompression, 
                int PNGcompression, bool TIFFcompression, 
                const QString& mimeType);
    int  saveAsTmpFile(const QString& filename, int JPEGcompression, 
                       int PNGcompression, bool TIFFcompression, 
                       const QString& mimeType=0);
        
    bool maxZoom();
    bool minZoom();
    bool exifRotated();
    int  imageWidth();
    int  imageHeight();

    void resizeImage(int w, int h);
    void rotateImage(double angle);

    void setBackgroundColor(const QColor& color);
    void setExifOrient(bool exifOrient);
    
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
    void drawRubber();
    void paintViewport(const QRect& er, bool antialias);

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

