#ifndef DBJOBINFO_H
#define DBJOBINFO_H

#include <QString>
#include "databaseurl.h"

namespace Digikam {

class DBJobInfo
{
public:
    enum Type {
        AlbumsJob=0,
        TagsJob,
        DatesJob,
        SearchesJob,
        GPSJob
    };

    DBJobInfo(Type jType);
    DBJobInfo();

    Type        type();

    bool folders;
    bool listAvailableImagesOnly;
    bool recursive;

private:
    Type jobType;
};

// ---------------------------------------------

class AlbumsDBJobInfo : public DBJobInfo
{
public:
    AlbumsDBJobInfo();

    int     albumRootId;
    QString album;
};

// ---------------------------------------------

class TagsDBJobInfo : public DBJobInfo
{
public:
    TagsDBJobInfo();

    bool       faceFolders;
    QString    specialTag;
    QList<int> tagsIds;
};

// ---------------------------------------------

class GPSDBJobInfo : public DBJobInfo
{
public:
    GPSDBJobInfo();

    bool  wantDirectQuery;
    qreal lat1;
    qreal lng1;
    qreal lat2;
    qreal lng2;
};

// ---------------------------------------------

class SearchesDBJobInfo : public DBJobInfo
{
public:
    SearchesDBJobInfo();

    bool       duplicates;
    int        searchId;
    double     threshold;
    QList<int> albumIds;
    QList<int> tagIds;
};

// ---------------------------------------------

class DatesDBJobInfo : public DBJobInfo
{
public:
    DatesDBJobInfo();

    QDate startDate;
    QDate endDate;
};

}
#endif // DBJOBINFO_H
