/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-17
 * Description : digital camera controller
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "downloadsettings.h"
#include "setupcamera.h"
#include "camiteminfo.h"
#include "dmetadata.h"
#include "dkcamera.h"
#include "dhistoryview.h"
#include "digikam_export.h"

namespace Digikam
{

class CameraCommand;

class DIGIKAM_EXPORT CameraController : public QThread
{
    Q_OBJECT

public:

    CameraController(QWidget* const parent, const QString& title, const QString& model,
                     const QString& port, const QString& path);
    ~CameraController();

    bool cameraThumbnailSupport() const;
    bool cameraDeleteSupport() const;
    bool cameraUploadSupport() const;
    bool cameraMkDirSupport() const;
    bool cameraDelDirSupport() const;
    bool cameraCaptureImageSupport() const;
    bool cameraCaptureImagePreviewSupport() const;

    QString cameraPath() const;
    QString cameraTitle() const;

    DKCamera::CameraDriverType cameraDriverType() const;

    QByteArray cameraMD5ID() const;

    void capture();
    void listRootFolder(bool useMetadata);
    void listFolders(const QString& folder = QString());
    void listFiles(const QString& folder, bool useMetadata);
    void getFreeSpace();
    void getMetadata(const QString& folder, const QString& file);
    void getCameraInformation();
    void getPreview();

    /** Get thumbnails for a list of camera items plus advanced information from metadata.
     */
    void getThumbsInfo(const CamItemInfoList& infoList, int thumbSize);

    void downloadPrep(const SetupCamera::ConflictRule& rule);
    void download(const DownloadSettings& downloadSettings);
    void download(const DownloadSettingsList& list);
    void upload(const QFileInfo& srcFileInfo, const QString& destFile, const QString& destFolder);
    void deleteFile(const QString& folder, const QString& file);
    void lockFile(const QString& folder, const QString& file, bool lock);
    void openFile(const QString& folder, const QString& file);

    QIcon mimeTypeThumbnail(const QString& itemName) const;

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLogMsg(const QString& msg, DHistoryView::EntryType type, const QString& folder, const QString& file);
    void signalCameraInformation(const QString& summary, const QString& manual,
                                 const QString& about);
    void signalFreeSpace(unsigned long kBSize, unsigned long kBAvail);
    void signalPreview(const QImage& preview);

    void signalConnected(bool val);
    void signalFolderList(const QStringList& folderList);
    void signalFileList(const CamItemInfoList& infoList);
    void signalUploaded(const CamItemInfo& itemInfo);
    void signalDownloaded(const QString& folder, const QString& file, int status);
    void signalDownloadComplete(const QString& sourceFolder, const QString& sourceFile,
                                const QString& destFolder, const QString& destFile);
    void signalSkipped(const QString& folder, const QString& file);
    void signalDeleted(const QString& folder, const QString& file, bool status);
    void signalLocked(const QString& folder, const QString& file, bool status);
    void signalThumbInfo(const QString& folder, const QString& file, const CamItemInfo& itemInfo, const QImage& thumb);
    void signalThumbInfoFailed(const QString& folder, const QString& file, const CamItemInfo& itemInfo);
    void signalMetadata(const QString& folder, const QString& file, const DMetadata& exifData);

    void signalInternalCheckRename(const QString& folder, const QString& file,
                                   const QString& destination, const QString& temp,
                                   const QString& script);
    void signalInternalDownloadFailed(const QString& folder, const QString& file);
    void signalInternalUploadFailed(const QString& folder, const QString& file, const QString& src);
    void signalInternalDeleteFailed(const QString& folder, const QString& file);
    void signalInternalLockFailed(const QString& folder, const QString& file);

public Q_SLOTS:

    void slotCancel();
    void slotConnect();

protected:

    void run();
    void executeCommand(CameraCommand* const cmd);

private Q_SLOTS:

    void slotCheckRename(const QString& folder, const QString& file,
                         const QString& destination, const QString& temp, const QString& script);
    void slotDownloadFailed(const QString& folder, const QString& file);
    void slotUploadFailed(const QString& folder, const QString& file, const QString& src);
    void slotDeleteFailed(const QString& folder, const QString& file);
    void slotLockFailed(const QString& folder, const QString& file);

private:

    void sendLogMsg(const QString& msg, DHistoryView::EntryType type=DHistoryView::StartingEntry,
                    const QString& folder=QString(), const QString& file=QString());

    void addCommand(CameraCommand* const cmd);
    bool queueIsEmpty() const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* CAMERACONTROLLER_H */
