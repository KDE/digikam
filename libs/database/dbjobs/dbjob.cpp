/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-01
 * Description : DB Jobs for listing and scanning
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "dbjob.h"

// Local includes

#include "coredbaccess.h"
#include "dbengineparameters.h"
#include "coredb.h"
#include "imagelister.h"
#include "digikam_export.h"
#include "digikam_debug.h"
#include "dbjobsthread.h"

namespace Digikam
{

DBJob::DBJob()
    : ActionJob()
{
}

DBJob::~DBJob()
{
    this->cancel();
}

// ----------------------------------------------

AlbumsJob::AlbumsJob(const AlbumsDBJobInfo& jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

AlbumsJob::~AlbumsJob()
{
}

void AlbumsJob::run()
{
    if (m_jobInfo.isFoldersJob())
    {
        QMap<int, int> albumNumberMap = CoreDbAccess().db()->getNumberOfImagesInAlbums();
        emit foldersData(albumNumberMap);
    }
    else
    {
        ImageLister lister;
        lister.setRecursive(m_jobInfo.isRecursive());
        lister.setListOnlyAvailable(m_jobInfo.isListAvailableImagesOnly());

        // Send data every 200 images to be more responsive
        Digikam::ImageListerJobGrowingPartsSendingReceiver receiver(this, 200, 2000, 100);
        lister.listAlbum(&receiver, m_jobInfo.albumRootId(), m_jobInfo.album());
        receiver.sendData();
    }

    emit signalDone();
}

// ----------------------------------------------

DatesJob::DatesJob(const DatesDBJobInfo& jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

DatesJob::~DatesJob()
{
}

void DatesJob::run()
{
    if (m_jobInfo.isFoldersJob())
    {
        QMap<QDateTime, int> dateNumberMap = CoreDbAccess().db()->getAllCreationDatesAndNumberOfImages();
        emit foldersData(dateNumberMap);
    }
    else
    {
        ImageLister lister;
        lister.setListOnlyAvailable(true);

        // Send data every 200 images to be more responsive
        ImageListerJobPartsSendingReceiver receiver(this, 200);
        lister.listDateRange(&receiver, m_jobInfo.startDate(), m_jobInfo.endDate());

        // Send rest
        receiver.sendData();
    }

    emit signalDone();
}

// ----------------------------------------------

GPSJob::GPSJob(const GPSDBJobInfo& jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

GPSJob::~GPSJob()
{
}

void GPSJob::run()
{
    if (m_jobInfo.isDirectQuery())
    {
        QList<QVariant> imagesInfoFromArea =
                CoreDbAccess().db()->getImageIdsFromArea(m_jobInfo.lat1(),
                                                         m_jobInfo.lat2(),
                                                         m_jobInfo.lng1(),
                                                         m_jobInfo.lng2(),
                                                         0,
                                                         QLatin1String("rating"));

        emit directQueryData(imagesInfoFromArea);
    }
    else
    {
        ImageLister lister;
        lister.setAllowExtraValues(true);
        lister.setListOnlyAvailable(m_jobInfo.isListAvailableImagesOnly());

        // Send data every 200 images to be more responsive
        ImageListerJobPartsSendingReceiver receiver(this, 200);
        lister.listAreaRange(&receiver,
                             m_jobInfo.lat1(),
                             m_jobInfo.lat2(),
                             m_jobInfo.lng1(),
                             m_jobInfo.lng2());
        // send rest
        receiver.sendData();
    }

    emit signalDone();
}

// ----------------------------------------------

TagsJob::TagsJob(const TagsDBJobInfo& jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

TagsJob::~TagsJob()
{
}

void TagsJob::run()
{
    if (m_jobInfo.isFoldersJob())
    {
        QMap<int, int> tagNumberMap = CoreDbAccess().db()->getNumberOfImagesInTags();
        //qCDebug(DIGIKAM_DBJOB_LOG) << tagNumberMap;
        emit foldersData(tagNumberMap);
    }
    else if (m_jobInfo.isFaceFoldersJob())
    {
        QMap<QString, QMap<int, int> > facesNumberMap;

        facesNumberMap[ImageTagPropertyName::autodetectedFace()]   =
            CoreDbAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::autodetectedFace());

        facesNumberMap[ImageTagPropertyName::tagRegion()]          =
            CoreDbAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::tagRegion());

        facesNumberMap[ImageTagPropertyName::autodetectedPerson()] =
            CoreDbAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::autodetectedPerson());

        emit faceFoldersData(facesNumberMap);
    }
    else
    {
        ImageLister lister;
        lister.setRecursive(m_jobInfo.isRecursive());
        lister.setListOnlyAvailable(m_jobInfo.isListAvailableImagesOnly());

        // Send data every 200 images to be more responsive
        ImageListerJobPartsSendingReceiver receiver(this, 200);

        if (!m_jobInfo.specialTag().isNull())
        {
            QString searchXml =
                lister.tagSearchXml(m_jobInfo.tagsIds().first(),
                                    m_jobInfo.specialTag(),
                                    m_jobInfo.isRecursive());
            lister.setAllowExtraValues(true); // pass property value as extra value, different binary protocol
            lister.listImageTagPropertySearch(&receiver, searchXml);
        }
        else
        {
            lister.listTag(&receiver, m_jobInfo.tagsIds());
        }

        // Finish sending
        receiver.sendData();
    }

    emit signalDone();
}

// ----------------------------------------------

SearchesJob::SearchesJob(const SearchesDBJobInfo& jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

SearchesJob::~SearchesJob()
{
}

void SearchesJob::run()
{
    if (!m_jobInfo.isDuplicatesJob())
    {
        QList<SearchInfo> infos;

        foreach(int id, m_jobInfo.searchIds())
        {
            infos << CoreDbAccess().db()->getSearchInfo(id);
        }

        ImageLister lister;
        lister.setListOnlyAvailable(m_jobInfo.isListAvailableImagesOnly());

        // Send data every 200 images to be more responsive
        ImageListerJobPartsSendingReceiver receiver(this, 200);

        foreach(SearchInfo info, infos)
        {
            if (info.type == DatabaseSearch::HaarSearch)
            {
                lister.listHaarSearch(&receiver, info.query);
            }
            else
            {
                bool ok;
                qlonglong referenceImageId = info.name.toLongLong(&ok);

                if (ok)
                {
                    lister.listSearch(&receiver, info.query, 0, referenceImageId);
                }
                else
                {
                    lister.listSearch(&receiver, info.query, 0, -1);
                }
            }

            if (!receiver.hasError)
            {
                receiver.sendData();
            }
        }
    }
    else
    {
        if (m_jobInfo.albumsIds().isEmpty() && m_jobInfo.tagsIds().isEmpty() && m_jobInfo.imageIds().isEmpty())
        {
            qCDebug(DIGIKAM_DBJOB_LOG) << "No album, tag or image ids passed for duplicates search";
            return;
        }

        if (m_jobInfo.minThreshold() == 0)
        {
            m_jobInfo.setMinThreshold(0.4);
        }

        DuplicatesProgressObserver observer(this);

        // Rebuild the duplicate albums
        HaarIface iface;

        if (m_jobInfo.isAlbumUpdate())
        {
            iface.rebuildDuplicatesAlbums(m_jobInfo.imageIds(),
                                          m_jobInfo.minThreshold(),
                                          m_jobInfo.maxThreshold(),
                                          static_cast<HaarIface::DuplicatesSearchRestrictions>(m_jobInfo.searchResultRestriction()),
                                          &observer);
        }
        else
        {
            iface.rebuildDuplicatesAlbums(m_jobInfo.albumsIds(),
                                          m_jobInfo.tagsIds(),
                                          static_cast<HaarIface::AlbumTagRelation>(m_jobInfo.albumTagRelation()),
                                          m_jobInfo.minThreshold(),
                                          m_jobInfo.maxThreshold(),
                                          static_cast<HaarIface::DuplicatesSearchRestrictions>(m_jobInfo.searchResultRestriction()),
                                          &observer);
        }
    }

    emit signalDone();
}

bool SearchesJob::isCanceled()
{
    return m_cancel;
}

} // namespace Digikam
