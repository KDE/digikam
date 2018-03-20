/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C)      2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DIGIKAM_APP_H
#define DIGIKAM_APP_H

// Qt includes

#include <QAction>
#include <QString>
#include <QMenu>

// KDE includes

#include <Solid/SolidNamespace>

// Local includes

#include "digikam_config.h"
#include "digikam_export.h"
#include "dxmlguiwindow.h"
#include "digikam_export.h"

namespace Solid
{
    class Device;
}

namespace Digikam
{

class Album;
class DigikamView;
class FaceScanSettings;
class FilterStatusBar;
class ImageInfo;
class ImageInfoList;
class CameraType;

class DIGIKAM_EXPORT DigikamApp : public DXmlGuiWindow
{
    Q_OBJECT

public:

    DigikamApp();
    ~DigikamApp();

    virtual void show();
    void restoreSession();

    QMenu* slideShowMenu() const;

    void autoDetect();
    void downloadFrom(const QString& cameraGuiPath);
    void downloadFromUdi(const QString& udi);
    void enableZoomPlusAction(bool val);
    void enableZoomMinusAction(bool val);
    void enableAlbumBackwardHistory(bool enable);
    void enableAlbumForwardHistory(bool enable);

    void startInternalDatabase();
    void stopInternalDatabase();

    void rebuild();

    DigikamView* view() const;

    static DigikamApp* instance();
    static QString scannerTargetPlace();

Q_SIGNALS:

    void signalEscapePressed();
    void signalNextItem();
    void signalPrevItem();
    void signalFirstItem();
    void signalLastItem();
    void signalCutAlbumItemsSelection();
    void signalCopyAlbumItemsSelection();
    void signalPasteAlbumItemsSelection();

    void signalWindowHasMoved();

    void queuedOpenCameraUiFromPath(const QString& path);
    void queuedOpenSolidDevice(const QString& udi);

protected:

    bool queryClose();
    void moveEvent(QMoveEvent* e);
    void closeEvent(QCloseEvent* e);

private:

    bool    setup();
    bool    setupICC();
    void    setupView();
    void    setupViewConnections();
    void    setupStatusBar();
    void    setupActions();
    void    setupAccelerators();
    void    setupExifOrientationActions();
    void    setupImageTransformActions();
    void    setupSelectToolsAction();
    void    loadCameras();
    void    populateThemes();
    void    preloadWindows();
    void    fillSolidMenus();
    bool    checkSolidCamera(const Solid::Device& cameraDevice);
    QString labelForSolidCamera(const Solid::Device& cameraDevice);
    void    openSolidCamera(const QString& udi, const QString& label = QString());
    void    openSolidUsmDevice(const QString& udi, const QString& label = QString());
    void    updateCameraMenu();
    void    updateQuickImportAction();
    void    initGui();
    void    showThumbBar(bool visible);
    void    showSideBars(bool visible);
    bool    thumbbarVisibility() const;
    void    customizedFullScreenMode(bool set);
    void    toggleShowBar();

private Q_SLOTS:

    void slotAlbumSelected(Album*);
    void slotImageSelected(const ImageInfoList&, const ImageInfoList&);
    void slotSelectionChanged(int selectionCount);
    void slotExit();
    void slotDBStat();
    void slotComponentsInfo();

    void slotRecurseAlbums(bool);
    void slotRecurseTags(bool);

    void slotAboutToShowForwardMenu();
    void slotAboutToShowBackwardMenu();

    void slotSetup();
    void slotSetupCamera();
    void slotSetupChanged();
    void slotColorManagementOptionsChanged();
    void slotToggleColorManagedView();
    void slotSetCheckedExifOrientationAction(const ImageInfo& info);
    void slotResetExifOrientationActions();
    void slotTransformAction();

    void slotOpenSolidCamera(QAction*);
    void slotOpenManualCamera(QAction*);
    void slotOpenSolidUsmDevice(QAction*);
    void slotOpenSolidDevice(const QString& udi);
    void slotOpenCameraUiFromPath(const QString& path);
    void slotSolidSetupDone(Solid::ErrorType errorType, QVariant errorData, const QString& udi);
    void slotSolidDeviceChanged(const QString& udi);
    void slotCameraAdded(CameraType*);
    void slotCameraRemoved(QAction*);
    void slotCameraAutoDetect();
    void downloadImages(const QString& folder);
    void cameraAutoDetect();

    void slotToggleLeftSideBar();
    void slotToggleRightSideBar();
    void slotPreviousLeftSideBarTab();
    void slotNextLeftSideBarTab();
    void slotPreviousRightSideBarTab();
    void slotNextRightSideBarTab();

    void slotToggleShowBar();
    void slotEditKeys();

    void slotMaintenance();
    void slotMaintenanceDone();
    void slotExpoBlending();
    void slotPanorama();
    void slotVideoSlideshow();
    void slotSendByMail();
    void slotPrintCreator();
    void slotHtmlGallery();
    void slotCalendar();
    void slotPresentation();
    void slotMediaServer();

    void slotDatabaseMigration();

    void slotZoomSliderChanged(int);
    void slotThumbSizeChanged(int);
    void slotZoomChanged(double);

    void slotSwitchedToPreview();
    void slotSwitchedToIconView();
    void slotSwitchedToMapView();
    void slotSwitchedToTableView();
    void slotSwitchedToTrashView();

    void slotImportAddImages();
    void slotImportAddFolders();
    void slotThemeChanged();

    void slotImportFromScanner();
    void slotEditMetadata();
    void slotEditGeolocation();

    void slotExportTool();
    void slotImportTool();

private:

    class Private;
    Private* const d;

    static DigikamApp* m_instance;
};

}  // namespace Digikam

#endif  // DIGIKAM_APP_H
