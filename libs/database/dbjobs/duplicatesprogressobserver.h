#ifndef DUPLICATESPROGRESSOBSERVER_H
#define DUPLICATESPROGRESSOBSERVER_H

#include "haariface.h"
#include "dbjob.h"

namespace Digikam {

class SearchesJob;

class DuplicatesProgressObserver : public HaarProgressObserver
{

public:

    DuplicatesProgressObserver(SearchesJob *thread);
    ~DuplicatesProgressObserver();

    virtual void totalNumberToScan(int number);
    virtual void processedNumber(int number);

private:

    SearchesJob *m_job;
};

} // namespace Digikam

#endif // DUPLICATESPROGRESSOBSERVER_H
