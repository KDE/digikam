/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : core image editor GUI implementation
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

// Qt includes

#include <QColor>
#include <QPointer>
#include <QRect>
#include <QString>
#include <QProgressDialog>
#include <QUrl>

//std includes
#include <queue>

// Local includes

#include "digikam_export.h"
#include "digikam_config.h"
#include "thumbbardock.h"
#include "previewtoolbar.h"
#include "savingcontext.h"
#include "dxmlguiwindow.h"

class QSplitter;
class QMenu;
class QAction;

class KSelectAction;
class KToolBarPopupAction;

namespace Digikam
{

class DAdjustableLabel;
class DCategorizedView;
class Canvas;
class DImageHistory;
class EditorTool;
class EditorStackView;
class ExposureSettingsContainer;
class IOFileSettings;
class ICCSettingsContainer;
class Sidebar;
class SidebarSplitter;
class SlideShowSettings;
class StatusProgressBar;
class VersionManager;
class VersionFileOperation;
class IccProfile;

class DIGIKAM_EXPORT EditorWindow : public DXmlGuiWindow
{
    Q_OBJECT

public:

    enum TransformType
    {
        RotateLeft,
        RotateRight,
        FlipHorizontal,
        FlipVertical
    };

    explicit EditorWindow(const QString& name);
    ~EditorWindow();

    const static QString CONFIG_GROUP_NAME;

public Q_SLOTS:

    virtual void slotSetup()    = 0;
    virtual void slotSetupICC() = 0;

Q_SIGNALS:

    void signalSelectionChanged(const QRect&);
    void signalNoCurrentItem();
    void signalPreviewModeChanged(int);
    void signalToolApplied();
    void signalPoint1Action();
    void signalPoint2Action();
    void signalAutoAdjustAction();

protected:

    bool                      m_nonDestructive;
    bool                      m_setExifOrientationTag;
    bool                      m_editingOriginalImage;
    bool                      m_actionEnabledState;
    bool                      m_cancelSlideShow;

    DAdjustableLabel*         m_resLabel;

    QColor                    m_bgColor;

    SidebarSplitter*          m_splitter;
    QSplitter*                m_vSplitter;

    QAction*                  m_openVersionAction;
    QAction*                  m_saveAction;
    QAction*                  m_saveAsAction;
    KToolBarPopupAction*      m_saveNewVersionAction;
    QAction*                  m_saveCurrentVersionAction;
    QAction*                  m_saveNewVersionAsAction;
    QMenu*                    m_saveNewVersionInFormatAction;
    QAction*                  m_exportAction;
    QAction*                  m_revertAction;
    QAction*                  m_discardChangesAction;
    QAction*                  m_fileDeleteAction;
    QAction*                  m_forwardAction;
    QAction*                  m_backwardAction;

    QAction*                  m_lastAction;
    QAction*                  m_firstAction;

    QAction*                  m_applyToolAction;
    QAction*                  m_closeToolAction;

    QAction*                  m_showBarAction;

    KToolBarPopupAction*      m_undoAction;
    KToolBarPopupAction*      m_redoAction;

    QMenu*                    m_contextMenu;
    QMenu*                    m_servicesMenu;
    QAction*                  m_serviceAction;

    EditorStackView*          m_stackView;
    Canvas*                   m_canvas;
    StatusProgressBar*        m_nameLabel;
    IOFileSettings*           m_IOFileSettings;
    QPointer<QProgressDialog> m_savingProgressDialog;

    SavingContext             m_savingContext;

    QString                   m_formatForRAWVersioning;
    QString                   m_formatForSubversions;

    //using QVector to store transforms
    QVector<TransformType>    m_transformQue;

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

    void printImage(const QUrl& url);

    bool promptForOverWrite();

    bool promptUserDelete(const QUrl& url);
    bool promptUserSave(const QUrl& url, SaveAskMode mode = AskIfNeeded, bool allowCancel = true);
    bool waitForSavingToComplete();
    void startingSave(const QUrl& url);
    bool startingSaveAs(const QUrl& url);
    bool startingSaveCurrentVersion(const QUrl& url);
    bool startingSaveNewVersion(const QUrl& url);
    bool startingSaveNewVersionAs(const QUrl& url);
    bool startingSaveNewVersionInFormat(const QUrl& url, const QString& format);
    bool checkPermissions(const QUrl& url);
    bool checkOverwrite(const QUrl& url);
    bool moveLocalFile(const QString& src, const QString& dest);
    void movingSaveFileFinished(bool successful);
    void colorManage();
    void execSavingProgressDialog();

