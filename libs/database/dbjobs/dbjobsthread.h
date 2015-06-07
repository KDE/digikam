#ifndef DBJOBSTHREAD_H
#define DBJOBSTHREAD_H

// KDCraw Includes

#include "KDCRAW/RActionThreadBase"

// Local includes

#include "databaseparameters.h"
#include "dbjobinfo.h"
#include "dbjob.h"

using namespace KDcrawIface;

namespace Digikam {

class DBJobsThread : public RActionThreadBase
{
    Q_OBJECT

public:

    explicit DBJobsThread(QObject* const parent);
    ~DBJobsThread();

    void datesListing(const QDate &startDate, const QDate &endDate, bool folders = false);

    void setUseMultiCore(const bool useMultiCore);

Q_SIGNALS:
    void signalData(const QByteArray &);

};

} // namespace Digikam

#endif // DBJOBSTHREAD_H
