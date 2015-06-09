#include "dbjob.h"
#include "KDCRAW/RActionJob"

namespace Digikam {

DBJob::DBJob()
    : RActionJob()
{
}

DBJob::~DBJob()
{
    this->cancel();
}

} // namespace Digikam
