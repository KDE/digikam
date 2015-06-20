/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-05
 * Description : DB Jobs Info
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dbjobinfo.h"

namespace Digikam
{

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
