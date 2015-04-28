/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager GUI
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "queuemgrwindow.moc"
#include "queuemgrwindow_p.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <knotifyconfigwidget.h>
#include <kselectaction.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktogglefullscreenaction.h>
#include <ktoolbar.h>
#include <ktoolinvocation.h>
#include <kwindowsystem.h>
#include <kxmlguifactory.h>
#include <kdebug.h>

// Libkdcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

// Local includes

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
#include "imagedialog.h"
#include "thumbnailsize.h"
#include "sidebar.h"
#include "uifilevalidator.h"
#include "dnotificationwrapper.h"
#include "scancontroller.h"

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
    : DXmlGuiWindow(0), d(new Private)
{
    setXMLFile("queuemgrwindowui.rc");

    // --------------------------------------------------------

    UiFileValidator validator(localXMLFile());

    if (!validator.isValid())
    {
        validator.fixConfigFile();
    }

    // --------------------------------------------------------

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
    setAutoSaveSettings("Batch Queue Manager Settings", true);

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
    vlay1->setSpacing(0);
    vlay1->setMargin(0);

    // ------------------------------------------------------------------------------

    QGroupBox* const queueSettingsBox = new QGroupBox(i18n("Queue Settings"), mainW);
    QVBoxLayout* const vlay2          = new QVBoxLayout(queueSettingsBox);
    d->queueSettingsView              = new QueueSettingsView(queueSettingsBox);
    vlay2->addWidget(d->queueSettingsView);
    vlay2->setSpacing(0);
    vlay2->setMargin(0);

    // ------------------------------------------------------------------------------

    QGroupBox* const toolsBox = new QGroupBox(i18n("Control Panel"), mainW);
    QVBoxLayout* const vlay3  = new QVBoxLayout(toolsBox);
    d->toolsView              = new ToolsView(toolsBox);
    vlay3->addWidget(d->toolsView);
    vlay3->setSpacing(0);
    vlay3->setMargin(0);

    // ------------------------------------------------------------------------------

    QGroupBox* const assignBox = new QGroupBox(i18n("Assigned Tools"), mainW);
    QVBoxLayout* const vlay4   = new QVBoxLayout(assignBox);
    d->assignedList            = new AssignedListView(assignBox);
    vlay4->addWidget(d->assignedList);
    vlay4->setSpacing(0);
    vlay4->setMargin(0);

    // ------------------------------------------------------------------------------

    QGroupBox* const toolSettingsBox = new QGroupBox(i18n("Tool Settings"), mainW);
    QVBoxLayout* const vlay5         = new QVBoxLayout(toolSettingsBox);
    d->toolSettings                  = new ToolSettingsView(toolSettingsBox);
    vlay5->addWidget(d->toolSettings);
    vlay5->setSpacing(0);
    vlay5->setMargin(0);

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
    d->statusProgressBar->setNotificationTitle(i18n("Batch Queue Manager"), KIcon("bqm-diff").pixmap(22));
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

    d->runAction = new KAction(KIcon("media-playback-start"), i18n("Run"), this);
    d->runAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_P));
    d->runAction->setEnabled(false);
    connect(d->runAction, SIGNAL(triggered()), this, SLOT(slotRun()));
    actionCollection()->addAction("queuemgr_run", d->runAction);

    d->stopAction = new KAction(KIcon("media-playback-stop"), i18n("Stop"), this);
    d->stopAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_S));
    d->stopAction->setEnabled(false);
    connect(d->stopAction, SIGNAL(triggered()), this, SLOT(slotStop()));
    actionCollection()->addAction("queuemgr_stop", d->stopAction);

    d->newQueueAction = new KAction(KIcon("bqm-addqueue"), i18n("New Queue"), this);
    connect(d->newQueueAction, SIGNAL(triggered()), d->queuePool, SLOT(slotAddQueue()));
    actionCollection()->addAction("queuemgr_newqueue", d->newQueueAction);

    d->removeQueueAction = new KAction(KIcon("bqm-rmqueue"), i18n("Remove Queue"), this);
    connect(d->removeQueueAction, SIGNAL(triggered()), d->queuePool, SLOT(slotRemoveCurrentQueue()));
    actionCollection()->addAction("queuemgr_removequeue", d->removeQueueAction);

    // TODO rename action to saveWorkflowAction to avoid confusion?
    d->saveQueueAction = new KAction(KIcon("document-save"), i18n("Save Workflow"), this);
    connect(d->saveQueueAction, SIGNAL(triggered()), this, SLOT(slotSaveWorkflow()));
    actionCollection()->addAction("queuemgr_savequeue", d->saveQueueAction);

    d->removeItemsSelAction = new KAction(KIcon("list-remove"), i18n("Remove items"), this);
    d->removeItemsSelAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_K));
    d->removeItemsSelAction->setEnabled(false);
    connect(d->removeItemsSelAction, SIGNAL(triggered()), d->queuePool, SLOT(slotRemoveSelectedItems()));
    actionCollection()->addAction("queuemgr_removeitemssel", d->removeItemsSelAction);

    d->removeItemsDoneAction = new KAction(i18n("Remove processed items"), this);
    d->removeItemsDoneAction->setEnabled(false);
    connect(d->removeItemsDoneAction, SIGNAL(triggered()), d->queuePool, SLOT(slotRemoveItemsDone()));
    actionCollection()->addAction("queuemgr_removeitemsdone", d->removeItemsDoneAction);

    d->clearQueueAction = new KAction(KIcon("edit-clear"), i18n("Clear Queue"), this);
    d->clearQueueAction->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_K));
    d->clearQueueAction->setEnabled(false);
    connect(d->clearQueueAction, SIGNAL(triggered()), d->queuePool, SLOT(slotClearList()));
    actionCollection()->addAction("queuemgr_clearlist", d->clearQueueAction);

    actionCollection()->addAction(KStandardAction::Close, "queuemgr_close",
                                  this, SLOT(close()));

    // -- 'Tools' menu actions -----------------------------------------------------

    d->moveUpToolAction = new KAction(KIcon("bqm-commit"), i18n("Move up"), this);
    connect(d->moveUpToolAction, SIGNAL(triggered()), d->assignedList, SLOT(slotMoveCurrentToolUp()));
    actionCollection()->addAction("queuemgr_toolup", d->moveUpToolAction);

    d->moveDownToolAction = new KAction(KIcon("bqm-update"), i18n("Move down"), this);
    connect(d->moveDownToolAction, SIGNAL(triggered()), d->assignedList, SLOT(slotMoveCurrentToolDown()));
    actionCollection()->addAction("queuemgr_tooldown", d->moveDownToolAction);

    d->removeToolAction = new KAction(KIcon("bqm-remove"), i18n("Remove tool"), this);
    connect(d->removeToolAction, SIGNAL(triggered()), d->assignedList, SLOT(slotRemoveCurrentTool()));
    actionCollection()->addAction("queuemgr_toolremove", d->removeToolAction);

    d->clearToolsAction = new KAction(KIcon("edit-clear-list"), i18n("Clear List"), this);
    connect(d->clearToolsAction, SIGNAL(triggered()), d->assignedList, SLOT(slotClearToolsList()));
    actionCollection()->addAction("queuemgr_toolsclear", d->clearToolsAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    createFullScreenAction("queuemgr_fullscreen");

    // -- Standard 'Configure' menu actions ----------------------------------------

    d->showMenuBarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());

    KStandardAction::keyBindings(this,            SLOT(slotEditKeys()),          actionCollection());
    KStandardAction::configureToolbars(this,      SLOT(slotConfToolbars()),      actionCollection());
    KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()), actionCollection());
    KStandardAction::preferences(this,            SLOT(slotSetup()),             actionCollection());

    // ---------------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    // -- Standard 'Help' menu actions ---------------------------------------------

    createHelpActions();
        
    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // ---------------------------------------------------------------------------------

    createGUI(xmlFile());

    d->showMenuBarAction->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080
}

