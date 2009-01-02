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

// Qt includes.

#include <qobject.h>
#include <qstring.h>

// Local includes.

#include "digikam_export.h"
#include "dimg.h"

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

    void   load(const QString& filename, IOFileSettingsContainer *iofileSettings, QWidget *parent=0);

    void   setICCSettings(ICCSettingsContainer *cmSettings);
    void   setExposureSettings(ExposureSettingsContainer *expoSettings);
    void   setExifOrient(bool exifOrient);

    void   undo();
    void   redo();
    void   restore();

    void   saveAs(const QString& file, IOFileSettingsContainer *iofileSettings,
                  bool setExifOrientationTag, const QString& mimeType=QString());

    void   switchToLastSaved(const QString& newFilename);
    void   abortSaving();
    void   setModified();
    void   readMetadataFromFile(const QString &file);
    void   clearUndoManager();
    void   setUndoManagerOrigin();
    void   updateUndoState();
    void   resetImage();

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

    void   changeGamma(double gamma);
    void   changeBrightness(double brightness);
    void   changeContrast(double contrast);
    void   changeBCG(double gamma, double brightness, double contrast);

    void   setBCG(double brightness, double contrast, double gamma);

    void   convertDepth(int depth);

    void   getUndoHistory(QStringList &titles);
    void   getRedoHistory(QStringList &titles);

    DImg*  getImg();
    uchar* getImage();

    void   putImage(uchar* data, int w, int h);
    void   putImage(uchar* data, int w, int h, bool sixteenBit);
    void   putImage(const QString &caller, uchar* data, int w, int h);
    void   putImage(const QString &caller, uchar* data, int w, int h, bool sixteenBit);

    uchar* getImageSelection();
    void   putImageSelection(const QString &caller, uchar* data);

    void   setEmbeddedICCToOriginalImage( QString profilePath);

    /** Convert a DImg image to a pixmap for screen using color 
        managemed view if necessary */
    QPixmap               convertToPixmap(DImg& img);

    QByteArray            getEmbeddedICC();
    QByteArray            getExif();
    QByteArray            getIptc();

    ICCSettingsContainer *getICCSettings();

    QString               getImageFileName();
    QString               getImageFilePath();
    QString               getImageFormat();

    QColor                underExposureColor();
    QColor                overExposureColor();

protected slots:

    void   slotImageLoaded(const LoadingDescription &loadingDescription, const DImg& img);
    void   slotImageSaved(const QString& filePath, bool success);
    void   slotLoadingProgress(const LoadingDescription &loadingDescription, float progress);
    void   slotSavingProgress(const QString& filePath, float progress);

signals:

    void   signalModified();
    void   signalUndoStateChanged(bool moreUndo, bool moreRedo, bool canSave);
    void   signalLoadingStarted(const QString& filename);
    void   signalLoadingProgress(const QString& filePath, float progress);
    void   signalImageLoaded(const QString& filePath, bool success);
    void   signalSavingProgress(const QString& filePath, float progress);
    void   signalImageSaved(const QString& filePath, bool success);

private slots:

    void slotUseRawImportSettings();
    void slotUseDefaultSettings();

private:

    void   exifRotate(const QString& filename);
    void   resetValues();

private:

    static DImgInterface *m_defaultInterface;

    DImgInterfacePrivate *d;
};

}  // namespace Digikam

#endif /* DIMGINTERFACE_H */
