#include "duplicatesprogressobserver.h"

#include "dbjob.h"

namespace Digikam {

DuplicatesProgressObserver::DuplicatesProgressObserver(SearchesJob *thread)
    : m_job(thread)
{
}

DuplicatesProgressObserver::~DuplicatesProgressObserver()
{
    m_job = 0;
}

void DuplicatesProgressObserver::totalNumberToScan(int number)
{
    m_job->totalSize(number);
}

void DuplicatesProgressObserver::processedNumber(int number)
{
    m_job->processedSize(number);
}

} // namespace Digikam