    void resetOrigin();
    void resetOriginSwitchFile();

    void addServicesMenuForUrl(const QUrl& url);
    void openWith(const QUrl& url, QAction* action);

    EditorStackView*           editorStackView()  const;
    ExposureSettingsContainer* exposureSettings() const;

    VersionFileOperation saveVersionFileOperation(const QUrl& url, bool fork);
    VersionFileOperation saveAsVersionFileOperation(const QUrl& url, const QUrl& saveLocation, const QString& format);
    VersionFileOperation saveInFormatVersionFileOperation(const QUrl& url, const QString& format);

    virtual bool hasOriginalToRestore();
    virtual DImageHistory resolvedImageHistory(const DImageHistory& history);

    virtual void moveFile();
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
    virtual QUrl saveDestinationUrl() = 0;

    virtual void saveIsComplete() = 0;
    virtual void saveAsIsComplete() = 0;
    virtual void saveVersionIsComplete() = 0;

protected Q_SLOTS:

    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();

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
    void slotCloseTool();
    void slotApplyTool();
    void slotUndoStateChanged();
    void slotThemeChanged();
    void slotToggleRightSideBar();
    void slotPreviousRightSideBarTab();
    void slotNextRightSideBarTab();
    void slotToolDone();
    void slotInsertText();
    void slotBorder();
    void slotTexture();
    void slotColorEffects();
    void slotCharcoal();
    void slotEmboss();
    void slotOilPaint();
    void slotBlurFX();
    void slotDistortionFX();
    void slotRainDrop();
    void slotFilmGrain();
    void slotBCG();
    void slotCB();
    void slotHSL();
    void slotAutoCorrection();
    void slotInvert();
    void slotBW();
    void slotWhiteBalance();
    void slotConvertTo8Bits();
    void slotConvertTo16Bits();
    void slotConvertToColorSpace(const IccProfile&);
    void slotProfileConversionTool();
    void slotChannelMixer();
    void slotCurvesAdjust();
    void slotLevelsAdjust();
    void slotFilm();
    void slotUpdateColorSpaceMenu();
    void slotRestoration();
    void slotBlur();
    void slotHealingClone();
    void slotSharpen();
    void slotNoiseReduction();
    void slotLocalContrast();
    void slotRedEye();
    void slotLensAutoFix();
    void slotAntiVignetting();
    void slotLensDistortion();
    void slotHotPixels();
    void slotPerspective();
    void slotFreeRotation();
    void slotShearTool();
    void slotContentAwareResizing();
    void slotResize();
    void slotRatioCrop();
    void slotRotateLeftIntoQue();
    void slotRotateRightIntoQue();
    void slotFlipHIntoQue();
    void slotFlipVIntoQue();

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

    bool startingSaveVersion(const QUrl& url, bool subversion, bool saveAs, const QString& format);

    void setPreviewModeMask(int mask);
    PreviewToolBar::PreviewMode previewMode() const;

    bool showFileSaveDialog(const QUrl& initialUrl, QUrl& newURL);

    /**
     * Sets up a temp file to save image contents to and updates the saving
     * context to use this file
     *
     * @param url file to save the image to
     */
    void setupTempSaveFile(const QUrl& url);

    /**
     * Sets the format to use in the saving context. Therefore multiple sources
     * are used starting with the extension found in the save dialog.
     *
     * @param filter filter selected in the dialog
     * @param targetUrl target url selected for the file to save
     * @return The valid extension which could be found, or a null string
     */
    QString selectValidSavingFormat(const QUrl& targetUrl);

    void addAction2ContextMenu(const QString& actionName, bool addDisabled = false);

    void loadTool(EditorTool* const tool);

private:

    class Private;
    Private* const d;

    friend class EditorToolIface;
};

}  // namespace Digikam

#endif // EDITOR_WINDOW_H
