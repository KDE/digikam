/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-31-01
 * Description : main digiKam interface implementation
 *
 * Copyright (C) 2007-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#ifndef DIGIKAMAPPPRIVATE_H
#define DIGIKAMAPPPRIVATE_H

// Qt includes

#include <QByteArray>
#include <QEventLoop>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QSignalMapper>
#include <QString>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kconfig.h>
#include <kaction.h>
#include <kselectaction.h>
#include <kmenu.h>
#include <kstatusbar.h>
#include <kcombobox.h>
#include <ksqueezedtextlabel.h>

// Local includes

#include "config-digikam.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "cameralist.h"
#include "splashscreen.h"
#include "dzoombar.h"
#include "digikamview.h"

class KToolBarPopupAction;
class KToggleAction;
class KActionMenu;

namespace Digikam
{

class DCOPIface;
class ImportUI;
class SearchTextBar;
class FilterStatusBar;
class TagsActionMngr;

class ProgressEntry
{
public:

    ProgressEntry()
        : progress(0), canCancel(false)
    {
    }

    QString message;
    float   progress;
    bool    canCancel;
};

class DigikamApp::Private
{
public:

    Private() :
        autoShowZoomToolTip(false),
        validIccPath(true),
        cameraMenu(0),
        usbMediaMenu(0),
        cardReaderMenu(0),
        quickImportMenu(0),
        config(0),
        newAction(0),
        moveSelectionToAlbumAction(0),
        deleteAction(0),
        renameAction(0),
        imageDeletePermanentlyAction(0),
        imageDeletePermanentlyDirectlyAction(0),
        imageTrashDirectlyAction(0),
        backwardActionMenu(0),
        forwardActionMenu(0),
        addImagesAction(0),
        propsEditAction(0),
        addFoldersAction(0),
        openInFileManagerAction(0),
        openInTerminalAction(0),
        refreshAction(0),
        writeAlbumMetadataAction(0),
        readAlbumMetadataAction(0),
        browseTagsAction(0),
        openTagMngrAction(0),
        newTagAction(0),
        deleteTagAction(0),
        editTagAction(0),
        assignTagAction(0),
        imageViewSelectionAction(0),
        imagePreviewAction(0),
#ifdef HAVE_KGEOMAP
        imageMapViewAction(0),
#endif // HAVE_KGEOMAP
        imageTableViewAction(0),
        imageIconViewAction(0),
        imageLightTableAction(0),
        imageAddLightTableAction(0),
        imageAddCurrentQueueAction(0),
        imageAddNewQueueAction(0),
        imageViewAction(0),
        imageWriteMetadataAction(0),
        imageReadMetadataAction(0),
        imageFindSimilarAction(0),
        imageSetExifOrientation1Action(0),
        imageSetExifOrientation2Action(0),
        imageSetExifOrientation3Action(0),
        imageSetExifOrientation4Action(0),
        imageSetExifOrientation5Action(0),
        imageSetExifOrientation6Action(0),
        imageSetExifOrientation7Action(0),
        imageSetExifOrientation8Action(0),
        imageRenameAction(0),
        imageRotateActionMenu(0),
        imageFlipActionMenu(0),
        imageAutoExifActionMenu(0),
        imageDeleteAction(0),
        imageExifOrientationActionMenu(0),
        cutItemsAction(0),
        copyItemsAction(0),
        pasteItemsAction(0),
        selectAllAction(0),
        selectNoneAction(0),
        selectInvertAction(0),
        zoomPlusAction(0),
        zoomMinusAction(0),
        zoomFitToWindowAction(0),
        zoomTo100percents(0),
        imageSortAction(0),
        imageSortOrderAction(0),
        imageGroupAction(0),
        imageGroupSortOrderAction(0),
        albumSortAction(0),
        recurseAlbumsAction(0),
        recurseTagsAction(0),
        showBarAction(0),
        showMenuBarAction(0),
        viewCMViewAction(0),
        slideShowAction(0),
        slideShowAllAction(0),
        slideShowSelectionAction(0),
        slideShowRecursiveAction(0),
        bqmAction(0),
        maintenanceAction(0),
        slideShowQmlAction(0),
        qualityAction(0),
        kipiHelpAction(0),
        addCameraSeparatorAction(0),
        quitAction(0),
        tipAction(0),
        backwardSignalMapper(0),
        forwardSignalMapper(0),
        manualCameraActionGroup(0),
        solidCameraActionGroup(0),
        solidUsmActionGroup(0),
        exifOrientationActionGroup(0),
        eventLoop(0),
        filterStatusBar(0),
        splashScreen(0),
        dcopIface(0),
        view(0),
        cameraList(0),
        tagsActionManager(0),
        zoomBar(0),
        statusLabel(0),
        mapViewSwitcher(0),
        modelCollection(0)
    {
    }

    bool                                autoShowZoomToolTip;
    bool                                validIccPath;

    KActionMenu*                        cameraMenu;
    KActionMenu*                        usbMediaMenu;
    KActionMenu*                        cardReaderMenu;
    KActionMenu*                        quickImportMenu;
    QHash<QString, QDateTime>           cameraAppearanceTimes;

    KSharedConfig::Ptr                  config;

