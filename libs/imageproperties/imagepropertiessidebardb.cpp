/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : image properties side bar using data from
 *               digiKam database.
 *
 * Copyright (C) 2004-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Martin Klapetek <martin dot klapetek at gmail dot com>
 * Copyright (C)      2011 by Michael G. Hansen <mike at mghansen dot de>
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

#include <kdebug.h>
#include <kfileitem.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kiconloader.h>

// Local includes

#include "config-digikam.h"
#include "applicationsettings.h"
#include "databaseinfocontainers.h"
#include "databasewatch.h"
#include "dimg.h"
#include "imageattributeswatch.h"
#include "imagedescedittab.h"
#include "imageinfo.h"
#include "imagepropertiestab.h"
#include "imagepropertiesmetadatatab.h"
#include "imagepropertiescolorstab.h"
#include "imagepropertiesversionstab.h"
#include "imageposition.h"
#include "tagscache.h"

#ifdef HAVE_KGEOMAP
#include "imagepropertiesgpstab.h"
#include "digikam2kgeomap_database.h"
#endif // HAVE_KGEOMAP

namespace Digikam
{

class ImagePropertiesSideBarDB::Private
{
public:

    Private() :
        dirtyDesceditTab(false),
        hasPrevious(false),
        hasNext(false),
        hasImageInfoOwnership(false),
        desceditTab(0)
    {
        desceditTab        = 0;
        versionsHistoryTab = 0;
        dirtyDesceditTab   = false;
    }

    bool                        dirtyDesceditTab;
    bool                        hasPrevious;
    bool                        hasNext;
    bool                        hasImageInfoOwnership;

    ImageInfoList               currentInfos;
    DImageHistory               currentHistory;
    ImageDescEditTab*           desceditTab;
    ImagePropertiesVersionsTab* versionsHistoryTab;
};

ImagePropertiesSideBarDB::ImagePropertiesSideBarDB(QWidget* const parent, SidebarSplitter* const splitter,
                                                   KMultiTabBarPosition side, bool mimimizedDefault)
    : ImagePropertiesSideBar(parent, splitter, side, mimimizedDefault),
      d(new Private)
{
    d->desceditTab        = new ImageDescEditTab(parent);
    d->versionsHistoryTab = new ImagePropertiesVersionsTab(parent);

    appendTab(d->desceditTab,        SmallIcon("imagecomment"), i18n("Captions/Tags"));
    appendTab(d->versionsHistoryTab, SmallIcon("view-catalog"), i18n("Versioning"));

    // ----------------------------------------------------------

    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));

    connect(d->desceditTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->desceditTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChangeDatabase(ImageChangeset)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChanged(ImageTagChangeset)));

    connect(ImageAttributesWatch::instance(), SIGNAL(signalFileMetadataChanged(KUrl)),
            this, SLOT(slotFileMetadataChanged(KUrl)));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotLoadMetadataFilters()));
}

ImagePropertiesSideBarDB::~ImagePropertiesSideBarDB()
{
    delete d;
}

void ImagePropertiesSideBarDB::itemChanged(const ImageInfo& info, const QRect& rect,
                                           DImg* const img, const DImageHistory& history)
{
    itemChanged(info.fileUrl(), info, rect, img, history);
}

void ImagePropertiesSideBarDB::itemChanged(const KUrl& url, const QRect& rect, DImg* const img)
{
    itemChanged(url, ImageInfo(), rect, img, DImageHistory());
}

void ImagePropertiesSideBarDB::itemChanged(const KUrl& url, const ImageInfo& info,
                                           const QRect& rect, DImg* const img, const DImageHistory& history)
{
    if ( !url.isValid() )
    {
        return;
    }

    m_currentURL = url;

    ImageInfoList list;

    if (!info.isNull())
    {
        list << info;
    }

    itemChanged(list, rect, img, history);
}

void ImagePropertiesSideBarDB::itemChanged(const ImageInfoList& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    m_currentURL = infos.first().fileUrl();

    itemChanged(infos, QRect(), 0, DImageHistory());
}

