/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window
 *
 * Copyright (C) 2013-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dxmlguiwindow.h"

// Qt includes

#include <QString>
#include <QList>
#include <QMap>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QToolButton>
#include <QEvent>
#include <QHoverEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeySequence>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include <QUrl>
#include <QUrlQuery>
#include <QIcon>
#include <QDir>
#include <QFileInfo>
#include <QResource>
#include <QStandardPaths>

// KDE includes

#include <ktogglefullscreenaction.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kwindowconfig.h>
#include <ksharedconfig.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>

#ifdef HAVE_KNOTIFYCONFIG
#   include <knotifyconfigwidget.h>
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "daboutdata.h"
#include "webbrowserdlg.h"

namespace Digikam
{

class DXmlGuiWindow::Private
{
public:

    Private()
    {
        fsOptions              = FS_NONE;
        fullScreenAction       = 0;
        fullScreenBtn          = 0;
        dirtyMainToolBar       = false;
        fullScreenHideToolBars = false;
        fullScreenHideThumbBar = true;
        fullScreenHideSideBars = false;
        thumbbarVisibility     = true;
        menubarVisibility      = true;
        statusbarVisibility    = true;
        libsInfoAction         = 0;
        showMenuBarAction      = 0;
        showStatusBarAction    = 0;
        about                  = 0;
        dbStatAction           = 0;
        anim                   = 0;
    }

public:

    /**
     * Settings taken from managed window configuration to handle toolbar visibility  in full-screen mode
     */
    bool                     fullScreenHideToolBars;

    /**
     * Settings taken from managed window configuration to handle thumbbar visibility in full-screen mode
     */
    bool                     fullScreenHideThumbBar;

    /**
     * Settings taken from managed window configuration to handle toolbar visibility  in full-screen mode
     */
    bool                     fullScreenHideSideBars;

    /**
     * Full-Screen options. See FullScreenOptions enum and setFullScreenOptions() for details.
     */
    int                      fsOptions;

    /**
     * Action plug in managed window to switch fullscreen state
     */
    KToggleFullScreenAction* fullScreenAction;

    /**
     * Show only if toolbar is hidden
     */
    QToolButton*             fullScreenBtn;

    /**
     * Used by slotToggleFullScreen() to manage state of full-screen button on managed window
     */
    bool                     dirtyMainToolBar;

    /**
     * Store previous visibility of toolbars before ful-screen mode.
     */
    QMap<KToolBar*, bool>    toolbarsVisibility;

    /**
     * Store previous visibility of thumbbar before ful-screen mode.
     */
    bool                     thumbbarVisibility;

    /**
     * Store previous visibility of menubar before ful-screen mode.
     */
    bool                     menubarVisibility;

    /**
     * Store previous visibility of statusbar before ful-screen mode.
     */
    bool                     statusbarVisibility;

    // Common Help actions
    QAction*                 dbStatAction;
    QAction*                 libsInfoAction;
    QAction*                 showMenuBarAction;
    QAction*                 showStatusBarAction;
    DAboutData*              about;
    DLogoAction*             anim;

