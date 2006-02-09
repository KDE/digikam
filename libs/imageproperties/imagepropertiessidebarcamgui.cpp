/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2006-02-08
 * Description :
 *
 * Copyright 2006 by Gilles Caulier
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

#include "gpiteminfo.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "cameraitempropertiestab.h"
#include "imagepropertiesexiftab.h"
#include "navigatebarwidget.h"
#include "imagepropertiessidebarcamgui.h"

namespace Digikam
{

class ImagePropertiesSideBarCamGuiPriv
{
public:

    ImagePropertiesSideBarCamGuiPriv()
    {
        dirtyExifTab       = false;
        dirtyCameraItemTab = false;
        exifTab            = 0;
        cameraItemTab      = 0;
        itemInfo           = 0;
        cameraView         = 0;
        cameraItem         = 0;
        exifData           = QByteArray();
        currentURL         = KURL();
    }

    bool                     dirtyExifTab;
    bool                     dirtyCameraItemTab;

    QByteArray               exifData;

    KURL                     currentURL;

    GPItemInfo              *itemInfo;
    
    ImagePropertiesEXIFTab  *exifTab;
    
    CameraIconView          *cameraView;

    CameraIconViewItem      *cameraItem;
    
    CameraItemPropertiesTab *cameraItemTab;
};

ImagePropertiesSideBarCamGui::ImagePropertiesSideBarCamGui(QWidget *parent, const char *name,
                                                           QSplitter *splitter, Side side,
                                                           bool mimimizedDefault)
                            : Digikam::Sidebar(parent, name, side, mimimizedDefault)
{
    d = new ImagePropertiesSideBarCamGuiPriv;
    d->cameraItemTab = new CameraItemPropertiesTab(parent, true);
    d->exifTab       = new ImagePropertiesEXIFTab(parent, true);
    
    setSplitter(splitter);
         
    appendTab(d->cameraItemTab, SmallIcon("info"), i18n("Properties"));
    appendTab(d->exifTab, SmallIcon("exifinfo"), i18n("EXIF"));

    // ----------------------------------------------------------
    
    connect(d->cameraItemTab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->cameraItemTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->cameraItemTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->cameraItemTab, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));

    connect(d->exifTab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->exifTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->exifTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->exifTab, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
                            
    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));
}

ImagePropertiesSideBarCamGui::~ImagePropertiesSideBarCamGui()
{
    delete d;
}

void ImagePropertiesSideBarCamGui::itemChanged(GPItemInfo* itemInfo, const KURL& url,
                                               const QByteArray& exifData,
                                               CameraIconView* view, CameraIconViewItem* item)
{
    if (!itemInfo)
        return;
    
    d->exifData           = exifData;
    d->itemInfo           = itemInfo;
    d->currentURL         = url;
    d->dirtyExifTab       = false;
    d->dirtyCameraItemTab = false;
    d->cameraView         = view;
    d->cameraItem         = item;
    
    slotChangedTab( getActiveTab() );    
}

void ImagePropertiesSideBarCamGui::slotNoCurrentItem(void)
{
    d->itemInfo           = 0;
    d->cameraItem         = 0;
    d->exifData           = QByteArray();
    d->currentURL         = KURL();
    d->dirtyExifTab       = false;
    d->dirtyCameraItemTab = false;

    d->cameraItemTab->setCurrentItem();
    d->exifTab->setCurrentURL();
}

void ImagePropertiesSideBarCamGui::slotChangedTab(QWidget* tab)
{
    if (!d->itemInfo)
        return;
    
    setCursor(KCursor::waitCursor());

    int currentItemType = NavigateBarWidget::ItemCurrent;

    if (d->cameraView->firstItem() == d->cameraItem)
        currentItemType = NavigateBarWidget::ItemFirst;
    else if (d->cameraView->lastItem() == d->cameraItem)
        currentItemType = NavigateBarWidget::ItemLast;
             
    if (tab == d->cameraItemTab && !d->dirtyCameraItemTab)
    {
        d->cameraItemTab->setCurrentItem(d->itemInfo, currentItemType);
        d->dirtyCameraItemTab = true;
    }
    else if (tab == d->exifTab && !d->dirtyExifTab)
    {
        if (d->exifData)
            d->exifTab->setCurrentData(d->exifData, d->itemInfo->name,
                                       currentItemType);
        else
            d->exifTab->setCurrentURL(d->currentURL, currentItemType);

       d->dirtyExifTab = true;
    }
    
    setCursor( KCursor::arrowCursor() );
}

}  // NameSpace Digikam

#include "imagepropertiessidebarcamgui.moc"

