/* ============================================================
 * File  : imageguiclient.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-12
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

#include "imageguiclient.h"

ImageGUIClient::ImageGUIClient(QWidget *parent)
    : QObject(parent), Digikam::GUIClient(),
      m_parent(parent)
{
    m_navPrevAction = new KAction(i18n("&Previous"), "back",
                                  KStdAccel::shortcut( KStdAccel::Prior),
                                  this, SIGNAL(signalPrev()),
                                  actionCollection(), "imageview_prev");
                
    m_navNextAction = new KAction(i18n("&Next"), "forward",
                                  KStdAccel::shortcut( KStdAccel::Next),
                                  this, SIGNAL(signalNext()),
                                  actionCollection(), "imageview_next"); 

    m_navFirstAction = new KAction(i18n("&First"), "start",
                                   KStdAccel::shortcut( KStdAccel::Home),
                                   this, SIGNAL(signalFirst()),
                                   actionCollection(), "imageview_first"); 
                
    m_navLastAction = new KAction(i18n("&Last"), "finish",
                                  KStdAccel::shortcut( KStdAccel::End),
                                  this, SIGNAL(signalLast()),
                                  actionCollection(), "imageview_last");

    m_saveAction = KStdAction::save(this, SIGNAL(signalSave()),
                                    actionCollection(), "imageview_save");
    m_saveAsAction = KStdAction::saveAs(this, SIGNAL(signalSaveAs()),
                                        actionCollection(), "imageview_saveas");
    m_restoreAction = KStdAction::revert(this, SIGNAL(signalRestore()),
                                        actionCollection(), "imageview_restore");
    m_saveAction->setEnabled(false);
    m_restoreAction->setEnabled(false);

    KStdAction::quit(this, SIGNAL(signalExit()), 
                     actionCollection(), "imageview_exit");

    m_zoomPlusAction = new KAction(i18n("Zoom &In"), "viewmag+",
                                   Key_Plus,
                                   this, SIGNAL(signalZoomPlus()),
                                   actionCollection(), "imageview_zoom_plus");

    m_zoomMinusAction = new KAction(i18n("Zoom &Out"), "viewmag-",
                                    Key_Minus, 
                                    this, SIGNAL(signalZoomMinus()),
                                    actionCollection(), "imageview_zoom_minus");

    m_zoomFitAction = new KToggleAction(i18n("Zoom &AutoFit"), "viewmagfit",
                                        Key_A, 
                                        this, SIGNAL(signalZoomFit()),
                                        actionCollection(), "imageview_zoom_fit");

    new KAction(i18n("&Resize..."), 0, 0,
                     this, SIGNAL(signalResize()),
                     actionCollection(), "imageview_resize");

    m_cropAction = new KAction(i18n("&Crop"), "crop",
                               CTRL+Key_C, 
                               this, SIGNAL(signalCrop()),
                               actionCollection(), "imageview_crop");
    m_cropAction->setEnabled(false);

    m_rotateAction = new KActionMenu(i18n("&Rotate"), "rotate_cw",
                                     actionCollection(),
                                     "imageview_rotate");
    m_rotateAction->setDelayed(false);

    m_rotateAction->insert( new KAction(i18n("90 degrees"),
                                        0, Key_1, this, SIGNAL(signalRotate90()),
                                        actionCollection(),
                                        "rotate_90") );

    m_rotateAction->insert( new KAction(i18n("180 degrees"),
                                        0, Key_2, this, SIGNAL(signalRotate180()),
                                        actionCollection(),
                                        "rotate_180") );

    m_rotateAction->insert( new KAction(i18n("270 degrees"),
                                        0, Key_3, this, SIGNAL(signalRotate270()),
                                        actionCollection(),
                                        "rotate_270") );

    m_flipAction = new KActionMenu(i18n("Flip"),
                                   "flip_image",
                                   actionCollection(),
                                   "imageview_flip");

    m_flipAction->insert( new KAction(i18n("Horizontally"), 0,
                                      this, SIGNAL(signalFlipHoriz()),
                                      actionCollection(),
                                      "flip_horizontal") );

    m_flipAction->insert( new KAction(i18n("Vertically"), 0,
                                      this, SIGNAL(signalFlipVert()),
                                      actionCollection(),
                                      "flip_vertical") );

    KStdAction::help(this, SLOT(slotHelp()), actionCollection(),
                     "imageview_help");
    KStdAction::aboutApp(this, SLOT(slotAboutApp()), actionCollection(),
                         "imageview_aboutapp");
    KStdAction::aboutKDE(this, SLOT(slotAboutKDE()), actionCollection(),
                         "imageview_aboutkde");
}

QStringList ImageGUIClient::guiDefinition() const
{
    QStringList guiDef;

    guiDef.append("MenuBar/Menu/&File/ /Action/imageview_prev/ ");
    guiDef.append("MenuBar/Menu/&File/ /Action/imageview_next/ ");
    guiDef.append("MenuBar/Menu/&File/ /Separator/ / ");
    guiDef.append("MenuBar/Menu/&File/ /Action/imageview_first/ ");
    guiDef.append("MenuBar/Menu/&File/ /Action/imageview_last/ ");
    guiDef.append("MenuBar/Menu/&File/ /Separator/ / ");
    guiDef.append("MenuBar/Menu/&File/ /Action/imageview_save/ ");
    guiDef.append("MenuBar/Menu/&File/ /Action/imageview_saveas/ ");
    guiDef.append("MenuBar/Menu/&File/ /Action/imageview_restore/ ");
    guiDef.append("MenuBar/Menu/&File/ /Separator/ / ");
    guiDef.append("MenuBar/Menu/&File/ /Action/imageview_exit/ ");

    guiDef.append("MenuBar/Menu/&Transform/ /Action/imageview_rotate/ ");
    guiDef.append("MenuBar/Menu/&Transform/ /Action/imageview_flip/ ");
    guiDef.append("MenuBar/Menu/&Transform/ /Action/imageview_crop/ ");
    guiDef.append("MenuBar/Menu/&Transform/ /Action/imageview_resize/ ");

    guiDef.append("MenuBar/Menu/&View/ /Action/imageview_zoom_plus/ ");
    guiDef.append("MenuBar/Menu/&View/ /Action/imageview_zoom_minus/ ");
    guiDef.append("MenuBar/Menu/&View/ /Action/imageview_zoom_fit/ ");

    guiDef.append("MenuBar/DefineGroup/Generic/ ");
    
    guiDef.append("MenuBar/Menu/&Help/ /Action/imageview_help/ ");
    guiDef.append("MenuBar/Menu/&Help/ /Action/imageview_aboutapp/ ");
    guiDef.append("MenuBar/Menu/&Help/ /Action/imageview_aboutkde/ ");
    
    guiDef.append("ToolBar/Action/imageview_first/ ");
    guiDef.append("ToolBar/Action/imageview_prev/ ");
    guiDef.append("ToolBar/Action/imageview_next/ ");
    guiDef.append("ToolBar/Action/imageview_last/ ");
    guiDef.append("ToolBar/Separator/ / ");
    guiDef.append("ToolBar/Action/imageview_save/ ");
    guiDef.append("ToolBar/Action/imageview_restore/ ");
    guiDef.append("ToolBar/Separator/ / ");
    guiDef.append("ToolBar/Action/imageview_zoom_plus/ ");
    guiDef.append("ToolBar/Action/imageview_zoom_minus/ ");
    guiDef.append("ToolBar/Action/imageview_zoom_fit/ ");
    guiDef.append("ToolBar/Separator/ / ");
    guiDef.append("ToolBar/Action/imageview_rotate/ ");
    guiDef.append("ToolBar/Action/imageview_crop/ ");

    guiDef.append("PopupMenu/Action/imageview_prev/ ");
    guiDef.append("PopupMenu/Action/imageview_next/ ");
    guiDef.append("PopupMenu/Separator/ / ");
    guiDef.append("PopupMenu/Action/imageview_rotate/ ");
    guiDef.append("PopupMenu/Action/imageview_flip/ ");
    guiDef.append("PopupMenu/Action/imageview_crop/ ");

    return guiDef;
}

void ImageGUIClient::slotHelp()
{
    KApplication::kApplication()->invokeHelp( "imageviewer.anchor", "digikam" );
}

void ImageGUIClient::slotAboutApp()
{
    KAboutApplication dlg;
    dlg.exec();
}

void ImageGUIClient::slotAboutKDE()
{
    KAboutKDE dlg;
    dlg.exec();
}

#include "imageguiclient.moc"
