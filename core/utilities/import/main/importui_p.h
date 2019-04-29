/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-16
 * Description : Camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IMPORTUI_PRIVATE_H
#define DIGIKAM_IMPORTUI_PRIVATE_H

// Qt includes

#include <QCheckBox>
#include <QDateTime>
#include <QString>
#include <QUrl>

// KDE includes

#include <kselectaction.h>

// Local includes

#include "dexpanderbox.h"
#include "digikam_config.h"
#include "camerathumbsctrl.h"
#include "cameracontroller.h"
#include "filtercombo.h"
#include "dzoombar.h"
#include "freespacewidget.h"
#include "importitempropertiessidebar.h"
#include "renamecustomizer.h"
#include "albumcustomizer.h"
#include "advancedsettings.h"
#include "dngconvertsettings.h"
#include "scriptingsettings.h"
#include "sidebar.h"
#include "filterstatusbar.h"
#include "statusprogressbar.h"
#include "importview.h"
#include "dnotificationwidget.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImportUI::Private
{
public:

    Private()
      : deleteAfter(false),
        busy(false),
        closed(false),
        downloadMenu(nullptr),
        deleteMenu(nullptr),
        imageMenu(nullptr),
        cameraCancelAction(nullptr),
        cameraCaptureAction(nullptr),
        cameraInfoAction(nullptr),
        increaseThumbsAction(nullptr),
        decreaseThumbsAction(nullptr),
        zoomFitToWindowAction(nullptr),
        zoomTo100percents(nullptr),
        deleteAction(nullptr),
        deleteNewAction(nullptr),
        deleteAllAction(nullptr),
        deleteSelectedAction(nullptr),
        downloadAction(nullptr),
        downloadNewAction(nullptr),
        downloadAllAction(nullptr),
        downloadSelectedAction(nullptr),
        downloadDelNewAction(nullptr),
        downloadDelAllAction(nullptr),
        downloadDelSelectedAction(nullptr),
        lockAction(nullptr),
        selectAllAction(nullptr),
        selectInvertAction(nullptr),
        selectLockedItemsAction(nullptr),
        selectNewItemsAction(nullptr),
        selectNoneAction(nullptr),
        uploadAction(nullptr),
        markAsDownloadedAction(nullptr),
        resumeAction(nullptr),
        pauseAction(nullptr),
        connectAction(nullptr),
        itemSortAction(nullptr),
        itemSortOrderAction(nullptr),
        itemsGroupAction(nullptr),
        showPreferencesAction(nullptr),
        showLogAction(nullptr),
        showBarAction(nullptr),
        imageViewSelectionAction(nullptr),
        iconViewAction(nullptr),
        camItemPreviewAction(nullptr),
#ifdef HAVE_MARBLE
        mapViewAction(nullptr),
#endif // HAVE_MARBLE
        viewCMViewAction(nullptr),
        cameraActions(nullptr),
        advBox(nullptr),
        splitter(nullptr),
        camThumbsCtrl(nullptr),
        controller(nullptr),
        //historyUpdater(0),
        view(nullptr),
        renameCustomizer(nullptr),
        albumCustomizer(nullptr),
        advancedSettings(nullptr),
        dngConvertSettings(nullptr),
        scriptingSettings(nullptr),
        filterStatusBar(nullptr),
        rightSideBar(nullptr),
        zoomBar(nullptr),
        statusProgressBar(nullptr),
        albumLibraryFreeSpace(nullptr),
        cameraFreeSpace(nullptr),
        progressTimer(nullptr),
        progressValue(0),
        historyView(nullptr),
        filterComboBox(nullptr),
        errorWidget(nullptr)
    {
    }

    static const QString          configGroupName;
    static const QString          configUseFileMetadata;
    static const QString          configUseDefaultTargetAlbum;
    static const QString          configLastTargetAlbum;
    static const QString          configDefaultTargetAlbumId;
    static const QString          configFileSaveConflictRule;
    static const QString          importFiltersConfigGroupName;

    bool                          deleteAfter;
    bool                          busy;
    bool                          closed;

    QString                       cameraTitle;

    QStringList                   autoRotateItemsList;
    QStringList                   currentlyDeleting;
    QSet<QString>                 foldersToScan;

    QMenu*                        downloadMenu;
    QMenu*                        deleteMenu;
    QMenu*                        imageMenu;

    QAction*                      cameraCancelAction;
    QAction*                      cameraCaptureAction;
    QAction*                      cameraInfoAction;
    QAction*                      increaseThumbsAction;
    QAction*                      decreaseThumbsAction;
    QAction*                      zoomFitToWindowAction;
    QAction*                      zoomTo100percents;
    QMenu*                        deleteAction;
    QAction*                      deleteNewAction;
    QAction*                      deleteAllAction;
    QAction*                      deleteSelectedAction;
    QMenu*                        downloadAction;
    QAction*                      downloadNewAction;
    QAction*                      downloadAllAction;
    QAction*                      downloadSelectedAction;
    QAction*                      downloadDelNewAction;
    QAction*                      downloadDelAllAction;
    QAction*                      downloadDelSelectedAction;
    QAction*                      lockAction;
    QAction*                      selectAllAction;
    QAction*                      selectInvertAction;
    QAction*                      selectLockedItemsAction;
    QAction*                      selectNewItemsAction;
    QAction*                      selectNoneAction;
    QAction*                      uploadAction;
    QAction*                      markAsDownloadedAction;
    QAction*                      resumeAction;
    QAction*                      pauseAction;
    QAction*                      connectAction;
    KSelectAction*                itemSortAction;
    KSelectAction*                itemSortOrderAction;
    KSelectAction*                itemsGroupAction;
    QAction*                      showPreferencesAction;
    QAction*                      showLogAction;
    QAction*                      showBarAction;
    KSelectAction*                imageViewSelectionAction;
    QAction*                      iconViewAction;
    QAction*                      camItemPreviewAction;
#ifdef HAVE_MARBLE
    QAction*                      mapViewAction;
#endif // HAVE_MARBLE
    QAction*                      viewCMViewAction;

    QActionGroup*                 cameraActions;

    QUrl                          lastDestURL;

    DExpanderBox*                 advBox;

    SidebarSplitter*              splitter;

    CameraThumbsCtrl*             camThumbsCtrl;
    CameraController*             controller;
    //CameraHistoryUpdater*         historyUpdater;

    ImportView*                   view;

    RenameCustomizer*             renameCustomizer;
    AlbumCustomizer*              albumCustomizer;
    AdvancedSettings*             advancedSettings;
    DNGConvertSettings*           dngConvertSettings;
    ScriptingSettings*            scriptingSettings;

    FilterStatusBar*              filterStatusBar;
    ImportItemPropertiesSideBarImport* rightSideBar;

    DZoomBar*                     zoomBar;
    StatusProgressBar*            statusProgressBar;

    FreeSpaceWidget*              albumLibraryFreeSpace;
    FreeSpaceWidget*              cameraFreeSpace;

    QTimer*                       progressTimer;

    float                         progressValue;

    DHistoryView*                 historyView;
    FilterComboBox*               filterComboBox;

    //CHUpdateItemMap               map;

    DNotificationWidget*          errorWidget;
};

const QString ImportUI::Private::configGroupName(QLatin1String("Camera Settings"));
const QString ImportUI::Private::configUseFileMetadata(QLatin1String("UseFileMetadata"));
const QString ImportUI::Private::configUseDefaultTargetAlbum(QLatin1String("UseDefaultTargetAlbum"));
const QString ImportUI::Private::configLastTargetAlbum(QLatin1String("LastTargetAlbum"));
const QString ImportUI::Private::configDefaultTargetAlbumId(QLatin1String("DefaultTargetAlbumId"));
const QString ImportUI::Private::configFileSaveConflictRule(QLatin1String("FileSaveConflictRule"));
const QString ImportUI::Private::importFiltersConfigGroupName(QLatin1String("Import Filters"));

} // namespace Digikam

#endif // DIGIKAM_IMPORTUI_PRIVATE_H