    QString                  configGroupName;
};

// --------------------------------------------------------------------------------------------------

DXmlGuiWindow::DXmlGuiWindow(QWidget* const parent, Qt::WindowFlags f)
    : KXmlGuiWindow(parent, f),
      d(new Private)
{
    m_expoBlendingAction    = 0;
    m_panoramaAction        = 0;
    m_videoslideshowAction  = 0;
    m_htmlGalleryAction     = 0;
    m_sendByMailAction      = 0;
    m_printCreatorAction    = 0;
    m_calendarAction        = 0;
    m_presentationAction    = 0;
    m_metadataEditAction    = 0;
    m_geolocationEditAction = 0;
    m_animLogo              = 0;

#ifdef HAVE_KSANE
    m_ksaneAction           = 0;
#endif

    installEventFilter(this);
}

DXmlGuiWindow::~DXmlGuiWindow()
{
    delete d;
}

void DXmlGuiWindow::setConfigGroupName(const QString& name)
{
    d->configGroupName = name;
}

QString DXmlGuiWindow::configGroupName() const
{
    return d->configGroupName;
}

void DXmlGuiWindow::closeEvent(QCloseEvent* e)
{
    if(fullScreenIsActive())
        slotToggleFullScreen(false);

    KXmlGuiWindow::closeEvent(e);
}

void DXmlGuiWindow::setFullScreenOptions(int options)
{
    d->fsOptions = options;
}

void DXmlGuiWindow::createHelpActions(bool coreOptions)
{
    d->libsInfoAction = new QAction(QIcon::fromTheme(QLatin1String("help-about")), i18n("Components Information"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction(QLatin1String("help_librariesinfo"), d->libsInfoAction);

    d->about          = new DAboutData(this);

    QAction* const rawCameraListAction = new QAction(QIcon::fromTheme(QLatin1String("image-x-adobe-dng")), i18n("Supported RAW Cameras"), this);
    connect(rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    actionCollection()->addAction(QLatin1String("help_rawcameralist"), rawCameraListAction);

    QAction* const donateMoneyAction   = new QAction(QIcon::fromTheme(QLatin1String("globe")), i18n("Donate..."), this);
    connect(donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction(QLatin1String("help_donatemoney"), donateMoneyAction);

    QAction* const recipesBookAction   = new QAction(QIcon::fromTheme(QLatin1String("globe")), i18n("Recipes Book..."), this);
    connect(recipesBookAction, SIGNAL(triggered()), this, SLOT(slotRecipesBook()));
    actionCollection()->addAction(QLatin1String("help_recipesbook"), recipesBookAction);

    QAction* const contributeAction    = new QAction(QIcon::fromTheme(QLatin1String("globe")), i18n("Contribute..."), this);
    connect(contributeAction, SIGNAL(triggered()), this, SLOT(slotContribute()));
    actionCollection()->addAction(QLatin1String("help_contribute"), contributeAction);

    QAction* const helpAction          = new QAction(QIcon::fromTheme(QLatin1String("help-contents")), i18n("Online Handbook..."), this);
    connect(helpAction, SIGNAL(triggered()), this, SLOT(slotHelpContents()));
    actionCollection()->addAction(QLatin1String("help_handbook"), helpAction);

    m_animLogo = new DLogoAction(this);
    actionCollection()->addAction(QLatin1String("logo_action"), m_animLogo);

    // Add options only for core components (typically all excepted Showfoto)
    if (coreOptions)
    {
        d->dbStatAction = new QAction(QIcon::fromTheme(QLatin1String("network-server-database")), i18n("Database Statistics"), this);
        connect(d->dbStatAction, SIGNAL(triggered()), this, SLOT(slotDBStat()));
        actionCollection()->addAction(QLatin1String("help_dbstat"), d->dbStatAction);
    }
}

void DXmlGuiWindow::cleanupActions()
{
    QAction* ac = actionCollection()->action(QLatin1String("help_about_kde"));
    if (ac) actionCollection()->removeAction(ac);

    ac          = actionCollection()->action(QLatin1String("help_donate"));
    if (ac) actionCollection()->removeAction(ac);

    ac          = actionCollection()->action(QLatin1String("help_contents"));
    if (ac) actionCollection()->removeAction(ac);

/*
    QList<QAction*> lst = actionCollection()->actions();

    foreach(QAction* const act, lst)
        qCDebug(DIGIKAM_WIDGETS_LOG) << "action: " << act->objectName();
*/
}

void DXmlGuiWindow::createSidebarActions()
{
    KActionCollection* const ac = actionCollection();
    QAction* const tlsb = new QAction(i18n("Toggle Left Side-bar"), this);
    connect(tlsb, SIGNAL(triggered()), this, SLOT(slotToggleLeftSideBar()));
    ac->addAction(QLatin1String("toggle-left-sidebar"), tlsb);
    ac->setDefaultShortcut(tlsb, Qt::CTRL + Qt::META + Qt::Key_Left);

    QAction* const trsb = new QAction(i18n("Toggle Right Side-bar"), this);
    connect(trsb, SIGNAL(triggered()), this, SLOT(slotToggleRightSideBar()));
    ac->addAction(QLatin1String("toggle-right-sidebar"), trsb);
    ac->setDefaultShortcut(trsb, Qt::CTRL + Qt::META + Qt::Key_Right);

    QAction* const plsb = new QAction(i18n("Previous Left Side-bar Tab"), this);
    connect(plsb, SIGNAL(triggered()), this, SLOT(slotPreviousLeftSideBarTab()));
    ac->addAction(QLatin1String("previous-left-sidebar-tab"), plsb);
    ac->setDefaultShortcut(plsb, Qt::CTRL + Qt::META + Qt::Key_Home);

    QAction* const nlsb = new QAction(i18n("Next Left Side-bar Tab"), this);
    connect(nlsb, SIGNAL(triggered()), this, SLOT(slotNextLeftSideBarTab()));
    ac->addAction(QLatin1String("next-left-sidebar-tab"), nlsb);
    ac->setDefaultShortcut(nlsb, Qt::CTRL + Qt::META + Qt::Key_End);

    QAction* const prsb = new QAction(i18n("Previous Right Side-bar Tab"), this);
    connect(prsb, SIGNAL(triggered()), this, SLOT(slotPreviousRightSideBarTab()));
    ac->addAction(QLatin1String("previous-right-sidebar-tab"), prsb);
    ac->setDefaultShortcut(prsb, Qt::CTRL + Qt::META + Qt::Key_PageUp);

    QAction* const nrsb = new QAction(i18n("Next Right Side-bar Tab"), this);
    connect(nrsb, SIGNAL(triggered()), this, SLOT(slotNextRightSideBarTab()));
    ac->addAction(QLatin1String("next-right-sidebar-tab"), nrsb);
    ac->setDefaultShortcut(nrsb, Qt::CTRL + Qt::META + Qt::Key_PageDown);
}

void DXmlGuiWindow::createSettingsActions()
{
    d->showMenuBarAction   = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());
#ifdef Q_OS_OSX
    // Under MacOS the menu bar visibility is managed by desktop.
    d->showMenuBarAction->setVisible(false);
#endif

    d->showStatusBarAction = actionCollection()->action(QLatin1String("options_show_statusbar"));

    if (!d->showStatusBarAction)
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "Status bar menu action cannot be found in action collection";

        d->showStatusBarAction = new QAction(i18n("Show Statusbar"), this);
        d->showStatusBarAction->setCheckable(true);
        d->showStatusBarAction->setChecked(true);
        connect(d->showStatusBarAction, SIGNAL(toggled(bool)), this, SLOT(slotShowStatusBar()));
        actionCollection()->addAction(QLatin1String("options_show_statusbar"), d->showStatusBarAction);
    }

    KStandardAction::keyBindings(this,            SLOT(slotEditKeys()),          actionCollection());
    KStandardAction::preferences(this,            SLOT(slotSetup()),             actionCollection());
    KStandardAction::configureToolbars(this,      SLOT(slotConfToolbars()),      actionCollection());

#ifdef HAVE_KNOTIFYCONFIG
    KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()), actionCollection());
#endif
}

QAction* DXmlGuiWindow::showMenuBarAction() const
{
    return d->showMenuBarAction;
}

QAction* DXmlGuiWindow::showStatusBarAction() const
{
    return d->showStatusBarAction;
}

void DXmlGuiWindow::slotShowMenuBar()
{
    menuBar()->setVisible(d->showMenuBarAction->isChecked());
}

void DXmlGuiWindow::slotShowStatusBar()
{
    statusBar()->setVisible(d->showStatusBarAction->isChecked());
}

void DXmlGuiWindow::slotConfNotifications()
{
#ifdef HAVE_KNOTIFYCONFIG
    KNotifyConfigWidget::configure(this);
#endif
}

void DXmlGuiWindow::editKeyboardShortcuts(KActionCollection* const extraac, const QString& actitle)
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection(actionCollection(), i18nc("general keyboard shortcuts", "General"));

