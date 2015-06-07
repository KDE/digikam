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

class DatesJob::Private
{
public:

    Private()
        : folders(0)
    {
    }

    bool       folders;
    QDate      startDate;
    QDate      endDate;
};

// -------------------------------------------------------

DatesJob::DatesJob()
    : DBJob(), d(new Private)
{
}

DatesJob::~DatesJob()
{
    delete d;
}

void DatesJob::setStartDate(const QDate &startDate)
{
    d->startDate = startDate;
}

void DatesJob::setEndDate(const QDate &endDate)
{
    d->endDate = endDate;
}

void DatesJob::setFoldersListing(bool folders)
{
    d->folders = folders;
}

void DatesJob::run()
{
    if (d->folders)
    {
        QMap<QDateTime, int> dateNumberMap = Digikam::DatabaseAccess().db()->getAllCreationDatesAndNumberOfImages();
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
        lister.listDateRange(&receiver, d->startDate, d->endDate);
        // send rest
        receiver.sendData();
    }
}

} // namespace Digikam
