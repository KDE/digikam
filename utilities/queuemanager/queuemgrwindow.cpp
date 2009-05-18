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

#include "queuemgrwindow.h"
#include "queuemgrwindow.moc"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kselectaction.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <ktoolinvocation.h>
#include <kwindowsystem.h>
#include <kxmlguifactory.h>
#include <kio/renamedialog.h>

// Libkdcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes

#include "batchtoolsmanager.h"
#include "actionthread.h"
#include "queuepool.h"
#include "queuelist.h"
#include "queuesettingsview.h"
#include "assignedlist.h"
#include "toolsettingsview.h"
#include "toolsview.h"
#include "componentsinfo.h"
#include "digikamapp.h"
#include "themeengine.h"
#include "dimg.h"
#include "dlogoaction.h"
#include "dmetadata.h"
#include "albumsettings.h"
#include "albummanager.h"
#include "loadingcacheinterface.h"
#include "imagewindow.h"
#include "rawcameradlg.h"
#include "imagedialog.h"
#include "thumbnailsize.h"
#include "queuemgrwindow_p.h"

namespace Digikam
{

QueueMgrWindow* QueueMgrWindow::m_instance = 0;

QueueMgrWindow* QueueMgrWindow::queueManagerWindow()
{
    if (!m_instance)
        new QueueMgrWindow();

    return m_instance;
}

bool QueueMgrWindow::queueManagerWindowCreated()
{
    return m_instance;
}

QueueMgrWindow::QueueMgrWindow()
              : KXmlGuiWindow(0), d(new QueueMgrWindowPriv)
{
    qRegisterMetaType<BatchToolSettings>("BatchToolSettings");
    qRegisterMetaType<BatchToolSet>("BatchToolSet");

    m_instance       = this;
    d->batchToolsMgr = new BatchToolsManager(this);
    d->thread        = new ActionThread(this);
    d->progressTimer = new QTimer(this);

    setWindowFlags(Qt::Window);
    setCaption(i18n("Batch Queue Manager"));
    // We don't want to be deleted on close
    setAttribute(Qt::WA_DeleteOnClose, false);

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
    slotAssignedToolsChanged(d->assignedList->assignedList());
}

QueueMgrWindow::~QueueMgrWindow()
{
    m_instance = 0;
    delete d;
}

BatchToolsManager* QueueMgrWindow::batchToolsManager() const
{
    return d->batchToolsMgr;
}

QMap<int, QString> QueueMgrWindow::queuesMap() const
{
    if (d->queuePool)
        return d->queuePool->queuesMap();

    return QMap<int, QString>();
}

bool QueueMgrWindow::isBusy() const
{
    return d->busy;
}

void QueueMgrWindow::closeEvent(QCloseEvent* e)
{
    if (!e) return;

    writeSettings();
    e->accept();
}

void QueueMgrWindow::setupUserArea()
{
    QWidget* mainW     = new QWidget(this);
    QGridLayout *grid  = new QGridLayout(mainW);

    // ------------------------------------------------------------------------------

    QGroupBox *queuesBox = new QGroupBox(i18n("Queues"), mainW);
    QVBoxLayout *vlay1   = new QVBoxLayout(queuesBox);
    d->queuePool         = new QueuePool(queuesBox);
    vlay1->addWidget(d->queuePool);
    vlay1->setSpacing(0);
    vlay1->setMargin(0);

    // ------------------------------------------------------------------------------

    QGroupBox *queueSettingsBox = new QGroupBox(i18n("Queue Settings"), mainW);
    QVBoxLayout *vlay2          = new QVBoxLayout(queueSettingsBox);
    d->queueSettingsView        = new QueueSettingsView(queueSettingsBox);
    vlay2->addWidget(d->queueSettingsView);
    vlay2->setSpacing(0);
    vlay2->setMargin(0);

    // ------------------------------------------------------------------------------

    QGroupBox *toolsBox = new QGroupBox(i18n("Batch Tools Available"), mainW);
    QVBoxLayout *vlay3  = new QVBoxLayout(toolsBox);
    d->toolsView        = new ToolsView(toolsBox);
    vlay3->addWidget(d->toolsView);
    vlay3->setSpacing(0);
    vlay3->setMargin(0);

    // ------------------------------------------------------------------------------

    QGroupBox *assignBox = new QGroupBox(i18n("Assigned Tools"), mainW);
    QVBoxLayout *vlay4   = new QVBoxLayout(assignBox);
    d->assignedList      = new AssignedListView(assignBox);
    vlay4->addWidget(d->assignedList);
    vlay4->setSpacing(0);
    vlay4->setMargin(0);

    // ------------------------------------------------------------------------------

    QGroupBox *toolSettingsBox = new QGroupBox(i18n("Tool Settings"), mainW);
    QVBoxLayout *vlay5         = new QVBoxLayout(toolSettingsBox);
    d->toolSettings            = new ToolSettingsView(toolSettingsBox);
    vlay5->addWidget(d->toolSettings);
    vlay5->setSpacing(0);
    vlay5->setMargin(0);

    // ------------------------------------------------------------------------------

    grid->addWidget(queuesBox,        0, 0, 2, 1);
    grid->addWidget(queueSettingsBox, 2, 0, 2, 1);
    grid->addWidget(toolsBox,         2, 1, 1, 2);
    grid->addWidget(assignBox,        0, 1, 2, 1);
    grid->addWidget(toolSettingsBox,  0, 2, 2, 2);
    grid->setColumnStretch(0, 5);
    grid->setColumnStretch(1, 5);
    grid->setColumnStretch(2, 5);
    grid->setRowStretch(0, 100);
    grid->setRowStretch(2, 70);
    grid->setSpacing(0);
    grid->setMargin(0);

    setCentralWidget(mainW);
}

void QueueMgrWindow::setupStatusBar()
{
    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignCenter);
    d->statusProgressBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->statusProgressBar, 100);
}

