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

DBJobsThread* DBJobsManager::startDatesJobThread(DatesDBJobInfo *jInfo)
{
    DBJobsThread *thread = new DBJobsThread(this);
    thread->datesListing(jInfo);
    thread->start();

    return thread;
}

} // namespace Digikam
