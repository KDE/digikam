#ifndef DATESJOB_H
#define DATESJOB_H

#include "dbjob.h"

namespace Digikam {

class DatesJob : public DBJob
{
    Q_OBJECT

public:

    DatesJob();
    ~DatesJob();

    void setStartDate(const QDate &startDate);
    void setEndDate(const QDate &endDate);
    void setFoldersListing(bool folders);

protected:

    void run();

private:

    class Private;
    Private *const d;
};

} // namespace Digikam

#endif // DATESJOB_H
