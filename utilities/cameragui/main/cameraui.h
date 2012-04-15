/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-16
 * Description : Camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef CAMERAUI_H
#define CAMERAUI_H

// Qt includes

#include <QDateTime>
#include <QString>
#include <QImage>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QMultiMap>

// KDE includes

#include <kxmlguiwindow.h>
#include <kurl.h>

// Local includes

#include "camiteminfo.h"
#include "dhistoryview.h"
#include "dmetadata.h"
#include "camerahistoryupdater.h"
#include "downloadsettings.h"

namespace Digikam
{

class Album;
class CollectionLocation;
class CameraHistoryUpdater;

class CameraUI : public KXmlGuiWindow
{
    Q_OBJECT

public:

    CameraUI(QWidget* const parent,
             const QString& cameraTitle,
             const QString& model, const QString& port,
             const QString& path, int startIndex);
    ~CameraUI();

    bool isBusy() const;
    bool isClosed() const;

    bool cameraDeleteSupport() const;
    bool cameraUploadSupport() const;
    bool cameraMkDirSupport() const;
    bool cameraDelDirSupport() const;

    bool chronologicOrder() const;

    QString cameraTitle() const;

    DownloadSettings downloadSettings() const;

Q_SIGNALS:

    void signalLastDestination(const KUrl&);
    void signalWindowHasMoved();

public Q_SLOTS:

    void slotDownload(bool onlySelected, bool deleteAfter, Album* pAlbum = 0);

protected:

    void closeEvent(QCloseEvent* e);
    void moveEvent(QMoveEvent* e);

private:

    void setupActions();
    void setupConnections();
    void setupUserArea();
    void setupStatusBar();
    void setupAccelerators();
    void setupCameraController(const QString& model, const QString& port, const QString& path);

    void readSettings();
    void saveSettings();
    bool createAutoAlbum(const KUrl& parentURL, const QString& sub,
                         const QDate& date, QString& errMsg) const;

    bool dialogClosed();
    void finishDialog();
    void showToolBars();
    void hideToolBars();
    void refreshFreeSpace();
    void refreshCollectionFreeSpace();
    void deleteItems(bool onlySelected, bool onlyDownloaded);
    void checkItem4Deletion(const CamItemInfo& info, QStringList& folders, QStringList& files,
                            CamItemInfoList& deleteList, CamItemInfoList& lockedList);

private Q_SLOTS:

    void slotClose();
    void slotCancelButton();
    void slotProcessUrl(const QString& url);

    void slotShowLog();
    void slotConnected(bool val);
    void slotBusy(bool val);
    void slotLogMsg(const QString& msg, DHistoryView::EntryType type, const QString& folder, const QString& file);
    void slotInformation();
    void slotCapture();
    void slotCameraInformation(const QString&, const QString&, const QString&);
    void slotCameraFreeSpaceInfo(unsigned long kBSize, unsigned long kBAvail);
    void slotCollectionLocationStatusChanged(const CollectionLocation& location, int oldStatus);
    void slotHistoryEntryClicked(const QVariant&);

    void slotFolderList(const QStringList& folderList);
    void slotFileList(const CamItemInfoList& fileList);

    void slotIncreaseThumbSize();
    void slotDecreaseThumbSize();
    void slotZoomSliderChanged(int size);
    void slotThumbSizeChanged(int size);

    void slotToggleFullScreen();
    void slotEscapePressed();

    void slotUpload();
    void slotUploadItems(const KUrl::List&);

    void slotDownloadNew();
    void slotDownloadSelected();
    void slotDownloadAll();

    void slotDownloadAndDeleteNew();
    void slotDownloadAndDeleteSelected();
    void slotDownloadAndDeleteAll();

    void slotDeleteNew();
    void slotDeleteSelected();
    void slotDeleteAll();

    void slotToggleLock();
    void slotMarkAsDownloaded();

    void slotFileView();
    void slotFileView(const CamItemInfo&);

    void slotUploaded(const CamItemInfo&);
    void slotDownloaded(const QString&, const QString&, int);
    void slotDownloadComplete(const QString& sourceFolder, const QString& sourceFile,
                              const QString& destFolder, const QString& destFile);
    void slotSkipped(const QString&, const QString&);
    void slotDeleted(const QString&, const QString&, bool);
    void slotLocked(const QString&, const QString&, bool);

    void slotNewSelection(bool);
    void slotItemsSelected(const CamItemInfo&, bool selected);

    void slotMetadata(const QString& folder, const QString& file, const DMetadata& meta);

    void slotlastPhotoFirst();
    void slotFilterChanged();

    void slotEditKeys();
    void slotShowMenuBar();
    void slotConfToolbars();
    void slotConfNotifications();
    void slotNewToolbarConfig();
    void slotSetup();
    void slotComponentsInfo();
    void slotDBStat();

    void slotSidebarTabTitleStyleChanged();

    void slotRefreshIconViewTimer();
    void slotRefreshIconView(const CHUpdateItemMap& map);

private:

    class CameraUIPriv;
    CameraUIPriv* const d;
};

}  // namespace Digikam

#endif /* CAMERAUI_H */