    if (extraac)
        dialog.addCollection(extraac, actitle);

    dialog.configure();
}

void DXmlGuiWindow::slotConfToolbars()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());
    saveMainWindowSettings(group);

    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void DXmlGuiWindow::slotNewToolbarConfig()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());
    applyMainWindowSettings(group);
}

void DXmlGuiWindow::createGeolocationEditAction()
{
#ifdef HAVE_MARBLE
    m_geolocationEditAction = new QAction(QIcon::fromTheme(QLatin1String("globe")), i18n("Edit Geolocation..."), this);
    actionCollection()->addAction(QLatin1String("geolocation_edit"), m_geolocationEditAction);
    actionCollection()->setDefaultShortcut(m_geolocationEditAction, Qt::CTRL + Qt::SHIFT + Qt::Key_G);

    connect(m_geolocationEditAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEditGeolocation()));
#endif
}

void DXmlGuiWindow::createMetadataEditAction()
{
    m_metadataEditAction = new QAction(QIcon::fromTheme(QLatin1String("format-text-code")), i18n("Edit Metadata..."), this);
    actionCollection()->addAction(QLatin1String("metadata_edit"), m_metadataEditAction);
    actionCollection()->setDefaultShortcut(m_metadataEditAction, Qt::CTRL + Qt::SHIFT + Qt::Key_M);

    connect(m_metadataEditAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEditMetadata()));
}

