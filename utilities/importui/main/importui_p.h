/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-16
 * Description : Camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMPORTUI_P_H
#define IMPORTUI_P_H

// Qt includes

#include <QCheckBox>
#include <QDateTime>
#include <QString>

// KDE includes

#include <khelpmenu.h>
#include <kselectaction.h>
#include <ktoggleaction.h>
#include <KActionMenu>
#include <kurl.h>

// Libkdcraw includes

#include <rexpanderbox.h>

// Local includes

#include "config-digikam.h"
#include "camerathumbsctrl.h"
#include "cameracontroller.h"
#include "filtercombo.h"
#include "dzoombar.h"
#include "freespacewidget.h"
#include "imagepropertiessidebarcamgui.h"
#include "renamecustomizer.h"
#include "albumcustomizer.h"
#include "advancedsettings.h"
#include "scriptingsettings.h"
#include "sidebar.h"
#include "filterstatusbar.h"
#include "statusprogressbar.h"
#include "importview.h"

using namespace KDcrawIface;

class KMessageWidget;

namespace Digikam
{

class ImportUI::Private
{
public:

    Private() :
        deleteAfter(false),
        busy(false),
        closed(false),
        downloadMenu(0),
        deleteMenu(0),
        imageMenu(0),
        cameraCancelAction(0),
        cameraCaptureAction(0),
        cameraInfoAction(0),
        increaseThumbsAction(0),
        decreaseThumbsAction(0),
        zoomFitToWindowAction(0),
        zoomTo100percents(0),
        deleteAction(0),
        deleteNewAction(0),
        deleteAllAction(0),
        deleteSelectedAction(0),
        downloadAction(0),
        downloadNewAction(0),
        downloadAllAction(0),
        downloadSelectedAction(0),
        downloadDelNewAction(0),
        downloadDelAllAction(0),
        downloadDelSelectedAction(0),
        lockAction(0),
        selectAllAction(0),
        selectInvertAction(0),
        selectLockedItemsAction(0),
        selectNewItemsAction(0),
        selectNoneAction(0),
        uploadAction(0),
        markAsDownloadedAction(0),
        resumeAction(0),
        pauseAction(0),
        connectAction(0),
        itemSortAction(0),
        itemSortOrderAction(0),
        itemsGroupAction(0),
        showMenuBarAction(0),
        showPreferencesAction(0),
        showLogAction(0),
        showBarAction(0),
        imageViewSelectionAction(0),
        iconViewAction(0),
        camItemPreviewAction(0),
#ifdef HAVE_KGEOMAP
        mapViewAction(0),
#endif // HAVE_KGEOMAP
        viewCMViewAction(0),
        cameraActions(0),
        helpMenu(0),
        advBox(0),
        splitter(0),
        camThumbsCtrl(0),
        controller(0),
        historyUpdater(0),
        view(0),
        renameCustomizer(0),
        albumCustomizer(0),
        advancedSettings(0),
        scriptingSettings(0),
        filterStatusBar(0),
        rightSideBar(0),
        zoomBar(0),
        statusProgressBar(0),
        albumLibraryFreeSpace(0),
        cameraFreeSpace(0),
        progressTimer(0),
        progressValue(0),
        historyView(0),
        filterComboBox(0),
        errorWidget(0)
    {
    }

    static const QString          configGroupName;
    static const QString          configUseFileMetadata;
    static const QString          configUseDefaultTargetAlbum;
    static const QString          configLastTargetAlbum;
    static const QString          configDefaultTargetAlbumId;
    static const QString          importFiltersConfigGroupName;

    bool                          deleteAfter;
    bool                          busy;
    bool                          closed;

    QString                       cameraTitle;

    QStringList                   autoRotateItemsList;
    QStringList                   currentlyDeleting;
    QSet<QString>                 foldersToScan;
    CamItemInfoList               filesToBeAdded;

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
    KActionMenu*                  deleteAction;
    QAction*                      deleteNewAction;
    QAction*                      deleteAllAction;
    QAction*                      deleteSelectedAction;
    KActionMenu*                  downloadAction;
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
    KToggleAction*                showMenuBarAction;
    QAction*                      showPreferencesAction;
    KToggleAction*                showLogAction;
    KToggleAction*                showBarAction;
    KSelectAction*                imageViewSelectionAction;
    KToggleAction*                iconViewAction;
    KToggleAction*                camItemPreviewAction;
#ifdef HAVE_KGEOMAP
    KToggleAction*                mapViewAction;
#endif // HAVE_KGEOMAP
    KToggleAction*                viewCMViewAction;

    QActionGroup*                 cameraActions;

    KUrl                          lastDestURL;

    KHelpMenu*                    helpMenu;

    RExpanderBox*                 advBox;

    SidebarSplitter*              splitter;

    CameraThumbsCtrl*             camThumbsCtrl;
    CameraController*             controller;
    CameraHistoryUpdater*         historyUpdater;

    ImportView*                   view;

    RenameCustomizer*             renameCustomizer;
    AlbumCustomizer*              albumCustomizer;
    AdvancedSettings*             advancedSettings;
    ScriptingSettings*            scriptingSettings;

    FilterStatusBar*              filterStatusBar;
    ImagePropertiesSideBarCamGui* rightSideBar;

    DZoomBar*                     zoomBar;
    StatusProgressBar*            statusProgressBar;

    FreeSpaceWidget*              albumLibraryFreeSpace;
    FreeSpaceWidget*              cameraFreeSpace;

    QTimer*                       progressTimer;

    float                         progressValue;

    DHistoryView*                 historyView;
    FilterComboBox*               filterComboBox;

    CHUpdateItemMap               map;

    KMessageWidget*               errorWidget;
};

const QString ImportUI::Private::configGroupName("Camera Settings");
const QString ImportUI::Private::configUseFileMetadata("UseFileMetadata");
const QString ImportUI::Private::configUseDefaultTargetAlbum("UseDefaultTargetAlbum");
const QString ImportUI::Private::configLastTargetAlbum("LastTargetAlbum");
const QString ImportUI::Private::configDefaultTargetAlbumId("DefaultTargetAlbumId");
const QString ImportUI::Private::importFiltersConfigGroupName("Import Filters");

}  // namespace Digikam

#endif // IMPORTUI_P_H
