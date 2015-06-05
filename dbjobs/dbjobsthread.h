#ifndef DBJOBSTHREAD_H
#define DBJOBSTHREAD_H

// KDCraw Includes

#include "KDCRAW/RActionThreadBase"

using namespace KDcrawIface;

namespace Digikam {

class DBJobsThread : public RActionThreadBase
{
    Q_OBJECT

public:

    explicit DBJobsThread(QObject* const parent);
    ~DBJobsThread();

    void setUseMultiCore(const bool useMultiCore);
    void cancel();
};

} // namespace Digikam

#endif // DBJOBSTHREAD_H