void DXmlGuiWindow::createPresentationAction()
{
    m_presentationAction = new QAction(QIcon::fromTheme(QLatin1String("view-presentation")), i18n("Presentation..."), this);

    actionCollection()->addAction(QLatin1String("presentation"), m_presentationAction);
    actionCollection()->setDefaultShortcut(m_presentationAction, Qt::ALT+Qt::SHIFT+Qt::Key_F9);

    connect(m_presentationAction, SIGNAL(triggered()),
            this, SLOT(slotPresentation()));
}

void DXmlGuiWindow::createExpoBlendingAction()
{
    m_expoBlendingAction = new QAction(QIcon::fromTheme(QLatin1String("expoblending")),
                                       i18nc("@action", "Create Stacked Images..."),
                                       this);

    actionCollection()->addAction(QLatin1String("expoblending"), m_expoBlendingAction);

    connect(m_expoBlendingAction, SIGNAL(triggered(bool)),
            this, SLOT(slotExpoBlending()));

}

void DXmlGuiWindow::createPanoramaAction()
{
#ifdef HAVE_PANORAMA
    m_panoramaAction = new QAction(QIcon::fromTheme(QLatin1String("panorama")),
                                    i18nc("@action", "Create panorama..."),
                                    this);
    actionCollection()->addAction(QLatin1String("panorama"), m_panoramaAction);

    connect(m_panoramaAction, SIGNAL(triggered(bool)),
            this, SLOT(slotPanorama()));
#endif
}

void DXmlGuiWindow::createVideoSlideshowAction()
{
#ifdef HAVE_MEDIAPLAYER
    m_videoslideshowAction = new QAction(QIcon::fromTheme(QLatin1String("media-record")),
                                         i18nc("@action", "Create video slideshow..."),
                                         this);
    actionCollection()->addAction(QLatin1String("videoslideshow"), m_videoslideshowAction);

    connect(m_videoslideshowAction, SIGNAL(triggered(bool)),
            this, SLOT(slotVideoSlideshow()));
#endif
}

