#include "dbjobinfo.h"

namespace Digikam {

DBJobInfo::DBJobInfo()
{
}

DBJobInfo::DBJobInfo(Type jType)
    : jobType(jType)
{
    folders                 = false;
    listAvailableImagesOnly = false;
    recursive               = false;
}

DBJobInfo::Type DBJobInfo::type()
{
    return jobType;
}

// ---------------------------------------------

AlbumsDBJobInfo::AlbumsDBJobInfo()
    : DBJobInfo(Type::AlbumsJob)
{
    albumRootId = -1;
}

// ---------------------------------------------

TagsDBJobInfo::TagsDBJobInfo()
    : DBJobInfo(Type::TagsJob)
{
    faceFolders = false;
}

// ---------------------------------------------

GPSDBJobInfo::GPSDBJobInfo()
    : DBJobInfo(Type::GPSJob)
{
    wantDirectQuery = false;
}

// ---------------------------------------------

SearchesDBJobInfo::SearchesDBJobInfo()
    : DBJobInfo(Type::SearchesJob)
{
    duplicates = false;
    threshold  = 0;
    searchId   = -1;
}

// ---------------------------------------------

DatesDBJobInfo::DatesDBJobInfo()
    : DBJobInfo(Type::DatesJob)
{
}

} // namespace Digikam
