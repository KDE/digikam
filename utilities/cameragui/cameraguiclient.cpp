/* ============================================================
 * File  : cameraguiclient.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-17
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <kaction.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kapplication.h>
#include <kaboutapplication.h>
#include <kaboutkde.h>

#include "cameraguiclient.h"

CameraGUIClient::CameraGUIClient(QWidget *parent)
    : QObject(parent), Digikam::GUIClient()
{
    m_downloadAction = new KActionMenu(i18n("Download"),
                                       "cameradownload",
                                       actionCollection(),
                                       "download");
    m_downloadAction->setDelayed(false);

    m_downloadSelAction = new KAction(i18n("Selected"),
                                      0,
                                      this,
                                      SIGNAL(signalDownloadSelected()),
                                      actionCollection());
    
    KAction* downloadAllAction = new KAction(i18n("All"),
                                             0,
                                             this,
                                             SIGNAL(signalDownloadAll()),
                                             actionCollection());

    m_downloadAction->insert(m_downloadSelAction);
    m_downloadAction->insert(downloadAllAction);

    m_deleteAction  = new KActionMenu(i18n("Delete"),
                                      "deleteimage",
                                      actionCollection(),
                                      "delete");
    m_deleteAction->setDelayed(false);

    m_deleteSelAction = new KAction(i18n("Selected"),
                                    0,
                                    this,
                                    SIGNAL(signalDeleteSelected()),
                                    actionCollection());
    
    KAction* deleteAllAction = new KAction(i18n("All"),
                                           0,
                                           this,
                                           SIGNAL(signalDeleteAll()),
                                           actionCollection());

    m_deleteAction->insert(m_deleteSelAction);
    m_deleteAction->insert(deleteAllAction);

    m_downloadSelAction->setEnabled(false);
    m_deleteSelAction->setEnabled(false);
    
    m_cancelAction = new KAction(i18n("Stop"),
                                 "stop",
                                 KShortcut(Key_Escape),
                                 this,
                                 SIGNAL(signalCancel()),
                                 actionCollection(),
                                 "cancel");

    KStdAction::quit(this, SIGNAL(signalExit()), 
                     actionCollection(), "exit");
    
    KStdAction::help(this, SLOT(slotHelp()), actionCollection(),
                     "help");
    KStdAction::aboutApp(this, SLOT(slotAboutApp()), actionCollection(),
                         "aboutapp");
    KStdAction::aboutKDE(this, SLOT(slotAboutKDE()), actionCollection(),
                         "aboutkde");
}

QStringList CameraGUIClient::guiDefinition() const
{
    QStringList guiDef;

    guiDef.append("MenuBar/Menu/&Camera/ /Action/download/ ");
    //guiDef.append("MenuBar/Menu/&Camera/ /Action/delete/ ");
    guiDef.append("MenuBar/Menu/&Camera/ /Separator/ / ");
    guiDef.append("MenuBar/Menu/&Camera/ /Action/cancel/ ");
    guiDef.append("MenuBar/Menu/&Camera/ /Separator/ / ");
    guiDef.append("MenuBar/Menu/&Camera/ /Action/exit/ ");
    
    guiDef.append("MenuBar/Menu/&Help/ /Action/help/ ");
    guiDef.append("MenuBar/Menu/&Help/ /Action/aboutapp/ ");
    guiDef.append("MenuBar/Menu/&Help/ /Action/aboutkde/ ");
    
    guiDef.append("ToolBar/Action/download/ ");
    //guiDef.append("ToolBar/Action/delete/ ");
    guiDef.append("ToolBar/Separator/ / ");
    guiDef.append("ToolBar/Action/cancel/ ");

    // enable i18n

    i18n( "&File" );
    i18n( "&Camera" );
    i18n( "&Help" );
    
    return guiDef;
}

void CameraGUIClient::slotHelp()
{
    //kapp->invokeHelp( ".anchor", "digikam" );
}

void CameraGUIClient::slotAboutApp()
{
    KAboutApplication dlg;
    dlg.exec();
}

void CameraGUIClient::slotAboutKDE()
{
    KAboutKDE dlg;
    dlg.exec();
}

#include "cameraguiclient.moc"
