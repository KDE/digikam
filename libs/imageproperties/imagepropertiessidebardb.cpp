/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : image properties side bar using data from
 *               digiKam database.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imagepropertiessidebardb.moc"

// Qt includes

#include <QRect>
#include <QColor>
#include <QSplitter>

// KDE includes


#include <kfileitem.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kiconloader.h>

// Local includes

#include "dimg.h"
#include "imageinfo.h"
#include "databasewatch.h"
#include "imagepropertiesgpstab.h"
#include "imagedescedittab.h"
#include "imageattributeswatch.h"
#include "imagepropertiestab.h"
#include "imagepropertiesmetadatatab.h"
#include "imagepropertiescolorstab.h"

namespace Digikam
{

class ImagePropertiesSideBarDBPriv
{
public:

    ImagePropertiesSideBarDBPriv()
    {
        desceditTab           = 0;
        dirtyDesceditTab      = false;
        hasPrevious           = false;
        hasNext               = false;
        hasImageInfoOwnership = false;
    }

    bool                  dirtyDesceditTab;

    ImageInfoList         currentInfos;

    ImageDescEditTab     *desceditTab;

    bool                  hasPrevious;
    bool                  hasNext;

    bool                  hasImageInfoOwnership;
};

ImagePropertiesSideBarDB::ImagePropertiesSideBarDB(QWidget *parent, SidebarSplitter *splitter,
                                                   KMultiTabBarPosition side, bool mimimizedDefault)
                        : ImagePropertiesSideBar(parent, splitter, side, mimimizedDefault),
                          d(new ImagePropertiesSideBarDBPriv)
{
    d->desceditTab = new ImageDescEditTab(parent);

    appendTab(d->desceditTab, SmallIcon("imagecomment"), i18n("Caption/Tags"));

    // ----------------------------------------------------------

    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));

    connect(d->desceditTab, SIGNAL(signalProgressBarMode(int, const QString&)),
            this, SIGNAL(signalProgressBarMode(int, const QString&)));

    connect(d->desceditTab, SIGNAL(signalProgressValue(int)),
            this, SIGNAL(signalProgressValue(int)));

    connect(d->desceditTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->desceditTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    DatabaseWatch *dbwatch = DatabaseAccess::databaseWatch();

    connect(dbwatch, SIGNAL(imageChange(const ImageChangeset&)),
            this, SLOT(slotImageChangeDatabase(const ImageChangeset&)));

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalFileMetadataChanged(const KUrl&)),
            this, SLOT(slotFileMetadataChanged(const KUrl&)));
}

ImagePropertiesSideBarDB::~ImagePropertiesSideBarDB()
{
    delete d;
}

void ImagePropertiesSideBarDB::itemChanged(const ImageInfo& info,
                                           const QRect& rect, DImg *img)
{
    itemChanged(info.fileUrl(), info, rect, img);
}

void ImagePropertiesSideBarDB::itemChanged(const KUrl& url, const QRect& rect, DImg *img)
{
    itemChanged(url, ImageInfo(), rect, img);
}

void ImagePropertiesSideBarDB::itemChanged(const KUrl& url, const ImageInfo& info,
                                           const QRect& rect, DImg *img)
{
    if ( !url.isValid() )
        return;

    m_currentURL = url;

    ImageInfoList list;
    if (!info.isNull())
        list << info;

    itemChanged(list, rect, img);
}

void ImagePropertiesSideBarDB::itemChanged(const ImageInfoList& infos)
{
    if (infos.isEmpty())
        return;

    m_currentURL = infos.first().fileUrl();

    itemChanged(infos, QRect(), 0);
}

void ImagePropertiesSideBarDB::itemChanged(ImageInfoList infos, const QRect& rect, DImg *img)
{
    m_currentRect = rect;
    m_image       = img;

    d->currentInfos      = infos;
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;
    d->dirtyDesceditTab  = false;

    // All tabs that store the ImageInfo list and access it after selection change
    // must release the image info here. slotChangedTab only handles the active tab!
    d->desceditTab->setItem();

    slotChangedTab( getActiveTab() );
}

