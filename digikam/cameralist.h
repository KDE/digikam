/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-03
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#ifndef CAMERALIST_H
#define CAMERALIST_H

// Qt includes.

#include <qptrlist.h>
#include <qobject.h>

class QString;

namespace Digikam
{

class CameraType;
class CameraListPrivate;

class CameraList : public QObject
{
    Q_OBJECT
    
public:

    CameraList(QObject *parent, const QString& file);
    ~CameraList();

    bool load();
    bool close();

    void insert(CameraType* ctype);
    void remove(CameraType* ctype);
    CameraType* find(const QString& title);
    CameraType* autoDetect(bool& retry);
    void clear();

    QPtrList<CameraType>* cameraList();

    static CameraList* instance();
    
private:

    static CameraList* instance_;
    CameraListPrivate *d;

    void insertPrivate(CameraType* ctype);
    void removePrivate(CameraType* ctype);

signals:

    void signalCameraAdded(CameraType*);
    void signalCameraRemoved(CameraType*);

};

}  // namespace Digikam

#endif /* CAMERALIST_H */
