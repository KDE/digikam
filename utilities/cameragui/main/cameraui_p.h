/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-16
 * Description : Camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QLabel>
#include <QString>

// KDE includes

#include <kcombobox.h>
#include <khelpmenu.h>
#include <kio/previewjob.h>
#include <kselectaction.h>
#include <ktoggleaction.h>
#include <kurl.h>

// LibKDcraw includes

#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "cameracontroller.h"
#include "cameraiconview.h"
#include "daboutdata.h"
#include "dlogoaction.h"
#include "dzoombar.h"
#include "freespacewidget.h"
#include "imagepropertiessidebarcamgui.h"
#include "ddatetimeedit.h"
#include "renamecustomizer.h"
#include "sidebar.h"
#include "statusnavigatebar.h"
#include "statusprogressbar.h"
#include "templateselector.h"

using namespace KDcrawIface;

namespace Digikam
{

class CameraUI::CameraUIPriv
{
public:

    enum DateFormatOptions
    {
        IsoDateFormat=0,
        TextDateFormat,
        LocalDateFormat
    };

    CameraUIPriv() :
        deleteAfter(false),
        busy(false),
        closed(false),
        fullScreen(false),
        removeFullScreenButton(false),
        fullScreenHideToolBar(false),
        autoRotateCheck(0),
        autoAlbumDateCheck(0),
        autoAlbumExtCheck(0),
        fixDateTimeCheck(0),
        convertJpegCheck(0),
        formatLabel(0),
        folderDateLabel(0),
        refreshIconViewTimer(0),
        downloadMenu(0),
        deleteMenu(0),
        imageMenu(0),
        cameraCancelAction(0),
        cameraCaptureAction(0),
        cameraInfoAction(0),
        decreaseThumbsAction(0),
        deleteAllAction(0),
        deleteSelectedAction(0),
        downloadAllAction(0),
        downloadDelAllAction(0),
        downloadDelSelectedAction(0),
        downloadSelectedAction(0),
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
        losslessFormat(0),
        folderDateFormat(0),
        helpMenu(0),
        dateTimeEdit(0),
        kdeJob(0),
        advBox(0),
        splitter(0),
        controller(0),
        historyUpdater(0),
        view(0),
        renameCustomizer(0),
        anim(0),
        templateSelector(0),
        rightSideBar(0),
        zoomBar(0),
        statusProgressBar(0),
        statusNavigateBar(0),
        albumLibraryFreeSpace(0),
        cameraFreeSpace(0),
        historyView(0),
        about(0)
    {
    }

    static const QString          configGroupName;
    static const QString          configUseMetadataDateEntry;

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

    QCheckBox*                    autoRotateCheck;
    QCheckBox*                    autoAlbumDateCheck;
    QCheckBox*                    autoAlbumExtCheck;
    QCheckBox*                    fixDateTimeCheck;
    QCheckBox*                    convertJpegCheck;

    QLabel*                       formatLabel;
    QLabel*                       folderDateLabel;

    QTimer*                       refreshIconViewTimer;

    KMenu*                        downloadMenu;
    KMenu*                        deleteMenu;
    KMenu*                        imageMenu;

    KUrl::List                    kdeTodo;

    KAction*                      cameraCancelAction;
    KAction*                      cameraCaptureAction;
    KAction*                      cameraInfoAction;
    KAction*                      decreaseThumbsAction;
    KAction*                      deleteAllAction;
    KAction*                      deleteSelectedAction;
    KAction*                      downloadAllAction;
    KAction*                      downloadDelAllAction;
    KAction*                      downloadDelSelectedAction;
    KAction*                      downloadSelectedAction;
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

    KComboBox*                    losslessFormat;
    KComboBox*                    folderDateFormat;

    KUrl                          lastDestURL;

    KHelpMenu*                    helpMenu;

    DDateTimeEdit*                dateTimeEdit;

    KIO::PreviewJob*              kdeJob;

    RExpanderBox*                 advBox;

    SidebarSplitter*              splitter;

    CameraController*             controller;
    CameraHistoryUpdater*         historyUpdater;

    CameraIconView*               view;

    RenameCustomizer*             renameCustomizer;

    DLogoAction*                  anim;

    TemplateSelector*             templateSelector;

    ImagePropertiesSideBarCamGui* rightSideBar;

    DZoomBar*                     zoomBar;
    StatusProgressBar*            statusProgressBar;
    StatusNavigateBar*            statusNavigateBar;

    FreeSpaceWidget*              albumLibraryFreeSpace;
    FreeSpaceWidget*              cameraFreeSpace;

    DHistoryView*                 historyView;

    DAboutData*                   about;
};

const QString CameraUI::CameraUIPriv::configGroupName("Camera Interface Settings");
const QString CameraUI::CameraUIPriv::configUseMetadataDateEntry("UseThemeBackgroundColor");

}  // namespace Digikam

#endif /* CAMERAUIPRIVATE_H */
