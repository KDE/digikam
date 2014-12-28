/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : core image editor GUI implementation
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

// Qt includes

#include <QColor>
#include <QPointer>
#include <QRect>
#include <QString>

// KDE includes

#include <kurl.h>
#include <kjob.h>
#include <kprogressdialog.h>

// Local includes

#include "digikam_export.h"
#include "thumbbardock.h"
#include "previewtoolbar.h"
#include "savingcontext.h"
#include "dxmlguiwindow.h"

class QSplitter;

class KSqueezedTextLabel;
class KAction;
class KActionMenu;
class KCategorizedView;
class KSelectAction;
class KToggleAction;
class KToolBarPopupAction;
class KMenu;

namespace Digikam
{

class Canvas;
class DImageHistory;
class EditorTool;
class EditorStackView;
class ExposureSettingsContainer;
class IOFileSettings;
class ImagePluginLoader;
class ICCSettingsContainer;
class Sidebar;
class SidebarSplitter;
class SlideShowSettings;
class StatusProgressBar;
class VersionManager;
class VersionFileOperation;

class DIGIKAM_EXPORT EditorWindow : public DXmlGuiWindow
{
    Q_OBJECT

public:

    explicit EditorWindow(const char* const name);
    ~EditorWindow();

    const static QString CONFIG_GROUP_NAME;

public Q_SLOTS:

    virtual bool setup() = 0;
    virtual bool setupICC() = 0;

Q_SIGNALS:

    void signalSelectionChanged(const QRect&);
    void signalNoCurrentItem();
    void signalPreviewModeChanged(int);
    void signalToolApplied();

protected:

    bool                      m_nonDestructive;
    bool                      m_cancelSlideShow;
    bool                      m_setExifOrientationTag;
    bool                      m_editingOriginalImage;

    KSqueezedTextLabel*       m_resLabel;

    QColor                    m_bgColor;

    SidebarSplitter*          m_splitter;
    QSplitter*                m_vSplitter;

    KAction*                  m_openVersionAction;
    KAction*                  m_saveAction;
    KAction*                  m_saveAsAction;
    KAction*                  m_saveNewVersionAction;
    KAction*                  m_saveCurrentVersionAction;
    KAction*                  m_saveNewVersionAsAction;
    KActionMenu*              m_saveNewVersionInFormatAction;
    KAction*                  m_exportAction;
    KAction*                  m_revertAction;
    KAction*                  m_discardChangesAction;
    KAction*                  m_fileDeleteAction;
    KAction*                  m_forwardAction;
    KAction*                  m_backwardAction;

    KAction*                  m_lastAction;
    KAction*                  m_firstAction;

    KAction*                  m_applyToolAction;
    KAction*                  m_closeToolAction;

    KToggleAction*            m_showBarAction;

    KToolBarPopupAction*      m_undoAction;
    KToolBarPopupAction*      m_redoAction;

    KActionMenu*              m_selectToolsAction;

    KMenu*                    m_contextMenu;
    KMenu*                    m_servicesMenu;
    QAction*                  m_serviceAction;

    EditorStackView*          m_stackView;
    Canvas*                   m_canvas;
    ImagePluginLoader*        m_imagePluginLoader;
    StatusProgressBar*        m_nameLabel;
    IOFileSettings*           m_IOFileSettings;
    QPointer<KProgressDialog> m_savingProgressDialog;

    SavingContext             m_savingContext;

    QString                   m_formatForRAWVersioning;
    QString                   m_formatForSubversions;

protected:

    enum SaveAskMode
    {
        AskIfNeeded,
        OverwriteWithoutAsking,
        AlwaysSaveAs,
        SaveVersionWithoutAsking = OverwriteWithoutAsking,
        AlwaysNewVersion         = AlwaysSaveAs
    };

protected:

    void saveStandardSettings();
    void readStandardSettings();
    void applyStandardSettings();
    void applyIOSettings();
    void applyColorManagementSettings();

    void setupStandardConnections();
    void setupStandardActions();
    void setupStatusBar();
    void setupContextMenu();
    void setupSelectToolsAction();
    void toggleStandardActions(bool val);
    void toggleZoomActions(bool val);
    void toggleNonDestructiveActions();
    void toggleToolActions(EditorTool* tool = 0);

