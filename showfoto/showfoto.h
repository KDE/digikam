/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
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
class ThumbBarView;
class ThumbBarItem;
class ImagePropertiesSideBar;
class SplashScreen;
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
    void setup(bool iccSetupPage=false);

private:
    
    bool                             m_fullScreenHideThumbBar;
    bool                             m_deleteItem2Trash;
    
    int                              m_itemsNb;

    KURL                             m_lastOpenedDirectory;
    
    KToggleAction                   *m_showBarAction;

    KAction                         *m_openFilesInFolderAction;
    KAction                         *m_fileOpenAction;
    
    KActionMenu                     *m_BCGAction;
    
    Digikam::ThumbBarView           *m_bar;
    Digikam::ThumbBarItem           *m_currentItem;
    Digikam::ImagePropertiesSideBar *m_rightSidebar;
    Digikam::SplashScreen           *m_splash;

private:

    void closeEvent(QCloseEvent* e);

    void setupActions();
    void setupConnections();
    void setupUserArea();

    void readSettings();
    void saveSettings();
    void applySettings();

    void toggleActions(bool val);
    void toggleActions2SlideShow(bool val);
    
    void toggleGUI2FullScreen();
   
    void toggleNavigation(int index);

    bool save();
    bool saveAs();
    void finishSaving(bool success);

    void saveIsComplete();
    void saveAsIsComplete(); 

private slots:

    void slotForward();
    void slotBackward();
    void slotLast();
    void slotFirst();
    void slotFilePrint();

    void slotOpenFile();
    void slotOpenURL(const KURL& url);
    void slotOpenFolder(const KURL& url);
    void slotOpenFilesInFolder();
    void slotDeleteCurrentItem();

    void slotToggleShowBar();
    void slotChangeBCG();
    
    void slotChanged();
    void slotUndoStateChanged(bool, bool, bool);
    void slotUpdateItemInfo();
        
    void slotDeleteCurrentItemResult( KIO::Job * job );

    void slotLoadingStarted(const QString &filename);
    void slotLoadingFinished(const QString &filename, bool success);
    void slotSavingStarted(const QString &filename);
};

}   // namespace ShowFoto

#endif /* SHOWFOTO_H */
