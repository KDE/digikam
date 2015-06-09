#ifndef DBJOBSMANAGER_H
#define DBJOBSMANAGER_H

#include <QObject>
#include "dbjobinfo.h"
#include "dbjobsthread.h"

namespace Digikam {

class DBJobsManager : public QObject
{
    Q_OBJECT
public:
    explicit DBJobsManager();
    ~DBJobsManager();

    static DBJobsManager* instance();

    DBJobsThread* startDatesJobThread(DatesDBJobInfo *jInfo);
    DBJobsThread* startGPSJobThread(GPSDBJobInfo *jInfo);

private:
    friend class DBJobsManagerCreator;
};

} // namespace Digikam

#endif // DBJOBSMANAGER_H
