/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Tom Albers <tomalbers@kde.nl>
 *          Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date   : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 * 
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2005-2006 by Tom Albers 
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

namespace KIO
{
class Job;
}

namespace Digikam
{
class SlideshowSettings;
}

namespace ShowFoto
{

class ShowFotoPriv;

class ShowFoto : public Digikam::EditorWindow
{
    Q_OBJECT
    
public:

    ShowFoto(const KURL::List& urlList);
    ~ShowFoto();
    
    virtual void show();
    bool setup(bool iccSetupPage=false);

private:

    bool queryClose();
    bool queryExit();

    void setupActions();
    void setupConnections();
    void setupUserArea();

    void readSettings();
    void saveSettings();
    void applySettings();

    void toggleActions(bool val);
    
    void toggleGUI2FullScreen();
   
    void toggleNavigation(int index);

    bool save();
    bool saveAs();
    void finishSaving(bool success);

    void saveIsComplete();
    void saveAsIsComplete(); 

    void slideShow(bool startWithCurrent, Digikam::SlideShowSettings& settings);

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

    void slotContextMenu();

private:
    
    ShowFotoPriv* d;

};

}   // namespace ShowFoto

#endif /* SHOWFOTO_H */
