#ifndef DBJOBSTHREAD_H
#define DBJOBSTHREAD_H

// KDCraw Includes

#include "KDCRAW/RActionThreadBase"

// Local includes

#include "databaseparameters.h"
#include "dbjobinfo.h"
#include "dbjob.h"
#include "haariface.h"
#include "imagelisterrecord.h"

using namespace KDcrawIface;

namespace Digikam {

class DBJobsThread : public RActionThreadBase
{
    Q_OBJECT

public:

    explicit DBJobsThread(QObject* const parent);
    ~DBJobsThread();

    void setUseMultiCore(const bool useMultiCore);

Q_SIGNALS:
    void finished();
    void data(const QList<ImageListerRecord> &records);
};

// ---------------------------------------------

class AlbumsDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit AlbumsDBJobsThread(QObject *const parent);
    ~AlbumsDBJobsThread();

    void albumsListing(AlbumsDBJobInfo *info);

Q_SIGNALS:

    void foldersData(const QMap<int, int> &);
    void faceFoldersData(const QMap<QString, QMap<int, int> > &);
};

// ---------------------------------------------

class TagsDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit TagsDBJobsThread(QObject *const parent);
    ~TagsDBJobsThread();

    void tagsListing(TagsDBJobInfo *info);

Q_SIGNALS:

    void foldersData(const QMap<int, int> &);
    void faceFoldersData(const QMap<QString, QMap<int, int> > &);
};

// ---------------------------------------------

class DatesDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit DatesDBJobsThread(QObject *const parent);
    ~DatesDBJobsThread();

    void datesListing(DatesDBJobInfo *info);

Q_SIGNALS:

    void foldersData(const QMap<QDateTime,int> &);
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

Q_SIGNALS:

    void directQueryData(const QList<QVariant> & data);
};

} // namespace Digikam

#endif // DBJOBSTHREAD_H
