/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager GUI
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef QUEUEMGRWINDOWPRIVATE_H
#define QUEUEMGRWINDOWPRIVATE_H

// KDE includes.

#include <kaction.h>
#include <kiconloader.h>

// Local includes.

#include "statusprogressbar.h"

namespace Digikam
{

class BatchToolsManager;

class QueueMgrWindowPriv
{

public:

    QueueMgrWindowPriv()
    {
        fullScreenHideToolBar  = false;
        fullScreen             = false;
        removeFullScreenButton = false;
        busy                   = false;
        clearQueueAction       = 0;
        moveUpToolAction       = 0;
        moveDownToolAction     = 0;
        removeToolAction       = 0;
        removeItemsSelAction   = 0;
        removeItemsDoneAction  = 0;
        clearToolsAction       = 0;
        fullScreenAction       = 0;
        donateMoneyAction      = 0;
        statusProgressBar      = 0;
        runAction              = 0;
        stopAction             = 0;
        removeQueueAction      = 0;
        newQueueAction         = 0;
        rawCameraListAction    = 0;
        libsInfoAction         = 0;
        themeMenuAction        = 0;
        contributeAction       = 0;
        assignedList           = 0;
        queuePool              = 0;
        queueSettingsView      = 0;
        toolsList              = 0;
        batchToolsMgr          = 0;
        toolSettings           = 0;
        showMenuBarAction      = 0;
        thread                 = 0;
        currentProcessItem     = 0;
        currentTaskItem        = 0;
        animLogo               = 0;
        progressCount          = 0;
        progressTimer          = 0;
        progressPix            = SmallIcon("process-working", 22);
    }

    bool                       fullScreenHideToolBar;
    bool                       fullScreen;
    bool                       removeFullScreenButton;
    bool                       busy;

    int                        progressCount;

    QTimer                    *progressTimer;

    QPixmap                    progressPix;

    QAction                   *clearQueueAction;
    QAction                   *removeItemsSelAction;
    QAction                   *removeItemsDoneAction;
    QAction                   *fullScreenAction;
    QAction                   *moveUpToolAction;
    QAction                   *moveDownToolAction;
    QAction                   *removeToolAction;
    QAction                   *clearToolsAction;

    KAction                   *runAction;
    KAction                   *stopAction;
    KAction                   *removeQueueAction;
    KAction                   *newQueueAction;
    KAction                   *donateMoneyAction;
    KAction                   *contributeAction;
    KAction                   *rawCameraListAction;
    KAction                   *libsInfoAction;

    KToggleAction             *showMenuBarAction;

    KSelectAction             *themeMenuAction;

    DLogoAction               *animLogo;
    QueueListViewItem         *currentProcessItem;

    AssignedListViewItem      *currentTaskItem;

    BatchToolsManager         *batchToolsMgr;

    StatusProgressBar         *statusProgressBar;

    ActionThread              *thread;

    ToolsListView             *toolsList;
    ToolSettingsView          *toolSettings;
    AssignedListView          *assignedList;
    QueuePool                 *queuePool;
    QueueSettingsView         *queueSettingsView;

    QueuePoolItemsList         itemsList;
};

}  // namespace Digikam

#endif /* QUEUEMGRWINDOWPRIVATE_H */