void QueueMgrWindow::setupConnections()
{
    // -- Assigned tools list connections -----------------------------------

    connect(d->assignedList, SIGNAL(signalToolSelected(const BatchToolSet&)),
            d->toolSettings, SLOT(slotToolSelected(const BatchToolSet&)));

    connect(d->assignedList, SIGNAL(signalAssignedToolsChanged(const AssignedBatchTools&)),
            d->queuePool, SLOT(slotAssignedToolsChanged(const AssignedBatchTools&)));

    connect(d->toolSettings, SIGNAL(signalSettingsChanged(const BatchToolSet&)),
            d->assignedList, SLOT(slotSettingsChanged(const BatchToolSet&)));

    connect(d->assignedList, SIGNAL(signalAssignedToolsChanged(const AssignedBatchTools&)),
            this, SLOT(slotAssignedToolsChanged(const AssignedBatchTools&)));

    connect(d->toolsView, SIGNAL(signalAssignTools(const QMap<int, QString>&)),
            d->assignedList, SLOT(slotAssignTools(const QMap<int, QString>&)));

    // -- Queued Items list connections -------------------------------------

    connect(d->queuePool, SIGNAL(signalQueueSelected(int, const QueueSettings&, const AssignedBatchTools&)),
            d->queueSettingsView, SLOT(slotQueueSelected(int, const QueueSettings&, const AssignedBatchTools&)));

    connect(d->queuePool, SIGNAL(signalQueueSelected(int, const QueueSettings&, const AssignedBatchTools&)),
            d->assignedList, SLOT(slotQueueSelected(int, const QueueSettings&, const AssignedBatchTools&)));

    connect(d->queueSettingsView, SIGNAL(signalSettingsChanged(const QueueSettings&)),
            d->queuePool, SLOT(slotSettingsChanged(const QueueSettings&)));

    connect(d->queuePool, SIGNAL(signalQueuePoolChanged()),
            this, SLOT(slotQueueContentsChanged()));

    connect(d->queuePool, SIGNAL(signalQueueContentsChanged()),
            this, SLOT(slotQueueContentsChanged()));

    connect(d->queuePool, SIGNAL(signalItemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));

    // -- Multithreaded interface connections -------------------------------

    connect(d->thread, SIGNAL(starting(const ActionData&)),
            this, SLOT(slotAction(const ActionData&)));

    connect(d->thread, SIGNAL(finished(const ActionData&)),
            this, SLOT(slotAction(const ActionData&)));

    // -- GUI connections ---------------------------------------------------

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->queueSettingsView, SLOT(slotUpdateTrackerPos()));

    connect(this, SIGNAL(signalWindowLostFocus()),
            d->queueSettingsView, SLOT(slotHideToolTipTracker()));

    // -- FileWatch connections ------------------------------

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(const QString &)));
}

void QueueMgrWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    d->runAction = new KAction(KIcon("media-playback-start"), i18n("Run"), this);
    d->runAction->setShortcut(Qt::CTRL+Qt::Key_P);
    d->runAction->setEnabled(false);
    connect(d->runAction, SIGNAL(triggered()), this, SLOT(slotRun()));
    actionCollection()->addAction("queuemgr_run", d->runAction);

    d->stopAction = new KAction(KIcon("media-playback-stop"), i18n("Stop"), this);
    d->stopAction->setShortcut(Qt::CTRL+Qt::Key_S);
    d->stopAction->setEnabled(false);
    connect(d->stopAction, SIGNAL(triggered()), this, SLOT(slotStop()));
    actionCollection()->addAction("queuemgr_stop", d->stopAction);

    d->newQueueAction = new KAction(KIcon("bqm-addqueue"), i18n("New Queue"), this);
    connect(d->newQueueAction, SIGNAL(triggered()), d->queuePool, SLOT(slotAddQueue()));
    actionCollection()->addAction("queuemgr_newqueue", d->newQueueAction);

    d->removeQueueAction = new KAction(KIcon("bqm-rmqueue"), i18n("Remove Queue"), this);
    connect(d->removeQueueAction, SIGNAL(triggered()), d->queuePool, SLOT(slotRemoveCurrentQueue()));
    actionCollection()->addAction("queuemgr_removequeue", d->removeQueueAction);

    d->removeItemsSelAction = new KAction(KIcon("list-remove"), i18n("Remove items"), this);
    d->removeItemsSelAction->setShortcut(Qt::CTRL+Qt::Key_K);
    d->removeItemsSelAction->setEnabled(false);
    connect(d->removeItemsSelAction, SIGNAL(triggered()), d->queuePool, SLOT(slotRemoveSelectedItems()));
    actionCollection()->addAction("queuemgr_removeitemssel", d->removeItemsSelAction);

    d->removeItemsDoneAction = new KAction(i18n("Remove processed items"), this);
    d->removeItemsDoneAction->setEnabled(false);
    connect(d->removeItemsDoneAction, SIGNAL(triggered()), d->queuePool, SLOT(slotRemoveItemsDone()));
    actionCollection()->addAction("queuemgr_removeitemsdone", d->removeItemsDoneAction);

    d->clearQueueAction = new KAction(KIcon("edit-clear"), i18n("Clear Queue"), this);
    d->clearQueueAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_K);
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

    d->fullScreenAction = actionCollection()->addAction(KStandardAction::FullScreen,
                          "queuemgr_fullscreen", this, SLOT(slotToggleFullScreen()));

    // -- Standard 'Configure' menu actions ----------------------------------------

    d->showMenuBarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());

    KStandardAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStandardAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // ---------------------------------------------------------------------------------

    d->themeMenuAction = new KSelectAction(i18n("&Themes"), this);
    connect(d->themeMenuAction, SIGNAL(triggered(const QString&)),
            this, SLOT(slotChangeTheme(const QString&)));
    actionCollection()->addAction("theme_menu", d->themeMenuAction);

    d->themeMenuAction->setItems(ThemeEngine::instance()->themeNames());
    slotThemeChanged();

    // -- Standard 'Help' menu actions ---------------------------------------------

    d->donateMoneyAction = new KAction(i18n("Donate Money..."), this);
    connect(d->donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction("queuemgr_donatemoney", d->donateMoneyAction);

    d->contributeAction = new KAction(i18n("Contribute..."), this);
    connect(d->contributeAction, SIGNAL(triggered()), this, SLOT(slotContribute()));
    actionCollection()->addAction("queuemgr_contribute", d->contributeAction);

    d->rawCameraListAction = new KAction(KIcon("kdcraw"), i18n("supported RAW cameras"), this);
    connect(d->rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    actionCollection()->addAction("queuemgr_rawcameralist", d->rawCameraListAction);

    d->libsInfoAction = new KAction(KIcon("help-about"), i18n("Components info"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction("queuemgr_librariesinfo", d->libsInfoAction);

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Keyboard-only actions added to <MainWindow> ------------------------------

    KAction *exitFullscreenAction = new KAction(i18n("Exit Fullscreen mode"), this);
    actionCollection()->addAction("editorwindow_exitfullscreen", exitFullscreenAction);
    exitFullscreenAction->setShortcut( QKeySequence(Qt::Key_Escape) );
    connect(exitFullscreenAction, SIGNAL(triggered()), this, SLOT(slotEscapePressed()));

    // ---------------------------------------------------------------------------------

    d->animLogo = new DLogoAction(this);
    actionCollection()->addAction("logo_action", d->animLogo);

    createGUI("queuemgrwindowui.rc");

    d->showMenuBarAction->setChecked(!menuBar()->isHidden());  // NOTE: workaround for B.K.O #171080
}

void QueueMgrWindow::slotFileChanged(const QString& /*filePath*/)
{
    // TODO
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
    // TODO
}

void QueueMgrWindow::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Batch Queue Manager Settings");
    // TODO
    config->sync();
}

void QueueMgrWindow::applySettings()
{
    AlbumSettings *settings   = AlbumSettings::instance();
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Batch Queue Manager Settings");

    d->thread->setExifSetOrientation(settings->getExifSetOrientation());
    d->queuePool->setEnableToolTips(settings->getShowToolTips());
}

void QueueMgrWindow::refreshStatusBar()
{
    int items       = d->queuePool->currentQueue()->pendingItemsCount();
    int tasks       = d->queuePool->currentQueue()->pendingTasksCount();
    int totalItems  = d->queuePool->totalPendingItems();
    int totalTasks  = d->queuePool->totalPendingTasks();
    QString message = i18n("Current Queue: ");

    switch (items)
    {
        case 0:
            message.append(i18n("No items"));
            break;
        default:
            message.append(i18np("1 item", "%1 items", items));
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

    message.append(" - Total: ");

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

    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, message);

    if (!d->busy)
    {
        bool b = (items != 0) ? true : false;
        d->removeItemsSelAction->setEnabled(b);
        d->removeItemsDoneAction->setEnabled(b);
        d->clearQueueAction->setEnabled(b);
        d->runAction->setEnabled(b && items);
    }
}

void QueueMgrWindow::slotToggleFullScreen()
{
    if (d->fullScreen) // out of fullscreen
    {
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset

        menuBar()->show();
        statusBar()->show();
        showToolBars();

        if (d->removeFullScreenButton)
        {
            QList<KToolBar *> toolbars = toolBars();
            foreach(KToolBar *toolbar, toolbars)
            {
                // name is set in ui.rc XML file
                if (toolbar->objectName() == "ToolBar")
                {
                    toolbar->removeAction(d->fullScreenAction);
                    break;
                }
            }
        }

        d->fullScreen = false;
    }
    else  // go to fullscreen
    {
        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();

        if (d->fullScreenHideToolBar)
        {
            hideToolBars();
        }
        else
        {
            showToolBars();

            QList<KToolBar *> toolbars = toolBars();
            KToolBar *mainToolbar = 0;
            foreach(KToolBar *toolbar, toolbars)
            {
                if (toolbar->objectName() == "ToolBar")
                {
                    mainToolbar = toolbar;
                    break;
                }
            }

            // add fullscreen action if necessary
            if ( mainToolbar && !mainToolbar->actions().contains(d->fullScreenAction) )
            {
                mainToolbar->addAction(d->fullScreenAction);
                d->removeFullScreenButton=true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->removeFullScreenButton=false;
            }
        }

        setWindowState( windowState() | Qt::WindowFullScreen ); // set
        d->fullScreen = true;
    }
}

void QueueMgrWindow::slotEscapePressed()
{
    if (d->fullScreen)
        d->fullScreenAction->activate(QAction::Trigger);
}

void QueueMgrWindow::showToolBars()
{
    QList<KToolBar *> toolbars = toolBars();
    foreach(KToolBar *toolbar, toolbars)
    {
        toolbar->show();
    }
}

void QueueMgrWindow::hideToolBars()
{
    QList<KToolBar *> toolbars = toolBars();
    foreach(KToolBar *toolbar, toolbars)
    {
        toolbar->hide();
    }
}

void QueueMgrWindow::slotDonateMoney()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=donation");
}

void QueueMgrWindow::slotContribute()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=contrib");
}

