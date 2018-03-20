/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager GUI
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "queuemgrwindow.h"
#include "queuemgrwindow_p.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeySequence>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include <QMessageBox>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "actions.h"
#include "album.h"
#include "batchtoolsmanager.h"
#include "actionthread.h"
#include "queuepool.h"
#include "workflowmanager.h"
#include "queuelist.h"
#include "queuesettings.h"
#include "queuesettingsview.h"
#include "assignedlist.h"
#include "toolsettingsview.h"
#include "toolsview.h"
#include "componentsinfo.h"
#include "digikamapp.h"
#include "thememanager.h"
#include "dimg.h"
#include "dlogoaction.h"
#include "albummanager.h"
#include "imagewindow.h"
#include "thumbnailsize.h"
#include "sidebar.h"
#include "dnotificationwrapper.h"

namespace Digikam
{

QueueMgrWindow* QueueMgrWindow::m_instance = 0;

QueueMgrWindow* QueueMgrWindow::queueManagerWindow()
{
    if (!m_instance)
    {
        new QueueMgrWindow();
    }

    return m_instance;
}

bool QueueMgrWindow::queueManagerWindowCreated()
{
    return m_instance;
}

QueueMgrWindow::QueueMgrWindow()
    : DXmlGuiWindow(0),
      d(new Private)
{
    setConfigGroupName(QLatin1String("Batch Queue Manager Settings"));
    setXMLFile(QLatin1String("queuemgrwindowui5.rc"));

    qRegisterMetaType<BatchToolSettings>("BatchToolSettings");
    qRegisterMetaType<BatchToolSet>("BatchToolSet");

    m_instance = this;
    BatchToolsManager::instance();        // Create first instance here
    WorkflowManager::instance();             // Create first instance here
    d->thread  = new ActionThread(this);

    setWindowFlags(Qt::Window);
    setCaption(i18n("Batch Queue Manager"));
    // We don't want to be deleted on close
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFullScreenOptions(FS_NONE);

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();

    // Make signals/slots connections

    setupConnections();

    //-------------------------------------------------------------

    readSettings();
    applySettings();
    setAutoSaveSettings(configGroupName(), true);

    populateToolsList();
    slotQueueContentsChanged();
}

QueueMgrWindow::~QueueMgrWindow()
{
    m_instance = 0;
    delete d;
}

QMap<int, QString> QueueMgrWindow::queuesMap() const
{
    if (d->queuePool)
    {
        return d->queuePool->queuesMap();
    }

    return QMap<int, QString>();
}

bool QueueMgrWindow::isBusy() const
{
    return d->busy;
}

void QueueMgrWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    writeSettings();
    DXmlGuiWindow::closeEvent(e);
    e->accept();
}

