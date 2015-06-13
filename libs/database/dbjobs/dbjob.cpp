#include "KDCRAW/RActionJob"

#include "dbjob.h"
#include "databaseaccess.h"
#include "databaseparameters.h"
#include "albumdb.h"
#include "imagelister.h"
#include "digikam_export.h"
#include "digikam_debug.h"
#include "dbjobsthread.h"

namespace Digikam {

DBJob::DBJob()
    : RActionJob()
{
}

DBJob::~DBJob()
{
    this->cancel();
}

// ----------------------------------------------

AlbumsJob::AlbumsJob(AlbumsDBJobInfo *jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

AlbumsJob::~AlbumsJob()
{
}

void AlbumsJob::run()
{
    if (m_jobInfo->folders)
    {
        QMap<int, int> albumNumberMap = DatabaseAccess().db()->getNumberOfImagesInAlbums();
        emit foldersData(albumNumberMap);
    }
    else
    {
        ImageLister lister;
        lister.setRecursive(m_jobInfo->recursive);
        lister.setListOnlyAvailable(m_jobInfo->listAvailableImagesOnly);
        // send data every 200 images to be more responsive
        Digikam::ImageListerJobGrowingPartsSendingReceiver receiver(this, 200, 2000, 100);
        lister.listAlbum(&receiver, m_jobInfo->albumRootId, m_jobInfo->album);
        receiver.sendData();
    }

    emit signalDone();
}

// ----------------------------------------------

DatesJob::DatesJob(DatesDBJobInfo *jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

DatesJob::~DatesJob()
{
}

void DatesJob::run()
{
    if (m_jobInfo->folders)
    {
        QMap<QDateTime, int> dateNumberMap = DatabaseAccess().db()->getAllCreationDatesAndNumberOfImages();
        emit foldersData(dateNumberMap);
    }
    else
    {
        ImageLister lister;
        lister.setListOnlyAvailable(true);
        // send data every 200 images to be more responsive
        ImageListerJobPartsSendingReceiver receiver(this, 200);
        lister.listDateRange(&receiver, m_jobInfo->startDate, m_jobInfo->endDate);
        // send rest
        receiver.sendData();
    }

    emit signalDone();
}

// ----------------------------------------------

GPSJob::GPSJob(GPSDBJobInfo *jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

GPSJob::~GPSJob()
{
}

void GPSJob::run()
{
    if (m_jobInfo->wantDirectQuery)
    {
        QList<QVariant> imagesInfoFromArea =
                DatabaseAccess().db()->getImageIdsFromArea(m_jobInfo->lat1,
                                                           m_jobInfo->lat2,
                                                           m_jobInfo->lng1,
                                                           m_jobInfo->lng2,
                                                           0,
                                                           QLatin1String("rating"));

        emit directQueryData(imagesInfoFromArea);
    }
    else
    {
        ImageLister lister;
        lister.setAllowExtraValues(true);
        lister.setListOnlyAvailable(m_jobInfo->listAvailableImagesOnly);
        // send data every 200 images to be more responsive
        ImageListerJobPartsSendingReceiver receiver(this, 200);
        lister.listAreaRange(&receiver,
                             m_jobInfo->lat1,
                             m_jobInfo->lat2,
                             m_jobInfo->lng1,
                             m_jobInfo->lng2);
        // send rest
        receiver.sendData();
    }

    emit signalDone();
}

// ----------------------------------------------

TagsJob::TagsJob(TagsDBJobInfo *jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

TagsJob::~TagsJob()
{
}

void TagsJob::run()
{
    if (m_jobInfo->folders)
    {
        QMap<int, int> tagNumberMap = DatabaseAccess().db()->getNumberOfImagesInTags();
        emit foldersData(tagNumberMap);
    }
    else if (m_jobInfo->faceFolders)
    {
        QMap<QString, QMap<int, int> > facesNumberMap;
        facesNumberMap[ImageTagPropertyName::autodetectedFace()] =
            DatabaseAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::autodetectedFace());
        facesNumberMap[ImageTagPropertyName::tagRegion()]        =
            DatabaseAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::tagRegion());

        emit faceFoldersData(facesNumberMap);
    }
    else
    {
        ImageLister lister;
        lister.setRecursive(m_jobInfo->recursive);
        lister.setListOnlyAvailable(m_jobInfo->listAvailableImagesOnly);
        // send data every 200 images to be more responsive
        ImageListerJobPartsSendingReceiver receiver(this, 200);

        if (!m_jobInfo->specialTag.isNull())
        {
            QString searchXml =
                lister.tagSearchXml(m_jobInfo->tagsIds.first(),
                                    m_jobInfo->specialTag,
                                    m_jobInfo->recursive);
            lister.setAllowExtraValues(true); // pass property value as extra value, different binary protocol
            lister.listImageTagPropertySearch(&receiver, searchXml);
        }
        else
        {
            lister.listTag(&receiver, m_jobInfo->tagsIds);
        }

        // finish sending
        receiver.sendData();
    }
    emit signalDone();
}

// ----------------------------------------------

SearchesJob::SearchesJob(SearchesDBJobInfo *jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

SearchesJob::~SearchesJob()
{
}

void SearchesJob::run()
{
    // TODO: CHECK WHEN DOES THIS VALUE CHANGE FROM ZERO
    int listingType = 0;

    if (!m_jobInfo->duplicates)
    {
        SearchInfo info = DatabaseAccess().db()->getSearchInfo(m_jobInfo->searchId);

        ImageLister lister;
        lister.setListOnlyAvailable(m_jobInfo->listAvailableImagesOnly);

        if (listingType == 0)
        {
            // send data every 200 images to be more responsive
            ImageListerJobPartsSendingReceiver receiver(this, 200);

            if (info.type == DatabaseSearch::HaarSearch)
            {
                lister.listHaarSearch(&receiver, info.query);
            }
            else
            {
                lister.listSearch(&receiver, info.query);
            }

            if (!receiver.hasError)
            {
                receiver.sendData();
            }
        }
        else
        {
            ImageListerJobReceiver receiver(this);
            // fast mode: limit results to 500
            lister.listSearch(&receiver, info.query, 500);

            if (!receiver.hasError)
            {
                receiver.sendData();
            }
        }
    }
    else
    {
        if (m_jobInfo->albumIds.isEmpty() && m_jobInfo->tagIds.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "No album ids passed for duplicates search";
            return;
        }

        if (m_jobInfo->threshold == 0)
        {
            m_jobInfo->threshold = 0.4;
        }

        DuplicatesProgressObserver observer(this);

        // rebuild the duplicate albums
        HaarIface iface;
        iface.rebuildDuplicatesAlbums(m_jobInfo->albumIds,
                                      m_jobInfo->tagIds,
                                      m_jobInfo->threshold,
                                      &observer);
    }
    emit signalDone();
}

} // namespace Digikam
