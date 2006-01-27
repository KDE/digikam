/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
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

#include <qstring.h>
#include <qrect.h>

// KDE includes.

#include <kmainwindow.h>
#include <kurl.h>

class QSplitter;
class QPopupMenu;
class QLabel;

class KAccel;
class KToolBarPopupAction;
class KToggleAction;
class KAction;
class KActionMenu;
class KSelectAction;

namespace Digikam
{

class Canvas;
class ImagePluginLoader;
class ICCSettingsContainer;
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

signals:

    void signalSelectionChanged( QRect* );

protected:

    // If current image file format is only available in read only,
    // typicially all RAW image file formats.
    bool                     m_isReadOnly;

    bool                     m_fullScreenHideToolBar;
    bool                     m_fullScreen;
    bool                     m_removeFullScreenButton;
    bool                     m_slideShowInFullScreen;
    bool                     m_rotatedOrFlipped;
    
    QLabel                  *m_zoomLabel;
    QLabel                  *m_resLabel;

    QSplitter               *m_splitter;

    QPopupMenu              *m_contextMenu;

    KAction                 *m_zoomPlusAction;
    KAction                 *m_zoomMinusAction;
    KAction                 *m_saveAction;
    KAction                 *m_saveAsAction;
    KAction                 *m_revertAction;
    KAction                 *m_filePrintAction;    
    KAction                 *m_fileDeleteAction;
    KAction                 *m_forwardAction;
    KAction                 *m_backwardAction;
    KAction                 *m_firstAction;
    KAction                 *m_lastAction;
    KAction                 *m_copyAction;
    KAction                 *m_resizeAction;
    KAction                 *m_cropAction;
    KAction                 *m_imagePluginsHelpAction;
    KAction                 *m_rotate90Action;
    KAction                 *m_rotate180Action;
    KAction                 *m_rotate270Action;
    KAction                 *m_flipHorzAction;
    KAction                 *m_flipVertAction;

    KActionMenu             *m_flipAction;
    KActionMenu             *m_rotateAction;
    KAccel                  *m_accel;
    
    KSelectAction           *m_viewHistogramAction;

    KToggleAction           *m_slideShowAction;
    KToggleAction           *m_zoomFitAction;
    KToggleAction           *m_fullScreenAction;

    KToolBarPopupAction     *m_undoAction;
    KToolBarPopupAction     *m_redoAction;

    Canvas                  *m_canvas;
    ImagePluginLoader       *m_imagePluginLoader;
    IOFileProgressBar       *m_nameLabel;
    ICCSettingsContainer    *m_ICCSettings;
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

protected slots:

    void slotSave()   { if (m_isReadOnly) saveAs(); else save(); };
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

    virtual void slotContextMenu();

    virtual void slotFilePrint()=0;
    virtual void slotDeleteCurrentItem()=0;
    virtual void slotBackward()=0;
    virtual void slotForward()=0;
    virtual void slotFirst()=0;
    virtual void slotLast()=0;
    virtual void slotUpdateItemInfo()=0;
    virtual void slotSetup()=0;

private slots:

    void slotRotatedOrFlipped();

private:

    void enter_loop();

private:
    
    EditorWindowPriv *d;

};

}  // namespace Digikam

#endif /* EDITORWINDOW_H */
