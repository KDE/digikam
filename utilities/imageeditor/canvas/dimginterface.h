/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2003-01-15
 * Description :
 *
 * Copyright 2003-2006 by Renchi Raju, Gilles Caulier
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

// KDE includes.

#include <klocale.h>

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

    bool   load(const QString& filename, bool *isReadOnly,
                ICCSettingsContainer *cmSettings, IOFileSettingsContainer* iofileSettings,
                QWidget *parent);
    void   setExifOrient(bool exifOrient);
    void   undo();
    void   redo();
    void   restore();
    bool   save(const QString& file, IOFileSettingsContainer *iofileSettings);
    bool   saveAs(const QString& file, IOFileSettingsContainer *iofileSettings,
                  const QString& mimeType=0);
    void   setModified (bool val);

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

    int    width();
    int    height();
    int    origWidth();
    int    origHeight();
    int    bytesDepth();
    bool   hasAlpha();
    bool   sixteenBit();
    bool   exifRotated();

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
    void   putImage(const QString &caller, uchar* data, int w, int h);
    uchar* getImageSelection();
    void   putImageSelection(uchar* data, bool saveUndo=true);

    // FIXME : remove these methods when all image plugins will be ported to DImg.
    uint*  getData();
    void   putData(uint* data, int w, int h);
    void   putData(const QString &caller, uint* data, int w, int h);
    uint*  getSelectedData();
    void   putSelectedData(uint* data, bool saveUndo=true);

signals:

    void   signalModified(bool moreUndo, bool moreRedo);

private:

    bool   saveAction(const QString& fileName, IOFileSettingsContainer *iofileSettings,
                      const QString& mimeType); 
    void   exifRotate(const QString& filename);

    DImgInterface();

private:

    DImgInterfacePrivate *d;

    static DImgInterface *m_instance;

    bool                  m_rotatedOrFlipped;
};

}  // namespace Digikam

#endif /* DIMGINTERFACE_H */