void QueueMgrWindow::setupUserArea()
{
    QWidget* const mainW          = new QWidget(this);
    QVBoxLayout* const mainLayout = new QVBoxLayout(mainW);

    // ------------------------------------------------------------------------------

    QGroupBox* const queuesBox = new QGroupBox(i18n("Queues"), mainW);
    QVBoxLayout* const vlay1   = new QVBoxLayout(queuesBox);
    d->queuePool               = new QueuePool(queuesBox);
    vlay1->addWidget(d->queuePool);
    vlay1->setContentsMargins(QMargins());
    vlay1->setSpacing(0);

    // ------------------------------------------------------------------------------

    QGroupBox* const queueSettingsBox = new QGroupBox(i18n("Queue Settings"), mainW);
    QVBoxLayout* const vlay2          = new QVBoxLayout(queueSettingsBox);
    d->queueSettingsView              = new QueueSettingsView(queueSettingsBox);
    vlay2->addWidget(d->queueSettingsView);
    vlay2->setContentsMargins(QMargins());
    vlay2->setSpacing(0);

    // ------------------------------------------------------------------------------

    QGroupBox* const toolsBox = new QGroupBox(i18n("Control Panel"), mainW);
    QVBoxLayout* const vlay3  = new QVBoxLayout(toolsBox);
    d->toolsView              = new ToolsView(toolsBox);
    vlay3->addWidget(d->toolsView);
    vlay3->setContentsMargins(QMargins());
    vlay3->setSpacing(0);

    // ------------------------------------------------------------------------------

    QGroupBox* const assignBox = new QGroupBox(i18n("Assigned Tools"), mainW);
    QVBoxLayout* const vlay4   = new QVBoxLayout(assignBox);
    d->assignedList            = new AssignedListView(assignBox);
    vlay4->addWidget(d->assignedList);
    vlay4->setContentsMargins(QMargins());
    vlay4->setSpacing(0);

    // ------------------------------------------------------------------------------

    QGroupBox* const toolSettingsBox = new QGroupBox(i18n("Tool Settings"), mainW);
    QVBoxLayout* const vlay5         = new QVBoxLayout(toolSettingsBox);
    d->toolSettings                  = new ToolSettingsView(toolSettingsBox);
    vlay5->addWidget(d->toolSettings);
    vlay5->setContentsMargins(QMargins());
    vlay5->setSpacing(0);

    // ------------------------------------------------------------------------------

    d->topSplitter = new SidebarSplitter(mainW);
    d->topSplitter->addWidget(queuesBox);
    d->topSplitter->addWidget(assignBox);
    d->topSplitter->addWidget(toolSettingsBox);

    d->bottomSplitter = new SidebarSplitter(mainW);
    d->bottomSplitter->addWidget(queueSettingsBox);
    d->bottomSplitter->addWidget(toolsBox);

    d->verticalSplitter = new SidebarSplitter(Qt::Vertical, mainW);
    d->verticalSplitter->addWidget(d->topSplitter);
    d->verticalSplitter->addWidget(d->bottomSplitter);

    mainLayout->addWidget(d->verticalSplitter);

    setCentralWidget(mainW);
}

void QueueMgrWindow::setupStatusBar()
{
    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->statusProgressBar->setMaximumHeight(fontMetrics().height() + 2);
    d->statusProgressBar->setNotify(true);
    d->statusProgressBar->setNotificationTitle(i18n("Batch Queue Manager"), QIcon::fromTheme(QLatin1String("run-build")));
    statusBar()->addWidget(d->statusProgressBar, 60);

    d->statusLabel = new QLabel(statusBar());
    d->statusLabel->setAlignment(Qt::AlignCenter);
    d->statusLabel->setMaximumHeight(fontMetrics().height() + 2);
    statusBar()->addWidget(d->statusLabel, 40);
}

void QueueMgrWindow::setupConnections()
{
    // -- Assigned tools list connections -----------------------------------

    connect(d->assignedList, SIGNAL(signalToolSelected(BatchToolSet)),
            d->toolSettings, SLOT(slotToolSelected(BatchToolSet)));

    connect(d->assignedList, SIGNAL(signalAssignedToolsChanged(AssignedBatchTools)),
            d->queuePool, SLOT(slotAssignedToolsChanged(AssignedBatchTools)));

    connect(d->toolSettings, SIGNAL(signalSettingsChanged(BatchToolSet)),
            d->assignedList, SLOT(slotSettingsChanged(BatchToolSet)));

    connect(d->assignedList, SIGNAL(signalAssignedToolsChanged(AssignedBatchTools)),
            this, SLOT(slotAssignedToolsChanged(AssignedBatchTools)));

    connect(d->toolsView, SIGNAL(signalAssignTools(QMap<int,QString>)),
            d->assignedList, SLOT(slotAssignTools(QMap<int,QString>)));

    // -- Queued Items list connections -------------------------------------

    connect(d->queuePool, SIGNAL(signalQueueSelected(int,QueueSettings,AssignedBatchTools)),
            d->queueSettingsView, SLOT(slotQueueSelected(int,QueueSettings,AssignedBatchTools)));

    connect(d->queuePool, SIGNAL(signalQueueSelected(int,QueueSettings,AssignedBatchTools)),
            d->assignedList, SLOT(slotQueueSelected(int,QueueSettings,AssignedBatchTools)));

    connect(d->queueSettingsView, SIGNAL(signalSettingsChanged(QueueSettings)),
            d->queuePool, SLOT(slotSettingsChanged(QueueSettings)));

    connect(d->queueSettingsView, SIGNAL(signalSettingsChanged(QueueSettings)),
            this, SLOT(slotQueueContentsChanged()));

    connect(d->queuePool, SIGNAL(signalQueueSelected(int,QueueSettings,AssignedBatchTools)),
            this, SLOT(slotQueueContentsChanged()));

    connect(d->queuePool, SIGNAL(signalQueuePoolChanged()),
            this, SLOT(slotQueueContentsChanged()));

    connect(d->queuePool, SIGNAL(signalQueueContentsChanged()),
            this, SLOT(slotQueueContentsChanged()));

    connect(d->queuePool, SIGNAL(signalItemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));

    // -- Multithreaded interface connections -------------------------------

    connect(d->thread, SIGNAL(signalStarting(Digikam::ActionData)),
            this, SLOT(slotAction(Digikam::ActionData)));

    connect(d->thread, SIGNAL(signalFinished(Digikam::ActionData)),
            this, SLOT(slotAction(Digikam::ActionData)));

    connect(d->thread, SIGNAL(signalQueueProcessed()),
            this, SLOT(slotQueueProcessed()));

    // -- GUI connections ---------------------------------------------------

    connect(d->toolsView, SIGNAL(signalHistoryEntryClicked(int,qlonglong)),
            this, SLOT(slotHistoryEntryClicked(int,qlonglong)));

    connect(d->toolsView, SIGNAL(signalAssignQueueSettings(QString)),
            this, SLOT(slotAssignQueueSettings(QString)));
}

void QueueMgrWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    KActionCollection* const ac = actionCollection();

    d->runAction = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")),
                               i18n("Run"), this);
    d->runAction->setEnabled(false);
    connect(d->runAction, SIGNAL(triggered()), this, SLOT(slotRun()));
    ac->addAction(QLatin1String("queuemgr_run"), d->runAction);
    ac->setDefaultShortcut(d->runAction, Qt::CTRL + Qt::Key_P);

    d->runAllAction = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")),
                               i18n("Run all"), this);
    d->runAllAction->setEnabled(false);
    connect(d->runAllAction, SIGNAL(triggered()), this, SLOT(slotRunAll()));
    ac->addAction(QLatin1String("queuemgr_run_all"), d->runAllAction);
    ac->setDefaultShortcut(d->runAllAction, Qt::ALT + Qt::CTRL + Qt::Key_P);

    d->stopAction = new QAction(QIcon::fromTheme(QLatin1String("media-playback-stop")), i18n("Stop"), this);
    d->stopAction->setEnabled(false);
    connect(d->stopAction, SIGNAL(triggered()), this, SLOT(slotStop()));
    ac->addAction(QLatin1String("queuemgr_stop"), d->stopAction);
    ac->setDefaultShortcut(d->stopAction, Qt::CTRL + Qt::Key_S);

    d->newQueueAction = new QAction(QIcon::fromTheme(QLatin1String("list-add")), i18n("New Queue"), this);
    connect(d->newQueueAction, SIGNAL(triggered()), d->queuePool, SLOT(slotAddQueue()));
    ac->addAction(QLatin1String("queuemgr_newqueue"), d->newQueueAction);

    d->removeQueueAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove Queue"), this);
    connect(d->removeQueueAction, SIGNAL(triggered()), d->queuePool, SLOT(slotRemoveCurrentQueue()));
    ac->addAction(QLatin1String("queuemgr_removequeue"), d->removeQueueAction);

    // TODO rename action to saveWorkflowAction to avoid confusion?
    d->saveQueueAction = new QAction(QIcon::fromTheme(QLatin1String("document-save")), i18n("Save Workflow"), this);
    connect(d->saveQueueAction, SIGNAL(triggered()), this, SLOT(slotSaveWorkflow()));
    ac->addAction(QLatin1String("queuemgr_savequeue"), d->saveQueueAction);

    d->removeItemsSelAction = new QAction(QIcon::fromTheme(QLatin1String("list-remove")), i18n("Remove items"), this);
    d->removeItemsSelAction->setEnabled(false);
    connect(d->removeItemsSelAction, SIGNAL(triggered()), d->queuePool, SLOT(slotRemoveSelectedItems()));
    ac->addAction(QLatin1String("queuemgr_removeitemssel"), d->removeItemsSelAction);
    ac->setDefaultShortcut(d->removeItemsSelAction, Qt::CTRL + Qt::Key_K);

    d->removeItemsDoneAction = new QAction(i18n("Remove processed items"), this);
    d->removeItemsDoneAction->setEnabled(false);
    connect(d->removeItemsDoneAction, SIGNAL(triggered()), d->queuePool, SLOT(slotRemoveItemsDone()));
    ac->addAction(QLatin1String("queuemgr_removeitemsdone"), d->removeItemsDoneAction);

    d->clearQueueAction = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")), i18n("Clear Queue"), this);
    d->clearQueueAction->setEnabled(false);
    connect(d->clearQueueAction, SIGNAL(triggered()), d->queuePool, SLOT(slotClearList()));
    ac->addAction(QLatin1String("queuemgr_clearlist"), d->clearQueueAction);
    ac->setDefaultShortcut(d->clearQueueAction, Qt::CTRL + Qt::SHIFT + Qt::Key_K);

    QAction* const close = buildStdAction(StdCloseAction, this, SLOT(close()), this);
    ac->addAction(QLatin1String("queuemgr_close"), close);

    // -- 'Tools' menu actions -----------------------------------------------------

    d->moveUpToolAction = new QAction(QIcon::fromTheme(QLatin1String("go-up")), i18n("Move up"), this);
    connect(d->moveUpToolAction, SIGNAL(triggered()), d->assignedList, SLOT(slotMoveCurrentToolUp()));
    ac->addAction(QLatin1String("queuemgr_toolup"), d->moveUpToolAction);

    d->moveDownToolAction = new QAction(QIcon::fromTheme(QLatin1String("go-down")), i18n("Move down"), this);
    connect(d->moveDownToolAction, SIGNAL(triggered()), d->assignedList, SLOT(slotMoveCurrentToolDown()));
    ac->addAction(QLatin1String("queuemgr_tooldown"), d->moveDownToolAction);

    d->removeToolAction = new QAction(QIcon::fromTheme(QLatin1String("list-remove")), i18n("Remove tool"), this);
    connect(d->removeToolAction, SIGNAL(triggered()), d->assignedList, SLOT(slotRemoveCurrentTool()));
    ac->addAction(QLatin1String("queuemgr_toolremove"), d->removeToolAction);

    d->clearToolsAction = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")), i18n("Clear List"), this);
    connect(d->clearToolsAction, SIGNAL(triggered()), d->assignedList, SLOT(slotClearToolsList()));
    ac->addAction(QLatin1String("queuemgr_toolsclear"), d->clearToolsAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    createFullScreenAction(QLatin1String("queuemgr_fullscreen"));

    // ---------------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    // Standard 'Help' menu actions
    createHelpActions();

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // Standard 'Configure' menu actions
    createSettingsActions();

    // ---------------------------------------------------------------------------------

    createGUI(xmlFile());
    cleanupActions();

    showMenuBarAction()->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080
}

