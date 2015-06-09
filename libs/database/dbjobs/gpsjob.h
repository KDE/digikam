#ifndef GPSJOB_H
#define GPSJOB_H

#include "dbjob.h"
#include "dbjobinfo.h"

namespace Digikam {

class GPSJob : public DBJob
{
    Q_OBJECT

public:

    GPSJob(GPSDBJobInfo *jobInfo);
    ~GPSJob();

protected:

    void run();

private:

    GPSDBJobInfo *m_jobInfo;
};

} // namespace Digikam

#endif // GPSJOB_H
