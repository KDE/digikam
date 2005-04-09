/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004-2005 by Gilles Caulier
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
 
#include <qlayout.h>
#include <qsize.h>
#include <qrect.h>
#include <qcheckbox.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kdebug.h>

// Local includes.

#include "albumiconview.h"
#include "albumiconitem.h"

#include "imagepropertiesgeneral.h"
#include "imagepropertiesexif.h"
#include "imagepropertieshistogram.h"
#include "imageproperties.h"


// Constructor with AlbumIconView and AlbumIconItem instance.

ImageProperties::ImageProperties(AlbumIconView* view, AlbumIconItem* currItem)
               : KDialogBase(Tabbed, QString::null, 
                             Help|User1|User2|Stretch|Close,
                             Close, view, 0, true, true, 
                             KStdGuiItem::guiItem(KStdGuiItem::Forward), 
                             KStdGuiItem::guiItem(KStdGuiItem::Back))
{
    m_view          = view; 
    m_currItem      = currItem;
    m_currURL       = currItem->fileItem()->url();
    m_mode          = MULTI;
    
    m_selectionArea = 0;
    m_imageData     = 0;
    m_imageWidth    = 0;
    m_imageHeight   = 0;
    
    enableButton(User1, m_currItem->nextItem() != 0);
    enableButton(User2, m_currItem->prevItem() != 0);
    
    setupGui();
}


ImageProperties::ImageProperties(QWidget *parent, const KURL& url, QRect *selectionArea, 
                                 uint* imageData, int imageWidth, int imageHeight)
               : KDialogBase(Tabbed, QString::null, 
                             Help|Stretch|Close,
                             Close, parent, 0, true, true)
{
    m_view          = 0L;
    m_currItem      = 0L;
    m_currURL       = url;
    m_mode          = SINGLE;
    
    m_imageData     = imageData;
    m_imageWidth    = imageWidth;
    m_imageHeight   = imageHeight;
    m_selectionArea = selectionArea;
    
    setupGui();
}

void ImageProperties::setupGui(void)
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    setHelp("propertiesmetadatahistogram.anchor", "digikam");        

    m_generalPage   = new ImagePropertiesGeneral(addPage(i18n("&General")));
    m_exifPage      = new ImagePropertiesEXIF(addPage(i18n("&EXIF")));
    m_histogramPage = new ImagePropertiesHistogram(addPage(i18n("&Histogram")),
                                                   m_selectionArea);
    
    // Read config.
    kapp->config()->setGroup("Image Properties Dialog");
    showPage(kapp->config()->readNumEntry("Tab Active", 0)); 

    // Init all info data.
    
    slotItemChanged();
    
    resize(configDialogSize("Image Properties Dialog"));
    parentWidget()->setCursor( KCursor::arrowCursor() );       
}

ImageProperties::~ImageProperties()
{
    kapp->config()->setGroup("Image Properties Dialog");
    kapp->config()->writeEntry("Tab Active", activePageIndex());

    saveDialogSize("Image Properties Dialog");

    delete m_generalPage;
    delete m_exifPage;
    delete m_histogramPage;
}

void ImageProperties::slotUser1()
{
    if (m_mode != MULTI)
        return;
    
    if (!m_currItem)
        return;

    if (!m_currItem->nextItem())
    {
        enableButton(User1, false);
        return;
    }
        
    m_currItem  = dynamic_cast<AlbumIconItem*>(m_currItem->nextItem());
    m_currURL   = m_currItem->fileItem()->url();    
    m_currItem->setSelected(true);
    m_view->ensureItemVisible(m_currItem);
    
    enableButton(User1, m_currItem->nextItem() != 0);
    enableButton(User2, m_currItem->prevItem() != 0);
    
    slotItemChanged();
}

void ImageProperties::slotUser2()
{
    if (m_mode != MULTI)
        return;
    
    if (!m_currItem)
        return;

    if (!m_currItem->prevItem())
    {
        enableButton(User2, false);
        return;
    }
    
    m_currItem    = dynamic_cast<AlbumIconItem*>(m_currItem->prevItem());
    m_currURL     = m_currItem->fileItem()->url();    
    m_currItem->setSelected(true);
    m_view->ensureItemVisible(m_currItem);
    
    enableButton(User1, m_currItem->nextItem() != 0);
    enableButton(User2, m_currItem->prevItem() != 0);

    slotItemChanged();
}

void ImageProperties::slotItemChanged()
{
    if (!m_currURL.isValid())
        return;

    
    setCursor(KCursor::waitCursor());

    setCaption(i18n("Properties for '%1'").arg(m_currURL.fileName()));

    m_generalPage->setCurrentURL(m_currURL);
    m_histogramPage->setData(m_currURL, m_imageData, m_imageWidth, m_imageHeight);
    m_exifPage->setCurrentURL(m_currURL);

    setCursor( KCursor::arrowCursor() );
}

#include "imageproperties.moc"

