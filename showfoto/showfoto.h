/* ============================================================
 * File  : showfoto.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-11-22
 * Copyright 2004 by Renchi Raju
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
 * ============================================================ */

#ifndef SHOWFOTO_H
#define SHOWFOTO_H

// KDE includes.

#include <kmainwindow.h>
#include <kurl.h>

class KAction;
class KToggleAction;
class KSelectAction;
class KConfig;
class KToolBarPopupAction;

class Canvas;
class ImagePluginLoader;

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

protected:

    void closeEvent(QCloseEvent* e);
    
private slots:

    void slotOpenFile();
    void slotNext();
    void slotPrev();
    void slotSave();
    void slotSaveAs();
    void slotAutoFit();
    void slotOpenURL(const KURL& url);
    void slotToggleFullScreen();
    void slotEscapePressed();
    void slotFileProperties();
    void slotToggleShowBar();
    void slotViewHistogram();
    void slotChangeBCG();
    void slotImagePluginsHelp();
    
    void slotChanged(bool, bool);
    void slotSelected(bool);
    
    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();

            
private:

    void setupActions();
    void applySettings();
    void saveSettings();
    bool promptUserSave();
    bool save();

private:

    Canvas                *m_canvas;
    
    ImagePluginLoader     *m_imagePluginLoader;
    
    Digikam::ThumbBarView *m_bar;
    
    KConfig               *m_config;
    
    KToggleAction         *m_zoomFitAction;
    KToggleAction         *m_fullScreenAction;
    KToggleAction         *m_showBarAction;
    
    KAction               *m_zoomPlusAction;
    KAction               *m_zoomMinusAction;
    KAction               *m_cropAction;
    KAction               *m_revertAction;
    KAction               *m_saveAction;
    KAction               *m_saveAsAction;
    KAction               *m_imagePluginsHelp;

    KAction               *m_copyAction;
    KToolBarPopupAction   *m_undoAction;
    KToolBarPopupAction   *m_redoAction;
        
    KSelectAction         *m_viewHistogramAction;
    
    bool                   m_fullScreen;
};

#endif /* SHOWFOTO_H */
