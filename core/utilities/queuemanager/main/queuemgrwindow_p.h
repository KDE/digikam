/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager GUI
 *
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_BQM_QUEUE_MGR_WINDOW_PRIVATE_H
#define DIGIKAM_BQM_QUEUE_MGR_WINDOW_PRIVATE_H

// Qt includes

#include <QLabel>
#include <QString>
#include <QAction>

// Local includes

#include "actionthread.h"
#include "assignedlist.h"
#include "queuelist.h"
#include "queuepool.h"
#include "queuesettingsview.h"
#include "statusprogressbar.h"
#include "sidebar.h"
#include "toolsettingsview.h"
#include "toolsview.h"

namespace Digikam
{

class BatchToolsFactory;

class Q_DECL_HIDDEN QueueMgrWindow::Private
{

public:

    explicit Private()
        : TOP_SPLITTER_CONFIG_KEY(QLatin1String("BqmTopSplitter")),
          BOTTOM_SPLITTER_CONFIG_KEY(QLatin1String("BqmBottomSplitter")),
          VERTICAL_SPLITTER_CONFIG_KEY(QLatin1String("BqmVerticalSplitter"))
    {
        busy                   = false;
        processingAllQueues    = false;
        clearQueueAction       = nullptr;
        moveUpToolAction       = nullptr;
        moveDownToolAction     = nullptr;
        removeToolAction       = nullptr;
        removeItemsSelAction   = nullptr;
        removeItemsDoneAction  = nullptr;
        clearToolsAction       = nullptr;
        donateMoneyAction      = nullptr;
        statusProgressBar      = nullptr;
        statusLabel            = nullptr;
        runAction              = nullptr;
        runAllAction           = nullptr;
        stopAction             = nullptr;
        removeQueueAction      = nullptr;
        newQueueAction         = nullptr;
        saveQueueAction        = nullptr;
        rawCameraListAction    = nullptr;
        topSplitter            = nullptr;
        bottomSplitter         = nullptr;
        verticalSplitter       = nullptr;
        contributeAction       = nullptr;
        assignedList           = nullptr;
        queuePool              = nullptr;
        queueSettingsView      = nullptr;
        toolsView              = nullptr;
        batchToolsMgr          = nullptr;
        toolSettings           = nullptr;
        thread                 = nullptr;
        currentQueueToProcess  = 0;
    }

    bool                     busy;
    bool                     processingAllQueues;

    int                      currentQueueToProcess;

    QLabel*                  statusLabel;

    QAction*                 clearQueueAction;
    QAction*                 removeItemsSelAction;
    QAction*                 removeItemsDoneAction;
    QAction*                 moveUpToolAction;
    QAction*                 moveDownToolAction;
    QAction*                 removeToolAction;
    QAction*                 clearToolsAction;

    QAction*                 runAction;
    QAction*                 runAllAction;
    QAction*                 stopAction;
    QAction*                 removeQueueAction;
    QAction*                 newQueueAction;
    QAction*                 saveQueueAction;
    QAction*                 donateMoneyAction;
    QAction*                 contributeAction;
    QAction*                 rawCameraListAction;

    SidebarSplitter*         topSplitter;
    SidebarSplitter*         bottomSplitter;
    SidebarSplitter*         verticalSplitter;

    BatchToolsFactory*       batchToolsMgr;

    StatusProgressBar*       statusProgressBar;

    ActionThread*            thread;

    ToolsView*               toolsView;
    ToolSettingsView*        toolSettings;
    AssignedListView*        assignedList;
    QueuePool*               queuePool;
    QueueSettingsView*       queueSettingsView;

    const QString            TOP_SPLITTER_CONFIG_KEY;
    const QString            BOTTOM_SPLITTER_CONFIG_KEY;
    const QString            VERTICAL_SPLITTER_CONFIG_KEY;
};

} // namespace Digikam

#endif // DIGIKAM_BQM_QUEUE_MGR_WINDOW_PRIVATE_H
