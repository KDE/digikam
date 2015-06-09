#include "datesjob.h"

// Qt includes

#include <QDataStream>
#include <QFile>
#include <QUrl>
#include <qloggingcategory.h>

// Local includes

#include "databaseaccess.h"
#include "databaseparameters.h"
#include "albumdb.h"
#include "imagelister.h"
#include "digikam_export.h"
#include "digikam_debug.h"

namespace Digikam {

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
        QByteArray           ba;
        QDataStream          os(&ba, QIODevice::WriteOnly);
        os << dateNumberMap;
        emit data(ba);
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
}

} // namespace Digikam
