/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C)      2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2002-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef DIGIKAMAPP_H
#define DIGIKAMAPP_H

// Qt includes

#include <QList>
#include <QAction>
#include <QString>

// KDE includes

#include <kurl.h>
#include <kxmlguiwindow.h>
#include <kio/global.h>
#include <kio/netaccess.h>
#include <solid/solidnamespace.h>

// Local includes

#include <config-digikam.h>
#include "digikam_export.h"

class KAction;

namespace Solid
{
class Device;
}

namespace Digikam
{

class DigikamView;
class FaceScanSettings;
class FilterStatusBar;
class ImageInfo;
class ImageInfoList;
class CameraType;

class DigikamApp : public KXmlGuiWindow
{
    Q_OBJECT

public:

    DigikamApp();
    ~DigikamApp();

    virtual void show();
    void restoreSession();

    static DigikamApp* instance();

    // KIPI Actions collections access.
    const QList<QAction*>& menuImageActions();
    const QList<QAction*>& menuBatchActions();
    const QList<QAction*>& menuAlbumActions();
    const QList<QAction*>& menuImportActions();
    const QList<QAction*>& menuExportActions();

    void autoDetect();
    void downloadFrom(const QString& cameraGuiPath);
    void downloadFromUdi(const QString& udi);
    QString currentDatabaseParameters();
    void enableZoomPlusAction(bool val);
    void enableZoomMinusAction(bool val);
    void enableAlbumBackwardHistory(bool enable);
    void enableAlbumForwardHistory(bool enable);

    void startInternalDatabase();
    void stopInternalDatabase();

    DigikamView* view() const;

Q_SIGNALS:

    void signalEscapePressed();
    void signalNextItem();
    void signalPrevItem();
    void signalFirstItem();
    void signalLastItem();
    void signalCutAlbumItemsSelection();
    void signalCopyAlbumItemsSelection();
    void signalPasteAlbumItemsSelection();
    void signalCancelButtonPressed();

    void signalWindowHasMoved();

    void queuedOpenCameraUiFromPath(const QString& path);
    void queuedOpenSolidDevice(const QString& udi);

public Q_SLOTS:

    /** Progress info: Connect a signal directly to these slots. */
    void enterProgress(const QString& message);
    void enterCancellableProgress(const QString& message);
    void progressValue(float progress);
    void finishProgress();

    /** Progress info: Provide a QObject as id */
    void enterProgress(QObject* object, const QString& message);
    void enterCancellableProgress(QObject* object, const QString& message);
    void progressValue(QObject* object, float progress);
    void finishProgress(QObject* object);

    /** Progress info: Provide an integer id */
    void enterProgress(quintptr id, const QString& message);
    void enterProgress(quintptr id, const QString& message, bool canCancel);
    void progressValue(quintptr id, float progress);
    void finishProgress(quintptr id);

protected:

    bool queryClose();
    void moveEvent(QMoveEvent* e);

private:

    bool setup();
    bool setupICC();
    void setupView();
    void setupStatusBar();
    void setupActions();
    void setupAccelerators();
    void setupExifOrientationActions();
    void loadPlugins();
    void loadCameras();
    void populateThemes();
    void preloadWindows();
    void fillSolidMenus();
    bool checkSolidCamera(const Solid::Device& cameraDevice);
    QString labelForSolidCamera(const Solid::Device& cameraDevice);
    void openSolidCamera(const QString& udi, const QString& label = QString());
    void openSolidUsmDevice(const QString& udi, const QString& label = QString());
    void runFingerPrintsGenerator(bool rebuildAll);
    void runFaceScanner(const FaceScanSettings& rebuildAll);
    void runThumbnailsGenerator(bool rebuildAll);
    void updateCameraMenu();
    void updateQuickImportAction();
    void initGui();
    void showToolBars(bool show=true);
    void showThumbBar(bool show=true);

private Q_SLOTS:

    void slotAlbumSelected(bool val);
    void slotTagSelected(bool val);
    void slotImageSelected(const ImageInfoList&, bool, bool, const ImageInfoList&);
    void slotSelectionChanged(int selectionCount);
    void slotExit();
    void slotShowTip();
    void slotShowKipiHelp();
    void slotComponentsInfo();
    void slotDBStat();

    void slotRecurseAlbums(bool);
    void slotRecurseTags(bool);

    void slotAboutToShowForwardMenu();
    void slotAboutToShowBackwardMenu();

    void slotSetup();
    void slotSetupCamera();
    void slotSetupChanged();
    void slotSetCheckedExifOrientationAction(const ImageInfo& info);
    void slotResetExifOrientationActions();

    void slotKipiPluginPlug();

    void slotOpenSolidCamera(QAction*);
    void slotOpenManualCamera(QAction*);
    void slotOpenSolidUsmDevice(QAction*);
    void slotOpenSolidDevice(const QString& udi);
    void slotOpenCameraUiFromPath(const QString& path);
    void slotSolidSetupDone(Solid::ErrorType errorType, QVariant errorData, const QString& udi);
    void slotSolidDeviceChanged(const QString& udi);
    void slotCameraAdded(CameraType*);
    void slotCameraRemoved(KAction*);
    void slotCameraAutoDetect();
    void downloadImages(const QString& folder);
    void cameraAutoDetect();

    void slotToggleShowBar();
    void slotShowMenuBar();
    void slotEditKeys();
    void slotConfToolbars();
    void slotConfNotifications();
    void slotToggleFullScreen();

    void slotDatabaseMigration();
    void slotDatabaseRescan();
    void slotRebuildAlbumThumbnails();
    void slotRebuildThumbnails();
    void slotWriteMetadataToAllImages();
    void slotGenerateFingerPrintsFirstTime();
    void slotRebuildFingerPrints();
    void slotRebuildFingerPrintsDone();

    void slotScanForFaces();
    void slotScanForFacesDone();

    void slotZoomSliderChanged(int);
    void slotThumbSizeChanged(int);
    void slotZoomChanged(double);

    void slotSwitchedToPreview();
    void slotSwitchedToIconView();
    void slotSwitchedToMapView();

    void slotImportAddImages();
    void slotImportAddFolders();
    void slotDIOResult(KJob*);
    void slotThemeChanged();

#ifdef USE_SCRIPT_IFACE
    void slotScriptConsole();
#endif

private:

    class DigikamAppPriv;
    DigikamAppPriv* const d;

    static DigikamApp* m_instance;
};

}  // namespace Digikam

#endif  // DIGIKAMAPP_H
