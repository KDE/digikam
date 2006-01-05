/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-17
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

// Qt includes.

#include <qobject.h>

// Local includes.

#include "gpiteminfo.h"

class QString;
class QStringList;
class QImage;

namespace Digikam
{

class CameraControllerPriv;

class CameraController : public QObject
{
    Q_OBJECT

public:

    CameraController(QWidget* parent, const QString& mode,
                     const QString& port, const QString& path);
    ~CameraController();

    void listFolders();
    void listFiles(const QString& folder);
    void getThumbnail(const QString& folder, const QString& file);
    void getExif(const QString& folder, const QString& file);

    void downloadPrep();
    void download(const QString& folder, const QString& file,
                  const QString& dest, bool autoRotate);

    void deleteFile(const QString& folder, const QString& file);

    void openFile(const QString& folder, const QString& file);
    
protected:

    void customEvent(QCustomEvent* e);
    
private:

    CameraControllerPriv *d;
    friend class CameraThread;

public slots:

    void slotCancel();
    void slotConnect();

private slots:

    void slotProcessNext();

signals:

    void signalBusy(bool val);
    void signalInfoMsg(const QString& msg);
    void signalErrorMsg(const QString& msg);

    void signalConnected(bool val);
    void signalFolderList(const QStringList& folderList);
    void signalFileList(const GPItemInfoList& infoList);
    void signalDownloaded(const QString& folder, const QString& file);
    void signalSkipped(const QString& folder, const QString& file);
    void signalDeleted(const QString& folder, const QString& file);
    void signalThumbnail(const QString& folder, const QString& file,
                         const QImage& thumb);
    void signalExif(const QString& folder, const QString& file);
};
    
}  // namespace Digikam

#endif /* CAMERACONTROLLER_H */
