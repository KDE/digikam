/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : image properties side bar using data from
 *               digiKam database.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepropertiessidebardb.h"

// Qt includes

#include <QRect>
#include <QColor>
#include <QSplitter>
#include <QFileInfo>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "coredbinfocontainers.h"
#include "coredbwatch.h"
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

#ifdef HAVE_MARBLE
#   include "imagepropertiesgpstab.h"
#   include "gpsimageinfosorter.h"
#endif // HAVE_MARBLE

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
                                                   Qt::Edge side, bool mimimizedDefault)
    : ImagePropertiesSideBar(parent, splitter, side, mimimizedDefault),
      d(new Private)
{
    d->desceditTab        = new ImageDescEditTab(parent);
    d->versionsHistoryTab = new ImagePropertiesVersionsTab(parent);

    appendTab(d->desceditTab,        QIcon::fromTheme(QLatin1String("edit-text-frame-update")), i18n("Captions"));
    appendTab(d->versionsHistoryTab, QIcon::fromTheme(QLatin1String("view-catalog")),           i18n("Versions"));

    // ----------------------------------------------------------

    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));

    connect(d->desceditTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->desceditTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChangeDatabase(ImageChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChanged(ImageTagChangeset)));

    connect(ImageAttributesWatch::instance(), SIGNAL(signalFileMetadataChanged(QUrl)),
            this, SLOT(slotFileMetadataChanged(QUrl)));

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

void ImagePropertiesSideBarDB::itemChanged(const QUrl& url, const QRect& rect, DImg* const img)
{
    itemChanged(url, ImageInfo(), rect, img, DImageHistory());
}