    // Album Actions
    KAction*                            newAction;
    KAction*                            moveSelectionToAlbumAction;
    KAction*                            deleteAction;
    KAction*                            renameAction;
    KAction*                            imageDeletePermanentlyAction;
    KAction*                            imageDeletePermanentlyDirectlyAction;
    KAction*                            imageTrashDirectlyAction;
    KToolBarPopupAction*                backwardActionMenu;
    KToolBarPopupAction*                forwardActionMenu;

    KAction*                            addImagesAction;
    KAction*                            propsEditAction;
    KAction*                            addFoldersAction;
    KAction*                            openInFileManagerAction;
    KAction*                            openInTerminalAction;
    KAction*                            refreshAction;
    KAction*                            writeAlbumMetadataAction;
    KAction*                            readAlbumMetadataAction;

    // Tag Actions
    KAction*                            browseTagsAction;
    KAction*                            openTagMngrAction;
    KAction*                            newTagAction;
    KAction*                            deleteTagAction;
    KAction*                            editTagAction;
    KAction*                            assignTagAction;

    // Image Actions
    KSelectAction*                      imageViewSelectionAction;
    KToggleAction*                      imagePreviewAction;
#ifdef HAVE_KGEOMAP
    KToggleAction*                      imageMapViewAction;
#endif // HAVE_KGEOMAP
    KToggleAction*                      imageTableViewAction;
    KToggleAction*                      imageIconViewAction;
    KAction*                            imageLightTableAction;
    KAction*                            imageAddLightTableAction;
    KAction*                            imageAddCurrentQueueAction;
    KAction*                            imageAddNewQueueAction;
    KAction*                            imageViewAction;
    KAction*                            imageWriteMetadataAction;
    KAction*                            imageReadMetadataAction;
    KAction*                            imageFindSimilarAction;
    KToggleAction*                      imageSetExifOrientation1Action;
    KToggleAction*                      imageSetExifOrientation2Action;
    KToggleAction*                      imageSetExifOrientation3Action;
    KToggleAction*                      imageSetExifOrientation4Action;
    KToggleAction*                      imageSetExifOrientation5Action;
    KToggleAction*                      imageSetExifOrientation6Action;
    KToggleAction*                      imageSetExifOrientation7Action;
    KToggleAction*                      imageSetExifOrientation8Action;
    KAction*                            imageRenameAction;
    KActionMenu*                        imageRotateActionMenu;
    KActionMenu*                        imageFlipActionMenu;
    KAction*                            imageAutoExifActionMenu;
    KAction*                            imageDeleteAction;
    KActionMenu*                        imageExifOrientationActionMenu;

    // Edit Actions
    KAction*                            cutItemsAction;
    KAction*                            copyItemsAction;
    KAction*                            pasteItemsAction;
    KAction*                            selectAllAction;
    KAction*                            selectNoneAction;
    KAction*                            selectInvertAction;

    // View Actions
    KAction*                            zoomPlusAction;
    KAction*                            zoomMinusAction;
    KAction*                            zoomFitToWindowAction;
    KAction*                            zoomTo100percents;
    KSelectAction*                      imageSortAction;
    KSelectAction*                      imageSortOrderAction;
    KSelectAction*                      imageGroupAction;
    KSelectAction*                      imageGroupSortOrderAction;
    KSelectAction*                      albumSortAction;
    KToggleAction*                      recurseAlbumsAction;
    KToggleAction*                      recurseTagsAction;
    KToggleAction*                      showBarAction;
    KToggleAction*                      showMenuBarAction;
    KToggleAction*                      viewCMViewAction;

    // Tools Actions
    KActionMenu*                        slideShowAction;
    KAction*                            slideShowAllAction;
    KAction*                            slideShowSelectionAction;
    KAction*                            slideShowRecursiveAction;
    KAction*                            bqmAction;
    KAction*                            maintenanceAction;
    KAction*                            slideShowQmlAction;
    KAction*                            qualityAction;

    // Application Actions
    KAction*                            kipiHelpAction;
    QAction*                            addCameraSeparatorAction;
    QAction*                            quitAction;
    QAction*                            tipAction;

    QSignalMapper*                      backwardSignalMapper;
    QSignalMapper*                      forwardSignalMapper;
    QActionGroup*                       manualCameraActionGroup;
    QActionGroup*                       solidCameraActionGroup;
    QActionGroup*                       solidUsmActionGroup;
    QActionGroup*                       exifOrientationActionGroup;

    QMap<QString, QPointer<ImportUI> >  cameraUIMap;

    QEventLoop*                         eventLoop;
    QString                             solidErrorMessage;

    FilterStatusBar*                    filterStatusBar;
    SplashScreen*                       splashScreen;
    DCOPIface*                          dcopIface;
    DigikamView*                        view;
    CameraList*                         cameraList;
    TagsActionMngr*                     tagsActionManager;
    DZoomBar*                           zoomBar;
    KSqueezedTextLabel*                 statusLabel;
    QString                             statusBarSelectionText;
    KComboBox*                          mapViewSwitcher;

    DigikamModelCollection*             modelCollection;
};

}  // namespace Digikam

#endif // ALBUMICONVIEW_H