void ImagePropertiesSideBarDB::slotNoCurrentItem()
{
    ImagePropertiesSideBar::slotNoCurrentItem();

    // All tabs that store the ImageInfo list and access it after selection change
    // must release the image info here. slotChangedTab only handles the active tab!
    d->desceditTab->setItem();
    d->currentInfos.clear();
    d->dirtyDesceditTab = false;
}

void ImagePropertiesSideBarDB::populateTags()
{
    d->desceditTab->populateTags();
}

void ImagePropertiesSideBarDB::slotChangedTab(QWidget* tab)
{
    setCursor(Qt::WaitCursor);

    // No database data available, for example in the case of image editor is
    // started from camera GUI.
    if (d->currentInfos.isEmpty())
    {
        if (tab == m_propertiesTab && !m_dirtyPropertiesTab)
        {
            m_propertiesTab->setCurrentURL(m_currentURL);
            ImagePropertiesSideBar::setImagePropertiesInformation(m_currentURL);
            m_dirtyPropertiesTab = true;
        }
        else if (tab == m_metadataTab && !m_dirtyMetadataTab)
        {
            if (m_image)
            {
                DMetadata data;
                data.setComments(m_image->getComments());
                data.setExif(m_image->getExif());
                data.setIptc(m_image->getIptc());
                data.setXmp(m_image->getXmp());
                m_metadataTab->setCurrentData(data, m_currentURL.fileName());
            }
            else
                m_metadataTab->setCurrentURL(m_currentURL);

            m_dirtyMetadataTab = true;
        }
        else if (tab == m_colorTab && !m_dirtyColorTab)
        {
            m_colorTab->setData(m_currentURL, m_currentRect, m_image);
            m_dirtyColorTab = true;
        }
        else if (tab == d->desceditTab && !d->dirtyDesceditTab)
        {
            // Do nothing here. We cannot get data from database !
            d->desceditTab->setItem();
            d->dirtyDesceditTab = true;
        }
        else if (tab == m_gpsTab && !m_dirtyGpsTab)
        {
            m_gpsTab->setCurrentURL(m_currentURL);
            m_dirtyGpsTab = true;
        }
    }
    else if (d->currentInfos.count() == 1)   // Data from database available...
    {
        if (tab == m_propertiesTab && !m_dirtyPropertiesTab)
        {
            m_propertiesTab->setCurrentURL(m_currentURL);
            setImagePropertiesInformation(m_currentURL);
            m_dirtyPropertiesTab = true;
        }
        else if (tab == m_metadataTab && !m_dirtyMetadataTab)
        {
            if (m_image)
            {
                DMetadata data;
                data.setComments(m_image->getComments());
                data.setExif(m_image->getExif());
                data.setIptc(m_image->getIptc());
                data.setXmp(m_image->getXmp());
                m_metadataTab->setCurrentData(data, m_currentURL.fileName());
            }
            else
                m_metadataTab->setCurrentURL(m_currentURL);

            m_dirtyMetadataTab = true;
        }
        else if (tab == m_colorTab && !m_dirtyColorTab)
        {
            m_colorTab->setData(m_currentURL, m_currentRect, m_image);
            m_dirtyColorTab = true;
        }
        else if (tab == d->desceditTab && !d->dirtyDesceditTab)
        {
            d->desceditTab->setItem(d->currentInfos.first());
            d->dirtyDesceditTab = true;
        }
        else if (tab == m_gpsTab && !m_dirtyGpsTab)
        {
            ImagePosition pos = d->currentInfos.first().imagePosition();
            if (pos.isEmpty())
            {
                m_gpsTab->setCurrentURL();
            }
            else
            {
                GPSInfo info;
                info.latitude  = pos.latitudeNumber();
                info.longitude = pos.longitudeNumber();
                info.altitude  = pos.altitude();
                info.dateTime  = d->currentInfos.first().dateTime();
                info.url       = d->currentInfos.first().fileUrl();
                m_gpsTab->setGPSInfoList(GPSInfoList() << info);
            }

            m_dirtyGpsTab = true;
        }
    }
    else  // Data from database available, multiple selection
    {
        if (tab == m_propertiesTab && !m_dirtyPropertiesTab)
        {
            m_propertiesTab->setCurrentURL(m_currentURL);
            setImagePropertiesInformation(m_currentURL);
            m_dirtyPropertiesTab = true;
        }
        else if (tab == m_metadataTab && !m_dirtyMetadataTab)
        {
            // No multiple selection supported.
            m_metadataTab->setCurrentURL();
            m_dirtyMetadataTab = true;
        }
        else if (tab == m_colorTab && !m_dirtyColorTab)
        {
            // No multiple selection supported.
            m_colorTab->setData();
            m_dirtyColorTab = true;
        }
        else if (tab == d->desceditTab && !d->dirtyDesceditTab)
        {
            d->desceditTab->setItems(d->currentInfos);
            d->dirtyDesceditTab = true;
        }
        else if (tab == m_gpsTab && !m_dirtyGpsTab)
        {
            GPSInfoList list;
            for (ImageInfoList::const_iterator it = d->currentInfos.constBegin(); 
                 it != d->currentInfos.constEnd(); ++it)
            {
                ImagePosition pos = (*it).imagePosition();
                if (!pos.isEmpty())
                {
                    GPSInfo info;
                    info.latitude  = pos.latitudeNumber();
                    info.longitude = pos.longitudeNumber();
                    info.altitude  = pos.altitude();
                    info.dateTime  = (*it).dateTime();
                    info.url       = (*it).fileUrl();
                    list.append(info);
                }
            }
            if (list.isEmpty())
            {
                m_gpsTab->setCurrentURL();
            }
            else
            {
                m_gpsTab->setGPSInfoList(list);
            }
            m_dirtyGpsTab = true;
        }
    }

    unsetCursor();
}

