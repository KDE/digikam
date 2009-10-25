/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LIGHTTABLEWINDOWPRIVATE_H
#define LIGHTTABLEWINDOWPRIVATE_H

// Qt includes

#include <QSplitter>

// KDE includes

#include <kaction.h>

// Local includes

#include "imagepropertiessidebardb.h"
#include "statusprogressbar.h"
#include "statuszoombar.h"
#include "lighttableview.h"
#include "lighttablebar.h"
#include "thumbbardock.h"

namespace Digikam
{

class LightTableWindowPriv
{

public:

    LightTableWindowPriv()
    {
        autoLoadOnRightPanel   = true;
        autoSyncPreview        = true;
        fullScreenHideToolBar  = false;
        fullScreen             = false;
        removeFullScreenButton = false;
        cancelSlideShow        = false;
        star0                  = 0;
        star1                  = 0;
        star2                  = 0;
        star3                  = 0;
        star4                  = 0;
        star5                  = 0;
        leftSideBar            = 0;
        rightSideBar           = 0;
        previewView            = 0;
        barView                = 0;
        hSplitter              = 0;
        barViewDock            = 0;
        syncPreviewAction      = 0;
        clearListAction        = 0;
        setItemLeftAction      = 0;
        setItemRightAction     = 0;
        editItemAction         = 0;
        removeItemAction       = 0;
        fileDeleteAction       = 0;
        fileDeleteFinalAction  = 0;
        slideShowAction        = 0;
        fullScreenAction       = 0;
        donateMoneyAction      = 0;
        zoomFitToWindowAction  = 0;
        zoomTo100percents      = 0;
        zoomPlusAction         = 0;
        zoomMinusAction        = 0;
        showThumbBarAction     = 0;
        statusProgressBar      = 0;
        leftZoomBar            = 0;
        rightZoomBar           = 0;
        forwardAction          = 0;
        backwardAction         = 0;
        firstAction            = 0;
        lastAction             = 0;
        navigateByPairAction   = 0;
        rawCameraListAction    = 0;
        libsInfoAction         = 0;
        dbStatAction           = 0;
        themeMenuAction        = 0;
        contributeAction       = 0;
        showMenuBarAction      = 0;
        clearOnCloseAction     = 0;
    }

    bool                      autoLoadOnRightPanel;
    bool                      autoSyncPreview;
    bool                      fullScreenHideToolBar;
    bool                      fullScreen;
    bool                      removeFullScreenButton;
    bool                      cancelSlideShow;

    SidebarSplitter          *hSplitter;
    ThumbBarDock             *barViewDock;

    KAction                  *setItemLeftAction;
    KAction                  *setItemRightAction;
    KAction                  *clearListAction;
    KAction                  *editItemAction;
    KAction                  *removeItemAction;
    KAction                  *fileDeleteAction;
    KAction                  *fileDeleteFinalAction;
    KAction                  *slideShowAction;
    KAction                  *zoomPlusAction;
    KAction                  *zoomMinusAction;
    KAction                  *zoomTo100percents;
    KAction                  *zoomFitToWindowAction;
    KAction                  *fullScreenAction;
    KToggleAction            *showThumbBarAction;

    // Rating actions.
    KAction                  *star0;
    KAction                  *star1;
    KAction                  *star2;
    KAction                  *star3;
    KAction                  *star4;
    KAction                  *star5;

    KAction                  *forwardAction;
    KAction                  *backwardAction;
    KAction                  *firstAction;
    KAction                  *lastAction;
    KAction                  *donateMoneyAction;
    KAction                  *contributeAction;
    KAction                  *rawCameraListAction;
    KAction                  *libsInfoAction;
    KAction                  *dbStatAction;

    KSelectAction            *themeMenuAction;

    KToggleAction            *syncPreviewAction;
    KToggleAction            *navigateByPairAction;
    KToggleAction            *showMenuBarAction;
    KToggleAction            *clearOnCloseAction;

    LightTableBar            *barView;

    LightTableView           *previewView;

    StatusZoomBar            *leftZoomBar;
    StatusZoomBar            *rightZoomBar;

    StatusProgressBar        *statusProgressBar;

    ImagePropertiesSideBarDB *leftSideBar;
    ImagePropertiesSideBarDB *rightSideBar;
};

}  // namespace Digikam

#endif /* LIGHTTABLEWINDOWPRIVATE_H */
