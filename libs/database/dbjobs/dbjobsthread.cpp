#include "dbjobsthread.h"
#include "datesjob.h"
#include "databaseaccess.h"
#include "dbjobinfo.h"
#include "gpsjob.h"

namespace Digikam {

DBJobsThread::DBJobsThread(QObject* const parent)
    : RActionThreadBase(parent)
{
}

DBJobsThread::~DBJobsThread()
{
}

void DBJobsThread::datesListing(DatesDBJobInfo *info)
{
    DatesJob *j = new DatesJob(info);

    connect(j, SIGNAL(data(const QByteArray &)),
            this, SIGNAL(data(const QByteArray &)));

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

void DBJobsThread::GPSListing(GPSDBJobInfo *info)
{
    GPSJob *j = new GPSJob(info);

    connect(j, SIGNAL(data(const QByteArray &)),
            this, SIGNAL(data(const QByteArray &)));

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

} // namespace Digikam