void DXmlGuiWindow::createCalendarAction()
{
    m_calendarAction = new QAction(QIcon::fromTheme(QLatin1String("view-calendar")),
                                    i18nc("@action", "Create Calendar..."),
                                    this);
    actionCollection()->addAction(QLatin1String("calendar"), m_calendarAction);

    connect(m_calendarAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCalendar()));
}

void DXmlGuiWindow::createSendByMailAction()
{
    m_sendByMailAction = new QAction(QIcon::fromTheme(QLatin1String("mail-send")),
                                    i18nc("@action", "Send by Mail..."),
                                    this);
    actionCollection()->addAction(QLatin1String("sendbymail"), m_sendByMailAction);

    connect(m_sendByMailAction, SIGNAL(triggered(bool)),
            this, SLOT(slotSendByMail()));
}

void DXmlGuiWindow::createPrintCreatorAction()
{
    m_printCreatorAction = new QAction(QIcon::fromTheme(QLatin1String("document-print")),
                                    i18nc("@action", "Print Creator..."),
                                    this);
    actionCollection()->addAction(QLatin1String("printcreator"), m_printCreatorAction);

    connect(m_printCreatorAction, SIGNAL(triggered(bool)),
            this, SLOT(slotPrintCreator()));
}

void DXmlGuiWindow::createHtmlGalleryAction()
{
#ifdef HAVE_HTMLGALLERY
    m_htmlGalleryAction = new QAction(QIcon::fromTheme(QLatin1String("text-html")),
                                      i18nc("@action", "Create Html gallery..."),
                                      this);

    actionCollection()->setDefaultShortcut(m_htmlGalleryAction, Qt::ALT+Qt::SHIFT+Qt::Key_H);
    actionCollection()->addAction(QLatin1String("htmlgallery"), m_htmlGalleryAction);

    connect(m_htmlGalleryAction, SIGNAL(triggered(bool)),
            this, SLOT(slotHtmlGallery()));
#endif
}

void DXmlGuiWindow::createKSaneAction()
{
#ifdef HAVE_KSANE
    m_ksaneAction = new KSaneAction(this);
    actionCollection()->addAction(QLatin1String("import_scan"), m_ksaneAction);

    connect(m_ksaneAction, SIGNAL(triggered(bool)),
            this, SLOT(slotImportFromScanner()));
#endif
}

void DXmlGuiWindow::createFullScreenAction(const QString& name)
{
    d->fullScreenAction = KStandardAction::fullScreen(0, 0, this, this);
    actionCollection()->addAction(name, d->fullScreenAction);
    d->fullScreenBtn    = new QToolButton(this);
    d->fullScreenBtn->setDefaultAction(d->fullScreenAction);
    d->fullScreenBtn->hide();

    connect(d->fullScreenAction, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleFullScreen(bool)));
}

void DXmlGuiWindow::readFullScreenSettings(const KConfigGroup& group)
{
    if (d->fsOptions & FS_TOOLBARS)
        d->fullScreenHideToolBars  = group.readEntry(s_configFullScreenHideToolBarsEntry,  false);

    if (d->fsOptions & FS_THUMBBAR)
        d->fullScreenHideThumbBar = group.readEntry(s_configFullScreenHideThumbBarEntry, true);

    if (d->fsOptions & FS_SIDEBARS)
        d->fullScreenHideSideBars  = group.readEntry(s_configFullScreenHideSideBarsEntry,  false);
}

