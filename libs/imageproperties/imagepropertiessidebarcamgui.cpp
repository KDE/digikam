/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-08
 * Description : simple image properties side bar used by 
 *               camera gui.
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

#include "dmetadata.h"
#include "gpiteminfo.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "cameraitempropertiestab.h"
#include "imagepropertiesmetadatatab.h"
#include "navigatebarwidget.h"
#include "imagepropertiessidebarcamgui.h"

namespace Digikam
{

class ImagePropertiesSideBarCamGuiPriv
{
public:

    ImagePropertiesSideBarCamGuiPriv()
    {
        dirtyMetadataTab   = false;
        dirtyCameraItemTab = false;
        metadataTab        = 0;
        cameraItemTab      = 0;
        itemInfo           = 0;
        cameraView         = 0;
        cameraItem         = 0;
        exifData           = QByteArray();
        currentURL         = KURL();
    }

    bool                        dirtyMetadataTab;
    bool                        dirtyCameraItemTab;

    QByteArray                  exifData;

    KURL                        currentURL;

    GPItemInfo                 *itemInfo;
    
    ImagePropertiesMetaDataTab *metadataTab;
    
    CameraIconView             *cameraView;

    CameraIconViewItem         *cameraItem;
    
    CameraItemPropertiesTab    *cameraItemTab;
};

ImagePropertiesSideBarCamGui::ImagePropertiesSideBarCamGui(QWidget *parent, const char *name,
                                                           QSplitter *splitter, Side side,
                                                           bool mimimizedDefault)
                            : Sidebar(parent, name, side, mimimizedDefault)
{
    d = new ImagePropertiesSideBarCamGuiPriv;
    d->cameraItemTab = new CameraItemPropertiesTab(parent, true);
    d->metadataTab   = new ImagePropertiesMetaDataTab(parent, true);
    
    setSplitter(splitter);
         
    appendTab(d->cameraItemTab, SmallIcon("info"), i18n("Properties"));
    appendTab(d->metadataTab, SmallIcon("exifinfo"), i18n("Metadata"));

    // ----------------------------------------------------------
    
    connect(d->cameraItemTab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->cameraItemTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->cameraItemTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->cameraItemTab, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));

    connect(d->metadataTab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->metadataTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->metadataTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->metadataTab, SIGNAL(signalLastItem()),
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
    d->dirtyMetadataTab   = false;
    d->dirtyCameraItemTab = false;
    d->cameraView         = view;
    d->cameraItem         = item;

    if (d->exifData.isEmpty())
    {
        DMetadata metaData(d->currentURL.path());
        d->exifData = metaData.getExif();
    }

    slotChangedTab( getActiveTab() );    
}

void ImagePropertiesSideBarCamGui::slotNoCurrentItem(void)
{
    d->itemInfo           = 0;
    d->cameraItem         = 0;
    d->exifData           = QByteArray();
    d->currentURL         = KURL();
    d->dirtyMetadataTab   = false;
    d->dirtyCameraItemTab = false;

    d->cameraItemTab->setCurrentItem();
    d->metadataTab->setCurrentURL();
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
        d->cameraItemTab->setCurrentItem(d->itemInfo, currentItemType,
                                         d->cameraItem->getDownloadName(), d->exifData,
                                         d->currentURL);
        
        d->dirtyCameraItemTab = true;
    }
    else if (tab == d->metadataTab && !d->dirtyMetadataTab)
    {
        d->metadataTab->setCurrentData(d->exifData, QByteArray(), 
                                       d->itemInfo->name, currentItemType);
 
        d->dirtyMetadataTab = true;
    }
    
    unsetCursor();
}

}  // NameSpace Digikam

#include "imagepropertiessidebarcamgui.moc"

