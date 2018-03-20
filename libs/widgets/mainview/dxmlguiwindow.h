/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DXML_GUI_WINDOW_H
#define DXML_GUI_WINDOW_H

// Qt includes

#include <QWidget>
#include <QObject>
#include <QAction>
#include <QWindow>

// KDE includes

#include <kxmlguiwindow.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"
#include "digikam_config.h"
#include "dlogoaction.h"

#ifdef HAVE_KSANE
#   include "ksaneaction.h"
#endif

class QEvent;

class KToolBar;

namespace Digikam
{

/** Optional parts which can be hidden or not from managed window configuration panel
 */
enum FullScreenOptions
{
    FS_TOOLBARS   = 0x00000001,                                 /// Manage Tools bar in full-screen mode.
    FS_THUMBBAR   = 0x00000002,                                 /// Manage Thumb bar in full-screen mode.
    FS_SIDEBARS   = 0x00000004,                                 /// Manage Side bars in full-screen mode.
    FS_NONE       = 0x00000008,                                 /// No full-screen options.

    FS_ALBUMGUI   = FS_TOOLBARS | FS_THUMBBAR | FS_SIDEBARS,    /// Album GUI Config.
    FS_EDITOR     = FS_TOOLBARS | FS_THUMBBAR | FS_SIDEBARS,    /// Image Editor Config.
    FS_LIGHTTABLE = FS_TOOLBARS | FS_SIDEBARS,                  /// Light Table Config.
    FS_IMPORTUI   = FS_TOOLBARS | FS_THUMBBAR | FS_SIDEBARS     /// Import UI Config.
};

enum StdActionType
{
    StdCopyAction = 0,
    StdPasteAction,
    StdCutAction,
    StdQuitAction,
    StdCloseAction,
    StdZoomInAction,
    StdZoomOutAction,
    StdOpenAction,
    StdSaveAction,
    StdSaveAsAction,
    StdRevertAction,
    StdBackAction,
    StdForwardAction
};

static const QString s_configFullScreenHideToolBarsEntry(QLatin1String("FullScreen Hide ToolBars"));
static const QString s_configFullScreenHideThumbBarEntry(QLatin1String("FullScreen Hide ThumbBar"));
static const QString s_configFullScreenHideSideBarsEntry(QLatin1String("FullScreen Hide SideBars"));

/** Data container to use in managed window.
 */
class DIGIKAM_EXPORT DXmlGuiWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:

    explicit DXmlGuiWindow(QWidget* const parent=0, Qt::WindowFlags f=KDE_DEFAULT_WINDOWFLAGS);
    ~DXmlGuiWindow();

    /** Manage config group name used by window instance to get/set settings from config file
     */
    void setConfigGroupName(const QString& name);
    QString configGroupName() const;

    /** List of Webservices export actions
     */
    QList<QAction*> exportActions() const;

    /** List of Webservices export actions
     */
    QList<QAction*> importActions() const;

    /** Create Geolocation Edit tool action.
     */
    void createGeolocationEditAction();

    /** Create Metadata Edit tool action.
     */
    void createMetadataEditAction();

    /** Create Presentation tool action.
     */
    void createPresentationAction();

    /** Create Calendar tool action.
     */
    void createCalendarAction();

    /** Create Exposure Blending tool action.
     */
    void createExpoBlendingAction();

    /** Create Panorama tool action.
     */
    void createPanoramaAction();

    /** Create Video Slideshow tool action.
     */
    void createVideoSlideshowAction();

    /** Create HTML Gallery tool action.
     */
    void createHtmlGalleryAction();

    /** Create Print Creator tool action.
     */
    void createPrintCreatorAction();

    /** Create Send by Mail tool action.
     */
    void createSendByMailAction();

    /** Create Media Server action to share through DLNA.
     */
    void createMediaServerAction();

    /** Create common actions to setup all digiKam main windows.
     */
    void createSettingsActions();

    /** Create common actions from Export menu for all digiKam main windows.
     */
    void createExportActions();

    /** Create common actions from Import menu for all digiKam main windows.
     */
    void createImportActions();

    /** Create common actions from Help menu for all digiKam main windows.
     */
    void createHelpActions(bool coreOptions=true);

    /** Cleanup unwanted actions from action collection.
     */
    void cleanupActions();

    /** Create common actions to handle side-bar through keyboard shortcuts.
     */
    void createSidebarActions();

    /** Set full-screen options to managed window
     */
    void setFullScreenOptions(int options);

    /** Create Full-screen action to action collection instance from managed window
     *  set through setManagedWindow(). This action is connected to slotToggleFullScreen() slot.
     *  'name' is action name used in KDE UI rc file.
     */
    void createFullScreenAction(const QString& name);

    /** Read full-screen settings fr    void slotConfNotifications();om KDE config file.
     */
    void readFullScreenSettings(const KConfigGroup& group);

    /** Return true if managed window is currently in Full Screen Mode.
     */
    bool fullScreenIsActive() const;

    static void openHandbook();
    static void restoreWindowSize(QWindow* const win, const KConfigGroup& group);
    static void saveWindowSize(QWindow* const win, KConfigGroup& group);

    static QAction* buildStdAction(StdActionType type,
                                   const QObject* const recvr,
                                   const char* const slot,
                                   QObject* const parent);

