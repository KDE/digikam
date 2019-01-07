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
 * Copyright (C) 2002-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "dxmlguiwindow.h"
#include "digikam_export.h"

namespace Solid
{
    class Device;
}

namespace Digikam
{

class Album;
class ItemIconView;
class FaceScanSettings;
class FilterStatusBar;
class ItemInfo;
class ItemInfoList;
class CameraType;

class DIGIKAM_EXPORT DigikamApp : public DXmlGuiWindow
{
    Q_OBJECT

public:

    explicit DigikamApp();
    ~DigikamApp();

    virtual void show();
    void restoreSession();

    void enableZoomPlusAction(bool val);
    void enableZoomMinusAction(bool val);
    void enableAlbumBackwardHistory(bool enable);
    void enableAlbumForwardHistory(bool enable);

    ItemIconView* view()    const;
    QMenu* slideShowMenu() const;

    static DigikamApp* instance();

public:

    DInfoInterface* infoIface(DPluginAction* const ac);

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

protected:

    bool queryClose();
    void moveEvent(QMoveEvent* e);
    void closeEvent(QCloseEvent* e);

private:

    void showThumbBar(bool visible);
    void showSideBars(bool visible);
    bool thumbbarVisibility() const;
    void customizedFullScreenMode(bool set);
    void customizedTrashView(bool set);
    void toggleShowBar();

private Q_SLOTS:

    void slotAlbumSelected(Album*);
    void slotImageSelected(const ItemInfoList&, const ItemInfoList&);
    void slotSelectionChanged(int selectionCount);
    void slotExit();
    void slotDBStat();
    void slotComponentsInfo();

    void slotRecurseAlbums(bool);
    void slotRecurseTags(bool);

    void slotAboutToShowForwardMenu();
    void slotAboutToShowBackwardMenu();

    void slotColorManagementOptionsChanged();
    void slotToggleColorManagedView();
    void slotSetCheckedExifOrientationAction(const ItemInfo& info);
    void slotResetExifOrientationActions();
    void slotTransformAction();

    void slotToggleLeftSideBar();
    void slotToggleRightSideBar();
    void slotPreviousLeftSideBarTab();
    void slotNextLeftSideBarTab();
    void slotPreviousRightSideBarTab();
    void slotNextRightSideBarTab();

    void slotToggleShowBar();

    void slotZoomSliderChanged(int);
    void slotThumbSizeChanged(int);
    void slotZoomChanged(double);

    void slotSwitchedToPreview();
    void slotSwitchedToIconView();
    void slotSwitchedToMapView();
    void slotSwitchedToTableView();
    void slotSwitchedToTrashView();

// -- Internal setup methods implemented in digikamapp_setup.cpp ----------------------------------------

public:

    void rebuild();

private:

    void setupView();
    void setupViewConnections();
    void setupStatusBar();
    void setupActions();
    void setupAccelerators();
    void setupExifOrientationActions();
    void setupImageTransformActions();
    void populateThemes();
    void preloadWindows();
    void initGui();

// -- Extra tool methods implemented in digikamapp_tools.cpp ----------------------------------------

private:

    void setupSelectToolsAction();

private Q_SLOTS:

    void slotMaintenance();
    void slotMaintenanceDone();
    void slotDatabaseMigration();

// -- Configure methods implemented in digikamapp_config.cpp ----------------------------------------

private:

    bool setup();
    bool setupICC();

private Q_SLOTS:

    void slotSetup();
    void slotSetupChanged();
    void slotEditKeys();
    void slotThemeChanged();

// -- Export tools methods implemented in digikamapp_export.cpp -------------------------------------

private Q_SLOTS:

    void slotExportTool();

// -- Import tools methods implemented in digikamapp_import.cpp -------------------------------------

private:

    void updateQuickImportAction();

private Q_SLOTS:

    void slotImportedImagefromScanner(const QUrl& url);
    void slotImportAddImages();
    void slotImportAddFolders();
    void slotImportTool();

// -- Camera management methods implemented in digikamapp_camera.cpp --------------------------------

public:

    void autoDetect();
    void downloadFrom(const QString& cameraGuiPath);
    void downloadFromUdi(const QString& udi);

Q_SIGNALS:

    void queuedOpenCameraUiFromPath(const QString& path);

private:

    void loadCameras();
    void updateCameraMenu();

private Q_SLOTS:

    void slotSetupCamera();
    void slotOpenManualCamera(QAction*);
    void slotCameraAdded(CameraType*);
    void slotCameraRemoved(QAction*);
    void slotCameraAutoDetect();
    void slotOpenCameraUiFromPath(const QString& path);
    void downloadImages(const QString& folder);
    void cameraAutoDetect();

// -- Solid based methods implemented in digikamapp_solid.cpp ---------------------------------------

Q_SIGNALS:

    void queuedOpenSolidDevice(const QString& udi);

private:

    void    fillSolidMenus();
    bool    checkSolidCamera(const Solid::Device& cameraDevice);
    QString labelForSolidCamera(const Solid::Device& cameraDevice);
    void    openSolidCamera(const QString& udi, const QString& label = QString());
    void    openSolidUsmDevice(const QString& udi, const QString& label = QString());

private Q_SLOTS:

    void slotOpenSolidCamera(QAction*);
    void slotOpenSolidUsmDevice(QAction*);
    void slotOpenSolidDevice(const QString& udi);
    void slotSolidSetupDone(Solid::ErrorType errorType, QVariant errorData, const QString& udi);
    void slotSolidDeviceChanged(const QString& udi);

// -- Internal private container --------------------------------------------------------------------

private:

    class Private;
    Private* const d;

    static DigikamApp* m_instance;
};

} // namespace Digikam

#endif // DIGIKAM_APP_H