void ImagePropertiesSideBarDB::itemChanged(const ImageInfoList& infos, const QRect& rect, DImg* const img, const DImageHistory& history)
{
    m_currentRect        = rect;
    m_image              = img;
    d->currentHistory    = history;
    d->currentInfos      = infos;
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;
    m_dirtyHistoryTab    = false;
    d->dirtyDesceditTab  = false;

    // slotChangedTab only handles the active tab.
    // Any tab that holds information reset above shall be reset here,
    // unless it is the active tab
    if (getActiveTab() != d->desceditTab)
    {
        d->desceditTab->setItem();
    }

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
                DMetadata data(m_image->getMetadata());
                m_metadataTab->setCurrentData(data, m_currentURL.fileName());
            }
            else
            {
                m_metadataTab->setCurrentURL(m_currentURL);
            }

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
#ifdef HAVE_KGEOMAP
        else if (tab == m_gpsTab && !m_dirtyGpsTab)
        {
            m_gpsTab->setCurrentURL(m_currentURL);
            m_dirtyGpsTab = true;
        }
#endif // HAVE_KGEOMAP
        else if (tab == d->versionsHistoryTab && !m_dirtyHistoryTab)
        {
            //TODO: Make a database-less parent class with only the filters tab
            d->versionsHistoryTab->clear();
            m_dirtyHistoryTab = true;
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
                DMetadata data(m_image->getMetadata());
                m_metadataTab->setCurrentData(data, m_currentURL.fileName());
            }
            else
            {
                m_metadataTab->setCurrentURL(m_currentURL);
            }

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
#ifdef HAVE_KGEOMAP
        else if (tab == m_gpsTab && !m_dirtyGpsTab)
        {
            GPSImageInfo info;

            if (!GPSImageInfo::fromImageInfo(d->currentInfos.first(), &info))
            {
                m_gpsTab->setCurrentURL();
            }
            else
            {
                m_gpsTab->setGPSInfoList(GPSImageInfo::List() << info);
            }

            m_dirtyGpsTab = true;
        }