void QueueMgrWindow::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection( actionCollection(), i18n( "General" ) );
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

void QueueMgrWindow::slotRawCameraList()
{
    RawCameraDlg dlg(this);
    dlg.exec();
}

void QueueMgrWindow::slotThemeChanged()
{
    QStringList themes(ThemeEngine::instance()->themeNames());
    int index = themes.indexOf(AlbumSettings::instance()->getCurrentTheme());
    if (index == -1)
        index = themes.indexOf(i18n("Default"));

    d->themeMenuAction->setCurrentItem(index);
}

void QueueMgrWindow::slotChangeTheme(const QString& theme)
{
    // Theme menu entry is returned with keyboard accelerator. We remove it.
    QString name = theme;
    name.remove(QChar('&'));
    AlbumSettings::instance()->setCurrentTheme(theme);
    ThemeEngine::instance()->slotChangeTheme(theme);
}

void QueueMgrWindow::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void QueueMgrWindow::addNewQueue()
{
    d->queuePool->slotAddQueue();
}

int QueueMgrWindow::currentQueueId()
{
    return (d->queuePool->currentIndex());
}

void QueueMgrWindow::loadImageInfos(const ImageInfoList& list, int queueId)
{
    QueueListView* queue = d->queuePool->findQueueById(queueId);
    if (queue) queue->slotAddItems(list);
}

