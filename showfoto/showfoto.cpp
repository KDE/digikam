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

#include <kaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kimageio.h>

#include "canvas.h"
#include "showfoto.h"

ShowFoto::ShowFoto(const KURL::List& urlList)
{

    m_canvas = new Canvas(this);
    setCentralWidget(m_canvas);

    setupActions();

    m_urlList = urlList;
    if (!urlList.isEmpty())
    {
        m_urlCurrent = urlList.first();
        m_canvas->load(m_urlCurrent.path());
    }

    resize(800,600);
}

ShowFoto::~ShowFoto()
{
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
        m_urlList = urls;
        m_urlCurrent = urls.first();
        m_canvas->load(m_urlCurrent.path());
    }
}

void ShowFoto::slotNext()
{
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);
    if (it != m_urlList.end()) {

        if (m_urlCurrent != m_urlList.last()) {
           KURL urlNext = *(++it);
           m_urlCurrent = urlNext;
           m_canvas->load(m_urlCurrent.path());
        }
    }
}

void ShowFoto::slotPrev()
{
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);
    if (it != m_urlList.begin()) {

        if (m_urlCurrent != m_urlList.first())
        {
            KURL urlPrev = *(--it);
            m_urlCurrent = urlPrev;
            m_canvas->load(m_urlCurrent.path());
        }
    }
}

void ShowFoto::slotAutoFit()
{
    bool checked = m_zoomFitAction->isChecked();

    m_zoomPlusAction->setEnabled(!checked);
    m_zoomMinusAction->setEnabled(!checked);

    m_canvas->slotToggleAutoZoom();
}

#include "showfoto.moc"