void QueueMgrWindow::refreshView()
{
    // NOTE: method called when something is changed from Database (tags, rating, etc...).
    //       There is nothing to do for the moment.
}

void QueueMgrWindow::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Batch Queue Manager Settings");

    d->verticalSplitter->restoreState(group, d->VERTICAL_SPLITTER_CONFIG_KEY);
    d->bottomSplitter->restoreState(group,   d->BOTTOM_SPLITTER_CONFIG_KEY);
    d->topSplitter->restoreState(group,      d->TOP_SPLITTER_CONFIG_KEY);

    readFullScreenSettings(group);
}

void QueueMgrWindow::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Batch Queue Manager Settings");

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

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Batch Queue Manager Settings");
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

    message.append(" / ");

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

    message.append(" / ");

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
        d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, i18n("Ready"));
        d->removeItemsSelAction->setEnabled(items > 0);
        d->removeItemsDoneAction->setEnabled((items - pendingItems) > 0);
        d->clearQueueAction->setEnabled(items > 0);
        d->runAction->setEnabled((tasks > 0) && (pendingItems > 0));
    }
}

void QueueMgrWindow::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection(actionCollection(), i18n("General"));
    dialog.configure();
}

void QueueMgrWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("Batch Queue Manager Settings"));
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void QueueMgrWindow::slotConfNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void QueueMgrWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group("Batch Queue Manager Settings"));
}

