#include "dbjobsthread.h"
#include "datesjob.h"
#include "databaseaccess.h"
#include "dbjobinfo.h"

namespace Digikam {

DBJobsThread::DBJobsThread(QObject* const parent)
    : RActionThreadBase(parent)
{
}

DBJobsThread::~DBJobsThread()
{
}

void DBJobsThread::datesListing(const QDate &startDate, const QDate &endDate, bool folders)
{
    DatesJob *j = new DatesJob();

    if(folders)
    {
        j->setFoldersListing(folders);
    }
    else
    {
        j->setStartDate(startDate);
        j->setEndDate(endDate);
    }

    connect(j, SIGNAL(data(const QByteArray &)),
            this, SIGNAL(signalData(const QByteArray &)));

    RJobCollection collection;
    collection.insert(j,0);
    appendJobs(collection);
}

} // namespace Digikam
