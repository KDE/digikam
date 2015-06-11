#include "dbjobsthread.h"
#include "databaseaccess.h"
#include "dbjobinfo.h"
#include "dbjob.h"
#include "duplicatesprogressobserver.h"

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

    connect(j, SIGNAL(done()),
            this, SIGNAL(finished()));

    connect(j, SIGNAL(data(const QByteArray &)),
            this, SIGNAL(data(const QByteArray &)));

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

void DBJobsThread::tagsListing(TagsDBJobInfo *info)
{
    TagsJob *j = new TagsJob(info);

    connect(j, SIGNAL(done()),
            this, SIGNAL(finished()));

    connect(j, SIGNAL(data(const QByteArray &)),
            this, SIGNAL(data(const QByteArray &)));

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

// -------------------------------------------------

GPSDBJobsThread::GPSDBJobsThread(QObject * const parent)
    : DBJobsThread(parent)
{
}

GPSDBJobsThread::~GPSDBJobsThread()
{
}

void GPSDBJobsThread::data(const QByteArray & data)
{
    emit signalData(this, data);
}

void GPSDBJobsThread::done()
{
    emit signalDone(this);
}

void GPSDBJobsThread::GPSListing(GPSDBJobInfo *info)
{
    GPSJob *j = new GPSJob(info);

    connect(j, SIGNAL(done()),
            this, SLOT(done()));

    connect(j, SIGNAL(data(const QByteArray &)),
            this, SLOT(data(const QByteArray &)));

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

// -------------------------------------------------

SearchesDBJobsThread::SearchesDBJobsThread(QObject * const parent)
    : DBJobsThread(parent)
{
}

SearchesDBJobsThread::~SearchesDBJobsThread()
{
}

void SearchesDBJobsThread::searchesListing(SearchesDBJobInfo *info)
{
    SearchesJob *j = new SearchesJob(info);

    connect(j, SIGNAL(done()),
            this, SIGNAL(finished()));

    if(info->duplicates)
    {

        connect(j, SIGNAL(totalSize(int)),
                this, SIGNAL(totalSize(int)));

        connect(j, SIGNAL(processedSize(int)),
                this, SIGNAL(processedSize(int)));
    }
    else
    {
        connect(j, SIGNAL(data(const QByteArray &)),
                this, SIGNAL(data(const QByteArray &)));
    }

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

} // namespace Digikam
