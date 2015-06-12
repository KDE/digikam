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

// -------------------------------------------------

AlbumsDBJobsThread::AlbumsDBJobsThread(QObject *const parent)
    : DBJobsThread(parent)
{
}

AlbumsDBJobsThread::~AlbumsDBJobsThread()
{
}

void AlbumsDBJobsThread::albumsListing(AlbumsDBJobInfo *info)
{
    AlbumsJob *j = new AlbumsJob(info);

    connect(j, SIGNAL(done()),
            this, SIGNAL(finished()));

    if(info->folders)
    {
        connect(j, SIGNAL(foldersData(QMap<int,int>)),
                this, SIGNAL(foldersData(QMap<int,int>)));
    }
    else
    {
        connect(j, SIGNAL(data(QList<ImageListerRecord>)),
                this, SIGNAL(data(QList<ImageListerRecord>)));
    }

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

// -------------------------------------------------

TagsDBJobsThread::TagsDBJobsThread(QObject *const parent)
    : DBJobsThread(parent)
{
}

TagsDBJobsThread::~TagsDBJobsThread()
{
}

void TagsDBJobsThread::tagsListing(TagsDBJobInfo *info)
{
    TagsJob *j = new TagsJob(info);

    connect(j, SIGNAL(done()),
            this, SIGNAL(finished()));

    if(info->folders)
    {
        connect(j, SIGNAL(foldersData(QMap<int,int>)),
                this, SIGNAL(foldersData(QMap<int,int>)));
    }
    else if(info->faceFolders)
    {
        connect(j, SIGNAL(faceFoldersData(QMap<QString,QMap<int,int> >)),
                this, SIGNAL(faceFoldersData(QMap<QString,QMap<int,int> >)));
    }
    else
    {
        connect(j, SIGNAL(data(QList<ImageListerRecord>)),
                this, SIGNAL(data(QList<ImageListerRecord>)));
    }

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

// -------------------------------------------------

DatesDBJobsThread::DatesDBJobsThread(QObject *const parent)
    : DBJobsThread(parent)
{
}

DatesDBJobsThread::~DatesDBJobsThread()
{
}

void DatesDBJobsThread::datesListing(DatesDBJobInfo *info)
{
    DatesJob *j = new DatesJob(info);

    connect(j, SIGNAL(done()),
            this, SIGNAL(finished()));

    if(info->folders)
    {
        connect(j, SIGNAL(foldersData(const QMap<QDateTime,int> &)),
                this, SIGNAL(foldersData(const QMap<QDateTime,int> &)));
    }
    else
    {
        connect(j, SIGNAL(data(const QList<ImageListerRecord> &)),
                this, SIGNAL(data(const QList<ImageListerRecord> &)));
    }

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

void GPSDBJobsThread::data(const QList<ImageListerRecord> &data)
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

    connect(j, SIGNAL(data(QList<ImageListerRecord>)),
            this, SLOT(data(QList<ImageListerRecord>)));

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
        connect(j, SIGNAL(data(QList<ImageListerRecord>)),
                this, SIGNAL(data(QList<ImageListerRecord>)));
    }

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

} // namespace Digikam
