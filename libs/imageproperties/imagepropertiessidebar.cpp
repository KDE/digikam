/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004-2006 by Gilles Caulier
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

// Qt includes. 
 
#include <qrect.h>
#include <qsplitter.h>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>

// Local includes.

#include "dimg.h"
#include "imagepropertiesexiftab.h"
#include "imagepropertiescolorstab.h"
#include "imagepropertiessidebar.h"

namespace Digikam
{

ImagePropertiesSideBar::ImagePropertiesSideBar(QWidget *parent, QSplitter *splitter, 
                                               Side side, bool mimimizedDefault)
                      : Digikam::Sidebar(parent, side, mimimizedDefault)
{
    m_img               = 0;
    m_currentRect       = 0;
    m_dirtyExifTab      = false;
    m_dirtyHistogramTab = false;
    
    m_exifTab      = new ImagePropertiesEXIFTab(parent, false);
    m_histogramTab = new ImagePropertiesColorsTab(parent, 0, false);
    
    setSplitter(splitter);
         
    appendTab(m_exifTab, SmallIcon("exifinfo"), i18n("EXIF"));    
    appendTab(m_histogramTab, SmallIcon("blend"), i18n("Colors"));
    loadViewState();
    
    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));
}

ImagePropertiesSideBar::~ImagePropertiesSideBar()
{
}

void ImagePropertiesSideBar::itemChanged(const KURL& url, QRect *rect, DImg *img)
{
    if (!url.isValid())
        return;
    
    m_currentURL        = url;
    m_currentRect       = rect;
    m_img               = img;
    m_dirtyExifTab      = false;
    m_dirtyHistogramTab = false;
    
    slotChangedTab( getActiveTab() );    
}

void ImagePropertiesSideBar::noCurrentItem(void)
{
    m_currentURL = KURL::KURL();
    m_exifTab->setCurrentURL();
    m_histogramTab->setData();
    m_dirtyExifTab      = false;
    m_dirtyHistogramTab = false;    
}

void ImagePropertiesSideBar::imageSelectionChanged(QRect *rect)
{
    m_currentRect = rect;
    
    if (m_dirtyHistogramTab)
       m_histogramTab->setSelection(rect);
    else
       slotChangedTab(m_histogramTab);
}

void ImagePropertiesSideBar::slotChangedTab(QWidget* tab)
{
    if (!m_currentURL.isValid())
        return;
    
    setCursor(KCursor::waitCursor());
    
    if (tab == m_exifTab && !m_dirtyExifTab)
    {
       m_exifTab->setCurrentURL(m_currentURL);
       m_dirtyExifTab = true;
    }
    else if (tab == m_histogramTab && !m_dirtyHistogramTab)
    {
       m_histogramTab->setData(m_currentURL, m_currentRect, m_img);
       m_dirtyHistogramTab = true;
    }
    
    setCursor( KCursor::arrowCursor() );
}

}  // NameSpace Digikam

#include "imagepropertiessidebar.moc"

