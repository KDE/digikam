/* ============================================================
 * File  : digikamcameraclient.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-18
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef DIGIKAMCAMERACLIENT_H
#define DIGIKAMCAMERACLIENT_H

#include <qobject.h>

#include "digikamcameraclientiface.h"

class CameraUI;
class CameraType;

class DigikamCameraClient : public QObject,
                            virtual public DigikamCameraClientIface
{
    Q_OBJECT

public:

    DigikamCameraClient();
    virtual ~DigikamCameraClient();

    ASYNC cameraOpen(QString libraryPath, QString downloadAlbum,
                     QString title, QString model,
                     QString port,  QString path);

    ASYNC cameraConnect();
    
    ASYNC cameraChangeLibraryPath(QString libraryPath);

    ASYNC cameraChangeDownloadAlbum(QString album);
    
    ASYNC cameraDownloadSelected();

    void close();

private:

    CameraUI *mCurrentUI;

private slots:

    void slotCameraUIFinished();

};

#endif /* DIGIKAMCAMERACLIENT_H */
