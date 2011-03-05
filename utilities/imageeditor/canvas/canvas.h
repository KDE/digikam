/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-09
 * Description : image editor canvas management class
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <Q3ScrollView>
#include <QRect>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>

// Local includes

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
class IccTransform;
class IOFileSettingsContainer;

class DIGIKAM_EXPORT Canvas : public Q3ScrollView
{
    Q_OBJECT

public:

    Canvas(QWidget* parent=0);
    ~Canvas();

    void    load(const QString& filename, IOFileSettingsContainer* IOFileSettings);
    void    preload(const QString& filename);

    void    saveAs(const QString& filename, IOFileSettingsContainer* IOFileSettings,
                   bool setExifOrientationTag, const QString& mimeType=QString());
    void    resetImage();
    void    abortSaving();
    void    applyTransform(const IccTransform& transform);
    void    setModified();
    void    readMetadataFromFile(const QString& file);
    QString ensureHasCurrentUuid();
    void    clearUndoHistory();
    void    setUndoHistoryOrigin();
    void    updateUndoState();
    DImg    currentImage();
    QString currentImageFileFormat();
    QString currentImageFilePath();

    DImgInterface* interface() const;
    void makeDefaultEditingCanvas();

    double snapZoom(double z);
    void   setZoomFactorSnapped(double zoom);

    double zoomFactor();
    double zoomMax();
    double zoomMin();
    void   setZoomFactor(double z);

    void   setFitToWindow(bool enable);
    bool   fitToWindow();
    bool   maxZoom();
    bool   minZoom();
    bool   exifRotated();
    int    imageWidth();
    int    imageHeight();
    QRect  getSelectedArea();
    QRect  visibleArea();

    // If current image file format is only available in read only,
    // typically all RAW image file formats.
    bool  isReadOnly();
    bool  hasChangesToSave();

    void  resizeImage(int w, int h);

    void  setBackgroundColor(const QColor& color);
    void  setICCSettings(const ICCSettingsContainer& cmSettings);
    void  setExposureSettings(ExposureSettingsContainer* expoSettings);
    void  setSoftProofingEnabled(bool enable);

    void  setExifOrient(bool exifOrient);

    void  toggleFitToWindow();
    void  fitToSelect();

Q_SIGNALS:

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
    void signalLoadingStarted(const QString& filename);
    void signalLoadingFinished(const QString& filename, bool success);
    void signalLoadingProgress(const QString& filePath, float progress);
    void signalSavingStarted(const QString& filename);
    void signalSavingFinished(const QString& filename, bool success);
    void signalSavingProgress(const QString& filePath, float progress);
    void signalSelectionChanged(const QRect&);
    void signalToggleOffFitToWindow();
    void signalUndoSteps(int);
    void signalRedoSteps(int);

public Q_SLOTS:

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

    void resizeEvent(QResizeEvent*);
    void viewportPaintEvent(QPaintEvent*);
    void contentsMousePressEvent(QMouseEvent*);
    void contentsMouseMoveEvent(QMouseEvent*);
    void contentsMouseReleaseEvent(QMouseEvent*);
    void contentsWheelEvent(QWheelEvent*);
    void keyPressEvent(QKeyEvent*);

private:

    QRect  calcSelectedArea();
    double calcAutoZoomFactor();
    void   updateAutoZoom();
    void   updateContentsSize(bool deleteRubber);

    void   paintViewport(const QRect& er, bool antialias);

    void   reset();

private Q_SLOTS:

    void slotSelected();
    void slotModified();
    void slotImageLoaded(const QString& filePath, bool success);
    void slotImageSaved(const QString& filePath, bool success);
    void slotCornerButtonPressed();
    void slotZoomChanged(double);
    void slotPanIconSelectionMoved(const QRect&, bool);
    void slotPanIconHiden();

private:

    CanvasPrivate* const d;
};

}  // namespace Digikam

#endif /* CANVAS_H */