void QueueMgrWindow::refreshView()
{
    // NOTE: method called when something is changed from Database (tags, rating, etc...).
    //       There is nothing to do for the moment.
}

void QueueMgrWindow::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    d->verticalSplitter->restoreState(group, d->VERTICAL_SPLITTER_CONFIG_KEY);
    d->bottomSplitter->restoreState(group,   d->BOTTOM_SPLITTER_CONFIG_KEY);
    d->topSplitter->restoreState(group,      d->TOP_SPLITTER_CONFIG_KEY);

    readFullScreenSettings(group);
}

void QueueMgrWindow::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    d->topSplitter->saveState(group,      d->TOP_SPLITTER_CONFIG_KEY);
    d->bottomSplitter->saveState(group,   d->BOTTOM_SPLITTER_CONFIG_KEY);
    d->verticalSplitter->saveState(group, d->VERTICAL_SPLITTER_CONFIG_KEY);

    config->sync();
}

void QueueMgrWindow::applySettings()
{
    // Do not apply general settings from config panel if BQM is busy.
    if (d->busy)
    {
        return;
    }

    d->queuePool->applySettings();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());
    readFullScreenSettings(group);
}

void QueueMgrWindow::refreshStatusBar()
{
    int items        = d->queuePool->currentQueue()->itemsCount();
    int pendingItems = d->queuePool->currentQueue()->pendingItemsCount();
    int tasks        = d->queuePool->currentQueue()->pendingTasksCount();
    int totalItems   = d->queuePool->totalPendingItems();
    int totalTasks   = d->queuePool->totalPendingTasks();
    QString message  = i18n("Current Queue: ");

    switch (pendingItems)
    {
        case 0:
            message.append(i18n("No items"));
            break;

        default:
            message.append(i18np("1 item", "%1 items", pendingItems));
            break;
    }

    message.append(QLatin1String(" / "));

    switch (tasks)
    {
        case 0:
            message.append(i18n("No tasks"));
            break;

        default:
            message.append(i18np("1 task", "%1 tasks", tasks));
            break;
    }

    message.append(i18n(" - Total: "));

    switch (totalItems)
    {
        case 0:
            message.append(i18n("No items"));
            break;

        default:
            message.append(i18np("1 item", "%1 items", totalItems));
            break;
    }

    message.append(QLatin1String(" / "));

    switch (totalTasks)
    {
        case 0:
            message.append(i18n("No tasks"));
            break;

        default:
            message.append(i18np("1 task", "%1 tasks", totalTasks));
            break;
    }

    d->statusLabel->setText(message);

    if (!d->busy)
    {
        d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode, i18n("Ready"));
        d->removeItemsSelAction->setEnabled(items > 0);
        d->removeItemsDoneAction->setEnabled((items - pendingItems) > 0);
        d->clearQueueAction->setEnabled(items > 0);
        d->runAction->setEnabled((tasks > 0) && (pendingItems > 0));
        d->runAllAction->setEnabled((totalTasks > 0) && (totalItems > 0));
    }
}

