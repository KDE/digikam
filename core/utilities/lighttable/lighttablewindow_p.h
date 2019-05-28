/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "itempropertiessidebardb.h"
#include "statusprogressbar.h"
#include "dzoombar.h"
#include "lighttableview.h"
#include "lighttablethumbbar.h"
#include "thumbbardock.h"
#include "drawdecoder.h"
#include "digikam_debug.h"
#include "componentsinfodlg.h"
#include "digikamapp.h"
#include "thememanager.h"
#include "dimg.h"
#include "dio.h"
#include "dmetadata.h"
#include "dfileoperations.h"
#include "metaenginesettings.h"
#include "applicationsettings.h"
#include "albummanager.h"
#include "loadingcacheinterface.h"
#include "deletedialog.h"
#include "iccsettings.h"
#include "imagewindow.h"
#include "itemdescedittab.h"
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
        setItemLeftAction(nullptr),
        setItemRightAction(nullptr),
        clearListAction(nullptr),
        editItemAction(nullptr),
        removeItemAction(nullptr),
        fileDeleteAction(nullptr),
        fileDeleteFinalAction(nullptr),
        slideShowAction(nullptr),
        leftZoomPlusAction(nullptr),
        leftZoomMinusAction(nullptr),
        leftZoomTo100percents(nullptr),
        leftZoomFitToWindowAction(nullptr),
        rightZoomPlusAction(nullptr),
        rightZoomMinusAction(nullptr),
        rightZoomTo100percents(nullptr),
        rightZoomFitToWindowAction(nullptr),
        forwardAction(nullptr),
        backwardAction(nullptr),
        firstAction(nullptr),
        lastAction(nullptr),
        showBarAction(nullptr),
        viewCMViewAction(nullptr),
        syncPreviewAction(nullptr),
        navigateByPairAction(nullptr),
        clearOnCloseAction(nullptr),
        leftFileName(nullptr),
        rightFileName(nullptr),
        hSplitter(nullptr),
        barViewDock(nullptr),
        thumbView(nullptr),
        previewView(nullptr),
        leftZoomBar(nullptr),
        rightZoomBar(nullptr),
        statusProgressBar(nullptr),
        leftSideBar(nullptr),
        rightSideBar(nullptr)
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

    ItemPropertiesSideBarDB* leftSideBar;
    ItemPropertiesSideBarDB* rightSideBar;
};

} // namespace Digikam

#endif // DIGIKAM_LIGHT_TABLE_WINDOW_PRIVATE_H
