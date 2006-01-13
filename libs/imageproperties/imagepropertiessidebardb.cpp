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

class ImagePropertiesSideBarDBPriv
{
public:

    ImagePropertiesSideBarDBPriv()
    {
        image             = 0;
        currentRect       = 0;
        currentView       = 0;
        currentItem       = 0;
        dirtyExifTab      = false;
        dirtyHistogramTab = false;
        dirtyDesceditTab  = false;
    }

    bool                      dirtyExifTab;
    bool                      dirtyHistogramTab;
    bool                      dirtyDesceditTab;

    QRect                    *currentRect;

    KURL                      currentURL;

    AlbumIconView            *currentView;

    AlbumIconItem            *currentItem;

    DImg                     *image;

    ImagePropertiesEXIFTab   *exifTab;
    ImagePropertiesColorsTab *histogramTab;
    ImageDescEditTab         *desceditTab;
};

ImagePropertiesSideBarDB::ImagePropertiesSideBarDB(QWidget *parent, QSplitter *splitter, 
                                                   Side side, bool mimimizedDefault, bool navBar)
                        : Digikam::Sidebar(parent, side, mimimizedDefault)
{
    d = new ImagePropertiesSideBarDBPriv;
    
    d->exifTab      = new ImagePropertiesEXIFTab(parent, navBar);
    d->histogramTab = new ImagePropertiesColorsTab(parent, 0, navBar);
    d->desceditTab  = new ImageDescEditTab(parent, navBar);
    
    setSplitter(splitter);
         
    appendTab(d->exifTab, SmallIcon("exifinfo"), i18n("EXIF"));
    appendTab(d->histogramTab, SmallIcon("blend"), i18n("Colors"));
    appendTab(d->desceditTab, SmallIcon("imagecomment"), i18n("Comments && Tags"));
    loadViewState();

    // ----------------------------------------------------------
    
    connect(d->exifTab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->exifTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->exifTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->exifTab, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));

    connect(d->histogramTab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->histogramTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->histogramTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->histogramTab, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
                            
    connect(d->desceditTab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->desceditTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->desceditTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->desceditTab, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
            
    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));
}

ImagePropertiesSideBarDB::~ImagePropertiesSideBarDB()
{
    delete d;
}

void ImagePropertiesSideBarDB::itemChanged(const KURL& url, AlbumIconView* view, 
                                           AlbumIconItem* item, QRect *rect, DImg *img)
{
    if (!url.isValid())
        return;
    
    d->currentURL        = url;
    d->currentRect       = rect;
    d->image             = img;
    d->currentView       = view;
    d->currentItem       = item;
    d->dirtyExifTab      = false;
    d->dirtyHistogramTab = false;
    d->dirtyDesceditTab  = false;
        
    slotChangedTab( getActiveTab() );    
}

void ImagePropertiesSideBarDB::noCurrentItem(void)
{
    d->currentURL        = KURL::KURL();
    d->currentItem       = 0;
    d->exifTab->setCurrentURL();
    d->histogramTab->setData();
    d->desceditTab->setItem();
    d->dirtyExifTab      = false;
    d->dirtyHistogramTab = false;
    d->dirtyDesceditTab  = false;
}

void ImagePropertiesSideBarDB::imageSelectionChanged(QRect *rect)
{
    d->currentRect = rect;
    
    if (d->dirtyHistogramTab)
       d->histogramTab->setSelection(rect);
    else
       slotChangedTab(d->histogramTab);
}

void ImagePropertiesSideBarDB::populateTags(void)
{
    d->desceditTab->populateTags();
}

void ImagePropertiesSideBarDB::slotChangedTab(QWidget* tab)
{
    setCursor(KCursor::waitCursor());

    // No database data available, for example in the case of image editor is 
    // launched from camera GUI.
    if (!d->currentView || !d->currentItem)
    {
        if (tab == d->exifTab && !d->dirtyExifTab)
        {
            d->exifTab->setCurrentURL(d->currentURL, NavigateBarWidget::ItemCurrent);
            d->dirtyExifTab = true;
        }
        else if (tab == d->histogramTab && !d->dirtyHistogramTab)
        {
            d->histogramTab->setData(d->currentURL, d->currentRect, d->image, NavigateBarWidget::ItemCurrent);
            d->dirtyHistogramTab = true;
        }
        else if (tab == d->desceditTab && !d->dirtyDesceditTab)
        {
            // Do nothing here. We cannot get data from database !
            d->desceditTab->setItem();
            d->dirtyDesceditTab = true;
        }
    }
    else    // Data from database available...
    {
        int currentItemType = NavigateBarWidget::ItemCurrent;
        
        if (d->currentView->firstItem() == d->currentItem)
           currentItemType = NavigateBarWidget::ItemFirst;
        else if (d->currentView->lastItem() == d->currentItem)
            currentItemType = NavigateBarWidget::ItemLast;
        
        if (tab == d->exifTab && !d->dirtyExifTab)
        {
            d->exifTab->setCurrentURL(d->currentURL, currentItemType);
            d->dirtyExifTab = true;
        }
        else if (tab == d->histogramTab && !d->dirtyHistogramTab)
        {
            d->histogramTab->setData(d->currentURL, d->currentRect, d->image, currentItemType);
            d->dirtyHistogramTab = true;
        }
        else if (tab == d->desceditTab && !d->dirtyDesceditTab)
        {
            d->desceditTab->setItem(d->currentView, d->currentItem, currentItemType);
           d->dirtyDesceditTab = true;
        }
    }

    setCursor( KCursor::arrowCursor() );
}

}  // NameSpace Digikam

#include "imagepropertiessidebardb.moc"