void QueueMgrWindow::slotSetup()
{
    setup(Setup::LastPageUsed);
}

void QueueMgrWindow::setup(Setup::Page page)
{
    Setup::execDialog(this, page);
}

void QueueMgrWindow::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void QueueMgrWindow::slotDBStat()
{
    showDigikamDatabaseStat();
}

bool QueueMgrWindow::queryClose()
{
    if (isBusy())
    {
        int result = QMessageBox::warning(this, i18n("Processing under progress"),
                                          i18n("Batch Queue Manager is running. Do you want to cancel current job?"),
                                          QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::Yes)
        {
            slotStop();
        }
        else if (result == QMessageBox::No)
        {
            return false;
        }
    }

    return true;
}

void QueueMgrWindow::addNewQueue()
{
    d->queuePool->slotAddQueue();
}

int QueueMgrWindow::currentQueueId() const
{
    return (d->queuePool->currentIndex());
}

void QueueMgrWindow::loadImageInfos(const ImageInfoList& list, int queueId)
{
    QueueListView* const queue = d->queuePool->findQueueByIndex(queueId);

    if (queue)
    {
        queue->slotAddItems(list);
    }
}

void QueueMgrWindow::loadImageInfosToCurrentQueue(const ImageInfoList& list)
{
    if (!d->queuePool->currentQueue())
    {
        addNewQueue();
    }

    d->queuePool->currentQueue()->slotAddItems(list);
}

void QueueMgrWindow::loadImageInfosToNewQueue(const ImageInfoList& list)
{
    QueueListView* const queue = d->queuePool->currentQueue();

    if (!queue || queue->itemsCount())
    {
        addNewQueue();
    }

    d->queuePool->currentQueue()->slotAddItems(list);
}

void QueueMgrWindow::slotQueueContentsChanged()
{
    if (d->busy)
    {
        refreshStatusBar();
    }
    else
    {
        // refreshStatusBar() and actions in tools view
        slotAssignedToolsChanged(d->assignedList->assignedList());
    }
}

void QueueMgrWindow::slotItemSelectionChanged()
{
    if (!d->busy)
    {
        int count = d->queuePool->currentQueue()->selectedItems().count();
        d->removeItemsSelAction->setEnabled((count != 0) ? true : false);
    }
}

