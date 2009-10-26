/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : main image editor GUI implementation
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QRect>
#include <QString>

// KDE includes

#include <kxmlguiwindow.h>
#include <kurl.h>
#include <kjob.h>

// Local includes

#include "digikam_export.h"
#include "thumbbardock.h"

class QSplitter;
class QLabel;

class KAction;
class KSelectAction;
class KToggleAction;
class KToolBarPopupAction;

namespace Digikam
{

class Canvas;
class DPopupMenu;
class DLogoAction;
class EditorStackView;
class EditorWindowPriv;
class IOFileSettingsContainer;
class ImagePluginLoader;
class SavingContextContainer;
class Sidebar;
class SidebarSplitter;
class SlideShowSettings;
class StatusProgressBar;
class ThumbBarView;

class DIGIKAM_EXPORT EditorWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:

    EditorWindow(const char *name);
    ~EditorWindow();

    virtual void applySettings(){};
    virtual bool setup()=0;
    virtual bool setupICC()=0;

Q_SIGNALS:

    void signalSelectionChanged(const QRect&);
    void signalNoCurrentItem();

protected:

    bool                     m_fullScreenHideThumbBar;
    bool                     m_cancelSlideShow;
    bool                     m_fullScreen;
    bool                     m_rotatedOrFlipped;
    bool                     m_setExifOrientationTag;

    QLabel                  *m_resLabel;

    QColor                   m_bgColor;

    SidebarSplitter         *m_splitter;
    QSplitter               *m_vSplitter;

    KAction                 *m_saveAction;
    KAction                 *m_saveAsAction;
    KAction                 *m_revertAction;
    KAction                 *m_fileDeleteAction;
    KAction                 *m_forwardAction;
    KAction                 *m_backwardAction;
    KAction                 *m_fullScreenAction;

    KAction                 *m_lastAction;
    KAction                 *m_firstAction;

    KSelectAction           *m_themeMenuAction;

    KToggleAction           *m_showBarAction;

    KToolBarPopupAction     *m_undoAction;
    KToolBarPopupAction     *m_redoAction;

    DLogoAction             *m_animLogo;
    DPopupMenu              *m_contextMenu;
    EditorStackView         *m_stackView;
    Canvas                  *m_canvas;
    ImagePluginLoader       *m_imagePluginLoader;
    StatusProgressBar       *m_nameLabel;
    IOFileSettingsContainer *m_IOFileSettings;
    SavingContextContainer  *m_savingContext;

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

    void printImage(const KUrl& url);

    void unLoadImagePlugins();
    void loadImagePlugins();

    bool promptForOverWrite();
    enum SaveOrSaveAs
    {
        AskIfNeeded,
        OverwriteWithoutAsking,
        AlwaysSaveAs
    };
    bool promptUserSave(const KUrl& url, SaveOrSaveAs = AskIfNeeded, bool allowCancel = true);
    bool waitForSavingToComplete();
    void startingSave(const KUrl& url);
    bool startingSaveAs(const KUrl& url);
    bool checkPermissions(const KUrl& url);
    void moveFile();
    void colorManage();

    EditorStackView* editorStackView() const;

    virtual void finishSaving(bool success);

    virtual void readSettings()               { readStandardSettings();     };
    virtual void saveSettings()               { saveStandardSettings();     };
    virtual void toggleActions(bool val)      { toggleStandardActions(val); };

    void toggleGUI2FullScreen();

    virtual ThumbBarDock *thumbBar() const=0;
    virtual Sidebar *rightSideBar() const=0;

    virtual void slideShow(bool startWithCurrent, SlideShowSettings& settings)=0;

    virtual void setupConnections()=0;
    virtual void setupActions()=0;
    virtual void setupUserArea()=0;
    virtual bool saveAs()=0;
    virtual bool save()=0;

    virtual void saveIsComplete()=0;
    virtual void saveAsIsComplete()=0;

protected Q_SLOTS:

    void slotSave();
    void slotSaveAs() { saveAs(); };

    void slotEditKeys();

    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();

    void slotConfToolbars();
    void slotNewToolbarConfig();

    void slotToggleFullScreen();
    void slotEscapePressed();

    void slotSelected(bool);

    void slotLoadingProgress(const QString& filePath, float progress);
    void slotSavingProgress(const QString& filePath, float progress);

    void slotNameLabelCancelButtonPressed();

    void slotThemeChanged();

    virtual void slotLoadingStarted(const QString& filename);
    virtual void slotLoadingFinished(const QString& filename, bool success);
    virtual void slotSavingStarted(const QString& filename);

    virtual void slotSetup(){ setup(); };
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
    void slotZoomSelected(int);
    void slotZoomChanged(bool isMax, bool isMin, double zoom);
    void slotSelectionChanged(const QRect& sel);
    void slotToggleFitToWindow();
    void slotToggleOffFitToWindow();
    void slotFitToSelect();
    void slotIncreaseZoom();
    void slotDecreaseZoom();
    void slotRawCameraList();
    void slotPrepareToLoad();
    void slotShowMenuBar();
    void slotCloseTool();
    void slotKioMoveFinished(KJob *job);

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

    /**
     * Sets up a temp file to save image contents to and updates the saving
     * context to use this file
     *
     * @param url file to save the image to
     */
    void setupTempSaveFile(const KUrl & url);

    void movingSaveFileFinished(bool successful);

private:

    EditorWindowPriv* const d;

    friend class EditorToolIface;
};

}  // namespace Digikam

#endif /* EDITORWINDOW_H */
