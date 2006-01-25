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

class QPopupMenu;

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
    bool                             m_fullScreenHideToolBar;
    bool                             m_fullScreenHideThumbBar;
    bool                             m_deleteItem2Trash;
    bool                             m_slideShowInFullScreen;
    
    // If current image file format is only available in read only,
    // typicially all RAW image file formats.
    bool                             m_isReadOnly;

    int                              m_itemsNb;

    QPopupMenu                      *m_contextMenu;

    KURL                             m_lastOpenedDirectory;
    
    KToggleAction                   *m_showBarAction;
    KToggleAction                   *m_slideShowAction;
    
    KAction                         *m_cropAction;
    KAction                         *m_imagePluginsHelpAction;
    
    KAction                         *m_openFilesInFolderAction;    
    KAction                         *m_fileOpenAction;
    
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

    Digikam::ImagePluginLoader       *m_imagePluginLoader;
    Digikam::ThumbBarView            *m_bar;
    Digikam::ThumbBarItem            *m_currentItem;
    Digikam::ImagePropertiesSideBar  *m_rightSidebar;
    Digikam::SplashScreen            *m_splash;
    Digikam::SlideShow               *m_slideShow;

private:

    void setupUserArea();
    void saveSettings();
    bool promptUserSave();
    void setupActions();
    
    void applySettings();
    bool save();
    bool saveAs();
    void toggleActions(bool val, bool slideShow=false);
    void toggleNavigation(int index);
    void loadPlugins();
    void unLoadPlugins();
    void finishSaving(bool success);

private slots:

    void slotOpenFile();
    void slotForward();
    void slotBackward();
    void slotLast();
    void slotFirst();
    void slotFilePrint();

    void slotOpenURL(const KURL& url);
    void slotOpenFolder(const KURL& url);
    void slotOpenFilesInFolder();
    void slotDeleteCurrentItem();

    void slotToggleFullScreen();
    void slotToggleSlideShow();
    void slotToggleShowBar();
    void slotViewHistogram();
    void slotChangeBCG();
    
    void slotChanged(bool, bool);
    void slotSelected(bool);
    void slotUpdateItemInfo(void);

    void slotSetup();
    void slotContextMenu();
        
    void slotDeleteCurrentItemResult( KIO::Job * job );

    void slotLoadingStarted(const QString &filename);
    void slotLoadingFinished(const QString &filename, bool success, bool isReadOnly);
    void slotLoadingProgress(const QString& filePath, float progress);
    void slotSavingStarted(const QString &filename);
    void slotSavingFinished(const QString &filename, bool success);
    void slotSavingProgress(const QString& filePath, float progress);
};

}   // namespace ShowFoto

#endif /* SHOWFOTO_H */