#endif // HAVE_KGEOMAP
        else if (tab == d->versionsHistoryTab && !m_dirtyHistoryTab)
        {
            d->versionsHistoryTab->setItem(d->currentInfos.first(), d->currentHistory);
            m_dirtyHistoryTab = true;
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
#ifdef HAVE_KGEOMAP
        else if (tab == m_gpsTab && !m_dirtyGpsTab)
        {
            GPSImageInfo::List list;

            for (ImageInfoList::const_iterator it = d->currentInfos.constBegin();
                 it != d->currentInfos.constEnd(); ++it)
            {
                GPSImageInfo info;

                if (GPSImageInfo::fromImageInfo(*it, &info))
                {
                    list << info;
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
#endif // HAVE_KGEOMAP
        else if (tab == d->versionsHistoryTab && !m_dirtyHistoryTab)
        {
            // FIXME: Any sensible multi-selection functionality? Must scale for large n!
            d->versionsHistoryTab->clear();
            m_dirtyHistoryTab = true;
        }
    }

#ifdef HAVE_KGEOMAP
    m_gpsTab->setActive(tab == m_gpsTab);
#endif // HAVE_KGEOMAP

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
        QWidget* const tab = getActiveTab();

        if (!tab)
        {
            return;
        }

        if (tab == m_propertiesTab
#ifdef HAVE_KGEOMAP
            || tab == m_gpsTab
#endif // HAVE_KGEOMAP
           )
        {
            ImageInfo& info = d->currentInfos.first();

            if (changeset.ids().contains(info.id()))
            {
                // trigger an update, if changes touch the tab's information
                DatabaseFields::Set set = changeset.changes();

                if ( (set & DatabaseFields::ImagesAll)           ||
                     (set & DatabaseFields::ImageInformationAll) ||
                     (set & DatabaseFields::ImageMetadataAll)    ||
                     (set & DatabaseFields::VideoMetadataAll)    ||
                     (set & DatabaseFields::ImageCommentsAll) )
                {
                    m_dirtyPropertiesTab = false;
                }
                else if (set & DatabaseFields::ImagePositionsAll)
                {
                    m_dirtyGpsTab = false;
                }

                if (tab == m_propertiesTab
#ifdef HAVE_KGEOMAP
                    || tab == m_gpsTab
#endif // HAVE_KGEOMAP
                   )
                {
                    // update now - reuse code form slotChangedTab
                    slotChangedTab(tab);
                }
            }
        }
    }
}

void ImagePropertiesSideBarDB::slotImageTagChanged(const ImageTagChangeset& changeset)
{
    if (!d->currentInfos.isEmpty())
    {
        QWidget* const tab = getActiveTab();

        if (!tab)
        {
            return;
        }

        if (tab == m_propertiesTab)
        {
            ImageInfo& info = d->currentInfos.first();

            if (changeset.ids().contains(info.id()))
            {
                m_dirtyPropertiesTab = false;
                slotChangedTab(tab);
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
    // TODO update, do we still need this method?
    //d->desceditTab->refreshTagsView();
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
            VideoMetadataContainer videoInfo = info.videoMetadataContainer();

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
            {
                str = i18n("Unknown");
            }
            else
            {
                QString mpixels;
                mpixels.setNum(commonInfo.width * commonInfo.height / 1000000.0, 'f', 2);
                str = i18nc("width x height (megapixels Mpx)", "%1x%2 (%3Mpx)",
                            commonInfo.width, commonInfo.height, mpixels);
            }

            m_propertiesTab->setImageDimensions(str);

            if (commonInfo.width == 0 || commonInfo.height == 0)
            {
                str = i18n("Unknown");
            }
            else
            {
                m_propertiesTab->aspectRatioToString(commonInfo.width, commonInfo.height, str);
            }

            m_propertiesTab->setImageRatio(str);
            m_propertiesTab->setImageBitDepth(i18n("%1 bpp", commonInfo.colorDepth));
            m_propertiesTab->setImageColorMode(commonInfo.colorModel.isEmpty() ? unavailable : commonInfo.colorModel);
            m_propertiesTab->setImageMime(commonInfo.format);

            // -- Photograph information ------------------------------------------

            m_propertiesTab->setPhotoInfoDisable(photoInfo.allFieldsNull);
            ImagePropertiesTab::shortenedMakeInfo(photoInfo.make);
            ImagePropertiesTab::shortenedModelInfo(photoInfo.model);
            m_propertiesTab->setPhotoMake(photoInfo.make.isEmpty()   ? unavailable : photoInfo.make);
            m_propertiesTab->setPhotoModel(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);

            if (commonInfo.creationDate.isValid())
            {
                str = KGlobal::locale()->formatDateTime(commonInfo.creationDate, KLocale::ShortDate, true);
                m_propertiesTab->setPhotoDateTime(str);
            }
            else
            {
                m_propertiesTab->setPhotoDateTime(unavailable);
            }

            m_propertiesTab->setPhotoLens(photoInfo.lens.isEmpty() ? unavailable : photoInfo.lens);
            m_propertiesTab->setPhotoAperture(photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture);

            if (photoInfo.focalLength35.isEmpty())
            {
                m_propertiesTab->setPhotoFocalLength(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
            }
            else
            {
                str = i18n("%1 (%2)", photoInfo.focalLength, photoInfo.focalLength35);
                m_propertiesTab->setPhotoFocalLength(str);
            }

            m_propertiesTab->setPhotoExposureTime(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime);
            m_propertiesTab->setPhotoSensitivity(photoInfo.sensitivity.isEmpty() ? unavailable : i18n("%1 ISO", photoInfo.sensitivity));

            if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
            {
                m_propertiesTab->setPhotoExposureMode(unavailable);
            }
            else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
            {
                m_propertiesTab->setPhotoExposureMode(photoInfo.exposureMode);
            }
            else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
            {
                m_propertiesTab->setPhotoExposureMode(photoInfo.exposureProgram);
            }
            else
            {
                str = QString("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
                m_propertiesTab->setPhotoExposureMode(str);
            }

            m_propertiesTab->setPhotoFlash(photoInfo.flashMode.isEmpty() ? unavailable : photoInfo.flashMode);
            m_propertiesTab->setPhotoWhiteBalance(photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance);

            // -- VideoMetadata information ------------------------------------------

            m_propertiesTab->setVideoInfoDisable(videoInfo.allFieldsNull);

            m_propertiesTab->setVideoAspectRatio(videoInfo.aspectRatio.isEmpty()           ? unavailable : videoInfo.aspectRatio);
            m_propertiesTab->setVideoDuration(videoInfo.duration.isEmpty()                 ? unavailable : videoInfo.duration);
            m_propertiesTab->setVideoFrameRate(videoInfo.frameRate.isEmpty()               ? unavailable : videoInfo.frameRate);
            m_propertiesTab->setVideoVideoCodec(videoInfo.videoCodec.isEmpty()             ? unavailable : videoInfo.videoCodec);
            m_propertiesTab->setVideoAudioBitRate(videoInfo.audioBitRate.isEmpty()         ? unavailable : videoInfo.audioBitRate);
            m_propertiesTab->setVideoAudioChannelType(videoInfo.audioChannelType.isEmpty() ? unavailable : videoInfo.audioChannelType);
            m_propertiesTab->setVideoAudioCompressor(videoInfo.audioCompressor.isEmpty()   ? unavailable : videoInfo.audioCompressor);

            // -- Caption / Tags ------------------------------------------

            m_propertiesTab->setCaption(info.comment());
            m_propertiesTab->setPickLabel(info.pickLabel());
            m_propertiesTab->setColorLabel(info.colorLabel());
            m_propertiesTab->setRating(info.rating());
            QList<int> tagIds = info.tagIds();
            m_propertiesTab->setTags(TagsCache::instance()->tagPaths(tagIds, TagsCache::NoLeadingSlash, TagsCache::NoHiddenTags),
                                     TagsCache::instance()->tagNames(tagIds, TagsCache::NoHiddenTags));
            m_propertiesTab->showOrHideCaptionAndTags();

            return;
        }
    }
}

ImagePropertiesVersionsTab* ImagePropertiesSideBarDB::getFiltersHistoryTab() const
{
    return d->versionsHistoryTab;
}

ImageDescEditTab* ImagePropertiesSideBarDB::imageDescEditTab() const
{
    return d->desceditTab;
}

void ImagePropertiesSideBarDB::doLoadState()
{
    ImagePropertiesSideBar::doLoadState();

    KConfigGroup group                = getConfigGroup();
    KConfigGroup groupVersionTab      = KConfigGroup(&group, entryName("Version Properties Tab"));
    d->versionsHistoryTab->readSettings(groupVersionTab);

    KConfigGroup groupCaptionsTagsTab = KConfigGroup(&group, entryName("Captions Tags Properties Tab"));
    d->desceditTab->readSettings(groupCaptionsTagsTab);
}

void ImagePropertiesSideBarDB::doSaveState()
{
    ImagePropertiesSideBar::doSaveState();

    KConfigGroup group           = getConfigGroup();
    KConfigGroup groupVersionTab = KConfigGroup(&group, entryName("Version Properties Tab"));
    d->versionsHistoryTab->writeSettings(groupVersionTab);

    KConfigGroup groupCaptionsTagsTab = KConfigGroup(&group, entryName("Captions Tags Properties Tab"));
    d->desceditTab->writeSettings(groupCaptionsTagsTab);
}

void ImagePropertiesSideBarDB::slotPopupTagsView()
{
    setActiveTab(d->desceditTab);
    d->desceditTab->setFocusToTagsView();
}

}  // namespace Digikam
