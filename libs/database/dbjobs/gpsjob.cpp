#include "gpsjob.h"
#include "imagelister.h"
#include "imagelisterreceiver.h"
#include "databaseaccess.h"
#include "databaseparameters.h"
#include "albumdb.h"
#include "digikam_export.h"
#include "digikam_debug.h"

namespace Digikam {

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

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << imagesInfoFromArea;
        emit data(ba);
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
}

} // namespace Digikam
