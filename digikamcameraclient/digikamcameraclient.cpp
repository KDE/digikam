/* ============================================================
 * File  : digikamcameraclient.cpp
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

#include <kapplication.h>
#include <klocale.h>

#include "digikamcameraclient.h"

#include "cameratype.h"
#include "cameraui.h"

DigikamCameraClient::DigikamCameraClient()
    : DCOPObject("DigikamCameraClient"),
      QObject()
{
    mCurrentUI = 0;
}

DigikamCameraClient::~DigikamCameraClient()
{

}

ASYNC DigikamCameraClient::cameraOpen(QString libraryPath, QString downloadAlbum,
                                      QString title, QString model,
                                      QString port, QString path)
{
    if (mCurrentUI) return;

    // Make sure to ref the app so that we don't
    // exit when camerui kmainwindow derefs at exit
    kapp->ref();

    CameraType ctype(title, model, port, path);
    CameraUI *cameraUI = new CameraUI(libraryPath, downloadAlbum, ctype);
    connect(cameraUI, SIGNAL(signalFinished()),
            this,     SLOT(slotCameraUIFinished()));

    cameraUI->show();

    mCurrentUI = cameraUI;
}

ASYNC DigikamCameraClient::cameraConnect()
{
    if (!mCurrentUI) return;

    mCurrentUI->connectCamera();
}

ASYNC DigikamCameraClient::cameraChangeLibraryPath(QString libraryPath)
{
    if (!mCurrentUI) return;

    mCurrentUI->changeLibraryPath(libraryPath);
}

ASYNC DigikamCameraClient::cameraChangeDownloadAlbum(QString album)
{
    if (!mCurrentUI) return;

    mCurrentUI->changeDownloadAlbum(album);
}

ASYNC DigikamCameraClient::cameraDownloadSelected()
{
    if (!mCurrentUI) return;

    mCurrentUI->downloadSelected();
}

void DigikamCameraClient::close()
{
    if (mCurrentUI) {
        mCurrentUI->close();
        mCurrentUI = 0;
    }

    kapp->deref();
}

void DigikamCameraClient::slotCameraUIFinished()
{
    mCurrentUI = 0;
}


#include "digikamcameraclient.moc"
