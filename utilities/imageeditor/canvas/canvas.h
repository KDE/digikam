/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-09
 * Description : image editor canvas management class
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes.

#include "digikam_export.h"
#include "dimg.h"

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
class DImgInterface;
class ExposureSettingsContainer;
class ICCSettingsContainer;
class IOFileSettingsContainer;

class DIGIKAM_EXPORT Canvas : public QScrollView
{
    Q_OBJECT

public:

    Canvas(QWidget *parent=0);
    ~Canvas();

    void  load(const QString& filename, IOFileSettingsContainer *IOFileSettings);
    void  preload(const QString& filename);

    void  saveAs(const QString& filename, IOFileSettingsContainer *IOFileSettings,
                 bool setExifOrientationTag, const QString& mimeType=QString());
    void  resetImage();
    void  switchToLastSaved(const QString& newFilename);
    void  abortSaving();
    void  setModified();
    void  readMetadataFromFile(const QString &file);
    void  clearUndoHistory();
    void  setUndoHistoryOrigin();
    void  updateUndoState();
    DImg  currentImage();
    QString currentImageFileFormat();
    QString currentImageFilePath();

    DImgInterface *interface() const;
    void makeDefaultEditingCanvas();

    double snapZoom(double z);
    void   setZoomFactorSnapped(double zoom);

    double zoomFactor();
    void  setZoomFactor(double z);
    bool  fitToWindow();
    bool  maxZoom();
    bool  minZoom();
    bool  exifRotated();
    int   imageWidth();
    int   imageHeight();
    QRect getSelectedArea();

    // If current image file format is only available in read only,
    // typicially all RAW image file formats.
    bool  isReadOnly();

    void  resizeImage(int w, int h);

    void  setBackgroundColor(const QColor& color);
    void  setICCSettings(ICCSettingsContainer *cmSettings);
    void  setExposureSettings(ExposureSettingsContainer *expoSettings);

    void  setExifOrient(bool exifOrient);

    void  increaseGamma();
    void  decreaseGamma();
    void  increaseBrightness();
    void  decreaseBrightness();
    void  increaseContrast();
    void  decreaseContrast();

    void  getUndoHistory(QStringList &titles);
    void  getRedoHistory(QStringList &titles);

    void  toggleFitToWindow();
    void  fitToSelect();

signals:

    void signalZoomChanged(double zoom);
    void signalMaxZoom();
    void signalMinZoom();
    void signalChanged();
    void signalUndoStateChanged(bool, bool, bool);
    void signalSelected(bool);
    void signalRightButtonClicked();
    void signalShowNextImage();
    void signalShowPrevImage();
    void signalPrepareToLoad();
    void signalLoadingStarted(const QString &filename);
    void signalLoadingFinished(const QString &filename, bool success);
    void signalLoadingProgress(const QString& filePath, float progress);
    void signalSavingStarted(const QString &filename);
    void signalSavingFinished(const QString &filename, bool success);
    void signalSavingProgress(const QString& filePath, float progress);
    void signalSelectionChanged(const QRect&);
    void signalToggleOffFitToWindow();

public slots:

    void slotIncreaseZoom();
    void slotDecreaseZoom();

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

    void slotSelectAll();
    void slotSelectNone();

protected:

    void resizeEvent(QResizeEvent* e);
    void viewportPaintEvent(QPaintEvent *e);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsWheelEvent(QWheelEvent *e);

private:

    QRect  calcSeletedArea();
    double calcAutoZoomFactor();
    void   updateAutoZoom();
    void   updateContentsSize(bool deleteRubber);

    void drawRubber();
    void paintViewport(const QRect& er, bool antialias);

    void reset();

private slots:

    void slotSelected();
    void slotModified();
    void slotImageLoaded(const QString& filePath, bool success);
    void slotImageSaved(const QString& filePath, bool success);
    void slotCornerButtonPressed();
    void slotZoomChanged(double);
    void slotPanIconSelectionMoved(const QRect&, bool);
    void slotPanIconHiden();

private:

    CanvasPrivate *d;
};

}  // namespace Digikam

#endif /* CANVAS_H */