void QueueMgrWindow::populateToolsList()
{
    BatchToolsList list = BatchToolsManager::instance()->toolsList();

    foreach(BatchTool* const tool, list)
    {
        d->toolsView->addTool(tool);
    }
}

void QueueMgrWindow::slotRun()
{
    d->currentQueueToProcess = 0;

    QueueListView* const queue = d->queuePool->currentQueue();
    QString msg;

    if (!queue)
    {
        msg = i18n("There is no queue to be run.");
    }
    else if (queue->pendingItemsCount() == 0)
    {
        msg = i18n("There is no item to process in the current queue (%1).",
                   d->queuePool->currentTitle());
    }
    else if (queue->settings().renamingRule == QueueSettings::CUSTOMIZE
             && queue->settings().renamingParser.isEmpty())
    {
        msg = i18n("Custom renaming rule is invalid for current queue (%1). "
                   "Please fix it.", d->queuePool->currentTitle());
    }
    else if (queue->assignedTools().m_toolsList.isEmpty())
    {
        msg = i18n("Assigned batch tools list is empty for current queue (%1). "
                   "Please assign tools.", d->queuePool->currentTitle());
    }

    if (!msg.isEmpty())
    {
        QMessageBox::critical(this, qApp->applicationName(), msg);
        processingAborted();
        return;
    }

    // Take a look if general settings are changed, as we cannot do it when BQM is busy.
    applySettings();

    d->statusProgressBar->setProgressTotalSteps(queue->pendingTasksCount());
    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressBarMode(StatusProgressBar::ProgressBarMode);
    d->toolsView->showTab(ToolsView::HISTORY);
    busy(true);

    d->processingAllQueues = false;
    processOneQueue();
}

void QueueMgrWindow::slotRunAll()
{
    d->currentQueueToProcess = 0;

    if (!d->queuePool->totalPendingItems())
    {
        QMessageBox::critical(this, qApp->applicationName(), i18n("There are no items to process in the queues."));
        processingAborted();
        return;
    }

    if (!d->queuePool->customRenamingRulesAreValid())
    {
        processingAborted();
        return;
    }

    if (!d->queuePool->assignedBatchToolsListsAreValid())
    {
        processingAborted();
        return;
    }

    // Take a look if general settings are changed, as we cannot do it when BQM is busy.
    applySettings();

    d->statusProgressBar->setProgressTotalSteps(d->queuePool->totalPendingTasks());
    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressBarMode(StatusProgressBar::ProgressBarMode);
    d->toolsView->showTab(ToolsView::HISTORY);
    busy(true);

    d->processingAllQueues = true;
    processOneQueue();
}

void QueueMgrWindow::processingAborted()
{
    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode);
    busy(false);
    refreshStatusBar();
}

void QueueMgrWindow::processOneQueue()
{
    d->assignedList->reset();

    d->queuePool->setCurrentIndex(d->currentQueueToProcess);
    QueuePoolItemsList itemsList = d->queuePool->queueItemsList(d->currentQueueToProcess);
    QueueSettings settings       = d->queuePool->currentQueue()->settings();

    if (!checkTargetAlbum(d->currentQueueToProcess))
    {
        processingAborted();
        return;
    }

    QList<AssignedBatchTools> tools4Items;

    foreach(const ItemInfoSet& item, itemsList)
    {
        AssignedBatchTools one         = d->queuePool->currentQueue()->assignedTools();
        one.m_itemUrl                  = item.info.fileUrl();
        QueueListViewItem* const cItem = d->queuePool->currentQueue()->findItemByUrl(one.m_itemUrl);
        one.m_destFileName             = cItem->destFileName();
        tools4Items.append(one);
    }

    d->thread->setSettings(settings);
    d->thread->processQueueItems(tools4Items);

    if (!d->thread->isRunning())
        d->thread->start();
}

