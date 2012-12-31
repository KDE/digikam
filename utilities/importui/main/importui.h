/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-16
 * Description : Camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
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
#include "importiconview.h"
#include "importview.h"

namespace Digikam
{

class Album;
class CollectionLocation;
class CameraHistoryUpdater;
class ImportIconView;

class ImportUI : public KXmlGuiWindow
{
    Q_OBJECT

public:

    ImportUI(QWidget* const parent,
             const QString& cameraTitle,
             const QString& model, const QString& port,
             const QString& path, int startIndex);
    ~ImportUI();

    static ImportUI* instance();

    bool isBusy() const;
    bool isClosed() const;

    bool    cameraDeleteSupport() const;
    bool    cameraUploadSupport() const;
    bool    cameraMkDirSupport() const;
    bool    cameraDelDirSupport() const;
    QString cameraTitle() const;

    void enableZoomPlusAction(bool val);
    void enableZoomMinusAction(bool val);

    DownloadSettings downloadSettings() const;

    CameraController* getCameraController() const;

Q_SIGNALS:

    void signalLastDestination(const KUrl&);
    void signalWindowHasMoved();
    void signalEscapePressed();
    void signalPreviewRequested(CamItemInfo, bool);
    void signalNewSelection(bool);

public Q_SLOTS:

    void slotDownload(bool onlySelected, bool deleteAfter, Album* pAlbum = 0);
    void slotUploadItems(const KUrl::List&); // public to be used in drag'n'drop

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
    void toggleLock(CamItemInfo& info);
    void setDownloaded(CamItemInfo& itemInfo, int status);
    void itemsSelectionSizeInfo(unsigned long& fSizeKB, unsigned long& dSizeKB);
    QMap<QString, int> countItemsByFolders() const;
    void checkItem4Deletion(const CamItemInfo& info, QStringList& folders, QStringList& files,
                            CamItemInfoList& deleteList, CamItemInfoList& lockedList);

    QString identifyCategoryforMime(const QString& mime);
    void autoRotateItems();

    bool checkDiskSpace(PAlbum* pAlbum);
    bool downloadCameraItems(PAlbum *pAlbum, bool onlySelected, bool deleteAfter);

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
    void slotZoomChanged(double zoom);
    void slotThumbSizeChanged(int size);

    void slotToggleFullScreen();
    void slotEscapePressed();

    void slotUpload();

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

    void slotUploaded(const CamItemInfo&);
    void slotDownloaded(const QString&, const QString&, int);
    void slotDownloadComplete(const QString& sourceFolder, const QString& sourceFile,
                              const QString& destFolder, const QString& destFile);
    void slotSkipped(const QString&, const QString&);
    void slotDeleted(const QString&, const QString&, bool);
    void slotLocked(const QString&, const QString&, bool);

    void slotDownloadNameChanged();
    void slotSelectNew();
    void slotSelectLocked();
    void slotProgressTimerDone();

    void slotNewSelection(bool);
    void slotImageSelected(const CamItemInfoList& selection, bool hasPrev, bool hasNext,
                           const CamItemInfoList& listAll);

    void slotItemsSelected(CamItemInfo info, bool selected);

    void slotSwitchedToPreview();
    void slotSwitchedToIconView();
    void slotSwitchedToMapView();

    void slotMetadata(const QString& folder, const QString& file, const DMetadata& meta);

    void slotFilterChanged();

    void slotEditKeys();
    void slotToggleShowBar();
    void slotShowMenuBar();
    void slotConfToolbars();
    void slotConfNotifications();
    void slotNewToolbarConfig();
    void slotSetup();
    void slotComponentsInfo();
    void slotDBStat();

    void slotSidebarTabTitleStyleChanged();

private:

    class Private;
    Private* const d;

    static ImportUI* m_instance;
};

}  // namespace Digikam

#endif /* CAMERAUI_H */