    void printImage(const KUrl& url);

    void unLoadImagePlugins();
    void loadImagePlugins();

    bool promptForOverWrite();

    bool promptUserSave(const KUrl& url, SaveAskMode mode = AskIfNeeded, bool allowCancel = true);
    bool waitForSavingToComplete();
    void startingSave(const KUrl& url);
    bool startingSaveAs(const KUrl& url);
    bool startingSaveCurrentVersion(const KUrl& url);
    bool startingSaveNewVersion(const KUrl& url);
    bool startingSaveNewVersionAs(const KUrl& url);
    bool startingSaveNewVersionInFormat(const KUrl& url, const QString& format);
    bool checkPermissions(const KUrl& url);
    bool checkOverwrite(const KUrl& url);
    bool moveLocalFile(const QString& src, const QString& dest);
    void moveFile();
    void colorManage();
    void execSavingProgressDialog();

    void resetOrigin();
    void resetOriginSwitchFile();

    void addServicesMenuForUrl(const KUrl& url);
    void openWith(const KUrl& url, QAction* action);

    EditorStackView*           editorStackView()  const;
    ExposureSettingsContainer* exposureSettings() const;
    KCategorizedView*          createToolSelectionView();

    VersionFileOperation saveVersionFileOperation(const KUrl& url, bool fork);
    VersionFileOperation saveAsVersionFileOperation(const KUrl& url, const KUrl& saveLocation, const QString& format);
    VersionFileOperation saveInFormatVersionFileOperation(const KUrl& url, const QString& format);


    virtual bool hasOriginalToRestore();
    virtual DImageHistory resolvedImageHistory(const DImageHistory& history);

    virtual void finishSaving(bool success);

    virtual void readSettings();
    virtual void saveSettings();
    virtual void toggleActions(bool val);

    virtual ThumbBarDock* thumbBar() const = 0;
    virtual Sidebar* rightSideBar() const = 0;

    virtual void slideShow(SlideShowSettings& settings) = 0;

    virtual void setupConnections() = 0;
    virtual void setupActions() = 0;
    virtual void setupUserArea() = 0;

    virtual void addServicesMenu() = 0;

    virtual VersionManager* versionManager() const;

    /**
     * Hook method that subclasses must implement to return the destination url
     * of the image to save. This may also be a remote url.
     *
     * This method will only be called while saving.
     *
     * @return destination for the file that is currently being saved.
     */
    virtual KUrl saveDestinationUrl() = 0;

    virtual void saveIsComplete() = 0;
    virtual void saveAsIsComplete() = 0;
    virtual void saveVersionIsComplete() = 0;

protected Q_SLOTS:

    void slotEditKeys();

    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();

    void slotConfToolbars();
    void slotConfNotifications();
    void slotNewToolbarConfig();

    void slotSelected(bool);

    void slotLoadingProgress(const QString& filePath, float progress);
    void slotSavingProgress(const QString& filePath, float progress);

    void slotNameLabelCancelButtonPressed();

    virtual void slotPrepareToLoad();
    virtual void slotLoadingStarted(const QString& filename);
    virtual void slotLoadingFinished(const QString& filename, bool success);
    virtual void slotSavingStarted(const QString& filename);
    virtual void slotFileOriginChanged(const QString& filePath);
    virtual void slotComponentsInfo();
    virtual void slotDiscardChanges();
    virtual void slotOpenOriginal();

    virtual bool saveOrSaveAs();

    virtual bool saveAs() = 0;
    virtual bool save() = 0;
    virtual bool saveNewVersion() = 0;
    virtual bool saveCurrentVersion() = 0;
    virtual bool saveNewVersionAs() = 0;
    virtual bool saveNewVersionInFormat(const QString&) = 0;
    virtual void slotFilePrint() = 0;
    virtual void slotFileWithDefaultApplication() = 0;
    virtual void slotDeleteCurrentItem() = 0;
    virtual void slotBackward() = 0;
    virtual void slotForward() = 0;
    virtual void slotFirst() = 0;
    virtual void slotLast() = 0;
    virtual void slotUpdateItemInfo() = 0;
    virtual void slotChanged() = 0;
    virtual void slotContextMenu() = 0;
    virtual void slotRevert() = 0;
    virtual void slotAddedDropedItems(QDropEvent* e) = 0;
    virtual void slotOpenWith(QAction* action=0) = 0;

private Q_SLOTS:

