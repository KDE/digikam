#include "dbjobinfo.h"

namespace Digikam {

DBJobInfo::DBJobInfo()
{
}

DBJobInfo::DBJobInfo(const DatabaseUrl &url)
{
    folders                 = false;
    listAvailableImagesOnly = false;
    recursive               = false;

    dbUrl = url;

    if (url.isAlbumUrl())
    {
        jobType = Type::AlbumsJob;
    }
    else if (url.isTagUrl())
    {
        jobType = Type::TagsJob;
    }
    else if (url.isDateUrl())
    {
        jobType = Type::DatesJob;
    }
    else if (url.isMapImagesUrl())
    {
        jobType = Type::GPSJob;
    }
}

DBJobInfo::Type DBJobInfo::type()
{
    return jobType;
}

DatabaseUrl DBJobInfo::databaseUrl()
{
    return dbUrl;
}

// ---------------------------------------------

TagsDBJobInfo::TagsDBJobInfo(const DatabaseUrl &url)
    : DBJobInfo(url)
{
    faceFolders = false;
}

// ---------------------------------------------

GPSDBJobInfo::GPSDBJobInfo(const DatabaseUrl &url)
    : DBJobInfo(url)
{
    wantDirectQuery = false;
}

// ---------------------------------------------

SearchDBJobInfo::SearchDBJobInfo(const DatabaseUrl &url)
    : DBJobInfo(url)
{
    duplicates = false;
    threshold  = 0;
}

} // namespace Digikam
