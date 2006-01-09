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
#include <qrect.h>

class QString;
class QStringList;
class QPixmap;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;
class QKeyEvent;
class QColor;

namespace Digikam
{

class CanvasPrivate;
class ICCSettingsContainer;
class IOFileSettingsContainer;

class Canvas : public QScrollView
{
    Q_OBJECT
    
public:

    Canvas(QWidget *parent=0);
    ~Canvas();

    bool load(const QString& filename, ICCSettingsContainer *ICCSettings,
              IOFileSettingsContainer *IOFileSettings, QWidget *parent);
    void preload(const QString& filename);
    int  save(const QString& filename, IOFileSettingsContainer *IOFileSettings);
    int  saveAs(const QString& filename, IOFileSettingsContainer *IOFileSettings,
                const QString& mimeType);
    int  saveAsTmpFile(const QString& filename, IOFileSettingsContainer *IOFileSettings,
                       const QString& mimeType=0);
    void setModified(bool val);
        
    bool  maxZoom();
    bool  minZoom();
    bool  exifRotated();
    int   imageWidth();
    int   imageHeight();
    QRect getSelectedArea();

    void resizeImage(int w, int h);

    void setBackgroundColor(const QColor& color);
    void setExifOrient(bool exifOrient);

    void increaseGamma();
    void decreaseGamma();
    void increaseBrightness();
    void decreaseBrightness();
    void increaseContrast();
    void decreaseContrast();

    void getUndoHistory(QStringList &titles);
    void getRedoHistory(QStringList &titles);

    void setHistogramPosition(const QPoint& pos);
    bool getHistogramPosition(QPoint& pos);

    int setHistogramType(int t);
    
protected:
    
    void resizeEvent(QResizeEvent* e);
    void viewportPaintEvent(QPaintEvent *e);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsWheelEvent(QWheelEvent *e);
    
private:

    void updateAutoZoom();
    bool updateHistogram(bool);
    void updateContentsSize();
    void drawRubber();

    void getHistogramRect(QRect &);
    void paintViewport(const QRect& er, bool antialias);

    void paintHistogram(const QRect& cr);
    void customEvent(QCustomEvent *);
    void createHistogramPixmap();
    void drawHistogramPixmapBusy();
    void drawHistogramPixmap();

    CanvasPrivate *d;

public slots:

    void slotIncreaseZoom();
    void slotDecreaseZoom();
    void slotSetAutoZoom(bool val);
    void slotToggleAutoZoom();

    void slotShowHistogram(bool val);
    void slotToggleShowHistogram();

    // image modifiers
    void slotRotate90();
    void slotRotate180();
    void slotRotate270();

    void slotFlipHoriz();
    void slotFlipVert();
    
    void slotCrop();
    
    void slotRestore();
    void slotUndo(int steps=1);
    void slotRedo(int steps=1);

    void slotCopy();

private slots:

    void slotSelected();
    void slotPaintSmooth();
    void slotModified(bool anyMoreUndo, bool anyMoreRedo);
    void slotContentsMoving(int, int);
    void slotContentsMovingPaintHistogram();
    void slotHistoMovingPaintHistogram();
    
signals:

    void signalZoomChanged(float zoom);
    void signalMaxZoom();
    void signalMinZoom();
    void signalChanged(bool, bool);
    void signalSelected(bool);
    void signalRightButtonClicked();
    void signalShowNextImage();
    void signalShowPrevImage();
};
    
}  // namespace Digikam

#endif /* CANVAS_H */

