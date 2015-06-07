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

    DBJobInfo();
    DBJobInfo(const DatabaseUrl &dbUrl);

    Type        type();
    DatabaseUrl databaseUrl();

    bool        folders;
    bool        listAvailableImagesOnly;
    bool        recursive;

private:
    Type        jobType;
    DatabaseUrl dbUrl;
};

// ---------------------------------------------

class TagsDBJobInfo : public DBJobInfo
{
public:
    TagsDBJobInfo(const DatabaseUrl &url);

    bool    faceFolders;
    QString specialTag;
};

// ---------------------------------------------

class GPSDBJobInfo : public DBJobInfo
{
public:
    GPSDBJobInfo(const DatabaseUrl &url);

    bool wantDirectQuery;
};

// ---------------------------------------------

class SearchDBJobInfo : public DBJobInfo
{
public:
    SearchDBJobInfo(const DatabaseUrl &url);

    bool       duplicates;
    double     threshold;
    QList<int> albumIds;
    QList<int> tagIds;
};

}
#endif // DBJOBINFO_H
