//////////////////////////////////////////////////////////////////////////////
//
//    IMLIBINTERFACE.H
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef IMLIBINTERFACE_H
#define IMLIBINTERFACE_H

// Qt lib includes.

#include <qobject.h>

class QWidget;
class QString;
class ImlibInterfacePrivate;

class ImlibInterface : public QObject 
{
    Q_OBJECT

public:

    ImlibInterface(QWidget *parent);
    ~ImlibInterface();

    void load(const QString& file);
    void preload(const QString& file);

    void paint(int dx, int dy, int dw, int dh,
               int sx, int sy);

    int width();
    int height();
    int origWidth();
    int origHeight();
    void zoom(double val);

    void rotate90();
    void rotate180();
    void rotate270();

    void flipHorizontal();
    void flipVertical();
    
    void crop(int x, int y, int w, int h);

    void changeGamma(int val);
    void changeBrightness(int val);
    void changeContrast(int val);

    void getBCGSettings(int& gamma, int& brightness,
                        int& contrast);

    int save(const QString& file);
    void restore();

private:

    ImlibInterfacePrivate *d;
};

#endif // IMLIBINTERFACE_H 