void ImagePropertiesSideBarDB::slotFileMetadataChanged(const KUrl& url)
{
    if (url == m_currentURL)
    {
        // trigger an update
        m_dirtyMetadataTab = false;

        if (getActiveTab() == m_metadataTab)
        {
            // update now - reuse code form slotChangedTab
            slotChangedTab( getActiveTab() );
        }
    }
}

void ImagePropertiesSideBarDB::slotImageChangeDatabase(const ImageChangeset& changeset)
{
    if (!d->currentInfos.isEmpty())
    {
        QWidget *tab = getActiveTab();
        if (!tab) return;

        if (tab == m_propertiesTab || tab == m_gpsTab)
        {
            ImageInfo& info = d->currentInfos.first();
            if (changeset.ids().contains(info.id()))
            {
                // trigger an update, if changes touch the tab's information
                DatabaseFields::Set set = changeset.changes();
                if ( (set & DatabaseFields::ImagesAll) ||
                     (set & DatabaseFields::ImageInformationAll) ||
                     (set & DatabaseFields::ImageMetadataAll) ||
                     (set & DatabaseFields::ImageCommentsAll) )
                    m_dirtyPropertiesTab = false;
                else if (set & DatabaseFields::ImagePositionsAll)
                    m_dirtyGpsTab = false;

                if ( tab == m_propertiesTab || tab == m_gpsTab)
                {
                    // update now - reuse code form slotChangedTab
                    slotChangedTab(tab);
                }
            }
        }
    }
}

void ImagePropertiesSideBarDB::slotAssignRating(int rating)
{
    d->desceditTab->assignRating(rating);
}

void ImagePropertiesSideBarDB::slotAssignRatingNoStar()
{
    d->desceditTab->assignRating(0);
}

void ImagePropertiesSideBarDB::slotAssignRatingOneStar()
{
    d->desceditTab->assignRating(1);
}

void ImagePropertiesSideBarDB::slotAssignRatingTwoStar()
{
    d->desceditTab->assignRating(2);
}

void ImagePropertiesSideBarDB::slotAssignRatingThreeStar()
{
    d->desceditTab->assignRating(3);
}

void ImagePropertiesSideBarDB::slotAssignRatingFourStar()
{
    d->desceditTab->assignRating(4);
}

void ImagePropertiesSideBarDB::slotAssignRatingFiveStar()
{
    d->desceditTab->assignRating(5);
}

void ImagePropertiesSideBarDB::refreshTagsView()
{
    d->desceditTab->refreshTagsView();
}

