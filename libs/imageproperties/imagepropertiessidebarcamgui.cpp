/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-08
 * Description : simple image properties side bar used by
 *               camera GUI.
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepropertiessidebarcamgui.h"
#include "imagepropertiessidebarcamgui.moc"

// Qt includes

#include <QSplitter>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kiconloader.h>

// Local includes

#include "dmetadata.h"
#include "gpiteminfo.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "cameraitempropertiestab.h"
#include "imagepropertiesgpstab.h"
#include "imagepropertiesmetadatatab.h"
#include "statusnavigatebar.h"

namespace Digikam
{

class ImagePropertiesSideBarCamGuiPriv
{
public:

    ImagePropertiesSideBarCamGuiPriv()
    {
        dirtyMetadataTab   = false;
        dirtyCameraItemTab = false;
        dirtyGpsTab        = false;
        metadataTab        = 0;
        cameraItemTab      = 0;
        itemInfo           = 0;
        cameraView         = 0;
        cameraItem         = 0;
        metaData           = DMetadata();
        currentURL         = KUrl();
    }

    bool                        dirtyMetadataTab;
    bool                        dirtyCameraItemTab;
    bool                        dirtyGpsTab;

    KUrl                        currentURL;

    DMetadata                   metaData;

    GPItemInfo                 *itemInfo;

    ImagePropertiesMetaDataTab *metadataTab;

    ImagePropertiesGPSTab      *gpsTab;

    CameraIconView             *cameraView;

    CameraIconItem             *cameraItem;

    CameraItemPropertiesTab    *cameraItemTab;
};

ImagePropertiesSideBarCamGui::ImagePropertiesSideBarCamGui(QWidget *parent,
                                                           SidebarSplitter *splitter,
                                                           KMultiTabBarPosition side,
                                                           bool mimimizedDefault)
                            : Sidebar(parent, splitter, side, mimimizedDefault),
                              d(new ImagePropertiesSideBarCamGuiPriv)
{
    d->cameraItemTab = new CameraItemPropertiesTab(parent);
    d->metadataTab   = new ImagePropertiesMetaDataTab(parent);
    d->gpsTab        = new ImagePropertiesGPSTab(parent);

    appendTab(d->cameraItemTab, SmallIcon("document-properties"), i18n("Properties"));
    appendTab(d->metadataTab, SmallIcon("exifinfo"), i18n("Metadata"));
    appendTab(d->gpsTab, SmallIcon("applications-internet"), i18n("Geolocation"));

    // ----------------------------------------------------------

    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));
}

ImagePropertiesSideBarCamGui::~ImagePropertiesSideBarCamGui()
{
    delete d;
}

void ImagePropertiesSideBarCamGui::itemChanged(GPItemInfo* itemInfo, const KUrl& url,
                                               const QByteArray& exifData,
                                               CameraIconView* view, CameraIconItem* item)
{
    if (!itemInfo)
        return;

    d->metaData.setExif(exifData);
    d->itemInfo           = itemInfo;
    d->currentURL         = url;
    d->dirtyMetadataTab   = false;
    d->dirtyCameraItemTab = false;
    d->dirtyGpsTab        = false;
    d->cameraView         = view;
    d->cameraItem         = item;

    if (exifData.isEmpty())
    {
        d->metaData = DMetadata(d->currentURL.path());
    }

    slotChangedTab(getActiveTab());
}

void ImagePropertiesSideBarCamGui::slotNoCurrentItem(void)
{
    d->itemInfo           = 0;
    d->cameraItem         = 0;
    d->metaData           = DMetadata();
    d->currentURL         = KUrl();
    d->dirtyMetadataTab   = false;
    d->dirtyCameraItemTab = false;
    d->dirtyGpsTab        = false;

    d->cameraItemTab->setCurrentItem();
    d->metadataTab->setCurrentURL();
    d->gpsTab->setCurrentURL();
}

void ImagePropertiesSideBarCamGui::slotChangedTab(QWidget* tab)
{
    if (!d->itemInfo)
        return;

    setCursor(Qt::WaitCursor);

    if (tab == d->cameraItemTab && !d->dirtyCameraItemTab)
    {
        d->cameraItemTab->setCurrentItem(d->itemInfo,
                                         d->cameraItem->getDownloadName(), d->metaData.getExif(),
                                         d->currentURL);

        d->dirtyCameraItemTab = true;
    }
    else if (tab == d->metadataTab && !d->dirtyMetadataTab)
    {
        d->metadataTab->setCurrentData(d->metaData, d->itemInfo->name);
        d->dirtyMetadataTab = true;
    }
    else if (tab == d->gpsTab && !d->dirtyGpsTab)
    {
        d->gpsTab->setMetadata(d->metaData, d->currentURL);
        d->dirtyGpsTab = true;
    }

    unsetCursor();
}

}  // namespace Digikam
