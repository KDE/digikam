/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2004-11-17
 * Description : simple image properties side bar (without support 
 *               of digiKam database).
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
#include <kiconloader.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "imagepropertiestab.h"
#include "imagepropertiesmetadatatab.h"
#include "imagepropertiescolorstab.h"
#include "imagepropertiessidebar.h"

namespace Digikam
{

ImagePropertiesSideBar::ImagePropertiesSideBar(QWidget *parent, const char *name, 
                                               QSplitter *splitter, Side side, 
                                               bool mimimizedDefault, bool navBar)
                      : Sidebar(parent, name, side, mimimizedDefault)
{
    m_image              = 0;
    m_currentRect        = 0;
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    
    m_propertiesTab = new ImagePropertiesTab(parent, navBar);
    m_metadataTab   = new ImagePropertiesMetaDataTab(parent, navBar);
    m_colorTab      = new ImagePropertiesColorsTab(parent, 0, navBar);
    
    setSplitter(splitter);
         
    appendTab(m_propertiesTab, SmallIcon("info"), i18n("Properties"));
    appendTab(m_metadataTab, SmallIcon("exifinfo"), i18n("Metadata"));
    appendTab(m_colorTab, SmallIcon("blend"), i18n("Colors"));
    
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

    if (url  == m_currentURL &&
        rect == m_currentRect &&
        img  == m_image)
        return;
    
    m_currentURL         = url;
    m_currentRect        = rect;
    m_image              = img;
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    
    slotChangedTab( getActiveTab() );    
}

void ImagePropertiesSideBar::slotNoCurrentItem(void)
{
    m_currentURL = KURL();
    m_propertiesTab->setCurrentURL();
    m_metadataTab->setCurrentURL();
    m_colorTab->setData();
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
}

void ImagePropertiesSideBar::slotImageSelectionChanged(QRect *rect)
{
    m_currentRect = rect;
    
    if (m_dirtyColorTab)
       m_colorTab->setSelection(rect);
    else
       slotChangedTab(m_colorTab);
}

void ImagePropertiesSideBar::slotChangedTab(QWidget* tab)
{
    if (!m_currentURL.isValid())
        return;
    
    setCursor(KCursor::waitCursor());
    
    if (tab == m_propertiesTab && !m_dirtyPropertiesTab)
    {
       m_propertiesTab->setCurrentURL(m_currentURL);
       m_dirtyPropertiesTab = true;
    }
    else if (tab == m_metadataTab && !m_dirtyMetadataTab)
    {
       m_metadataTab->setCurrentURL(m_currentURL);
       m_dirtyMetadataTab = true;
    }
    else if (tab == m_colorTab && !m_dirtyColorTab)
    {
       m_colorTab->setData(m_currentURL, m_currentRect, m_image);
       m_dirtyColorTab = true;
    }
    
    unsetCursor();
}

}  // NameSpace Digikam

#include "imagepropertiessidebar.moc"

