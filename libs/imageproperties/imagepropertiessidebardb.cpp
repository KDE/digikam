/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description : image properties side bar using data from 
 *               digiKam database.
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
#include "albumiconitem.h"
#include "albumiconview.h"
#include "imagepropertiesexiftab.h"
#include "imagepropertiescolorstab.h"
#include "imagedescedittab.h"
#include "navigatebarwidget.h"
#include "imagepropertiessidebardb.h"

namespace Digikam
{

ImagePropertiesSideBarDB::ImagePropertiesSideBarDB(QWidget *parent, QSplitter *splitter, 
                                                   Side side, bool mimimizedDefault, bool navBar)
                        : Digikam::Sidebar(parent, side, mimimizedDefault)
{
    m_img               = 0;
    m_currentRect       = 0;
    m_currentView       = 0;
    m_currentItem       = 0;
    m_dirtyExifTab      = false;
    m_dirtyHistogramTab = false;
    m_dirtyDesceditTab  = false;

    m_exifTab      = new ImagePropertiesEXIFTab(parent, navBar);
    m_histogramTab = new ImagePropertiesColorsTab(parent, 0, navBar);
    m_desceditTab  = new ImageDescEditTab(parent, navBar);
    
    setSplitter(splitter);
         
    appendTab(m_exifTab, SmallIcon("exifinfo"), i18n("EXIF"));    
    appendTab(m_histogramTab, SmallIcon("blend"), i18n("Colors"));
    appendTab(m_desceditTab, SmallIcon("imagecomment"), i18n("Comments && Tags"));
    loadViewState();

    // ----------------------------------------------------------
    
    connect(m_exifTab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(m_exifTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(m_exifTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(m_exifTab, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));

    connect(m_histogramTab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(m_histogramTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(m_histogramTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(m_histogramTab, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
                            
    connect(m_desceditTab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(m_desceditTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(m_desceditTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(m_desceditTab, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
            
    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));
}

ImagePropertiesSideBarDB::~ImagePropertiesSideBarDB()
{
}

void ImagePropertiesSideBarDB::itemChanged(const KURL& url, AlbumIconView* view, 
                                           AlbumIconItem* item, QRect *rect, DImg *img)
{
    if (!url.isValid())
        return;
    
    m_currentURL        = url;
    m_currentRect       = rect;
    m_img               = img;
    m_currentView       = view;
    m_currentItem       = item;
    m_dirtyExifTab      = false;
    m_dirtyHistogramTab = false;
    m_dirtyDesceditTab  = false;
        
    slotChangedTab( getActiveTab() );    
}

void ImagePropertiesSideBarDB::noCurrentItem(void)
{
    m_currentURL        = KURL::KURL();
    m_currentItem       = 0;
    m_exifTab->setCurrentURL();
    m_histogramTab->setData();
    m_desceditTab->setItem();
    m_dirtyExifTab      = false;
    m_dirtyHistogramTab = false;
    m_dirtyDesceditTab  = false;
}

void ImagePropertiesSideBarDB::imageSelectionChanged(QRect *rect)
{
    m_currentRect = rect;
    
    if (m_dirtyHistogramTab)
       m_histogramTab->setSelection(rect);
    else
       slotChangedTab(m_histogramTab);
}

void ImagePropertiesSideBarDB::populateTags(void)
{
    m_desceditTab->populateTags();
}

void ImagePropertiesSideBarDB::slotChangedTab(QWidget* tab)
{
    setCursor(KCursor::waitCursor());

    // No database data available, for example in the case of image editor is 
    // launched from camera GUI.
    if (!m_currentView || !m_currentItem)
    {
        if (tab == m_exifTab && !m_dirtyExifTab)
        {
            m_exifTab->setCurrentURL(m_currentURL, NavigateBarWidget::ItemCurrent);
            m_dirtyExifTab = true;
        }
        else if (tab == m_histogramTab && !m_dirtyHistogramTab)
        {
            m_histogramTab->setData(m_currentURL, m_currentRect, m_img, NavigateBarWidget::ItemCurrent);
            m_dirtyHistogramTab = true;
        }
        else if (tab == m_desceditTab && !m_dirtyDesceditTab)
        {
            // Do nothing here. We cannot get data from database !
            m_desceditTab->setItem();
            m_dirtyDesceditTab = true;
        }
    }
    else    // Data from database available...
    {
        int currentItemType = NavigateBarWidget::ItemCurrent;
        
        if (m_currentView->firstItem() == m_currentItem) 
           currentItemType = NavigateBarWidget::ItemFirst;
        else if (m_currentView->lastItem() == m_currentItem) 
            currentItemType = NavigateBarWidget::ItemLast;
        
        if (tab == m_exifTab && !m_dirtyExifTab)
        {
            m_exifTab->setCurrentURL(m_currentURL, currentItemType);
            m_dirtyExifTab = true;
        }
        else if (tab == m_histogramTab && !m_dirtyHistogramTab)
        {
            m_histogramTab->setData(m_currentURL, m_currentRect, m_img, currentItemType);
            m_dirtyHistogramTab = true;
        }
        else if (tab == m_desceditTab && !m_dirtyDesceditTab)
        {
            m_desceditTab->setItem(m_currentView, m_currentItem, currentItemType);
           m_dirtyDesceditTab = true;
        }
    }

    setCursor( KCursor::arrowCursor() );
}

}  // NameSpace Digikam

#include "imagepropertiessidebardb.moc"
