/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2003-01-15
 * Description : DImg interface for image editor
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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

#include <qimage.h>
#include <qobject.h>

// Locale includes.

#include "dimg.h"

class QWidget;
class QString;
class QPixmap;

namespace Digikam
{

class ICCSettingsContainer;
class IOFileSettingsContainer;
class DImgInterfacePrivate;

class DImgInterface : public QObject
{
    Q_OBJECT

public:

    static DImgInterface* instance();

    ~DImgInterface();

    void   load(const QString& filename, IOFileSettingsContainer *iofileSettings, QWidget *parent=0);

    void   setICCSettings(ICCSettingsContainer *cmSettings);
    void   setExifOrient(bool exifOrient);
    void   undo();
    void   redo();
    void   restore();

    void   saveAs(const QString& file, IOFileSettingsContainer *iofileSettings,
                  bool setExifOrientationTag, const QString& mimeType=QString::null);

    void   switchToLastSaved(const QString& newFilename);
    void   abortSaving();
    void   setModified();
    void   readMetadataFromFile(const QString &file);
    void   clearUndoManager();
    void   setUndoManagerOrigin();
    void   updateUndoState();

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

    QByteArray            getEmbeddedICC();
    QByteArray            getExif();
    QByteArray            getIptc();    

    ICCSettingsContainer *getICCSettings();

    QString               getImageFileName();

protected slots:

    void   slotImageLoaded(const QString&, const DImg& img);
    void   slotImageSaved(const QString& filePath, bool success);
    void   slotLoadingProgress(const QString& filePath, float progress);
    void   slotSavingProgress(const QString& filePath, float progress);

signals:

    void   signalColorManagementTool();
    
    void   signalModified();
    void   signalUndoStateChanged(bool moreUndo, bool moreRedo, bool canSave);

    void   signalLoadingProgress(const QString& filePath, float progress);
    void   signalImageLoaded(const QString& filePath, bool success);
    void   signalSavingProgress(const QString& filePath, float progress);
    void   signalImageSaved(const QString& filePath, bool success);

private:

    void   exifRotate(const QString& filename);

    DImgInterface();

private:

    static DImgInterface *m_instance;
    
    DImgInterfacePrivate *d;
};

}  // namespace Digikam

#endif /* DIMGINTERFACE_H */
