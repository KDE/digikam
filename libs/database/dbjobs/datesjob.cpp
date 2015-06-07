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
        : cancel(0)
    {
    }

    bool       cancel;
    QByteArray data;
};

// -------------------------------------------------------

DatesJob::DatesJob()
    : RActionJob(), d(new Private)
{

}

DatesJob::~DatesJob()
{
    slotCancel();
    delete d;
}

void DatesJob::setData(const QByteArray &data)
{
    d->data = data;
}

void DatesJob::run()
{
    if(!d->cancel)
    {
//        bool        folders = (metaData(QLatin1String("folders")) == QLatin1String("true"));
        QUrl        url;
        QDataStream ds(d->data);
        ds >> url;

        DatabaseParameters dbParameters(url);
        DatabaseAccess::setParameters(dbParameters);

//        if (folders)
//        {
//            QMap<QDateTime, int> dateNumberMap = Digikam::DatabaseAccess().db()->getAllCreationDatesAndNumberOfImages();
//            QByteArray           ba;
//            QDataStream          os(&ba, QIODevice::WriteOnly);
//            os << dateNumberMap;
//            SlaveBase::data(ba);
//        }
//        else
//        {
            ImageLister lister;
            lister.setListOnlyAvailable(true);
            // send data every 200 images to be more responsive
            ImageListerJobPartsSendingReceiver receiver(this, 200);
            lister.list(&receiver, url);
            // send rest
            receiver.sendData();
//        }
    }
}

void DatesJob::slotCancel()
{
    d->cancel = true;
}

void DatesJob::slotData(const QByteArray &data)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "DATES JOB RECEIVED: " << data.toHex();
}

} // namespace Digikam
