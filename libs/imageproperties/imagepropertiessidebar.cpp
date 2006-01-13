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
class ImagePropertiesSideBarPriv
{
public:

    ImagePropertiesSideBarPriv()
    {
        image             = 0;
        currentRect       = 0;
        dirtyExifTab      = false;
        dirtyHistogramTab = false;
    }

    bool                      dirtyExifTab;
    bool                      dirtyHistogramTab;
 
    QRect                    *currentRect;
 
    KURL                      currentURL;
 
    DImg                     *image;
     
    ImagePropertiesEXIFTab   *exifTab;
    ImagePropertiesColorsTab *histogramTab;
};

ImagePropertiesSideBar::ImagePropertiesSideBar(QWidget *parent, QSplitter *splitter, 
                                               Side side, bool mimimizedDefault)
                      : Digikam::Sidebar(parent, side, mimimizedDefault)
{
    d = new ImagePropertiesSideBarPriv;
    
    d->exifTab      = new ImagePropertiesEXIFTab(parent, false);
    d->histogramTab = new ImagePropertiesColorsTab(parent, 0, false);
    
    setSplitter(splitter);
         
    appendTab(d->exifTab, SmallIcon("exifinfo"), i18n("EXIF"));
    appendTab(d->histogramTab, SmallIcon("blend"), i18n("Colors"));
    loadViewState();
    
    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));
}

ImagePropertiesSideBar::~ImagePropertiesSideBar()
{
    delete d;
}

void ImagePropertiesSideBar::itemChanged(const KURL& url, QRect *rect, DImg *img)
{
    if (!url.isValid())
        return;
    
    d->currentURL        = url;
    d->currentRect       = rect;
    d->image               = img;
    d->dirtyExifTab      = false;
    d->dirtyHistogramTab = false;
    
    slotChangedTab( getActiveTab() );    
}

void ImagePropertiesSideBar::noCurrentItem(void)
{
    d->currentURL = KURL::KURL();
    d->exifTab->setCurrentURL();
    d->histogramTab->setData();
    d->dirtyExifTab      = false;
    d->dirtyHistogramTab = false;
}

void ImagePropertiesSideBar::imageSelectionChanged(QRect *rect)
{
    d->currentRect = rect;
    
    if (d->dirtyHistogramTab)
       d->histogramTab->setSelection(rect);
    else
       slotChangedTab(d->histogramTab);
}

void ImagePropertiesSideBar::slotChangedTab(QWidget* tab)
{
    if (!d->currentURL.isValid())
        return;
    
    setCursor(KCursor::waitCursor());
    
    if (tab == d->exifTab && !d->dirtyExifTab)
    {
       d->exifTab->setCurrentURL(d->currentURL);
       d->dirtyExifTab = true;
    }
    else if (tab == d->histogramTab && !d->dirtyHistogramTab)
    {
       d->histogramTab->setData(d->currentURL, d->currentRect, d->image);
       d->dirtyHistogramTab = true;
    }
    
    setCursor( KCursor::arrowCursor() );
}

}  // NameSpace Digikam

#include "imagepropertiessidebar.moc"