void QueueMgrWindow::busy(bool busy)
{
    d->busy = busy;
    d->runAction->setEnabled(!d->busy);
    d->runAllAction->setEnabled(!d->busy);
    d->newQueueAction->setEnabled(!d->busy);
    d->saveQueueAction->setEnabled(!d->busy);
    d->removeQueueAction->setEnabled(!d->busy);
    d->removeItemsSelAction->setEnabled(!d->busy);
    d->removeItemsDoneAction->setEnabled(!d->busy);
    d->clearQueueAction->setEnabled(!d->busy);
    d->stopAction->setEnabled(d->busy);

    d->queuePool->setBusy(d->busy);
    d->queueSettingsView->setBusy(d->busy);
    d->toolsView->setBusy(d->busy);
    d->assignedList->setBusy(d->busy);
    d->toolSettings->setBusy(d->busy);

    // To update status of Tools actions.
    slotAssignedToolsChanged(d->assignedList->assignedList());

    // To update status of Queue items actions.
    slotItemSelectionChanged();

    d->busy ? d->queuePool->setCursor(Qt::WaitCursor) : d->queuePool->unsetCursor();
    d->busy ? m_animLogo->start() : m_animLogo->stop();

    emit signalBqmIsBusy(d->busy);
}

void QueueMgrWindow::slotAssignedToolsChanged(const AssignedBatchTools& tools)
{
    if (d->busy)
    {
        d->moveUpToolAction->setEnabled(false);
        d->moveDownToolAction->setEnabled(false);
        d->removeToolAction->setEnabled(false);
        d->clearToolsAction->setEnabled(false);
        return;
    }

    switch (tools.m_toolsList.count())
    {
        case 0:
        {
            d->moveUpToolAction->setEnabled(false);
            d->moveDownToolAction->setEnabled(false);
            d->removeToolAction->setEnabled(false);
            d->clearToolsAction->setEnabled(false);
            break;
        }

        case 1:
        {
            d->moveUpToolAction->setEnabled(false);
            d->moveDownToolAction->setEnabled(false);
            d->removeToolAction->setEnabled(true);
            d->clearToolsAction->setEnabled(true);
            break;
        }

        default:
        {
            d->moveUpToolAction->setEnabled(true);
            d->moveDownToolAction->setEnabled(true);
            d->removeToolAction->setEnabled(true);
            d->clearToolsAction->setEnabled(true);
            break;
        }
    }

    refreshStatusBar();
}

bool QueueMgrWindow::checkTargetAlbum(int queueId)
{
    QueueListView* const queue = d->queuePool->findQueueByIndex(queueId);

    if (!queue)
    {
        return false;
    }

    if (!queue->settings().useOrgAlbum)
    {
        QString queueName              = d->queuePool->queueTitle(queueId);
        QUrl    processedItemsAlbumUrl = queue->settings().workingUrl;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Target album for queue " << queueName << " is: " << processedItemsAlbumUrl.toLocalFile();

        if (processedItemsAlbumUrl.isEmpty())
        {
            QMessageBox::critical(this, i18n("Processed items album settings"),
                                  i18n("Album to host processed items from queue \"%1\" is not set. "
                                       "Please select one from Queue Settings panel.", queueName));
            return false;
        }

        QFileInfo dir(processedItemsAlbumUrl.toLocalFile());

        if (!dir.exists() || !dir.isWritable())
        {
            QMessageBox::critical(this, i18n("Processed items album settings"),
                                  i18n("Album to host processed items from queue \"%1\" "
                                       "is not available or not writable. "
                                       "Please set another one from Queue Settings panel.", queueName));
            return false;
        }
    }

    return true;
}

void QueueMgrWindow::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)
    emit signalWindowHasMoved();
}

void QueueMgrWindow::slotHistoryEntryClicked(int queueId, qlonglong itemId)
{
    if (d->busy)
    {
        return;
    }

    QueueListView* const view = d->queuePool->findQueueByIndex(queueId);

    if (view)
    {
        QueueListViewItem* const item = view->findItemById(itemId);

        if (item)
        {
            d->queuePool->setCurrentIndex(queueId);
            view->scrollToItem(item);
            view->setCurrentItem(item);
            item->setSelected(true);
        }
    }
}