void ImagePropertiesSideBarDB::itemChanged(const QUrl& url, const ImageInfo& info,
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

    if (tab == m_propertiesTab && !m_dirtyPropertiesTab)
    {
        m_propertiesTab->setCurrentURL(m_currentURL);
        if (d->currentInfos.isEmpty())
        {
            ImagePropertiesSideBar::setImagePropertiesInformation(m_currentURL);
        }
        else
        {
            setImagePropertiesInformation(m_currentURL);
        }
        m_dirtyPropertiesTab = true;
    }
    else if (tab == m_metadataTab && !m_dirtyMetadataTab)
    {
        if (d->currentInfos.count() > 1)
        {
            // No multiple selection supported. Only if all items belong to
            // the same group display metadata of main item.
            ImageInfo mainItem = d->currentInfos.singleGroupMainItem();
            if (!mainItem.isNull())
            {
                m_metadataTab->setCurrentURL(mainItem.fileUrl());
            }
            else
            {
                m_metadataTab->setCurrentURL();
            }
        }
        else if (m_image)
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
        if (d->currentInfos.count() > 1)
        {
            // No multiple selection supported. Only if all items belong to
            // the same group display metadata of main item.
            ImageInfo mainItem = d->currentInfos.singleGroupMainItem();
            if (!mainItem.isNull())
            {
                m_colorTab->setData(mainItem.fileUrl());
            }
            else
            {
                m_colorTab->setData();
            }
        }
        else
        {
            m_colorTab->setData(m_currentURL, m_currentRect, m_image);
        }
        m_dirtyColorTab = true;
    }
    else if (tab == d->desceditTab && !d->dirtyDesceditTab)
    {

        if (d->currentInfos.count() == 0)
        {
            // Do nothing here. We cannot get data from database !
            d->desceditTab->setItem();
        }
        else if (d->currentInfos.count() == 1)
        {
            d->desceditTab->setItem(d->currentInfos.first());
        }
        else
        {
            d->desceditTab->setItems(d->currentInfos);
        }
        d->dirtyDesceditTab = true;
    }
#ifdef HAVE_MARBLE
    else if (tab == m_gpsTab && !m_dirtyGpsTab)
    {

        if (d->currentInfos.count() == 0)
        {
            m_gpsTab->setCurrentURL(m_currentURL);
        }
        else
        {
            GPSImageInfo::List list;

            for (ImageInfoList::const_iterator it = d->currentInfos.constBegin();
                 it != d->currentInfos.constEnd(); ++it)
            {
                GPSImageInfo info;

                if (GPSImageInfofromImageInfo(*it, &info))
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
        }

        m_dirtyGpsTab = true;
    }
#endif // HAVE_MARBLE
    else if (tab == d->versionsHistoryTab && !m_dirtyHistoryTab)
    {
        //TODO: Make a database-less parent class with only the filters tab
        if (d->currentInfos.count() == 0 || d->currentInfos.count() > 1)
        {
            // FIXME: Any sensible multi-selection functionality? Must scale for large n!
            d->versionsHistoryTab->clear();
        }
        else
        {
            d->versionsHistoryTab->setItem(d->currentInfos.first(), d->currentHistory);
        }
        m_dirtyHistoryTab = true;
    }

#ifdef HAVE_MARBLE
    m_gpsTab->setActive(tab == m_gpsTab);
#endif // HAVE_MARBLE

    unsetCursor();
}

void ImagePropertiesSideBarDB::slotFileMetadataChanged(const QUrl& url)
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
#ifdef HAVE_MARBLE
            || tab == m_gpsTab
#endif // HAVE_MARBLE
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
#ifdef HAVE_MARBLE
                    || tab == m_gpsTab
#endif // HAVE_MARBLE
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

void ImagePropertiesSideBarDB::setImagePropertiesInformation(const QUrl& url)
{
    foreach(const ImageInfo& info, d->currentInfos)
    {
        if (info.fileUrl() == url)
        {
            QString str;
            QString unavailable(i18n("<i>unavailable</i>"));
            QFileInfo fileInfo(url.toLocalFile());

            // -- File system information -----------------------------------------

            ImageCommonContainer commonInfo  = info.imageCommonContainer();
            ImageMetadataContainer photoInfo = info.imageMetadataContainer();
            VideoMetadataContainer videoInfo = info.videoMetadataContainer();

            str = QLocale().toString(commonInfo.fileModificationDate, QLocale::ShortFormat);
            m_propertiesTab->setFileModifiedDate(str);

            str = QString::fromUtf8("%1 (%2)").arg(ImagePropertiesTab::humanReadableBytesCount(fileInfo.size()))
                                    .arg(QLocale().toString(commonInfo.fileSize));
            m_propertiesTab->setFileSize(str);

            //  These infos are not stored in DB
            m_propertiesTab->setFileOwner(QString::fromUtf8("%1 - %2").arg(fileInfo.owner()).arg(fileInfo.group()));
            m_propertiesTab->setFilePermissions(ImagePropertiesTab::permissionsString(fileInfo));

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
                str = QLocale().toString(commonInfo.creationDate, QLocale::ShortFormat);
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
                str = QString::fromUtf8("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
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
    KConfigGroup groupVersionTab      = KConfigGroup(&group, entryName(QLatin1String("Version Properties Tab")));
    d->versionsHistoryTab->readSettings(groupVersionTab);

    KConfigGroup groupCaptionsTagsTab = KConfigGroup(&group, entryName(QLatin1String("Captions Tags Properties Tab")));
    d->desceditTab->readSettings(groupCaptionsTagsTab);
}

void ImagePropertiesSideBarDB::doSaveState()
{
    ImagePropertiesSideBar::doSaveState();

    KConfigGroup group           = getConfigGroup();
    KConfigGroup groupVersionTab = KConfigGroup(&group, entryName(QLatin1String("Version Properties Tab")));
    d->versionsHistoryTab->writeSettings(groupVersionTab);

    KConfigGroup groupCaptionsTagsTab = KConfigGroup(&group, entryName(QLatin1String("Captions Tags Properties Tab")));
    d->desceditTab->writeSettings(groupCaptionsTagsTab);
}

void ImagePropertiesSideBarDB::slotPopupTagsView()
{
    setActiveTab(d->desceditTab);
    d->desceditTab->setFocusToTagsView();
}

#ifdef HAVE_MARBLE

bool ImagePropertiesSideBarDB::GPSImageInfofromImageInfo(const ImageInfo& imageInfo, GPSImageInfo* const gpsImageInfo)
{
    const ImagePosition pos = imageInfo.imagePosition();

    if (pos.isEmpty() || !pos.hasCoordinates())
    {
        return false;
    }

    gpsImageInfo->coordinates.setLatLon(pos.latitudeNumber(), pos.longitudeNumber());

    if (pos.hasAltitude())
    {
        gpsImageInfo->coordinates.setAlt(pos.altitude());
    }

    gpsImageInfo->dateTime  = imageInfo.dateTime();
    gpsImageInfo->rating    = imageInfo.rating();
    gpsImageInfo->url       = imageInfo.fileUrl();
    gpsImageInfo->id        = imageInfo.id();

    return true;
}

#endif // HAVE_MARBLE

}  // namespace Digikam
