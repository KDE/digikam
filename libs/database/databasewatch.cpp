/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Keeping image properties in sync.
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QMetaType>

// Local includes

#include "databasewatch.h"
#include "databasewatch.moc"

namespace Digikam
{

DatabaseWatch::DatabaseWatch()
{
    qRegisterMetaType<ImageChangeset>("ImageChangeset");
    qRegisterMetaType<ImageTagChangeset>("ImageTagChangeset");
}

DatabaseWatch::~DatabaseWatch()
{
}

void DatabaseWatch::sendImageChange(ImageChangeset changeset)
{
    emit imageChange(changeset);
}

void DatabaseWatch::sendImageTagChange(ImageTagChangeset changeset)
{
    emit imageTagChange(changeset);
}

void DatabaseWatch::sendCollectionImageChange(CollectionImageChangeset changeset)
{
    emit collectionImageChange(changeset);
}

void DatabaseWatch::sendAlbumChange(AlbumChangeset changeset)
{
    emit albumChange(changeset);
}

void DatabaseWatch::sendTagChange(TagChangeset changeset)
{
    emit tagChange(changeset);
}

void DatabaseWatch::sendAlbumRootChange(AlbumRootChangeset changeset)
{
}

void DatabaseWatch::sendSearchChange(SearchChangeset changeset)
{
    emit searchChange(changeset);
}




} // namespace Digikam
