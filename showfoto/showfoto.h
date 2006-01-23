/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 * 
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
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

#ifndef SHOWFOTO_H
#define SHOWFOTO_H

// KDE includes.

#include <kurl.h>

// Local includes.

#include "editorwindow.h"

class QLabel;
class QPopupMenu;
class QSplitter;

class KAction;
class KActionMenu;
class KToggleAction;
class KSelectAction;
class KConfig;
class KToolBarPopupAction;

namespace KIO
{
class Job;
}

namespace Digikam
{
class ImagePluginLoader;
class ThumbBarView;
class ThumbBarItem;
class ImagePropertiesSideBar;
class ICCSettingsContainer;
class IOFileSettingsContainer;
class SavingContextContainer;
class SplashScreen;
class SlideShow;
}

namespace ShowFoto
{

class ShowFoto : public Digikam::EditorWindow
{
    Q_OBJECT
    
public:

    ShowFoto(const KURL::List& urlList);
    ~ShowFoto();
    
    virtual void show();

private:
    
    bool                             m_removeFullScreenButton;
    bool                             m_fullScreen;
    bool                             m_fullScreenHideToolBar;
    bool                             m_fullScreenHideThumbBar;
    bool                             m_deleteItem2Trash;
    bool                             m_slideShowInFullScreen;
    
    // If current image file format is only available in read only,
    // typicially all RAW image file formats.
    bool                             m_isReadOnly;

    int                              m_itemsNb;

    QLabel                          *m_nameLabel;
    QLabel                          *m_zoomLabel;
    QLabel                          *m_resLabel;
        
    QPopupMenu                      *m_contextMenu;

    QSplitter                       *m_splitter;

    KURL                             m_lastOpenedDirectory;
                          
    KConfig                         *m_config;
    
    KAction                         *m_zoomPlusAction;
    KAction                         *m_zoomMinusAction;
    KToggleAction                   *m_zoomFitAction;
    KToggleAction                   *m_fullScreenAction;
    KToggleAction                   *m_showBarAction;
    KToggleAction                   *m_slideShowAction;
    
    KAction                         *m_cropAction;
    KAction                         *m_imagePluginsHelpAction;
    
    KAction                         *m_revertAction;
    KAction                         *m_saveAction;
    KAction                         *m_saveAsAction;
    KAction                         *m_filePrintAction;    
    KAction                         *m_fileDeleteAction;
    KAction                         *m_openFilesInFolderAction;    
    KAction                         *m_copyAction;
    KAction                         *m_fileOpenAction;
    
    KAction                         *m_forwardAction;
    KAction                         *m_backAction;
    KAction                         *m_firstAction;
    KAction                         *m_lastAction;
                          
    KAction                         *m_rotate90Action;
    KAction                         *m_rotate180Action;
    KAction                         *m_rotate270Action;
    
    KAction                         *m_resizeAction;
    KAction                         *m_flipHorzAction;
    KAction                         *m_flipVertAction;
    
    KActionMenu                     *m_flipAction;
    KActionMenu                     *m_BCGAction;
    KActionMenu                     *m_rotateAction;
    
    KToolBarPopupAction             *m_undoAction;
    KToolBarPopupAction             *m_redoAction;
        
    KSelectAction                   *m_viewHistogramAction;

    Digikam::ImagePluginLoader       *m_imagePluginLoader;
    Digikam::ThumbBarView            *m_bar;
    Digikam::ThumbBarItem            *m_currentItem;
    Digikam::ImagePropertiesSideBar  *m_rightSidebar;
    Digikam::ICCSettingsContainer    *m_ICCSettings;
    Digikam::IOFileSettingsContainer *m_IOFileSettings;
    Digikam::SplashScreen            *m_splash;
    Digikam::SavingContextContainer  *m_savingContext;
    Digikam::SlideShow               *m_slideShow;

private:

    void setupActions();
    void applySettings();
    void saveSettings();
    bool promptUserSave();
    bool save();
    bool saveAs();
    void toggleActions(bool val, bool slideShow=false);
    void toggleNavigation(int index);
    void loadPlugins();
    void unLoadPlugins();
    void finishSaving(bool success);

private slots:

    void slotOpenFile();
    void slotNext();
    void slotPrev();
    void slotLast();
    void slotFirst();
    void slotFilePrint();
    void slotOpenURL(const KURL& url);
    void slotOpenFolder(const KURL& url);
    void slotOpenFilesInFolder();
    void slotDeleteCurrentItem();

    void slotSave()   { if (m_isReadOnly) saveAs(); else save(); };
    void slotSaveAs() { saveAs(); };
    
    void slotToggleFullScreen();
    void slotToggleSlideShow();
    void slotAutoFit();
    void slotZoomChanged(float zoom);
    void slotEscapePressed();
    void slotToggleShowBar();
    void slotViewHistogram();
    void slotChangeBCG();
    
    void slotChanged(bool, bool);
    void slotSelected(bool);
    void slotUpdateItemInfo(void);
    
    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();

    void slotConfToolbars();
    void slotNewToolbarConfig();
    void slotSetup();
    void slotContextMenu();
        
    void slotDeleteCurrentItemResult( KIO::Job * job );

    void slotLoadingStarted(const QString &filename);
    void slotLoadingFinished(const QString &filename, bool success, bool isReadOnly);
    void slotLoadingProgress(const QString& filePath, float progress);
    void slotSavingStarted(const QString &filename);
    void slotSavingFinished(const QString &filename, bool success);
    void slotSavingProgress(const QString& filePath, float progress);

protected:

    void closeEvent(QCloseEvent* e);
};

}   // namespace ShowFoto

#endif /* SHOWFOTO_H */
