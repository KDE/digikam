/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_LIGHT_TABLE_WINDOW_PRIVATE_H
#define DIGIKAM_LIGHT_TABLE_WINDOW_PRIVATE_H

// Qt includes

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include <QSplitter>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Local includes

#include "digikam_globals.h"
#include "imagepropertiessidebardb.h"
#include "statusprogressbar.h"
#include "dzoombar.h"
#include "lighttableview.h"
#include "lighttablethumbbar.h"
#include "thumbbardock.h"
#include "drawdecoder.h"
#include "digikam_debug.h"
#include "componentsinfo.h"
#include "digikamapp.h"
#include "thememanager.h"
#include "dimg.h"
#include "dio.h"
#include "dmetadata.h"
#include "dfileoperations.h"
#include "metadatasettings.h"
#include "metadataedit.h"
#include "applicationsettings.h"
#include "albummanager.h"
#include "loadingcacheinterface.h"
#include "deletedialog.h"
#include "iccsettings.h"
#include "imagewindow.h"
#include "imagegps.h"
#include "imagedescedittab.h"
#include "presentationmngr.h"
#include "slideshowbuilder.h"
#include "slideshow.h"
#include "setup.h"
#include "syncjob.h"
#include "lighttablepreview.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "coredbchangesets.h"
#include "collectionscanner.h"
#include "scancontroller.h"
#include "tagsactionmngr.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "dexpanderbox.h"
#include "dbinfoiface.h"
#include "calwizard.h"
#include "expoblendingmanager.h"
#include "mailwizard.h"
#include "advprintwizard.h"
#include "dmediaserverdlg.h"
#include "wsstarter.h"

#ifdef HAVE_MARBLE
#   include "geolocationedit.h"
#endif

#ifdef HAVE_HTMLGALLERY
#   include "htmlwizard.h"
#endif

#ifdef HAVE_PANORAMA
#   include "panomanager.h"
#endif

#ifdef HAVE_MEDIAPLAYER
#   include "vidslidewizard.h"
#endif

namespace Digikam
{

class DAdjustableLabel;

class Q_DECL_HIDDEN LightTableWindow::Private
{

public:

    Private()
      : autoLoadOnRightPanel(true),
        autoSyncPreview(true),
        fromLeftPreview(true),
        setItemLeftAction(0),
        setItemRightAction(0),
        clearListAction(0),
        editItemAction(0),
        removeItemAction(0),
        fileDeleteAction(0),
        fileDeleteFinalAction(0),
        slideShowAction(0),
        leftZoomPlusAction(0),
        leftZoomMinusAction(0),
        leftZoomTo100percents(0),
        leftZoomFitToWindowAction(0),
        rightZoomPlusAction(0),
        rightZoomMinusAction(0),
        rightZoomTo100percents(0),
        rightZoomFitToWindowAction(0),
        forwardAction(0),
        backwardAction(0),
        firstAction(0),
        lastAction(0),
        showBarAction(0),
        viewCMViewAction(0),
        syncPreviewAction(0),
        navigateByPairAction(0),
        clearOnCloseAction(0),
        leftFileName(0),
        rightFileName(0),
        hSplitter(0),
        barViewDock(0),
        thumbView(0),
        previewView(0),
        leftZoomBar(0),
        rightZoomBar(0),
        statusProgressBar(0),
        leftSideBar(0),
        rightSideBar(0)
    {
    }

    void addPageUpDownActions(LightTableWindow* const q, QWidget* const w)
    {
        defineShortcut(w, Qt::Key_Down,  q, SLOT(slotForward()));
        defineShortcut(w, Qt::Key_Right, q, SLOT(slotForward()));
        defineShortcut(w, Qt::Key_Up,    q, SLOT(slotBackward()));
        defineShortcut(w, Qt::Key_Left,  q, SLOT(slotBackward()));
    }

public:

    bool                      autoLoadOnRightPanel;
    bool                      autoSyncPreview;
    bool                      fromLeftPreview;

    QAction*                  setItemLeftAction;
    QAction*                  setItemRightAction;
    QAction*                  clearListAction;
    QAction*                  editItemAction;
    QAction*                  removeItemAction;
    QAction*                  fileDeleteAction;
    QAction*                  fileDeleteFinalAction;
    QAction*                  slideShowAction;
    QAction*                  leftZoomPlusAction;
    QAction*                  leftZoomMinusAction;
    QAction*                  leftZoomTo100percents;
    QAction*                  leftZoomFitToWindowAction;
    QAction*                  rightZoomPlusAction;
    QAction*                  rightZoomMinusAction;
    QAction*                  rightZoomTo100percents;
    QAction*                  rightZoomFitToWindowAction;

    QAction*                  forwardAction;
    QAction*                  backwardAction;
    QAction*                  firstAction;
    QAction*                  lastAction;

    QAction*                  showBarAction;
    QAction*                  viewCMViewAction;
    QAction*                  syncPreviewAction;
    QAction*                  navigateByPairAction;
    QAction*                  clearOnCloseAction;

    DAdjustableLabel*         leftFileName;
    DAdjustableLabel*         rightFileName;

    SidebarSplitter*          hSplitter;
    ThumbBarDock*             barViewDock;

    LightTableThumbBar*       thumbView;

    LightTableView*           previewView;

    DZoomBar*                 leftZoomBar;
    DZoomBar*                 rightZoomBar;

    StatusProgressBar*        statusProgressBar;

    ImagePropertiesSideBarDB* leftSideBar;
    ImagePropertiesSideBarDB* rightSideBar;
};

} // namespace Digikam

#endif // DIGIKAM_LIGHT_TABLE_WINDOW_PRIVATE_H
