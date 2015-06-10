#include "dbjobsthread.h"
#include "databaseaccess.h"
#include "dbjobinfo.h"
#include "dbjob.h"

namespace Digikam {

DBJobsThread::DBJobsThread(QObject* const parent)
    : RActionThreadBase(parent)
{
}

DBJobsThread::~DBJobsThread()
{
}

void DBJobsThread::albumsListing(AlbumsDBJobInfo *info)
{
    AlbumsJob *j = new AlbumsJob(info);

    connect(j, SIGNAL(data(const QByteArray &)),
            this, SIGNAL(data(const QByteArray &)));

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
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

void DBJobsThread::tagsListing(TagsDBJobInfo *info)
{
    TagsJob *j = new TagsJob(info);

    connect(j, SIGNAL(data(const QByteArray &)),
            this, SIGNAL(data(const QByteArray &)));

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

void DBJobsThread::searchesListing(SearchesDBJobInfo *info)
{
    SearchesJob *j = new SearchesJob(info);

    connect(j, SIGNAL(data(const QByteArray &)),
            this, SIGNAL(data(const QByteArray &)));

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

} // namespace Digikam
