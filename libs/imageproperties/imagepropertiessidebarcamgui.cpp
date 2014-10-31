/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-08
 * Description : simple image properties side bar used by
 *               camera GUI.
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
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

#include "imagepropertiessidebarcamgui.moc"

// Qt includes

#include <QSplitter>

// KDE includes

#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kiconloader.h>

// Local includes

#include "config-digikam.h"
#include "dmetadata.h"
#include "camiteminfo.h"
#include "cameraitempropertiestab.h"
#include "imagepropertiesmetadatatab.h"

#ifdef HAVE_KGEOMAP
#include "imagepropertiesgpstab.h"
#endif // HAVE_KGEOMAP

namespace Digikam
{

class ImagePropertiesSideBarCamGui::Private
{
public:

    Private() :
        dirtyMetadataTab(false),
        dirtyCameraItemTab(false),
        dirtyGpsTab(false),
#ifdef HAVE_KGEOMAP
        gpsTab(0),
#endif // HAVE_KGEOMAP
        metadataTab(0),
        cameraItemTab(0)
    {
    }

    bool                        dirtyMetadataTab;
    bool                        dirtyCameraItemTab;
    bool                        dirtyGpsTab;

    DMetadata                   metaData;

    CamItemInfo                 itemInfo;

#ifdef HAVE_KGEOMAP
    ImagePropertiesGPSTab*      gpsTab;
#endif // HAVE_KGEOMAP

    ImagePropertiesMetaDataTab* metadataTab;
    CameraItemPropertiesTab*    cameraItemTab;
};

ImagePropertiesSideBarCamGui::ImagePropertiesSideBarCamGui(QWidget* const parent,
                                                           SidebarSplitter* const splitter,
                                                           KMultiTabBarPosition side,
                                                           bool mimimizedDefault)
    : Sidebar(parent, splitter, side, mimimizedDefault),
      d(new Private)
{
    d->cameraItemTab = new CameraItemPropertiesTab(parent);
    d->metadataTab   = new ImagePropertiesMetaDataTab(parent);

    appendTab(d->cameraItemTab, SmallIcon("document-properties"),   i18n("Properties"));
    appendTab(d->metadataTab,   SmallIcon("exifinfo"),              i18n("Metadata")); // krazy:exclude=iconnames

#ifdef HAVE_KGEOMAP
    d->gpsTab        = new ImagePropertiesGPSTab(parent);
    appendTab(d->gpsTab,        SmallIcon("applications-internet"), i18n("Geolocation"));
#endif // HAVE_KGEOMAP

    // ----------------------------------------------------------

    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));
}

ImagePropertiesSideBarCamGui::~ImagePropertiesSideBarCamGui()
{
    delete d;
}

void ImagePropertiesSideBarCamGui::applySettings()
{
    /// @todo Still needed?

    /// @todo Are load/saveState called by the creator?
}

KUrl ImagePropertiesSideBarCamGui::url() const
{
    return d->itemInfo.url();
}

void ImagePropertiesSideBarCamGui::itemChanged(const CamItemInfo& itemInfo, const DMetadata& meta)
{
    if (itemInfo.isNull())
    {
        return;
    }

    d->metaData           = meta;
    d->itemInfo           = itemInfo;
    d->dirtyMetadataTab   = false;
    d->dirtyCameraItemTab = false;
    d->dirtyGpsTab        = false;

    slotChangedTab(getActiveTab());
}

void ImagePropertiesSideBarCamGui::slotNoCurrentItem()
{
    d->itemInfo           = CamItemInfo();
    d->metaData           = DMetadata();
    d->dirtyMetadataTab   = false;
    d->dirtyCameraItemTab = false;
    d->dirtyGpsTab        = false;

    d->cameraItemTab->setCurrentItem();
    d->metadataTab->setCurrentURL();

#ifdef HAVE_KGEOMAP
    d->gpsTab->setCurrentURL();
#endif // HAVE_KGEOMAP
}

void ImagePropertiesSideBarCamGui::slotChangedTab(QWidget* tab)
{
    if (d->itemInfo.isNull())
    {
        return;
    }

    setCursor(Qt::WaitCursor);

    if (tab == d->cameraItemTab && !d->dirtyCameraItemTab)
    {
        d->cameraItemTab->setCurrentItem(d->itemInfo, d->metaData);

        d->dirtyCameraItemTab = true;
    }
    else if (tab == d->metadataTab && !d->dirtyMetadataTab)
    {
        d->metadataTab->setCurrentData(d->metaData, d->itemInfo.name);
        d->dirtyMetadataTab = true;
    }
#ifdef HAVE_KGEOMAP
    else if (tab == d->gpsTab && !d->dirtyGpsTab)
    {
        d->gpsTab->setMetadata(d->metaData, d->itemInfo.url());
        d->dirtyGpsTab = true;
    }

    d->gpsTab->setActive(tab == d->gpsTab);
#endif // HAVE_KGEOMAP

    unsetCursor();
}

void ImagePropertiesSideBarCamGui::doLoadState()
{
    /// @todo This code is taken from ImagePropertiesSideBar::doLoadState()
    ///       Ideally ImagePropertiesSideBarCamGui should be a subclass of
    ///       ImagePropertiesSideBar
    Sidebar::doLoadState();

    KConfigGroup group = getConfigGroup();

    KConfigGroup groupCameraItemTab    = KConfigGroup(&group, entryName("Camera Item Properties Tab"));
    d->cameraItemTab->readSettings(groupCameraItemTab);

#ifdef HAVE_KGEOMAP
    KConfigGroup groupGPSTab            = KConfigGroup(&group, entryName("GPS Properties Tab"));
    d->gpsTab->readSettings(groupGPSTab);
#endif // HAVE_KGEOMAP

    const KConfigGroup groupMetadataTab = KConfigGroup(&group, entryName("Metadata Properties Tab"));
    d->metadataTab->readSettings(groupMetadataTab);
}

void ImagePropertiesSideBarCamGui::doSaveState()
{
    /// @todo This code is taken from ImagePropertiesSideBar::doSaveState()
    ///       Ideally ImagePropertiesSideBarCamGui should be a subclass of
    ///       ImagePropertiesSideBar

    Sidebar::doSaveState();

    KConfigGroup group = getConfigGroup();

    KConfigGroup groupCameraItemTab = KConfigGroup(&group, entryName("Camera Item Properties Tab"));
    d->cameraItemTab->writeSettings(groupCameraItemTab);

#ifdef HAVE_KGEOMAP
    KConfigGroup groupGPSTab        = KConfigGroup(&group, entryName("GPS Properties Tab"));
    d->gpsTab->writeSettings(groupGPSTab);
#endif // HAVE_KGEOMAP

    KConfigGroup groupMetadataTab   = KConfigGroup(&group, entryName("Metadata Properties Tab"));
    d->metadataTab->writeSettings(groupMetadataTab);
}

}  // namespace Digikam
