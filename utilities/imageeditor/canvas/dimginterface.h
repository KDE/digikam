/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-15
 * Description : DImg interface for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIMGINTERFACE_H
#define DIMGINTERFACE_H

// Qt includes

#include <QObject>
#include <QString>
#include <QByteArray>

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "dimagehistory.h"

class QWidget;
class QPixmap;

namespace Digikam
{

class ICCSettingsContainer;
class ExposureSettingsContainer;
class IOFileSettingsContainer;
class LoadingDescription;
class DImgInterfacePrivate;

class DIGIKAM_EXPORT DImgInterface : public QObject
{
    Q_OBJECT

public:

    static DImgInterface* defaultInterface();
    static void setDefaultInterface(DImgInterface *defaultInterface);

    DImgInterface();
    ~DImgInterface();

    void   load(const QString& filename, IOFileSettingsContainer *iofileSettings);
    void   applyTransform(const IccTransform& transform);
    void   updateColorManagement();
    void   setSoftProofingEnabled(bool enabled);

    void   setICCSettings(ICCSettingsContainer* cmSettings);
    ICCSettingsContainer* getICCSettings();

    void   setExposureSettings(ExposureSettingsContainer* expoSettings);
    ExposureSettingsContainer* getExposureSettings();

    void   setExifOrient(bool exifOrient);
    void   setDisplayingWidget(QWidget *widget);

    void   undo();
    void   redo();
    void   restore();

    void   saveAs(const QString& file, IOFileSettingsContainer *iofileSettings,
                  bool setExifOrientationTag, const QString& mimeType=QString());

    void   addLastSavedToHistory(const QString& filename);
    void   switchToLastSaved(const QString& newFilename);
    void   abortSaving();
    void   setModified();
    void   setModified(const FilterAction& action);
    void   readMetadataFromFile(const QString& file);
    void   clearUndoManager();
    void   setUndoManagerOrigin();
    void   updateUndoState();
    void   resetImage();
    bool   hasChangesToSave();
    QString ensureHasCurrentUuid();

    void   zoom(double val);

    void   paintOnDevice(QPaintDevice* p,
                         int sx, int sy, int sw, int sh,
                         int dx, int dy, int dw, int dh,
                         int antialias);
    void   paintOnDevice(QPaintDevice* p,
                         int sx, int sy, int sw, int sh,
                         int dx, int dy, int dw, int dh,
                         int mx, int my, int mw, int mh,
                         int antialias);

    bool   imageValid();
    int    width();
    int    height();
    int    origWidth();
    int    origHeight();
    int    bytesDepth();
    bool   hasAlpha();
    bool   sixteenBit();
    bool   exifRotated();
    bool   isReadOnly();

    void   setSelectedArea(int x, int y, int w, int h);
    void   getSelectedArea(int& x, int& y, int& w, int& h);

    void   rotate90(bool saveUndo=true);
    void   rotate180(bool saveUndo=true);
    void   rotate270(bool saveUndo=true);

    void   flipHoriz(bool saveUndo=true);
    void   flipVert(bool saveUndo=true);

    void   crop(int x, int y, int w, int h);

    void   resize(int w, int h);

    void   convertDepth(int depth);

    void   getUndoHistory(QStringList& titles);
    void   getRedoHistory(QStringList& titles);

    DImg*  getImg();
    uchar* getImage();

    void   putImage(uchar* data, int w, int h);
    void   putImage(uchar* data, int w, int h, bool sixteenBit);
    void   putImage(const QString& caller, uchar* data, int w, int h);
    //TODO: remove the variants not passing a FilterAction, once fully implemented
    void   putImage(const QString& caller, const FilterAction& action, uchar* data, int w, int h);
    void   putImage(const QString& caller, uchar* data, int w, int h, bool sixteenBit);
    void   putImage(const QString& caller, const FilterAction& action, uchar* data, int w, int h, bool sixteenBit);

    uchar* getImageSelection();
    void   putImageSelection(const QString& caller, uchar* data);
    void   putImageSelection(const QString& caller, const FilterAction& action, uchar* data);

    void   putIccProfile(const IccProfile& profile);

    /** Convert a DImg image to a pixmap for screen using color
        managed view if necessary */
    QPixmap               convertToPixmap(DImg& img);

    IccProfile            getEmbeddedICC();
    KExiv2Data            getMetadata();
    DImageHistory         getImageHistory();
    DImageHistory         getInitialImageHistory();
    /** This takes ImageHistory from UndoManager's current history point
        and sets it as current image's ImageHistory */
    void                  setImageHistoryToCurrent();

    QString               getImageFileName();
    QString               getImageFilePath();
    QString               getImageFormat();

protected Q_SLOTS:

    void   slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img);
    void   slotImageSaved(const QString& filePath, bool success);
    void   slotLoadingProgress(const LoadingDescription& loadingDescription, float progress);
    void   slotSavingProgress(const QString& filePath, float progress);

Q_SIGNALS:

    void   signalModified();
    void   signalModifiedWithFilterAction();
    void   signalUndoStateChanged(bool moreUndo, bool moreRedo, bool canSave);

    void   signalLoadingStarted(const QString& filename);
    void   signalLoadingProgress(const QString& filePath, float progress);
    void   signalImageLoaded(const QString& filePath, bool success);
    void   signalSavingProgress(const QString& filePath, float progress);
    void   signalImageSaved(const QString& filePath, bool success);

private Q_SLOTS:

    void slotLoadRawFromTool();
    void slotLoadRaw();

private:

    void   load(const LoadingDescription& description);
    void   loadCurrent();
    void   exifRotate(const QString& filename);
    void   resetValues();

private:

    static DImgInterface *m_defaultInterface;

    DImgInterfacePrivate* const d;
};

}  // namespace Digikam

#endif /* DIMGINTERFACE_H */
