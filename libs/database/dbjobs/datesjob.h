#ifndef DATESJOB_H
#define DATESJOB_H

#include "dbjob.h"
#include "dbjobinfo.h"

namespace Digikam {

class DatesJob : public DBJob
{
    Q_OBJECT

public:

    DatesJob(DatesDBJobInfo *jobInfo);
    ~DatesJob();

protected:

    void run();

private:

    DatesDBJobInfo *m_jobInfo;
};

} // namespace Digikam

#endif // DATESJOB_H
