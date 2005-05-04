/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2003-01-15
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

#ifndef IMLIBINTERFACE_H
#define IMLIBINTERFACE_H

// Qt includes.

#include <qobject.h>
#include <klocale.h>

class QWidget;
class QString;
class QPixmap;

namespace Digikam
{

class ImlibInterfacePrivate;

class ImlibInterface : public QObject
{

    Q_OBJECT
    
public:

    static ImlibInterface* instance();

    ~ImlibInterface();

    bool load(const QString& filename);
    void preload(const QString& filename);
    void setExifOrient(bool exifOrient);
    void undo();
    void redo();
    void restore();
    bool save(const QString& file, int JPEGcompression, 
              int PNGcompression, bool TIFFcompression);
    bool saveAs(const QString& file, int JPEGcompression, 
                int PNGcompression, bool TIFFcompression,
                const QString& mimeType=0);
    void setModified (bool val);
        
    void zoom(double val);

    void paintOnDevice(QPaintDevice* p,
                       int sx, int sy, int sw, int sh,
                       int dx, int dy, int dw, int dh,
                       int antialias);
    void paintOnDevice(QPaintDevice* p,
                       int sx, int sy, int sw, int sh,
                       int dx, int dy, int dw, int dh,
                       int mx, int my, int mw, int mh,
                       int antialias);
    
    int  width();
    int  height();
    int  origWidth();
    int  origHeight();
    bool hasAlpha();    
    bool exifRotated();

    void setSelectedArea(int x, int y, int w, int h);
    void getSelectedArea(int& x, int& y, int& w, int& h);
    
    void rotate90(bool saveUndo=true);
    void rotate180(bool saveUndo=true);
    void rotate270(bool saveUndo=true);
    
    void flipHoriz(bool saveUndo=true);
    void flipVert(bool saveUndo=true);
    
    void crop(int x, int y, int w, int h);

    void resize(int w, int h);
    
    void changeGamma(double gamma);
    void changeBrightness(double brightness);
    void changeContrast(double contrast);
    void changeBCG(double gamma, double brightness, double contrast);

    void setBCG(double brightness, double contrast, double gamma);
    
    uint* getData();
    void  putData(uint* data, int w, int h);
    void  putData(const QString &caller, uint* data, int w, int h);
    uint* getSelectedData();
    void  putSelectedData(uint* data, bool saveUndo=true);

    void getUndoHistory(QStringList &titles);
    void getRedoHistory(QStringList &titles);
    
signals:

    void signalModified(bool moreUndo, bool moreRedo);
    
private:

    bool saveAction(const QString& saveFile, int JPEGcompression, 
                    int PNGcompression, bool TIFFcompression, 
                    const QString& mimeType); 
    bool saveTIFF(const QString& saveFile, bool compress);
    void exifRotate(const QString& filename);
        
    ImlibInterface();
    ImlibInterfacePrivate *d;
    static ImlibInterface *m_instance;
    bool m_rotatedOrFlipped;
};

}

#endif /* IMLIBINTERFACE_H */