void QueueMgrWindow::slotQueueContentsChanged()
{
    refreshStatusBar();
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
    BatchToolsList list = d->batchToolsMgr->toolsList();
    foreach(BatchTool *tool, list)
    {
        d->toolsView->addTool(tool);
    }
}

void QueueMgrWindow::slotShowMenuBar()
{
    const bool visible = menuBar()->isVisible();
    menuBar()->setVisible(!visible);
}

void QueueMgrWindow::slotRun()
{
    d->itemsList.clear();
    d->itemsList = d->queuePool->totalPendingItemsList();

    if (d->itemsList.empty())
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

    d->statusProgressBar->setProgressTotalSteps(d->queuePool->totalPendingTasks());
    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);
    d->queueSettingsView->slotHideToolTipTracker();
    busy(true);

    processOne();
}

void QueueMgrWindow::slotStop()
{
    d->progressTimer->stop();

    if (d->currentProcessItem)
        d->currentProcessItem->setCanceled();

    if (d->currentTaskItem)
        d->currentTaskItem->setCanceled();

    d->itemsList.clear();
    d->thread->cancel();
    processingAborted();
}

void QueueMgrWindow::processingAborted()
{
    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode);
    busy(false);
    refreshStatusBar();
}

void QueueMgrWindow::processOne()
{
    if (d->itemsList.empty())
    {
        processingAborted();
        return;
    }

    ItemInfoSet set = d->itemsList.first();
    d->queuePool->setCurrentIndex(set.queueId);
    if (!checkTargetAlbum(set.queueId))
    {
        processingAborted();
        return;
    }

    QueueSettings settings        = d->queuePool->currentQueue()->settings();
    AssignedBatchTools tools4Item = d->queuePool->currentQueue()->assignedTools();
    tools4Item.itemUrl            = set.info.fileUrl();
    QueueListViewItem* item       = d->queuePool->currentQueue()->findItemByUrl(tools4Item.itemUrl);
    if (item)
    {
        d->itemsList.removeFirst();

        if (!tools4Item.toolsMap.isEmpty())
        {
            d->thread->setWorkingUrl(settings.targetUrl);
            d->thread->processFile(tools4Item);
            if (!d->thread->isRunning())
                d->thread->start();
        }
        else
        {
            processOne();
        }
    }
}

