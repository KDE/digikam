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

#include <qlayout.h>

#include <kaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kimageio.h>

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
                     actionCollection());
    KStdAction::quit(this, SLOT(close()),
                     actionCollection());
    KStdAction::forward(this, SLOT(slotNext()),
                        actionCollection());
    KStdAction::back(this, SLOT(slotPrev()),
                     actionCollection());

    m_zoomPlusAction =
        KStdAction::zoomIn(m_canvas, SLOT(slotIncreaseZoom()),
                           actionCollection());
    m_zoomMinusAction =
        KStdAction::zoomOut(m_canvas, SLOT(slotDecreaseZoom()),
                       actionCollection());
    m_zoomFitAction
        = new KToggleAction(i18n("Zoom &AutoFit"), "viewmagfit",
                            Key_A,
                            this, SLOT(slotAutoFit()),
                            actionCollection(), "zoom_fit");
    
    createGUI("showfotoui.rc", false);
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
        for (KURL::List::const_iterator it = urls.begin(); it != urls.end(); ++it)
        {
            new ThumbBarItem(m_bar, *it);
        }
    }
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

#include "showfoto.moc"