void DXmlGuiWindow::slotToggleFullScreen(bool set)
{
    KToggleFullScreenAction::setFullScreen(this, set);

    customizedFullScreenMode(set);

    if (!set)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "TURN OFF fullscreen";

        // restore menubar

        if (d->menubarVisibility)
            menuBar()->setVisible(true);

        // restore statusbar

        if (d->statusbarVisibility)
            statusBar()->setVisible(true);

        // restore sidebars

        if ((d->fsOptions & FS_SIDEBARS) && d->fullScreenHideSideBars)
            showSideBars(true);

        // restore thummbbar

        if ((d->fsOptions & FS_THUMBBAR) && d->fullScreenHideThumbBar)
            showThumbBar(d->thumbbarVisibility);

        // restore toolbars and manage full-screen button

        showToolBars(true);
        d->fullScreenBtn->hide();

        if (d->dirtyMainToolBar)
        {
            KToolBar* const mainbar = mainToolBar();

            if (mainbar)
            {
                mainbar->removeAction(d->fullScreenAction);
            }
        }
    }
    else
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "TURN ON fullscreen";

        // hide menubar

#ifdef Q_OS_WIN
        d->menubarVisibility = d->showMenuBarAction->isChecked();
#else
        d->menubarVisibility = menuBar()->isVisible();
#endif
        menuBar()->setVisible(false);

        // hide statusbar

#ifdef Q_OS_WIN
        d->statusbarVisibility = d->showStatusBarAction->isChecked();
#else
        d->statusbarVisibility = statusBar()->isVisible();
#endif
        statusBar()->setVisible(false);

        // hide sidebars

        if ((d->fsOptions & FS_SIDEBARS) && d->fullScreenHideSideBars)
            showSideBars(false);

        // hide thummbbar

        d->thumbbarVisibility = thumbbarVisibility();

        if ((d->fsOptions & FS_THUMBBAR) && d->fullScreenHideThumbBar)
            showThumbBar(false);

        // hide toolbars and manage full-screen button

        if ((d->fsOptions & FS_TOOLBARS) && d->fullScreenHideToolBars)
        {
            showToolBars(false);
        }
        else
        {
            showToolBars(true);

            // add fullscreen action if necessary in toolbar

            KToolBar* const mainbar = mainToolBar();

            if (mainbar && !mainbar->actions().contains(d->fullScreenAction))
            {
                if (mainbar->actions().isEmpty())
                {
                    mainbar->addAction(d->fullScreenAction);
                }
                else
                {
                    mainbar->insertAction(mainbar->actions().first(), d->fullScreenAction);
                }

                d->dirtyMainToolBar = true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->dirtyMainToolBar = false;
            }
        }
    }
}

bool DXmlGuiWindow::fullScreenIsActive() const
{
    if (d->fullScreenAction)
        return d->fullScreenAction->isChecked();

    qCDebug(DIGIKAM_WIDGETS_LOG) << "FullScreenAction is not initialized";
    return false;
}

bool DXmlGuiWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == this)
    {
        if (ev && (ev->type() == QEvent::HoverMove) && fullScreenIsActive())
        {
            // We will handle a stand alone FullScreen button action on top/right corner of screen
            // only if managed window tool bar is hidden, and if we switched already in Full Screen mode.

            KToolBar* const mainbar = mainToolBar();

            if (mainbar)
            {
                if (((d->fsOptions & FS_TOOLBARS) && d->fullScreenHideToolBars) || !mainbar->isVisible())
                {
                    QHoverEvent* const mev = dynamic_cast<QHoverEvent*>(ev);

                    if (mev)
                    {
                        QPoint pos(mev->pos());
                        QRect  desktopRect = QApplication::desktop()->screenGeometry(this);

                        QRect sizeRect(QPoint(0, 0), d->fullScreenBtn->size());
                        QRect topLeft, topRight;
                        QRect topRightLarger;

                        desktopRect        = QRect(desktopRect.y(), desktopRect.y(), desktopRect.width(), desktopRect.height());
                        topLeft            = sizeRect;
                        topRight           = sizeRect;

                        topLeft.moveTo(desktopRect.x(), desktopRect.y());
                        topRight.moveTo(desktopRect.x() + desktopRect.width() - sizeRect.width() - 1, topLeft.y());

                        topRightLarger     = topRight.adjusted(-25, 0, 0, 10);

                        if (topRightLarger.contains(pos))
                        {
                            d->fullScreenBtn->move(topRight.topLeft());
                            d->fullScreenBtn->show();
                        }
                        else
                        {
                            d->fullScreenBtn->hide();
                        }

                        return false;
                    }
                }
            }
        }
    }

    // pass the event on to the parent class
    return QObject::eventFilter(obj, ev);
}

void DXmlGuiWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        if (fullScreenIsActive())
        {
            d->fullScreenAction->activate(QAction::Trigger);
        }
    }
}

KToolBar* DXmlGuiWindow::mainToolBar() const
{
    QList<KToolBar*> toolbars = toolBars();
    KToolBar* mainToolbar     = 0;

    foreach(KToolBar* const toolbar, toolbars)
    {
        if (toolbar && (toolbar->objectName() == QLatin1String("mainToolBar")))
        {
            mainToolbar = toolbar;
            break;
        }
    }

    return mainToolbar;
}

void DXmlGuiWindow::showToolBars(bool visible)
{
    // We will hide toolbars: store previous state for future restoring.
    if (!visible)
    {
        d->toolbarsVisibility.clear();

        foreach(KToolBar* const toolbar, toolBars())
        {
            if (toolbar)
            {
                bool visibility = toolbar->isVisible();
                d->toolbarsVisibility.insert(toolbar, visibility);
            }
        }
    }

    // Switch toolbars visibility
    for (QMap<KToolBar*, bool>::const_iterator it = d->toolbarsVisibility.constBegin(); it != d->toolbarsVisibility.constEnd(); ++it)
    {
        KToolBar* const toolbar = it.key();
        bool visibility         = it.value();

        if (toolbar)
        {
            if (visible && visibility)
                toolbar->show();
            else
                toolbar->hide();
        }
    }

    // We will show toolbars: restore previous state.
    if (visible)
    {
        for (QMap<KToolBar*, bool>::const_iterator it = d->toolbarsVisibility.constBegin(); it != d->toolbarsVisibility.constEnd(); ++it)
        {
            KToolBar* const toolbar = it.key();
            bool visibility         = it.value();

            if (toolbar)
            {
                visibility ? toolbar->show() : toolbar->hide();
            }
        }
    }
}

void DXmlGuiWindow::showSideBars(bool visible)
{
    Q_UNUSED(visible);
}

void DXmlGuiWindow::showThumbBar(bool visible)
{
    Q_UNUSED(visible);
}

void DXmlGuiWindow::customizedFullScreenMode(bool set)
{
    Q_UNUSED(set);
}

bool DXmlGuiWindow::thumbbarVisibility() const
{
    return true;
}

void DXmlGuiWindow::slotHelpContents()
{
    openHandbook();
}

void DXmlGuiWindow::openHandbook()
{
    QUrl url = QUrl(QString::fromUtf8("https://docs.kde.org/trunk5/en/extragear-graphics/%1/index.html")
               .arg(QApplication::applicationName()));

    WebBrowserDlg* const browser = new WebBrowserDlg(url, qApp->activeWindow());
    browser->show();
}

void DXmlGuiWindow::restoreWindowSize(QWindow* const win, const KConfigGroup& group)
{
    KWindowConfig::restoreWindowSize(win, group);
}

void DXmlGuiWindow::saveWindowSize(QWindow* const win, KConfigGroup& group)
{
    KWindowConfig::saveWindowSize(win, group);
}