    /**
     * If we have some local breeze icon resource, prefer it.
     */
    static void setupIconTheme();

protected:

    DLogoAction* m_animLogo;

    QAction*     m_metadataEditAction;
    QAction*     m_geolocationEditAction;
    QAction*     m_presentationAction;
    QAction*     m_calendarAction;
    QAction*     m_htmlGalleryAction;
    QAction*     m_printCreatorAction;
    QAction*     m_sendByMailAction;
    QAction*     m_expoBlendingAction;
    QAction*     m_panoramaAction;
    QAction*     m_videoslideshowAction;
    QAction*     m_mediaServerAction;

    QAction*     m_exportDropboxAction;
    QAction*     m_exportFacebookAction;
    QAction*     m_exportFlickrAction;
    QAction*     m_exportGdriveAction;
    QAction*     m_exportGphotoAction;
    QAction*     m_exportImageshackAction;
    QAction*     m_exportImgurAction;
    QAction*     m_exportPiwigoAction;
    QAction*     m_exportRajceAction;
    QAction*     m_exportSmugmugAction;
    QAction*     m_exportYandexfotkiAction;

#ifdef HAVE_MEDIAWIKI
    QAction*     m_exportMediawikiAction;
#endif

#ifdef HAVE_VKONTAKTE
    QAction*     m_exportVkontakteAction;
#endif

#ifdef HAVE_KIO
    QAction*     m_exportFileTransferAction;
#endif

    QAction*     m_importGphotoAction;
    QAction*     m_importSmugmugAction;

#ifdef HAVE_KIO
    QAction*     m_importFileTransferAction;
#endif

#ifdef HAVE_KSANE
    KSaneAction* m_ksaneAction;
#endif

protected:

    QAction* showMenuBarAction()   const;
    QAction* showStatusBarAction() const;

    /** Call this method from your main window to show keyboard shortcut config dialog
     *  with an extra action collection to configure. This method is called by slotEditKeys()
     *  which can be re-implement in child class for cutomization.
     */
    void editKeyboardShortcuts(KActionCollection* const extraac=0, const QString& actitle=QString());

    void closeEvent(QCloseEvent* e);
    void keyPressEvent(QKeyEvent* e);
    bool eventFilter(QObject* obj, QEvent* ev);

    /** Re-implement this method if you want to manage sidebars visibility in full-screen mode.
     *  By default this method do nothing.
     */
    virtual void showSideBars(bool visible);

    /** Re-implement this method if you want to manage thumbbar visibility in full-screen mode.
     *  By default this method do nothing.
     */
    virtual void showThumbBar(bool visible);

    /** Re-implement this method if you want to manage customized view visibility in full-screen mode.
     *  This method is called by switchWindowToFullScreen(). By default this method do nothing.
     */
    virtual void customizedFullScreenMode(bool set);

    /** Re-implement this method if managed window has a thumbbar. This must return visibility state of it.
     */
    virtual bool thumbbarVisibility() const;

private Q_SLOTS:

    void slotToggleFullScreen(bool);
    void slotShowMenuBar();
    void slotShowStatusBar();
    void slotConfNotifications();
    void slotConfToolbars();
    void slotNewToolbarConfig();

    void slotRawCameraList();
    void slotDonateMoney();
    void slotRecipesBook();
    void slotContribute();
    void slotHelpContents();

    // Slots for common Help Actions
    virtual void slotComponentsInfo()          {};
    virtual void slotDBStat()                  {};

    // Slots for common Sidebar Actions
    virtual void slotToggleLeftSideBar()       {};
    virtual void slotToggleRightSideBar()      {};
    virtual void slotPreviousLeftSideBarTab()  {};
    virtual void slotNextLeftSideBarTab()      {};
    virtual void slotPreviousRightSideBarTab() {};
    virtual void slotNextRightSideBarTab()     {};

    // Slots for common Settings actions
    virtual void slotEditKeys()                { editKeyboardShortcuts(); };
    virtual void slotSetup() = 0;

    // Called by KSane action.
    virtual void slotImportFromScanner()       {};

    // Called by Metadata Edit tool.
    virtual void slotEditMetadata()            {};

    // Called by Geolocation Edit tool.
    virtual void slotEditGeolocation()         {};

    // Called by Presentation tool.
    virtual void slotPresentation()            {};

    // Called by SendByMail tool.
    virtual void slotSendByMail()              {};

    // Called by PrintCreator tool.
    virtual void slotPrintCreator()            {};

    // Called by HTML Gallery tool.
    virtual void slotHTMLGallery()             {};

    // Called by Calendar tool.
    virtual void slotCalendar()                {};

    // Called by Video Slideshow tool.
    virtual void slotVideoSlideshow()          {};

    // Called by Panorama tool.
    virtual void slotPanorama()                {};

    // Called by Exposure Blending tool.
    virtual void slotExpoBlending()            {};

    // Called by Media Server tool.
    virtual void slotMediaServer()             {};

    // Called by Webservices Export tools.
    virtual void slotExportTool()              {};

    // Called by Webservices Import tools.
    virtual void slotImportTool()              {};

private:

    /** Used by slotToggleFullScreen() to switch tool-bar visibility in managed window
     */
    void showToolBars(bool visible);

    /** Return main tool bar instance created in managed window.
     */
    KToolBar* mainToolBar() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DXML_GUI_WINDOW_H
