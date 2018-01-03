/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-16
 * Description : Camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMPORTUI_H
#define IMPORTUI_H

// Qt includes

#include <QDateTime>
#include <QString>
#include <QImage>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QMultiMap>
#include <QUrl>

// Local includes

#include "camiteminfo.h"
#include "dhistoryview.h"
#include "dmetadata.h"
//#include "camerahistoryupdater.h"
#include "downloadsettings.h"
#include "importiconview.h"
#include "dxmlguiwindow.h"
#include "digikam_export.h"

namespace Digikam
{

class Album;
class PAlbum;
class CollectionLocation;
class CameraHistoryUpdater;
class ImportIconView;
class CameraThumbsCtrl;

class DIGIKAM_EXPORT ImportUI : public DXmlGuiWindow
{
    Q_OBJECT

public:

    explicit ImportUI(const QString& cameraTitle, const QString& model,
             const QString& port, const QString& path, int startIndex);
    virtual ~ImportUI();

    static ImportUI* instance();

    bool isBusy() const;
    bool isClosed() const;

    bool    cameraDeleteSupport() const;
    bool    cameraUploadSupport() const;
    bool    cameraMkDirSupport() const;
    bool    cameraDelDirSupport() const;
    bool    cameraUseUMSDriver() const;
    bool    cameraUseGPhotoDriver() const;
    QString cameraTitle() const;

    void enableZoomPlusAction(bool val);
    void enableZoomMinusAction(bool val);

    DownloadSettings downloadSettings() const;

    CameraThumbsCtrl* getCameraThumbsCtrl() const;

Q_SIGNALS:

    void signalLastDestination(const QUrl&);
    void signalWindowHasMoved();
    void signalEscapePressed();
    void signalPreviewRequested(const CamItemInfo&, bool);
    void signalNewSelection(bool);

public Q_SLOTS:

    void slotDownload(bool onlySelected, bool deleteAfter, Album* pAlbum = 0);
    void slotUploadItems(const QList<QUrl>&); // public to be used in drag'n'drop

protected:

    void closeEvent(QCloseEvent* e);
    void moveEvent(QMoveEvent* e);

private:

    void setupActions();
    void updateActions();
    void setupConnections();
    void setupUserArea();
    void setupStatusBar();
    void setupAccelerators();
    void setupCameraController(const QString& model, const QString& port, const QString& path);

    void readSettings();
    void saveSettings();
    bool createAutoAlbum(const QUrl& parentURL, const QString& sub,
                         const QDate& date, QString& errMsg) const;

    bool dialogClosed();
    void finishDialog();
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
    bool downloadCameraItems(PAlbum* pAlbum, bool onlySelected, bool deleteAfter);
    bool createSubAlbums(QUrl& downloadUrl, const CamItemInfo& info);
    bool createSubAlbum(QUrl& downloadUrl, const QString& subalbum, const QDate& date);
    bool createDateBasedSubAlbum(QUrl& downloadUrl, const CamItemInfo& info);
    bool createExtBasedSubAlbum(QUrl& downloadUrl, const CamItemInfo& info);

    void showThumbBar(bool visible);
    void showSideBars(bool visible);
    bool thumbbarVisibility() const;
    void customizedFullScreenMode(bool set);
    void toogleShowBar();
    void setInitialSorting();
    void sidebarTabTitleStyleChanged();
    void updateRightSideBar(const CamItemInfo& info);

private Q_SLOTS:

    void slotClose();
    void slotCancelButton();

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

    void slotZoomSliderChanged(int size);
    void slotZoomChanged(double zoom);
    void slotThumbSizeChanged(int size);

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

    void slotUpdateDownloadName();
    void slotSelectNew();
    void slotSelectLocked();
    void slotProgressTimerDone();

    void slotNewSelection(bool);
    void slotImageSelected(const CamItemInfoList& selection, const CamItemInfoList& listAll);

    void slotSwitchedToPreview();
    void slotSwitchedToIconView();
    void slotSwitchedToMapView();

    void slotMetadata(const QString& folder, const QString& file, const DMetadata& meta);

    void setFilter(Filter *);

    void slotToggleShowBar();
    void slotSetup();
    void slotColorManagementOptionsChanged();
    void slotToggleColorManagedView();
    void slotComponentsInfo();
    void slotDBStat();
    void slotToggleRightSideBar();
    void slotPreviousRightSideBarTab();
    void slotNextRightSideBarTab();

    void slotSetupChanged();

private:

    class Private;
    Private* const d;

    static ImportUI* m_instance;
};

} // namespace Digikam

#endif // IMPORTUI_H