void ImagePropertiesSideBarDB::setImagePropertiesInformation(const KUrl& url)
{
    foreach(const ImageInfo& info, d->currentInfos)
    {
        if (info.fileUrl() == url)
        {
            QString str;
            QString unavailable(i18n("<i>unavailable</i>"));
            KFileItem fi(KFileItem::Unknown, KFileItem::Unknown, url);

            // -- File system information -----------------------------------------

            ImageCommonContainer commonInfo  = info.imageCommonContainer();
            ImageMetadataContainer photoInfo = info.imageMetadataContainer();

            str = KGlobal::locale()->formatDateTime(commonInfo.fileModificationDate, KLocale::ShortDate, true);
            m_propertiesTab->setFileModifiedDate(str);

            str = QString("%1 (%2)").arg(KIO::convertSize(commonInfo.fileSize))
                                    .arg(KGlobal::locale()->formatNumber(commonInfo.fileSize, 0));
            m_propertiesTab->setFileSize(str);

            //  These infos are not stored in DB
            m_propertiesTab->setFileOwner(QString("%1 - %2").arg(fi.user()).arg(fi.group()));
            m_propertiesTab->setFilePermissions(fi.permissionsString());

            // -- Image Properties --------------------------------------------------

            if (commonInfo.width == 0 || commonInfo.height == 0)
                str = i18n("Unknown");
            else
            {
                QString mpixels;
                mpixels.setNum(commonInfo.width * commonInfo.height / 1000000.0, 'f', 2);
                str = i18nc("width x height (megapixels Mpx)", "%1x%2 (%3Mpx)",
                           commonInfo.width, commonInfo.height, mpixels);
            }
            m_propertiesTab->setImageDimensions(str);
            m_propertiesTab->setImageBitDepth(i18n("%1 bpp", commonInfo.colorDepth));
            m_propertiesTab->setImageColorMode(commonInfo.colorModel.isEmpty() ? unavailable : commonInfo.colorModel);
            m_propertiesTab->setImageMime(commonInfo.format);

            /* TODO : This info is not stored by DB
            m_propertiesTab->setImageCompression(compression.isEmpty() ? unavailable : compression);
            */
            m_propertiesTab->hideImageCompression();

            // -- Photograph information ------------------------------------------

            m_propertiesTab->setPhotoInfoDisable(photoInfo.allFieldsNull);

            m_propertiesTab->setPhotoMake(photoInfo.make.isEmpty() ? unavailable : photoInfo.make);
            m_propertiesTab->setPhotoModel(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);

            if (commonInfo.creationDate.isValid())
            {
                str = KGlobal::locale()->formatDateTime(commonInfo.creationDate, KLocale::ShortDate, true);
                m_propertiesTab->setPhotoDateTime(str);
            }
            else
                m_propertiesTab->setPhotoDateTime(unavailable);

            m_propertiesTab->setPhotoLens(photoInfo.lens.isEmpty() ? unavailable : photoInfo.lens);
            m_propertiesTab->setPhotoAperture(photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture);

            if (photoInfo.focalLength35.isEmpty())
                m_propertiesTab->setPhotoFocalLength(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
            else
            {
                str = i18n("%1 (35mm: %2)", photoInfo.focalLength, photoInfo.focalLength35);
                m_propertiesTab->setPhotoFocalLength(str);
            }

            m_propertiesTab->setPhotoExposureTime(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime);
            m_propertiesTab->setPhotoSensitivity(photoInfo.sensitivity.isEmpty() ? unavailable : i18n("%1 ISO", photoInfo.sensitivity));

            if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                m_propertiesTab->setPhotoExposureMode(unavailable);
            else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                m_propertiesTab->setPhotoExposureMode(photoInfo.exposureMode);
            else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
                m_propertiesTab->setPhotoExposureMode(photoInfo.exposureProgram);
            else
            {
                str = QString("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
                m_propertiesTab->setPhotoExposureMode(str);
            }

            m_propertiesTab->setPhotoFlash(photoInfo.flashMode.isEmpty() ? unavailable : photoInfo.flashMode);
            m_propertiesTab->setPhotoWhiteBalance(photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance);
            return;
        }
    }
}

}  // namespace Digikam
