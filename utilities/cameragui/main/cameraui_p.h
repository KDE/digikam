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

#ifndef CAMERAUIPRIVATE_H
#define CAMERAUIPRIVATE_H

// Qt includes

#include <QCheckBox>
#include <QDateTime>
#include <QString>

// KDE includes

#include <khelpmenu.h>
#include <kselectaction.h>
#include <ktoggleaction.h>
#include <kurl.h>

// LibKDcraw includes

#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "camerathumbsctrl.h"
#include "cameracontroller.h"
#include "cameraiconview.h"
#include "filtercombo.h"
#include "daboutdata.h"
#include "dlogoaction.h"
#include "dzoombar.h"
#include "freespacewidget.h"
#include "imagepropertiessidebarcamgui.h"
#include "renamecustomizer.h"
#include "albumcustomizer.h"
#include "advancedsettings.h"
#include "sidebar.h"
#include "statusnavigatebar.h"
#include "statusprogressbar.h"

using namespace KDcrawIface;

namespace Digikam
{

class CameraUI::CameraUIPriv
{
public:

    CameraUIPriv() :
        deleteAfter(false),
        busy(false),
        closed(false),
        fullScreen(false),
        removeFullScreenButton(false),
        fullScreenHideToolBar(false),
        refreshIconViewTimer(0),
        downloadMenu(0),
        deleteMenu(0),
        imageMenu(0),
        cameraCancelAction(0),
        cameraCaptureAction(0),
        cameraInfoAction(0),
        decreaseThumbsAction(0),
        deleteNewAction(0),
        deleteAllAction(0),
        deleteSelectedAction(0),
        downloadNewAction(0),
        downloadAllAction(0),
        downloadSelectedAction(0),
        downloadDelNewAction(0),
        downloadDelAllAction(0),
        downloadDelSelectedAction(0),
        fullScreenAction(0),
        imageViewAction(0),
        increaseThumbsAction(0),
        libsInfoAction(0),
        dbStatAction(0),
        lockAction(0),
        selectAllAction(0),
        selectInvertAction(0),
        selectLockedItemsAction(0),
        selectNewItemsAction(0),
        selectNoneAction(0),
        uploadAction(0),
        markAsDownloadedAction(0),
        lastPhotoFirstAction(0),
        showMenuBarAction(0),
        showLogAction(0),
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
        anim(0),
        rightSideBar(0),
        zoomBar(0),
        statusProgressBar(0),
        statusNavigateBar(0),
        albumLibraryFreeSpace(0),
        cameraFreeSpace(0),
        historyView(0),
        filterComboBox(0),
        about(0)
    {
    }

    static const QString          configGroupName;
    static const QString          configUseMetadataDateEntry;
    static const QString          configUseDefaultTargetAlbum;
    static const QString          configDefaultTargetAlbumId;
    static const QString          importFiltersConfigGroupName;

    bool                          deleteAfter;
    bool                          busy;
    bool                          closed;
    bool                          fullScreen;
    bool                          removeFullScreenButton;
    bool                          fullScreenHideToolBar;

    QString                       cameraTitle;

    QStringList                   currentlyDeleting;
    QSet<QString>                 foldersToScan;
    CamItemInfoList               filesToBeAdded;

    QTimer*                       refreshIconViewTimer;

    KMenu*                        downloadMenu;
    KMenu*                        deleteMenu;
    KMenu*                        imageMenu;

    KAction*                      cameraCancelAction;
    KAction*                      cameraCaptureAction;
    KAction*                      cameraInfoAction;
    KAction*                      decreaseThumbsAction;
    KAction*                      deleteNewAction;
    KAction*                      deleteAllAction;
    KAction*                      deleteSelectedAction;
    KAction*                      downloadNewAction;
    KAction*                      downloadAllAction;
    KAction*                      downloadSelectedAction;
    KAction*                      downloadDelNewAction;
    KAction*                      downloadDelAllAction;
    KAction*                      downloadDelSelectedAction;
    KAction*                      fullScreenAction;
    KAction*                      imageViewAction;
    KAction*                      increaseThumbsAction;
    KAction*                      libsInfoAction;
    KAction*                      dbStatAction;
    KAction*                      lockAction;
    KAction*                      selectAllAction;
    KAction*                      selectInvertAction;
    KAction*                      selectLockedItemsAction;
    KAction*                      selectNewItemsAction;
    KAction*                      selectNoneAction;
    KAction*                      uploadAction;
    KAction*                      markAsDownloadedAction;
    KToggleAction*                lastPhotoFirstAction;
    KToggleAction*                showMenuBarAction;
    KToggleAction*                showLogAction;

    KUrl                          lastDestURL;

    KHelpMenu*                    helpMenu;

    RExpanderBox*                 advBox;

    SidebarSplitter*              splitter;

    CameraThumbsCtrl*             camThumbsCtrl;
    CameraController*             controller;
    CameraHistoryUpdater*         historyUpdater;

    CameraIconView*               view;

    RenameCustomizer*             renameCustomizer;
    AlbumCustomizer*              albumCustomizer;
    AdvancedSettings*             advancedSettings;

    DLogoAction*                  anim;

    ImagePropertiesSideBarCamGui* rightSideBar;

    DZoomBar*                     zoomBar;
    StatusProgressBar*            statusProgressBar;
    StatusNavigateBar*            statusNavigateBar;

    FreeSpaceWidget*              albumLibraryFreeSpace;
    FreeSpaceWidget*              cameraFreeSpace;

    DHistoryView*                 historyView;
    FilterComboBox*               filterComboBox;

    CHUpdateItemMap               map;

    DAboutData*                   about;
};

const QString CameraUI::CameraUIPriv::configGroupName("Camera Settings");
const QString CameraUI::CameraUIPriv::configUseMetadataDateEntry("UseThemeBackgroundColor");
const QString CameraUI::CameraUIPriv::configUseDefaultTargetAlbum("UseDefaultTargetAlbum");
const QString CameraUI::CameraUIPriv::configDefaultTargetAlbumId("DefaultTargetAlbumId");
const QString CameraUI::CameraUIPriv::importFiltersConfigGroupName("Import Filters");

}  // namespace Digikam

#endif /* CAMERAUIPRIVATE_H */
