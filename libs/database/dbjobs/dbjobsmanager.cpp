#include "dbjobsmanager.h"

#include "dbjobsthread.h"
#include "dbjobinfo.h"
#include <qdebug.h>

namespace Digikam {

class DBJobsManagerCreator
{
public:

    DBJobsManager object;
};

Q_GLOBAL_STATIC(DBJobsManagerCreator, creator)

// -----------------------------------------------

DBJobsManager::DBJobsManager()
{
}

DBJobsManager::~DBJobsManager()
{
}

DBJobsManager *DBJobsManager::instance()
{
    return &creator->object;
}

DBJobsThread *DBJobsManager::startAlbumsJobThread(AlbumsDBJobInfo *jInfo)
{
    DBJobsThread *thread = new DBJobsThread(this);
    thread->albumsListing(jInfo);
    thread->start();

    return thread;
}

DBJobsThread* DBJobsManager::startDatesJobThread(DatesDBJobInfo *jInfo)
{
    DBJobsThread *thread = new DBJobsThread(this);
    thread->datesListing(jInfo);
    thread->start();

    return thread;
}

DBJobsThread *DBJobsManager::startTagsJobThread(TagsDBJobInfo *jInfo)
{
    DBJobsThread *thread = new DBJobsThread(this);
    thread->tagsListing(jInfo);
    thread->start();

    return thread;
}

SearchesDBJobsThread *DBJobsManager::startSearchesJobThread(SearchesDBJobInfo *jInfo)
{
    SearchesDBJobsThread *thread = new SearchesDBJobsThread(this);
    thread->searchesListing(jInfo);
    thread->start();

    return thread;
}

GPSDBJobsThread *DBJobsManager::startGPSJobThread(GPSDBJobInfo *jInfo)
{
    GPSDBJobsThread *thread = new GPSDBJobsThread(this);
    thread->GPSListing(jInfo);
    thread->start();

    return thread;
}

} // namespace Digikam