void QueueMgrWindow::slotSetup()
{
    setup(Setup::LastPageUsed);
}

void QueueMgrWindow::setup(Setup::Page page)
{
    Setup::exec(this, page);
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
        int result = KMessageBox::warningYesNo(this,
                                               i18n("Batch Queue Manager is running. Do you want to cancel current job?"),
                                               i18n("Processing under progress"));

        if (result == KMessageBox::Yes)
        {
            slotStop();
        }
        else if (result == KMessageBox::No)
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

void QueueMgrWindow::slotShowMenuBar()
{
    menuBar()->setVisible(d->showMenuBarAction->isChecked());
}

void QueueMgrWindow::slotRun()
{
    d->currentQueueToProcess = 0;

    if (!d->queuePool->totalPendingItems())
    {
        KMessageBox::error(this, i18n("There are no items to process in the queues."));
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
    d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);
    d->toolsView->showTab(ToolsView::HISTORY);
    busy(true);

    processOneQueue();
}

void QueueMgrWindow::processingAborted()
{
    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode);
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
    d->busy ? ScanController::instance()->suspendCollectionScan()
            : ScanController::instance()->resumeCollectionScan();
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
        KUrl    processedItemsAlbumUrl = queue->settings().workingUrl;
        kDebug() << "Target album for queue " << queueName << " is: " << processedItemsAlbumUrl.toLocalFile();

        if (processedItemsAlbumUrl.isEmpty())
        {
            KMessageBox::error(this,
                            i18n("Album to host processed items from queue \"%1\" is not set. "
                                    "Please select one from Queue Settings panel.", queueName),
                            i18n("Processed items album settings"));
            return false;
        }

        QFileInfo dir(processedItemsAlbumUrl.toLocalFile());

        if (!dir.exists() || !dir.isWritable())
        {
            KMessageBox::error(this,
                            i18n("Album to host processed items from queue \"%1\" "
                                    "is not available or not writable. "
                                    "Please set another one from Queue Settings panel.", queueName),
                            i18n("Processed items album settings"));
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
            cItem->setDestFileName(ad.destUrl.fileName());
            cItem->setDone();
            addHistoryMessage(cItem, ad.message, DHistoryView::SuccessEntry);
            d->statusProgressBar->setProgressValue(d->statusProgressBar->progressValue() + 1);
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
        int queueId  = d->queuePool->currentIndex();
        int itemId   = cItem->info().id();
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

    if (d->currentQueueToProcess == d->queuePool->count())
    {
        // Pop-up a message to bring user when all is done.
        DNotificationWrapper("batchqueuecompleted", i18n("Batch queue finished"),
                             this, windowTitle());

        processingAborted();
        return;
    }
    else
    {
        // We will process next queue from the pool.
        processOneQueue();
    }
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

            //kDebug() << tools.m_toolsList;

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
    statusBarMenuAction()->setEnabled(!set);
    toolBarMenuAction()->setEnabled(!set);
    d->showMenuBarAction->setEnabled(!set);
}

}  // namespace Digikam
