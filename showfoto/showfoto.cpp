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
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kimageio.h>
#include <kaccel.h>
#include <kpropertiesdialog.h>
#include <kdeversion.h>
#include <kio/netaccess.h>

// Local includes.

#include "canvas.h"
#include "thumbbar.h"
#include "showfoto.h"

ShowFoto::ShowFoto(const KURL::List& urlList)
{
    m_config = kapp->config();
    m_fullScreen = false;
    
    QWidget* widget = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(widget);

    m_canvas = new Canvas(widget);
    m_bar    = new ThumbBarView(widget);
    lay->addWidget(m_canvas);
    lay->addWidget(m_bar);

    setCentralWidget(widget);
    setupActions();
    applySettings();

    connect(m_bar, SIGNAL(signalURLSelected(const KURL&)),
            SLOT(slotOpenURL(const KURL&)));

    for (KURL::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        new ThumbBarItem(m_bar, *it);
    }

    setAutoSaveSettings();
    applyMainWindowSettings(m_config);
}

ShowFoto::~ShowFoto()
{
    saveSettings();
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

    new KAction(i18n("Properties"), 0,
                ALT+Key_Return,
                this, SLOT(slotFileProperties()),
                actionCollection(), "file_properties");

    // ---------------------------------------------------------------
    
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

    m_showBarAction =
        new KToggleAction(i18n("Hide thumbnails"), 0, Key_T,
                          this, SLOT(slotToggleShowBar()),
                          actionCollection(), "show_thumbs");

    // ---------------------------------------------------------------
    
    new KAction(i18n("Rotate 90"), 0, Key_9,
                m_canvas, SLOT(slotRotate90()),
                actionCollection(), "rotate_90");
    new KAction(i18n("Rotate 180"), 0, Key_8,
                m_canvas, SLOT(slotRotate180()),
                actionCollection(), "rotate_180");
    new KAction(i18n("Rotate 270"), 0, Key_7,
                m_canvas, SLOT(slotRotate270()),
                actionCollection(), "rotate_270");

    new KAction(i18n("Flip Horizontally"), 0, Key_Asterisk,
                m_canvas, SLOT(slotFlipHoriz()),
                actionCollection(), "flip_horiz");
    new KAction(i18n("Flip Vertically"), 0, Key_Slash,
                m_canvas, SLOT(slotFlipVert()),
                actionCollection(), "flip_vert");
    
    // ---------------------------------------------------------------
    
    createGUI("showfotoui.rc", false);

    KAccel *accel = new KAccel(this);
    accel->insert("Exit fullscreen", i18n("Exit Fullscreen"),
                  i18n("Exit out of the fullscreen mode"),
                  Key_Escape, this, SLOT(slotEscapePressed()),
                  false, true);
}

void ShowFoto::applySettings()
{
    bool showBar = false;
    m_config->setGroup("MainWindow");
    showBar = m_config->readBoolEntry("Show Thumbnails", true);

    if (!showBar)
        m_showBarAction->activate();
}

void ShowFoto::saveSettings()
{
    m_config->setGroup("MainWindow");
    m_config->writeEntry("Show Thumbnails", !m_showBarAction->isChecked());
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
    QString localFile;
#if KDE_IS_VERSION(3,2,0)
    KIO::NetAccess::download(url, localFile, this);
#else
    KIO::NetAccess::download(url, localFile);
#endif
    m_canvas->load(localFile);
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

void ShowFoto::slotToggleShowBar()
{
    if (m_showBarAction->isChecked())
    {
        m_bar->hide();
    }
    else
    {
        m_bar->show();
    }
}

#include "showfoto.moc"