void QueueMgrWindow::slotAction(const ActionData& ad)
{
    switch(ad.status)
    {
        case ActionData::BatchStarted:
        {
            processing(ad.fileUrl);
            break;
        }
        case ActionData::BatchDone:
        {
            processed(ad.fileUrl, ad.destUrl);
            processOne();
            break;
        }
        case ActionData::BatchFailed:
        {
            processingFailed(ad.fileUrl);
            processOne();
            break;
        }
        case ActionData::TaskStarted:
        {
            d->assignedList->setCurrentTool(ad.index);
            d->currentTaskItem = d->assignedList->findTool(ad.index);
            d->assignedList->scrollToItem(d->currentTaskItem);
            break;
        }
        case ActionData::TaskDone:
        {
            d->currentTaskItem->setDone();
            d->currentTaskItem = 0;
            d->statusProgressBar->setProgressValue(d->statusProgressBar->progressValue()+1);
            break;
        }
        case ActionData::TaskFailed:
        {
            d->currentTaskItem->setCanceled();
            d->currentTaskItem = 0;
            d->statusProgressBar->setProgressValue(d->statusProgressBar->progressValue()+1);
            break;
        }
        default:    // NONE
        {
            break;
        }
    }
}

void QueueMgrWindow::slotProgressTimerDone()
{
    QPixmap ico(d->progressPix.copy(0, d->progressCount*22, 22, 22));
    if (d->currentProcessItem)
        d->currentProcessItem->setProgressIcon(ico);
    if (d->currentTaskItem)
        d->currentTaskItem->setProgressIcon(ico);

    d->progressCount++;
    if (d->progressCount == 8) d->progressCount = 0;

    d->progressTimer->start(300);
}

void QueueMgrWindow::processing(const KUrl& url)
{
    d->assignedList->reset();
    d->currentProcessItem = d->queuePool->currentQueue()->findItemByUrl(url);
    if (d->currentProcessItem)
    {
        d->currentProcessItem->reset();
        d->queuePool->currentQueue()->setCurrentItem(d->currentProcessItem);
        d->queuePool->currentQueue()->scrollToItem(d->currentProcessItem);
    }

    d->progressCount = 0;
    d->progressTimer->start(300);
}

