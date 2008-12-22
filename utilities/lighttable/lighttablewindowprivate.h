/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qsplitter.h>

// KDE includes.

#include <kaction.h>
#include <kaccel.h>

// Local includes.

#include "imagepropertiessidebardb.h"
#include "statusprogressbar.h"
#include "statuszoombar.h"
#include "lighttableview.h"
#include "lighttablebar.h"

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
        accelerators           = 0;
        leftSidebar            = 0;
        rightSidebar           = 0;
        previewView            = 0;
        barView                = 0;
        hSplitter              = 0;
        vSplitter              = 0;
        syncPreviewAction      = 0;
        clearListAction        = 0;
        setItemLeftAction      = 0;
        setItemRightAction     = 0;
        editItemAction         = 0;
        removeItemAction       = 0;
        fileDeleteAction       = 0;
        slideShowAction        = 0;
        fullScreenAction       = 0;
        donateMoneyAction      = 0;
        zoomFitToWindowAction  = 0;
        zoomTo100percents      = 0;
        zoomPlusAction         = 0;
        zoomMinusAction        = 0;
        statusProgressBar      = 0;
        leftZoomBar            = 0;
        rightZoomBar           = 0;
        forwardAction          = 0;
        backwardAction         = 0;
        firstAction            = 0;
        lastAction             = 0;
        navigateByPairAction   = 0;
        rawCameraListAction    = 0;
        themeMenuAction        = 0;
        contributeAction       = 0;
        showMenuBarAction      = 0;
    }

    bool                      autoLoadOnRightPanel;
    bool                      autoSyncPreview;
    bool                      fullScreenHideToolBar;
    bool                      fullScreen;
    bool                      removeFullScreenButton;
    bool                      cancelSlideShow;

    QSplitter                *hSplitter;
    QSplitter                *vSplitter;

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

    KAction                  *setItemLeftAction;
    KAction                  *setItemRightAction;
    KAction                  *clearListAction;
    KAction                  *editItemAction;
    KAction                  *removeItemAction;
    KAction                  *fileDeleteAction;
    KAction                  *slideShowAction;
    KAction                  *donateMoneyAction;
    KAction                  *contributeAction;
    KAction                  *zoomPlusAction;
    KAction                  *zoomMinusAction;
    KAction                  *zoomTo100percents;
    KAction                  *zoomFitToWindowAction;
    KAction                  *rawCameraListAction;

    KSelectAction            *themeMenuAction;

    KToggleAction            *fullScreenAction;
    KToggleAction            *syncPreviewAction;
    KToggleAction            *navigateByPairAction;
    KToggleAction            *showMenuBarAction;

    KAccel                   *accelerators;

    LightTableBar            *barView;

    LightTableView           *previewView;

    StatusZoomBar            *leftZoomBar;
    StatusZoomBar            *rightZoomBar;

    StatusProgressBar        *statusProgressBar;

    ImagePropertiesSideBarDB *leftSidebar;
    ImagePropertiesSideBarDB *rightSidebar;
};

}  // namespace Digikam