void QueueMgrWindow::slotAction(const ActionData& ad)
{
    QueueListViewItem* const cItem = d->queuePool->currentQueue()->findItemByUrl(ad.fileUrl);

    switch (ad.status)
    {
        case ActionData::BatchStarted:
        {
            if (cItem)
            {
                cItem->reset();
                d->queuePool->currentQueue()->setCurrentItem(cItem);
                d->queuePool->currentQueue()->scrollToItem(cItem);
                d->queuePool->setItemBusy(cItem->info().id());
                addHistoryMessage(cItem, i18n("Processing..."), DHistoryView::StartingEntry);
            }
            break;
        }

        case ActionData::BatchDone:
        {
            if (cItem)
            {
                cItem->setDestFileName(ad.destUrl.fileName());
                cItem->setDone();
                addHistoryMessage(cItem, ad.message, DHistoryView::SuccessEntry);
                d->statusProgressBar->setProgressValue(d->statusProgressBar->progressValue() + 1);
            }
            break;
        }

        case ActionData::BatchFailed:
        {
            if (cItem)
            {
                cItem->setFailed();
                addHistoryMessage(cItem, i18n("Failed to process item..."), DHistoryView::ErrorEntry);
                addHistoryMessage(cItem, ad.message, DHistoryView::ErrorEntry);
                d->statusProgressBar->setProgressValue(d->statusProgressBar->progressValue() + 1);
            }
            break;
        }

        case ActionData::BatchCanceled:
        {
            if (cItem)
            {
                cItem->setCanceled();
                addHistoryMessage(cItem, i18n("Process Cancelled..."), DHistoryView::CancelEntry);
                d->statusProgressBar->setProgressValue(d->statusProgressBar->progressValue() + 1);
            }
            break;
        }

        default:         // NONE
        {
            break;
        }
    }
}

void QueueMgrWindow::addHistoryMessage(QueueListViewItem* const cItem, const QString& msg, DHistoryView::EntryType type)
{
    if (cItem)
    {
        int itemId   = cItem->info().id();
        int queueId  = d->queuePool->currentIndex();
        QString text = i18n("Item \"%1\" from queue \"%2\": %3", cItem->info().name(),
                            d->queuePool->queueTitle(queueId), msg);
        d->toolsView->addHistoryEntry(text, type, queueId, itemId);
    }
    else
    {
        d->toolsView->addHistoryEntry(msg, type);
    }
}

void QueueMgrWindow::slotStop()
{
    d->thread->cancel();
    d->queuePool->currentQueue()->cancelItems();
    processingAborted();
}

void QueueMgrWindow::slotQueueProcessed()
{
    if (!d->busy)
    {
        return;
    }

    d->currentQueueToProcess++;
    QString msg;

    if (!d->processingAllQueues) {
        msg = i18n("Batch queue finished");
    }
    else if (d->currentQueueToProcess == d->queuePool->count())
    {
        msg = i18n("All batch queues finished");
    }
    else
    {
        // We will process next queue from the pool.
        processOneQueue();
        return;
    }

    DNotificationWrapper(QLatin1String("batchqueuecompleted"), msg, this,
                         windowTitle());
    processingAborted();
}

void QueueMgrWindow::slotAssignQueueSettings(const QString& title)
{
    if (!title.isEmpty())
    {
        Workflow q                 = WorkflowManager::instance()->findByTitle(title);
        QueueListView* const queue = d->queuePool->currentQueue();

        if (queue)
        {
            queue->setSettings(q.qSettings);
            AssignedBatchTools tools;
            tools.m_toolsList = q.aTools;

            //qCDebug(DIGIKAM_GENERAL_LOG) << tools.m_toolsList;

            queue->setAssignedTools(tools);
            d->queuePool->slotQueueSelected(d->queuePool->currentIndex());
        }
    }
}

void QueueMgrWindow::slotSaveWorkflow()
{
    if (d->queuePool->saveWorkflow())
    {
        d->toolsView->showTab(ToolsView::WORKFLOW);
    }
}

void QueueMgrWindow::customizedFullScreenMode(bool set)
{
    showStatusBarAction()->setEnabled(!set);
    toolBarMenuAction()->setEnabled(!set);
    showMenuBarAction()->setEnabled(!set);
}

}  // namespace Digikam
