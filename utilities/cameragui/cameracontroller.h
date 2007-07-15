/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2004-09-17
 * Description : digital camera controller
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

// Qt includes.

#include <QThread>
#include <QString>
#include <QFileInfo>
#include <QCustomEvent>

// Local includes.

#include "downloadsettingscontainer.h"
#include "gpiteminfo.h"

namespace Digikam
{

class CameraCommand;
class RenameResult;
class CameraControllerPriv;

class CameraController : public QThread
{
    Q_OBJECT

public:

    CameraController(QWidget* parent, const QString& title, const QString& model,
                     const QString& port, const QString& path);
    ~CameraController();

    void listFolders();
    void listFiles(const QString& folder);
    void getThumbnail(const QString& folder, const QString& file);
    void getExif(const QString& folder, const QString& file);
    void getCameraInformations();
    QString getCameraPath();
    QString getCameraTitle();

    void downloadPrep();
    void download(DownloadSettingsContainer downloadSettings);
    void upload(const QFileInfo& srcFileInfo, const QString& destFile, const QString& destFolder);
    void deleteFile(const QString& folder, const QString& file);
    void lockFile(const QString& folder, const QString& file, bool lock);
    void openFile(const QString& folder, const QString& file);

signals:

    void signalBusy(bool val);
    void signalInfoMsg(const QString& msg);
    void signalErrorMsg(const QString& msg);
    void signalCameraInformations(const QString& summary, const QString& manual, const QString& about);

    void signalConnected(bool val);
    void signalFolderList(const QStringList& folderList);
    void signalFileList(const GPItemInfoList& infoList);
    void signalUploaded(const GPItemInfo& itemInfo);
    void signalDownloaded(const QString& folder, const QString& file, int status);
    void signalSkipped(const QString& folder, const QString& file);
    void signalDeleted(const QString& folder, const QString& file, bool status);
    void signalLocked(const QString& folder, const QString& file, bool status);
    void signalThumbnail(const QString& folder, const QString& file, const QImage& thumb);
    void signalExifFromFile(const QString& folder, const QString& file);
    void signalExifData(const QByteArray& exifData);

public slots:

    void slotCancel();
    void slotConnect();

protected:

    void run();
    void executeCommand(CameraCommand *cmd);

signals:
    void signalInternalNeedRename(const QString &folder, const QString &file, const QString &dest, RenameResult *renameResult);
    void signalInternalDownloadFailed(const QString &folder, const QString &file);
    void signalInternalUploadFailed(const QString &folder, const QString &file, const QString &src);
    void signalInternalDeleteFailed(const QString &folder, const QString &file);
    void signalInternalLockFailed(const QString &folder, const QString &file);
    void signalInternalOpen(const QString &folder, const QString &file, const QString &dest);

private slots:

    void slotNeedRename(const QString &folder, const QString &file, const QString &dest, RenameResult *renameResult);
    void slotDownloadFailed(const QString &folder, const QString &file);
    void slotUploadFailed(const QString &folder, const QString &file, const QString &src);
    void slotDeleteFailed(const QString &folder, const QString &file);
    void slotLockFailed(const QString &folder, const QString &file);
    void slotOpen(const QString &folder, const QString &file, const QString &dest);

private:

    void sendBusy(bool val);
    void sendError(const QString& msg);
    void sendInfo(const QString& msg);

    CameraControllerPriv *d;
};

}  // namespace Digikam

#endif /* CAMERACONTROLLER_H */
