/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles 
 * Date   : 2003-02-03
 * Description : Cameras list container
 * 
 * Copyright 2003-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
    bool save();
    void clear();

    void insert(CameraType* ctype);
    void remove(CameraType* ctype);

    CameraType* find(const QString& title);
    CameraType* autoDetect(bool& retry);
    QPtrList<CameraType>* cameraList();
    
    bool changeCameraAccessTime(const QString& cameraTitle, const QDateTime& newDate);

    static CameraList* instance();

signals:

    void signalCameraAdded(CameraType*);
    void signalCameraRemoved(CameraType*);

private:

    void insertPrivate(CameraType* ctype);
    void removePrivate(CameraType* ctype);

private:

    static CameraList *m_instance;
    CameraListPrivate *d;

};

}  // namespace Digikam

#endif /* CAMERALIST_H */