QAction* DXmlGuiWindow::buildStdAction(StdActionType type, const QObject* const recvr,
                                       const char* const slot, QObject* const parent)
{
    switch(type)
    {
        case StdCopyAction:
            return KStandardAction::copy(recvr, slot, parent);
            break;
        case StdPasteAction:
            return KStandardAction::paste(recvr, slot, parent);
            break;
        case StdCutAction:
            return KStandardAction::cut(recvr, slot, parent);
            break;
        case StdQuitAction:
            return KStandardAction::quit(recvr, slot, parent);
            break;
        case StdCloseAction:
            return KStandardAction::close(recvr, slot, parent);
            break;
        case StdZoomInAction:
            return KStandardAction::zoomIn(recvr, slot, parent);
            break;
        case StdZoomOutAction:
            return KStandardAction::zoomOut(recvr, slot, parent);
            break;
        case StdOpenAction:
            return KStandardAction::open(recvr, slot, parent);
            break;
        case StdSaveAction:
            return KStandardAction::save(recvr, slot, parent);
            break;
        case StdSaveAsAction:
            return KStandardAction::saveAs(recvr, slot, parent);
            break;
        case StdRevertAction:
            return KStandardAction::revert(recvr, slot, parent);
            break;
        case StdBackAction:
            return KStandardAction::back(recvr, slot, parent);
            break;
        case StdForwardAction:
            return KStandardAction::forward(recvr, slot, parent);
            break;
        default:
            return 0;
            break;
    }
}

void DXmlGuiWindow::slotRawCameraList()
{
    showRawCameraList();
}

void DXmlGuiWindow::slotDonateMoney()
{
    WebBrowserDlg* const browser
        = new WebBrowserDlg(QUrl(QLatin1String("https://www.digikam.org/donate/")),
                            qApp->activeWindow());
    browser->show();
}

void DXmlGuiWindow::slotRecipesBook()
{
    WebBrowserDlg* const browser
        = new WebBrowserDlg(QUrl(QLatin1String("https://www.digikam.org/recipes_book/")),
                            qApp->activeWindow());
    browser->show();
}

void DXmlGuiWindow::slotContribute()
{
    WebBrowserDlg* const browser
        = new WebBrowserDlg(QUrl(QLatin1String("https://www.digikam.org/contribute/")),
                            qApp->activeWindow());
    browser->show();
}

void DXmlGuiWindow::setupIconTheme()
{
    // Let QStandardPaths handle this, it will look for app local stuff
    // this means e.g. for mac: "<APPDIR>/../Resources" and for win: "<APPDIR>/data".

    bool hasBreeze                = false;
    const QString breezeIcons     = QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("breeze.rcc"));

    if (!breezeIcons.isEmpty() && QFile::exists(breezeIcons))
    {
        QResource::registerResource(breezeIcons);
        hasBreeze = true;
    }

    bool hasBreezeDark            = false;
    const QString breezeDarkIcons = QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("breeze-dark.rcc"));

    if (!breezeDarkIcons.isEmpty() && QFile::exists(breezeDarkIcons))
    {
        QResource::registerResource(breezeDarkIcons);
        hasBreezeDark = true;
    }

    if (hasBreeze || hasBreezeDark)
    {
        // Tell Qt about the theme
        QIcon::setThemeSearchPaths(QStringList() << QLatin1String(":/icons"));

        // Tell icons loader an co. about the theme
        KConfigGroup cg(KSharedConfig::openConfig(), "Icons");

        if (hasBreeze)
        {
            QIcon::setThemeName(QLatin1String("breeze"));
            cg.writeEntry("Theme", "breeze");
            qCDebug(DIGIKAM_WIDGETS_LOG) << "Breeze icons ressource file found";
        }
        else if (hasBreezeDark)
        {
            QIcon::setThemeName(QLatin1String("breeze-dark"));
            cg.writeEntry("Theme", "breeze-dark");
            qCDebug(DIGIKAM_WIDGETS_LOG) << "Breeze-dark icons ressource file found";
        }
        else
        {
            qCDebug(DIGIKAM_WIDGETS_LOG) << "No icons ressource file found";
        }

        cg.sync();
    }
}

} // namespace Digikam
