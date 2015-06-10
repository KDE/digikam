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

    void albumsListing(AlbumsDBJobInfo *info);
    void datesListing(DatesDBJobInfo *info);
    void GPSListing(GPSDBJobInfo *info);
    void tagsListing(TagsDBJobInfo *info);
    void searchesListing(SearchesDBJobInfo *info);

    void setUseMultiCore(const bool useMultiCore);

Q_SIGNALS:
    void data(const QByteArray &);

};

} // namespace Digikam

#endif // DBJOBSTHREAD_H
