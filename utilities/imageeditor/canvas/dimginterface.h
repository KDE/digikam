/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2003-01-15
 * Description :
 *
 * Copyright 2003-2005 by Renchi Raju, Gilles Caulier
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

class DImgInterfacePrivate;

class DImgInterface : public QObject
{
    Q_OBJECT

public:

    static DImgInterface* instance();

    ~DImgInterface();

    bool   load(const QString& filename, bool *isReadOnly);
    void   setExifOrient(bool exifOrient);
    void   undo();
    void   redo();
    void   restore();
    bool   save(const QString& file, int JPEGcompression, 
                int PNGcompression, bool TIFFcompression);
    bool   saveAs(const QString& file, int JPEGcompression, 
                  int PNGcompression, bool TIFFcompression,
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

    void   getUndoHistory(QStringList &titles);
    void   getRedoHistory(QStringList &titles);

    // New methods for image plugins using DImg.
    DImg   getImage();
    void   putImage(DImg& image);
    void   putImage(const QString &caller, DImg& image);

    // FIXME : remove these methods when all image plugins will be ported to DImg.
    uint*  getData();
    void   putData(uint* data, int w, int h);
    void   putData(const QString &caller, uint* data, int w, int h);
    uint*  getSelectedData();
    void   putSelectedData(uint* data, bool saveUndo=true);

signals:

    void   signalModified(bool moreUndo, bool moreRedo);

private:

    bool   saveAction(const QString& fileName, int JPEGcompression, 
                      int PNGcompression, bool TIFFcompression, 
                      const QString& mimeType); 
    void   exifRotate(const QString& filename);

    DImgInterface();

    DImgInterfacePrivate *d;

    static DImgInterface *m_instance;

    bool                  m_rotatedOrFlipped;
};

}

#endif /* DIMGINTERFACE_H */
