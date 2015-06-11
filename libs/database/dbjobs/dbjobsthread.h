#ifndef DBJOBSTHREAD_H
#define DBJOBSTHREAD_H

// KDCraw Includes

#include "KDCRAW/RActionThreadBase"

// Local includes

#include "databaseparameters.h"
#include "dbjobinfo.h"
#include "dbjob.h"
#include "haariface.h"

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

    void setUseMultiCore(const bool useMultiCore);

Q_SIGNALS:
    void data(const QByteArray &);
    void finished();

};

// ---------------------------------------------

class SearchesDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit SearchesDBJobsThread(QObject* const parent);
    ~SearchesDBJobsThread();

    void searchesListing(SearchesDBJobInfo *info);

Q_SIGNALS:

    void processedSize(int number);
    void totalSize(int number);
};

// ---------------------------------------------

class GPSDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit GPSDBJobsThread(QObject* const parent);
    ~GPSDBJobsThread();

    void GPSListing(GPSDBJobInfo *info);

public Q_SLOTS:

    void data(const QByteArray &);
    void done();

Q_SIGNALS:

    void signalDone(GPSDBJobsThread*);
    void signalData(GPSDBJobsThread*, const QByteArray &);
};

} // namespace Digikam

#endif // DBJOBSTHREAD_H