void QueueMgrWindow::processed(const KUrl& url, const KUrl& tmp)
{
    d->progressTimer->stop();

    QueueSettings settings = d->queuePool->currentQueue()->settings();
    KUrl dest              = settings.targetUrl;
    dest.setFileName(d->currentProcessItem->destFileName());

    if (settings.conflictRule != QueueSettings::OVERWRITE)
    {
        struct stat statBuf;
        if (::stat(QFile::encodeName(dest.path()), &statBuf) == 0)
        {
            KIO::RenameDialog dlg(this, i18n("Save Queued Image from '%1' as",
                                  url.fileName()),
                                  tmp, dest,
                                  KIO::RenameDialog_Mode(KIO::M_SINGLE | KIO::M_OVERWRITE | KIO::M_SKIP));

            switch (dlg.exec())
            {
                case KIO::R_CANCEL:
                {
                    slotStop();
                    return;
                    break;
                }
                case KIO::R_SKIP:
                {
                    dest = KUrl();
                    if (d->currentProcessItem)
                        d->currentProcessItem->setCanceled();
                    break;
                }
                case KIO::R_RENAME:
                {
                    dest = dlg.newDestUrl();
                    break;
                }
                default:    // Overwrite.
                    break;
            }
        }
    }

    if (!dest.isEmpty())
    {
        if (::rename(QFile::encodeName(tmp.path()), QFile::encodeName(dest.path())) != 0)
        {
            KMessageBox::error(this, i18n("Failed to save image %1", dest.fileName()));
            if (d->currentProcessItem)
                d->currentProcessItem->setFailed();
        }
        else
        {
            if (d->currentProcessItem)
            {
                d->currentProcessItem->setDestFileName(dest.fileName());
                d->currentProcessItem->setDone();
            }

            // TODO: assign attributes from original image.
        }
    }

    d->currentProcessItem = 0;
}

void QueueMgrWindow::processingFailed(const KUrl&)
{
    if (d->currentProcessItem)
        d->currentProcessItem->setCanceled();

    if (d->currentTaskItem)
        d->currentTaskItem->setCanceled();

    d->currentProcessItem = 0;
}

void QueueMgrWindow::busy(bool busy)
{
    d->busy = busy;
    d->runAction->setEnabled(!d->busy);
    d->newQueueAction->setEnabled(!d->busy);
    d->removeQueueAction->setEnabled(!d->busy);
    d->removeItemsSelAction->setEnabled(!d->busy);
    d->removeItemsDoneAction->setEnabled(!d->busy);
    d->clearQueueAction->setEnabled(!d->busy);
    d->queuePool->setEnabled(!d->busy);
    d->queueSettingsView->setEnabled(!d->busy);
    d->toolsView->setEnabled(!d->busy);
    d->assignedList->setEnabled(!d->busy);
    d->toolSettings->setEnabled(!d->busy);
    d->stopAction->setEnabled(d->busy);

    // To update status of Tools actions.
    slotAssignedToolsChanged(d->assignedList->assignedList());

    // To update status of Queue items actions.
    slotItemSelectionChanged();

    d->busy ? d->queuePool->setCursor(Qt::WaitCursor) : d->queuePool->unsetCursor();
    d->busy ? d->animLogo->start() : d->animLogo->stop();
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

    switch (tools.toolsMap.count())
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
    QueueListView* queue = d->queuePool->findQueueById(queueId);
    if (!queue)
        return false;

    QString queueName              = d->queuePool->tabText(queueId);
    KUrl    processedItemsAlbumUrl = queue->settings().targetUrl;
    kDebug(50003) << "Target album for queue " << queueName << " is: " << processedItemsAlbumUrl.path() << endl;

    if (processedItemsAlbumUrl.isEmpty())
    {
        KMessageBox::error(this,
                        i18n("Album to host processed items from queue \"%1\" is not set. "
                             "Please select one from Queue Settings panel.", queueName),
                        i18n("Processed items album settings"));
        return false;
    }

    QFileInfo dir(processedItemsAlbumUrl.path());

    if ( !dir.exists() || !dir.isWritable() )
    {
        KMessageBox::error(this,
                        i18n("Album to host processed items from queue \"%1\" "
                             "is not available or not writable. "
                             "Please set another one from Queue Settings panel.", queueName),
                        i18n("Processed items album settings"));
        return false;
    }

    return true;
}

void QueueMgrWindow::moveEvent(QMoveEvent *e)
{
    Q_UNUSED(e)
    emit signalWindowHasMoved();
}

bool QueueMgrWindow::event(QEvent *e)
{
    switch (e->type())
    {
        case QEvent::WindowDeactivate:
            emit signalWindowLostFocus();
            break;
        default:
            break;
    }
    return KXmlGuiWindow::event(e);
}

}  // namespace Digikam
