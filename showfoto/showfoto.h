/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-22
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
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

#include <kmainwindow.h>
#include <kurl.h>

class QLabel;
class QPopupMenu;

class KAction;
class KActionMenu;
class KToggleAction;
class KSelectAction;
class KConfig;
class KToolBarPopupAction;

class Canvas;
class ImagePluginLoader;
class SplashScreen;

namespace KIO
{
class Job;
}

namespace Digikam
{
class ThumbBarView;
}

class ShowFoto : public KMainWindow
{
    Q_OBJECT
    
public:

    ShowFoto(const KURL::List& urlList);
    ~ShowFoto();
    
    virtual void show();

protected:

    void closeEvent(QCloseEvent* e);
    
private slots:

    void slotOpenFile();
    void slotNext();
    void slotPrev();
    void slotLast();
    void slotFirst();
    void slotSave();
    void slotSaveAs();
    void slotFilePrint();
    void slotOpenURL(const KURL& url);
    void slotDeleteCurrentItem();
    void slotFileProperties();
    
    void slotToggleFullScreen();
    void slotAutoFit();
    void slotZoomChanged(float zoom);
    void slotResize();
    void slotEscapePressed();
    void slotToggleShowBar();
    void slotViewHistogram();
    void slotChangeBCG();
    void slotImagePluginsHelp();
    
    void slotChanged(bool, bool);
    void slotSelected(bool);
    
    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();

    void slotEditKeys();
    void slotConfToolbars();
    void slotNewToolbarConfig();
    void slotSetup();
    void slotContextMenu();
        
    void slotDeleteCurrentItemResult( KIO::Job * job );
               
private:

    void setupActions();
    void applySettings();
    void saveSettings();
    bool promptUserSave();
    bool save();
    void toggleActions(bool val);
    void loadPlugins();
    void unLoadPlugins();

private:

    Canvas                *m_canvas;
    
    ImagePluginLoader     *m_imagePluginLoader;
    
    Digikam::ThumbBarView *m_bar;
    
    SplashScreen          *m_splash;
    
    QLabel                *m_nameLabel;
    QLabel                *m_zoomLabel;
    QLabel                *m_resLabel;
        
    QPopupMenu            *m_contextMenu;
                               
    KConfig               *m_config;
    
    KAction               *m_zoomPlusAction;
    KAction               *m_zoomMinusAction;
    KToggleAction         *m_zoomFitAction;
    KToggleAction         *m_fullScreenAction;
    KToggleAction         *m_showBarAction;
    
    KAction               *m_cropAction;
    KAction               *m_imagePluginsHelpAction;
    
    KAction               *m_revertAction;
    KAction               *m_saveAction;
    KAction               *m_saveAsAction;
    KAction               *m_propertiesAction;
    KAction               *m_filePrintAction;    
    KAction               *m_fileDeleteAction;
    
    KAction               *m_forwardAction;
    KAction               *m_backAction;
    KAction               *m_firstAction;
    KAction               *m_lastAction;
                          
    KActionMenu           *m_rotateAction;
    KAction               *m_rotate90Action;
    KAction               *m_rotate180Action;
    KAction               *m_rotate270Action;
    
    KAction               *m_resizeAction;
    KActionMenu           *m_flipAction;
    KAction               *m_flipHorzAction;
    KAction               *m_flipVertAction;
    
    KActionMenu           *m_BCGAction;
    
    KAction               *m_copyAction;
    KToolBarPopupAction   *m_undoAction;
    KToolBarPopupAction   *m_redoAction;
        
    KSelectAction         *m_viewHistogramAction;
    
    int                    m_JPEGCompression;
    int                    m_PNGCompression;
    
    bool                   m_fullScreen;
    bool                   m_TIFFCompression;
    bool                   m_fullScreenHideToolBar;
    bool                   m_fullScreenHideThumbBar;
    bool                   m_deleteItem2Trash;
    bool                   m_disableBCGActions;
};

#endif /* SHOWFOTO_H */