    void slotSetUnderExposureIndicator(bool);
    void slotSetOverExposureIndicator(bool);
    void slotColorManagementOptionsChanged();
    void slotToggleColorManagedView();
    void slotSoftProofingOptions();
    void slotUpdateSoftProofingState();
    void slotSavingFinished(const QString& filename, bool success);
    void slotToggleSlideShow();
    void slotZoomTo100Percents();
    void slotZoomChanged(bool isMax, bool isMin, double zoom);
    void slotSelectionChanged(const QRect& sel);
    void slotSelectionSetText(const QRect& sel);
    void slotToggleFitToWindow();
    void slotToggleOffFitToWindow();
    void slotFitToSelect();
    void slotIncreaseZoom();
    void slotDecreaseZoom();
    void slotShowMenuBar();
    void slotCloseTool();
    void slotApplyTool();
    void slotKioMoveFinished(KJob* job);
    void slotUndoStateChanged();
    void slotSelectToolsMenuAboutToShow();
    void slotThemeChanged();
    void slotToggleRightSideBar();
    void slotPreviousRightSideBarTab();
    void slotNextRightSideBarTab();

private:

    void enterWaitingLoop();
    void quitWaitingLoop();
    void showSideBars(bool visible);
    void showThumbBar(bool visible);
    void customizedFullScreenMode(bool set);
    bool thumbbarVisibility() const;
    void setColorManagedViewIndicatorToolTip(bool available, bool cmv);
    void setUnderExposureToolTip(bool uei);
    void setOverExposureToolTip(bool oei);

    void setToolStartProgress(const QString& toolName);
    void setToolProgress(int progress);
    void setToolStopProgress();

    void setToolInfoMessage(const QString& txt);

    bool startingSaveVersion(const KUrl& url, bool subversion, bool saveAs, const QString& format);

    void setPreviewModeMask(int mask);
    PreviewToolBar::PreviewMode previewMode() const;

    bool showFileSaveDialog(const KUrl& initialUrl, KUrl& newURL);

    /**
     * Sets up a temp file to save image contents to and updates the saving
     * context to use this file
     *
     * @param url file to save the image to
     */
    void setupTempSaveFile(const KUrl& url);

    /**
     * Returns a list of filters that can be passed to a KFileDialog for all
     * writable image types.
     *
     * @return list of filters for KFileDialog
     */
    QStringList getWritingFilters();

    /**
     * Find the KFileDialog filter that belongs to an extension.
     *
     * @param allFilters list with all filters
     * @param extension the extension to search for
     * @return filter string or empty string if not found
     */
    QString findFilterByExtension(const QStringList& allFilters,
                                  const QString& extension);

    /**
     * Tries to extract a file extension from a KFileDialog filter.
     *
     * @param filter to extract the file extension from
     * @return file extension found in the filter or an empty string if no
     *         extension was found
     */
    QString getExtensionFromFilter(const QString& filter);

    /**
     * Sets the format to use in the saving context. Therefore multiple sources
     * are used starting with the extension found in the save dialog.
     *
     * @param filter filter selected in the dialog
     * @param targetUrl target url selected for the file to save
     * @param autoFilter filter that indicates automatic format selection
     * @return The valid extension which could be found, or a null string
     */
    QString selectValidSavingFormat(const QString& filter,
                                    const KUrl& targetUrl,
                                    const QString& autoFilter);

    void movingSaveFileFinished(bool successful);

    void addAction2ContextMenu(const QString& actionName, bool addDisabled = false);

private:

    class Private;
    Private* const d;

    friend class EditorToolIface;
};

}  // namespace Digikam

#endif /* EDITORWINDOW_H */
