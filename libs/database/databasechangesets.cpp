/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-01
 * Description : Recording changes on the database
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

// Local includes

#include "databasechangesets.h"

namespace Digikam
{

ImageChangeset::ImageChangeset()
{
}

ImageChangeset::ImageChangeset(QList<qlonglong> ids, DatabaseFields::Set changes)
    : m_ids(ids), m_changes(changes)
{
}

ImageChangeset::ImageChangeset(qlonglong id, DatabaseFields::Set changes)
    : m_changes(changes)
{
    m_ids << id;
}

QList<qlonglong> ImageChangeset::ids() const
{
    return m_ids;
}

bool ImageChangeset::containsImage(qlonglong id) const
{
    return m_ids.contains(id);
}

DatabaseFields::Set ImageChangeset::changes() const
{
    return m_changes;
}

// ------------------------ //

ImageTagChangeset::ImageTagChangeset()
    : m_operation(Unknown)
{
}

ImageTagChangeset::ImageTagChangeset(QList<qlonglong> ids, QList<int> tags, Operation op)
    : m_ids(ids), m_tags(tags), m_operation(op)
{
}

ImageTagChangeset::ImageTagChangeset(qlonglong id, QList<int> tags, Operation op)
    : m_tags(tags), m_operation(op)
{
    m_ids << id;
}

ImageTagChangeset::ImageTagChangeset(qlonglong id, int tag, Operation op)
    : m_operation(op)
{
    m_ids << id;
    m_tags << tag;
}

ImageTagChangeset &ImageTagChangeset::operator<<(const ImageTagChangeset &other)
{
    if (m_operation != other.m_operation)
        m_operation = Unknown;

    m_ids << other.m_ids;
    m_tags << other.m_tags;

    return *this;
}

QList<qlonglong> ImageTagChangeset::ids() const
{
    return m_ids;
}

bool ImageTagChangeset::containsImage(qlonglong id) const
{
    return m_ids.contains(id);
}

QList<int> ImageTagChangeset::tags() const
{
    return m_tags;
}

bool ImageTagChangeset::containsTag(int id)
{
    return (m_operation == RemovedAll) || m_tags.contains(id);
}

ImageTagChangeset::Operation ImageTagChangeset::operation() const
{
    return m_operation;
}

// ------------------------ //

CollectionImageChangeset::CollectionImageChangeset()
    : m_operation(Unknown)
{
}

CollectionImageChangeset::CollectionImageChangeset(QList<qlonglong> ids, QList<int> albums, Operation op)
    : m_ids(ids), m_albums(albums), m_operation(op)
{
}

CollectionImageChangeset::CollectionImageChangeset(QList<qlonglong> ids, int album, Operation op)
    : m_ids(ids), m_operation(op)
{
    m_albums << album;
}

CollectionImageChangeset::CollectionImageChangeset(qlonglong id, int album, Operation op)
    : m_operation(op)
{
    m_ids << id;
    m_albums << album;
}

CollectionImageChangeset &CollectionImageChangeset::operator<<(const CollectionImageChangeset &other)
{
    if (m_operation != other.m_operation)
        m_operation = Unknown;

    m_ids << other.m_ids;
    m_albums << other.m_albums;

    return *this;
}

QList<qlonglong> CollectionImageChangeset::ids() const
{
    return m_ids;
}

bool CollectionImageChangeset::containsImage(qlonglong id) const
{
    return (m_operation == RemovedAll) || m_ids.contains(id);
}

QList<int> CollectionImageChangeset::albums() const
{
    return m_albums;
}

bool CollectionImageChangeset::containsAlbum(int id)
{
    return m_albums.contains(id);
}

CollectionImageChangeset::Operation CollectionImageChangeset::operation() const
{
    return m_operation;
}

// ------------------------ //

AlbumChangeset::AlbumChangeset()
    : m_id(-1), m_operation(Unknown)
{
}

AlbumChangeset::AlbumChangeset(int albumId, Operation operation)
    : m_id(albumId), m_operation(operation)
{
}

int AlbumChangeset::albumId() const
{
    return m_id;
}

AlbumChangeset::Operation AlbumChangeset::operation() const
{
    return m_operation;
}

// ------------------------ //

TagChangeset::TagChangeset()
    : m_id(-1), m_operation(Unknown)
{
}

TagChangeset::TagChangeset(int albumId, Operation operation)
    : m_id(albumId), m_operation(operation)
{
}

int TagChangeset::tagId() const
{
    return m_id;
}

TagChangeset::Operation TagChangeset::operation() const
{
    return m_operation;
}

// ------------------------ //

AlbumRootChangeset::AlbumRootChangeset()
    : m_id(-1), m_operation(Unknown)
{
}

AlbumRootChangeset::AlbumRootChangeset(int albumId, Operation operation)
    : m_id(albumId), m_operation(operation)
{
}

int AlbumRootChangeset::albumRootId() const
{
    return m_id;
}

AlbumRootChangeset::Operation AlbumRootChangeset::operation() const
{
    return m_operation;
}

// ------------------------ //

SearchChangeset::SearchChangeset()
    : m_id(-1), m_operation(Unknown)
{
}

SearchChangeset::SearchChangeset(int albumId, Operation operation)
    : m_id(albumId), m_operation(operation)
{
}

int SearchChangeset::searchId() const
{
    return m_id;
}

SearchChangeset::Operation SearchChangeset::operation() const
{
    return m_operation;
}


}



