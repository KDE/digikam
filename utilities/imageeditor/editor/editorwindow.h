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

// Qt includes.

#include <qcolor.h>
#include <qstring.h>
#include <qrect.h>

// KDE includes.

#include <kmainwindow.h>
#include <kurl.h>

// Local includes.

#include "sidebar.h"
#include "digikam_export.h"

class QSplitter;
class QPopupMenu;
class QLabel;

class KToolBarPopupAction;
class KToggleAction;
class KAction;
class KSelectAction;

namespace Digikam
{

class Sidebar;
class DPopupMenu;
class Canvas;
class ImagePluginLoader;
class IOFileSettingsContainer;
class SavingContextContainer;
class StatusProgressBar;
class SlideShowSettings;
class EditorStackView;
class EditorWindowPriv;

class DIGIKAM_EXPORT EditorWindow : public KMainWindow
{
    Q_OBJECT

public:

    EditorWindow(const char *name);
    ~EditorWindow();

    virtual void applySettings(){};
    virtual bool setup(bool iccSetupPage=false)=0;

signals:

    void signalSelectionChanged( const QRect & );
    void signalNoCurrentItem();

protected:

    bool                     m_cancelSlideShow;
    bool                     m_fullScreen;
    bool                     m_rotatedOrFlipped;
    bool                     m_setExifOrientationTag;

    QLabel                  *m_resLabel;

    QColor                   m_bgColor;

    QSplitter               *m_splitter;

    KAction                 *m_saveAction;
    KAction                 *m_saveAsAction;
    KAction                 *m_revertAction;
    KAction                 *m_fileDeleteAction;
    KAction                 *m_forwardAction;
    KAction                 *m_backwardAction;
    KAction                 *m_firstAction;
    KAction                 *m_lastAction;

    KToggleAction           *m_fullScreenAction;

    KSelectAction           *m_themeMenuAction;

    KToolBarPopupAction     *m_undoAction;
    KToolBarPopupAction     *m_redoAction;

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
    void setupStandardAccelerators();
    void setupStatusBar();
    void setupContextMenu();
    void toggleStandardActions(bool val);
    void toggleZoomActions(bool val);

    void printImage(KURL url);

    void plugActionAccel(KAction* action);
    void unplugActionAccel(KAction* action);

    void unLoadImagePlugins();
    void loadImagePlugins();

    bool promptForOverWrite();
    bool promptUserSave(const KURL& url);
    bool waitForSavingToComplete();
    void startingSave(const KURL& url);
    bool startingSaveAs(const KURL& url);
    bool checkPermissions(const KURL& url);
    bool moveFile();

    EditorStackView* editorStackView() const;

    virtual void finishSaving(bool success);

    virtual void readSettings()               { readStandardSettings();     };
    virtual void saveSettings()               { saveStandardSettings();     };
    virtual void toggleActions(bool val)      { toggleStandardActions(val); };
    virtual void toggleGUI2FullScreen()       {};

    virtual void slideShow(bool startWithCurrent, SlideShowSettings& settings)=0;

    virtual void setupConnections()=0;
    virtual void setupActions()=0;
    virtual void setupUserArea()=0;
    virtual bool saveAs()=0; 
    virtual bool save()=0;

    virtual void saveIsComplete()=0;
    virtual void saveAsIsComplete()=0; 

    virtual Sidebar *rightSideBar() const=0;

protected slots:

    void slotSave();
    void slotSaveAs() { saveAs(); };

    void slotEditKeys();
    void slotResize();

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
    virtual void slotLoadingFinished(const QString &filename, bool success);
    virtual void slotSavingStarted(const QString &filename);

    virtual void slotSetup(){ setup(); };
    virtual void slotChangeTheme(const QString& theme);

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

private slots:

    void slotToggleUnderExposureIndicator();
    void slotToggleOverExposureIndicator();
    void slotToggleColorManagedView();
    void slotRotatedOrFlipped();
    void slotSavingFinished(const QString &filename, bool success);
    void slotDonateMoney();
    void slotContribute();
    void slotToggleSlideShow();
    void slotZoomTo100Percents();
    void slotZoomSelected();
    void slotZoomTextChanged(const QString &);
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

private:

    void enter_loop();
    void hideToolBars();
    void showToolBars();
    void setColorManagedViewIndicatorToolTip(bool available, bool cmv);
    void setUnderExposureToolTip(bool uei);
    void setOverExposureToolTip(bool oei);

    void setToolStartProgress(const QString& toolName);
    void setToolProgress(int progress);
    void setToolStopProgress();

private:

    EditorWindowPriv *d;

    friend class EditorToolIface;
};

}  // namespace Digikam

#endif /* EDITORWINDOW_H */
