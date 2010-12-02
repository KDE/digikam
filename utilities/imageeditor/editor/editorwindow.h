/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : main image editor GUI implementation
 *
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include <kxmlguiwindow.h>
#include <kurl.h>
#include <kjob.h>
#include <kprogressdialog.h>

// Local includes

#include "digikam_export.h"
#include "thumbbardock.h"
#include "previewtoolbar.h"
#include "savingcontextcontainer.h"

class QSplitter;
class QLabel;

class KAction;
class KSelectAction;
class KToggleAction;
class KToolBarPopupAction;

namespace Digikam
{

class Canvas;
class DImageHistory;
class DPopupMenu;
class DLogoAction;
class EditorStackView;
class ExposureSettingsContainer;
class IOFileSettingsContainer;
class ImagePluginLoader;
class ICCSettingsContainer;
class Sidebar;
class SidebarSplitter;
class SlideShowSettings;
class StatusProgressBar;
class ThumbBarView;
class VersionManager;
class VersionFileOperation;

class DIGIKAM_EXPORT EditorWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:

    EditorWindow(const char* name);
    ~EditorWindow();

    virtual bool setup()=0;
    virtual bool setupICC()=0;

    const static QString CONFIG_GROUP_NAME;

Q_SIGNALS:

    void signalSelectionChanged(const QRect&);
    void signalNoCurrentItem();
    void signalPreviewModeChanged(int);
    void signalToolApplied();

protected:

    bool                     m_nonDestructive;

    bool                     m_fullScreenHideThumbBar;
    bool                     m_cancelSlideShow;
    bool                     m_fullScreen;
    bool                     m_rotatedOrFlipped;
    bool                     m_setExifOrientationTag;
    bool                     m_editingOriginalImage;

    QLabel*                  m_resLabel;

    QColor                   m_bgColor;

    SidebarSplitter*         m_splitter;
    QSplitter*               m_vSplitter;

    KAction*                 m_openVersionAction;
    KAction*                 m_saveAction;
    KAction*                 m_saveAsAction;
    KAction*                 m_saveNewVersionAction;
    KAction*                 m_saveCurrentVersionAction;
    KAction*                 m_exportAction;
    KAction*                 m_revertAction;
    KAction*                 m_discardChangesAction;
    KAction*                 m_fileDeleteAction;
    KAction*                 m_forwardAction;
    KAction*                 m_backwardAction;
    KAction*                 m_fullScreenAction;

    KAction*                 m_lastAction;
    KAction*                 m_firstAction;

    KAction*                 m_applyToolAction;
    KAction*                 m_closeToolAction;

    KSelectAction*           m_themeMenuAction;

    KToggleAction*           m_showBarAction;

    KToolBarPopupAction*     m_undoAction;
    KToolBarPopupAction*     m_redoAction;

    DLogoAction*             m_animLogo;
    DPopupMenu*              m_contextMenu;
    EditorStackView*         m_stackView;
    Canvas*                  m_canvas;
    ImagePluginLoader*       m_imagePluginLoader;
    StatusProgressBar*       m_nameLabel;
    IOFileSettingsContainer* m_IOFileSettings;
    QPointer<KProgressDialog> m_savingProgressDialog;

    SavingContextContainer   m_savingContext;

    QString                  m_formatForRAWVersioning;
    QString                  m_formatForSubversions;

protected:

    void saveStandardSettings();
    void readStandardSettings();
    void applyStandardSettings();

    void setupStandardConnections();
    void setupStandardActions();
    void setupStatusBar();
    void setupContextMenu();
    void toggleStandardActions(bool val);
    void toggleZoomActions(bool val);
    void toggleNonDestructiveActions();
    void toggleToolActions(bool val);

    void printImage(const KUrl& url);

    void unLoadImagePlugins();
    void loadImagePlugins();

    bool promptForOverWrite();
    virtual bool hasChangesToSave();
    virtual bool hasOriginalToRestore();
    virtual DImageHistory resolvedImageHistory(const DImageHistory& history);

    enum SaveAskMode
    {
        AskIfNeeded,
        OverwriteWithoutAsking,
        AlwaysSaveAs,
        SaveVersionWithoutAsking = OverwriteWithoutAsking,
        AlwaysNewVersion = AlwaysSaveAs
    };

    bool promptUserSave(const KUrl& url, SaveAskMode mode = AskIfNeeded, bool allowCancel = true);
    bool waitForSavingToComplete();
    void startingSave(const KUrl& url);
    bool startingSaveAs(const KUrl& url);
    bool startingSaveCurrentVersion(const KUrl& url);
    bool startingSaveNewVersion(const KUrl& url);
    bool checkPermissions(const KUrl& url);
    bool moveLocalFile(const QString& src, const QString& dest, bool destinationExisted);
    void moveFile();
    void colorManage();
    void execSavingProgressDialog();

