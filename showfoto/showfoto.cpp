/* ============================================================
 * File  : showfoto.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-11-22
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

// Qt includes. 
 
#include <qlayout.h>

// KDE includes.

#include <kaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kimageio.h>
#include <kaccel.h>
#include <kpropertiesdialog.h>
#include <kdeversion.h>

// Local includes.

#include "canvas.h"
#include "thumbbar.h"
#include "showfoto.h"

ShowFoto::ShowFoto(const KURL::List& urlList)
{
    QWidget* widget = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(widget);

    m_canvas = new Canvas(widget);
    m_bar    = new ThumbBarView(widget);
    lay->addWidget(m_canvas);
    lay->addWidget(m_bar);

    m_fullScreen = false;
    
    setCentralWidget(widget);
    setupActions();

    connect(m_bar, SIGNAL(signalURLSelected(const KURL&)),
            SLOT(slotOpenURL(const KURL&)));

    if (!urlList.isEmpty())
    {
        for (KURL::List::const_iterator it = urlList.begin();
             it != urlList.end(); ++it)
        {
            new ThumbBarItem(m_bar, *it);
        }
    }

    resize(800,600);
}

ShowFoto::~ShowFoto()
{
    delete m_bar;
    delete m_canvas;
}

void ShowFoto::setupActions()
{

    KStdAction::open(this, SLOT(slotOpenFile()),
                     actionCollection(), "open_file");
    KStdAction::quit(this, SLOT(close()),
                     actionCollection());
    KStdAction::forward(this, SLOT(slotNext()),
                        actionCollection(), "go_fwd");
    KStdAction::back(this, SLOT(slotPrev()),
                     actionCollection(), "go_bwd");

    m_fileproperties = new KAction(i18n("Properties"), "exifinfo",
                                   ALT+Key_Return,
                                   this, SLOT(slotFileProperties()),
                                   actionCollection(), "file_properties");
                     
    m_zoomPlusAction =
        KStdAction::zoomIn(m_canvas, SLOT(slotIncreaseZoom()),
                           actionCollection(), "zoom_plus");
    m_zoomMinusAction =
        KStdAction::zoomOut(m_canvas, SLOT(slotDecreaseZoom()),
                            actionCollection(), "zoom_minus");
    m_zoomFitAction
        = new KToggleAction(i18n("Zoom &AutoFit"), "viewmagfit",
                            Key_A,
                            this, SLOT(slotAutoFit()),
                            actionCollection(), "zoom_fit");

#if KDE_IS_VERSION(3,2,0)
    m_fullScreenAction =
        KStdAction::fullScreen(this, SLOT(slotToggleFullScreen()),
                               actionCollection(), this, "full_screen");
#else 
    m_fullScreenAction =
        new KToggleAction(i18n("Fullscreen"), "window_fullscreen",
                          CTRL+SHIFT+Key_F, this,
                          SLOT(slotToggleFullScreen()),
                          actionCollection(), "full_screen");
#endif
    
    createGUI("showfotoui.rc", false);

    KAccel *accel = new KAccel(this);
    accel->insert("Exit fullscreen", i18n("Exit Fullscreen"),
                  i18n("Exit out of the fullscreen mode"),
                  Key_Escape, this, SLOT(slotEscapePressed()),
                  false, true);
}

void ShowFoto::slotOpenFile()
{
    QString mimes = KImageIO::mimeTypes(KImageIO::Reading).join(" ");
    KURL::List urls =  KFileDialog::getOpenURLs(QString::null,
                                                mimes,
                                                this,
                                                i18n("Open images"));
        
    if (!urls.isEmpty())
    {
        m_bar->clear();
        for (KURL::List::const_iterator it = urls.begin();
             it != urls.end(); ++it)
        {
            new ThumbBarItem(m_bar, *it);
        }
    }
}

void ShowFoto::slotFileProperties()
{
    ThumbBarItem* curr = m_bar->currentItem();
    
    if (curr)
        (void) new KPropertiesDialog( curr->url(), this, "props dialog", true );
}

void ShowFoto::slotNext()
{
    ThumbBarItem* curr = m_bar->currentItem();
    if (curr && curr->next())
    {
        m_bar->setSelected(curr->next());
    }
}

void ShowFoto::slotPrev()
{
    ThumbBarItem* curr = m_bar->currentItem();
    if (curr && curr->prev())
    {
        m_bar->setSelected(curr->prev());
    }
}

void ShowFoto::slotOpenURL(const KURL& url)
{
    m_canvas->load(url.path());
}

void ShowFoto::slotAutoFit()
{
    bool checked = m_zoomFitAction->isChecked();

    m_zoomPlusAction->setEnabled(!checked);
    m_zoomMinusAction->setEnabled(!checked);

    m_canvas->slotToggleAutoZoom();
}

void ShowFoto::slotToggleFullScreen()
{
    if (m_fullScreen)
    {

#if QT_VERSION >= 0x030300
        setWindowState( windowState() & ~WindowFullScreen );
#else
        showNormal();
#endif
        menuBar()->show();
        QObject* obj = child("mainToolBar","KToolBar");
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            toolBar->show();
        }
    
        m_fullScreen = false;
    }
    else
    {
        // hide the menubar and the statusbar
        menuBar()->hide();

        QObject* obj = child("mainToolBar","KToolBar");
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            toolBar->hide();
        }

        showFullScreen();
        m_fullScreen = true;
    }
}

void ShowFoto::slotEscapePressed()
{
    if (!m_fullScreen)
        return;

    m_fullScreenAction->activate();
}

#include "showfoto.moc"

