/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-17
 * Description : digital camera controller
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QThread>
#include <QString>
#include <QFileInfo>
#include <QCustomEvent>
#include <QPixmap>

// Local includes

#include "downloadsettingscontainer.h"
#include "gpiteminfo.h"
#include "dkcamera.h"
#include "dhistoryview.h"

namespace Digikam
{

class CameraCommand;
class RenameResult;

class CameraController : public QThread
{
    Q_OBJECT

public:

    CameraController(QWidget* parent, const QString& title, const QString& model,
                     const QString& port, const QString& path);
    ~CameraController();

    bool cameraThumbnailSupport();
    bool cameraDeleteSupport();
    bool cameraUploadSupport();
    bool cameraMkDirSupport();
    bool cameraDelDirSupport();
    bool cameraCaptureImageSupport();

    QString cameraPath();
    QString cameraTitle();

    DKCamera::CameraDriverType cameraDriverType();

    QByteArray cameraMD5ID();

    void capture();
    void listFolders();
    void listFiles(const QString& folder);
    void getFreeSpace();
    void getExif(const QString& folder, const QString& file);
    void getCameraInformation();
    void getPreview();
    void getThumbnail(const QString& folder, const QString& file);
    /** Get thumbnials for a list of camera item. 'list' is a list of QStringList composed of
        2 values : item path and item filename.
     */
    void getThumbnails(const QList<QVariant>& list);

    void downloadPrep();
    void download(const DownloadSettingsContainer& downloadSettings);
    void upload(const QFileInfo& srcFileInfo, const QString& destFile, const QString& destFolder);
    void deleteFile(const QString& folder, const QString& file);
    void lockFile(const QString& folder, const QString& file, bool lock);
    void openFile(const QString& folder, const QString& file);

    QPixmap mimeTypeThumbnail(const QString& itemName);

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLogMsg(const QString& msg, DHistoryView::EntryType type, const QString& folder, const QString& file);
    void signalCameraInformation(const QString& summary, const QString& manual,
                                 const QString& about);
    void signalFreeSpace(unsigned long kBSize, unsigned long kBAvail);
    void signalPreview(const QImage& preview);

    void signalConnected(bool val);
    void signalFolderList(const QStringList& folderList);
    void signalFileList(const GPItemInfoList& infoList);
    void signalUploaded(const GPItemInfo& itemInfo);
    void signalDownloaded(const QString& folder, const QString& file, int status);
    void signalDownloadComplete(const QString& sourceFolder, const QString& sourceFile,
                                const QString& destFolder, const QString& destFile);
    void signalSkipped(const QString& folder, const QString& file);
    void signalDeleted(const QString& folder, const QString& file, bool status);
    void signalLocked(const QString& folder, const QString& file, bool status);
    void signalThumbnail(const QString& folder, const QString& file, const QImage& thumb);
    void signalThumbnailFailed(const QString& folder, const QString& file);
    void signalExifFromFile(const QString& folder, const QString& file);
    void signalExifData(const QByteArray& exifData);

    void signalInternalCheckRename(const QString& folder, const QString& file,
                                   const QString& destination, const QString& temp);
    void signalInternalDownloadFailed(const QString& folder, const QString& file);
    void signalInternalUploadFailed(const QString& folder, const QString& file, const QString& src);
    void signalInternalDeleteFailed(const QString& folder, const QString& file);
    void signalInternalLockFailed(const QString& folder, const QString& file);
    void signalInternalOpen(const QString& folder, const QString& file, const QString& dest);

public Q_SLOTS:

    void slotCancel();
    void slotConnect();

protected:

    void run();
    void executeCommand(CameraCommand* cmd);

private Q_SLOTS:

    void slotCheckRename(const QString& folder, const QString& file,
                         const QString& destination, const QString& temp);
    void slotDownloadFailed(const QString& folder, const QString& file);
    void slotUploadFailed(const QString& folder, const QString& file, const QString& src);
    void slotDeleteFailed(const QString& folder, const QString& file);
    void slotLockFailed(const QString& folder, const QString& file);
    void slotOpen(const QString& folder, const QString& file, const QString& dest);

private:

    void sendBusy(bool val);
    void sendLogMsg(const QString& msg, DHistoryView::EntryType type=DHistoryView::StartingEntry,
                    const QString& folder=QString(), const QString& file=QString());

    void addCommand(CameraCommand* cmd);
    bool queueIsEmpty();

private:

    class CameraControllerPriv;
    CameraControllerPriv* const d;
};

}  // namespace Digikam

#endif /* CAMERACONTROLLER_H */
