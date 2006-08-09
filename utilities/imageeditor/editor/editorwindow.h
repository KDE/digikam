/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-20
 * Description : main image editor GUI implementation
 *
 * Copyright 2006 by Gilles Caulier
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

class QSplitter;
class QPopupMenu;
class QLabel;

class KToolBarPopupAction;
class KToggleAction;
class KAction;

namespace Digikam
{

class Canvas;
class ImagePluginLoader;
class IOFileSettingsContainer;
class SavingContextContainer;
class IOFileProgressBar;
class EditorWindowPriv;
class SlideShow;

class EditorWindow : public KMainWindow
{
    Q_OBJECT

public:

    EditorWindow(const char *name);
    ~EditorWindow();

    virtual void applySettings(){};
    virtual void setup(bool iccSetupPage=false)=0;

signals:

    void signalSelectionChanged( QRect* );
    void signalNoCurrentItem();

protected:

    bool                     m_fullScreen;
    bool                     m_rotatedOrFlipped;
    bool                     m_setExifOrientationTag;

    QLabel                  *m_zoomLabel;
    QLabel                  *m_resLabel;

    QColor                   m_bgColor;

    QSplitter               *m_splitter;

    QPopupMenu              *m_contextMenu;

    KAction                 *m_saveAction;
    KAction                 *m_saveAsAction;
    KAction                 *m_revertAction;
    KAction                 *m_fileDeleteAction;
    KAction                 *m_forwardAction;
    KAction                 *m_backwardAction;
    KAction                 *m_firstAction;
    KAction                 *m_lastAction;

    KToggleAction           *m_slideShowAction;
    KToggleAction           *m_fullScreenAction;

    KToolBarPopupAction     *m_undoAction;
    KToolBarPopupAction     *m_redoAction;

    Canvas                  *m_canvas;
    ImagePluginLoader       *m_imagePluginLoader;
    IOFileProgressBar       *m_nameLabel;
    IOFileSettingsContainer *m_IOFileSettings;
    SavingContextContainer  *m_savingContext;
    SlideShow               *m_slideShow;

protected:

    void saveStandardSettings();
    void readStandardSettings();
    void applyStandardSettings();

    void setupStandardConnections();
    void setupStandardActions();
    void setupStandardAccelerators();
    void setupStatusBar();
    void toggleStandardActions(bool val);

    void printImage(KURL url);

    void plugActionAccel(KAction* action);
    void unplugActionAccel(KAction* action);

    void unLoadImagePlugins();
    void loadImagePlugins();

    bool promptUserSave(const KURL& url);
    bool waitForSavingToComplete();
    void startingSave(const KURL& url);
    bool startingSaveAs(const KURL& url);
    bool checkPermissions(const KURL& url);
    bool moveFile();

    virtual void finishSaving(bool success);

    virtual void readSettings()               { readStandardSettings(); };
    virtual void saveSettings()               { saveStandardSettings(); };
    virtual void toggleActions(bool val)      { toggleStandardActions(val); };
    virtual void toggleActions2SlideShow(bool){};
    virtual void toggleGUI2SlideShow()        {};
    virtual void toggleGUI2FullScreen()       {};

    virtual void setupConnections()=0;
    virtual void setupActions()=0;
    virtual void setupUserArea()=0;
    virtual bool saveAs()=0; 
    virtual bool save()=0;

    virtual void saveIsComplete()=0;
    virtual void saveAsIsComplete()=0; 

protected slots:

    void slotSave();
    void slotSaveAs() { saveAs(); };

    void slotImagePluginsHelp();
    void slotEditKeys();
    void slotResize();

    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();

    void slotConfToolbars();
    void slotNewToolbarConfig();

    void slotToggleSlideShow();
    void slotToggleFullScreen();
    void slotEscapePressed();

    void slotToggleAutoZoom();
    void slotZoomChanged(float zoom);
    void slotViewHistogram();
    void slotSelected(bool);

    void slotLoadingProgress(const QString& filePath, float progress);
    void slotSavingProgress(const QString& filePath, float progress);

    void slotNameLabelCancelButtonPressed();

    virtual void slotLoadingStarted(const QString& filename);
    virtual void slotLoadingFinished(const QString &filename, bool success);
    virtual void slotSavingStarted(const QString &filename);

    virtual void slotContextMenu();

    virtual void slotSetup() { setup(); };

    virtual void slotFilePrint()=0;
    virtual void slotDeleteCurrentItem()=0;
    virtual void slotBackward()=0;
    virtual void slotForward()=0;
    virtual void slotFirst()=0;
    virtual void slotLast()=0;
    virtual void slotUpdateItemInfo()=0;
    virtual void slotChanged()=0;

private slots:

    void slotRotatedOrFlipped();
    void slotSavingFinished(const QString &filename, bool success);

private:

    void enter_loop();
    void hideToolBars();
    void showToolBars();

private:

    EditorWindowPriv *d;

};

}  // namespace Digikam

#endif /* EDITORWINDOW_H */
