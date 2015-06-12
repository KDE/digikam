#ifndef DBJOB_H
#define DBJOB_H

#include "KDCRAW/RActionJob"
#include "dbjobinfo.h"
#include "dbjobsthread.h"
#include "imagelisterrecord.h"
#include "duplicatesprogressobserver.h"

using namespace KDcrawIface;

namespace Digikam {

class DuplicatesProgressObserver;

class DBJob : public RActionJob
{
    Q_OBJECT

public:

    DBJob();
    ~DBJob();

Q_SIGNALS:

    void data(const QList<ImageListerRecord> &records);
    void done();
};

// ----------------------------------------------

class AlbumsJob : public DBJob
{
    Q_OBJECT

public:

    AlbumsJob(AlbumsDBJobInfo *jobInfo);
    ~AlbumsJob();

protected:

    void run();

Q_SIGNALS:

    void foldersData(const QMap<int, int> &);

private:

    AlbumsDBJobInfo *m_jobInfo;
};

// ----------------------------------------------

class DatesJob : public DBJob
{
    Q_OBJECT

public:

    DatesJob(DatesDBJobInfo *jobInfo);
    ~DatesJob();

protected:

    void run();

Q_SIGNALS:

    void foldersData(const QMap<QDateTime, int>& datesStatMap);

private:

    DatesDBJobInfo *m_jobInfo;
};

// ----------------------------------------------

class GPSJob : public DBJob
{
    Q_OBJECT

public:

    GPSJob(GPSDBJobInfo *jobInfo);
    ~GPSJob();

protected:

    void run();

Q_SIGNALS:

    void directQueryData(const QList<QVariant> & data);

private:

    GPSDBJobInfo *m_jobInfo;
};

// ----------------------------------------------

class TagsJob : public DBJob
{
    Q_OBJECT

public:

    TagsJob(TagsDBJobInfo *jobInfo);
    ~TagsJob();

protected:

    void run();

Q_SIGNALS:

    void foldersData(const QMap<int, int> & data);
    void faceFoldersData(const QMap<QString, QMap<int, int> > & data);

private:

    TagsDBJobInfo *m_jobInfo;
};

// ----------------------------------------------

class SearchesJob : public DBJob
{
    Q_OBJECT

public:

    SearchesJob(SearchesDBJobInfo *jobInfo);
    ~SearchesJob();

Q_SIGNALS:

    void processedSize(int);
    void totalSize(int);

protected:

    void run();

private:

    SearchesDBJobInfo *m_jobInfo;
};


} // namespace Digikam

#endif // DBJOB_H