    EditorStackView*           editorStackView()  const;
    ExposureSettingsContainer* exposureSettings() const;
    ICCSettingsContainer*      cmSettings()       const;

    virtual void finishSaving(bool success);

    virtual void readSettings();
    virtual void saveSettings();
    virtual void toggleActions(bool val);

    void toggleGUI2FullScreen();

    virtual ThumbBarDock* thumbBar() const=0;
    virtual Sidebar* rightSideBar() const=0;

    virtual void slideShow(bool startWithCurrent, SlideShowSettings& settings)=0;

    virtual void setupConnections()=0;
    virtual void setupActions()=0;
    virtual void setupUserArea()=0;
    virtual bool saveAs()=0;
    virtual bool save()=0;
    virtual bool saveNewVersion()=0;
    virtual bool saveCurrentVersion()=0;

    virtual VersionManager* versionManager();
    VersionFileOperation savingVersionFileInfo(const KUrl& url, bool fork);

    /**
     * Hook method that subclasses must implement to return the destination url
     * of the image to save. This may also be a remote url.
     *
     * This method will only be called while saving.
     *
     * @return destination for the file that is currently being saved.
     */
    virtual KUrl saveDestinationUrl() = 0;

    virtual void saveIsComplete()=0;
    virtual void saveAsIsComplete()=0;
    virtual void saveVersionIsComplete()=0;

protected Q_SLOTS:

    void slotSave();
    void slotSaveAs();
    void slotSaveCurrentVersion();
    void slotSaveNewVersion();

    void slotEditKeys();

    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();

    void slotConfToolbars();
    void slotConfNotifications();
    void slotNewToolbarConfig();

    void slotToggleFullScreen();
    void slotEscapePressed();

    void slotSelected(bool);

    void slotLoadingProgress(const QString& filePath, float progress);
    void slotSavingProgress(const QString& filePath, float progress);

    void slotNameLabelCancelButtonPressed();

    void slotThemeChanged();

    virtual void slotPrepareToLoad();
    virtual void slotLoadingStarted(const QString& filename);
    virtual void slotLoadingFinished(const QString& filename, bool success);
    virtual void slotSavingStarted(const QString& filename);

    virtual void slotSetup()
    {
        setup();
    };
    virtual void slotChangeTheme(const QString& theme);

    virtual void slotComponentsInfo();

    virtual void slotFilePrint()=0;
    virtual void slotDeleteCurrentItem()=0;
    virtual void slotBackward()=0;
    virtual void slotForward()=0;
    virtual void slotFirst()=0;
    virtual void slotLast()=0;
    virtual void slotUpdateItemInfo()=0;
    virtual void slotChanged()=0;
    virtual void slotContextMenu()=0;
    virtual void slotRevert()=0;
    virtual void slotDiscardChanges();
    virtual void slotOpenOriginal();

private Q_SLOTS:

    void slotSetUnderExposureIndicator(bool);
    void slotSetOverExposureIndicator(bool);
    void slotColorManagementOptionsChanged();
    void slotToggleColorManagedView();
    void slotSoftProofingOptions();
    void slotUpdateSoftProofingState();
    void slotRotatedOrFlipped();
    void slotSavingFinished(const QString& filename, bool success);
    void slotDonateMoney();
    void slotContribute();
    void slotToggleSlideShow();
    void slotZoomTo100Percents();
    void slotZoomChanged(bool isMax, bool isMin, double zoom);
    void slotSelectionChanged(const QRect& sel);
    void slotToggleFitToWindow();
    void slotToggleOffFitToWindow();
    void slotFitToSelect();
    void slotIncreaseZoom();
    void slotDecreaseZoom();
    void slotRawCameraList();
    void slotShowMenuBar();
    void slotCloseTool();
    void slotApplyTool();
    void slotKioMoveFinished(KJob* job);
    void slotUndoStateChanged(bool, bool, bool);

private:

    void enterWaitingLoop();
    void quitWaitingLoop();
    void hideToolBars();
    void showToolBars();
    void setColorManagedViewIndicatorToolTip(bool available, bool cmv);
    void setUnderExposureToolTip(bool uei);
    void setOverExposureToolTip(bool oei);

    void setToolStartProgress(const QString& toolName);
    void setToolProgress(int progress);
    void setToolStopProgress();

    void setToolInfoMessage(const QString& txt);

    bool startingSaveVersion(const KUrl& url, bool subversion);

    void setPreviewModeMask(int mask);
    PreviewToolBar::PreviewMode previewMode();

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
     * @return true if a valid extension could be found, else false
     */
    bool selectValidSavingFormat(const QString& filter,
                                 const KUrl& targetUrl,
                                 const QString& autoFilter);

    void movingSaveFileFinished(bool successful);

private:

    class EditorWindowPriv;
    EditorWindowPriv* const d;

    friend class EditorToolIface;
};

}  // namespace Digikam

#endif /* EDITORWINDOW_H */
