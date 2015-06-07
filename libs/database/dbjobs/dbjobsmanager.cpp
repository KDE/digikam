#include "dbjobsmanager.h"

#include "dbjobsthread.h"
#include "dbjobinfo.h"
#include <qdebug.h>

namespace Digikam {

class DBJobsManager::Private
{
public:

    Private()
    {
        thread = 0;
    }

    DBJobsThread *thread;
    DBJobInfo     jobInfo;
};

DBJobsManager::DBJobsManager(DBJobInfo &jobInfo)
    : d(new Private)
{
    d->thread = new DBJobsThread(this);
    d->jobInfo = jobInfo;

    connect(d->thread, SIGNAL(signalData(QByteArray)),
            this, SLOT(data(QByteArray)));
}

DBJobsManager::~DBJobsManager()
{
    delete d;
}

void DBJobsManager::start()
{
    if(d->jobInfo.type() == DBJobInfo::DatesJob)
    {
        d->thread->datesListing(d->jobInfo.databaseUrl().startDate(),
                                d->jobInfo.databaseUrl().endDate(),
                                d->jobInfo.folders);
        d->thread->start();
    }

}

void DBJobsManager::cancel()
{
    d->thread->cancel();
}

void DBJobsManager::data(const QByteArray &ba)
{
    qDebug() << "Data from New Manager: " << ba.toHex();
}

} // namespace Digikam
