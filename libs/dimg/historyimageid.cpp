/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class holding properties of referenced files used in non-dest. editing
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "historyimageid.h"

// Qt includes

#include <QtCore/QFileInfo>

// KDE includes

#include <kurl.h>

namespace Digikam 
{

HistoryImageId::HistoryImageId()
            : m_type(InvalidType), m_fileSize(0)
{
}

HistoryImageId::HistoryImageId(const QString& uuid, Type type)
            : m_type(type), m_uuid(uuid)
{
}

void HistoryImageId::setType(HistoryImageId::Type type)
{
    m_type = type;
}

void HistoryImageId::setUuid(const QString& uuid)
{
    m_uuid = uuid;
}

void HistoryImageId::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}

void HistoryImageId::setCreationDate(const QDateTime& creationDate)
{
    m_creationDate = creationDate;
}

void HistoryImageId::setPathOnDisk(const QString& filePath)
{
    KUrl url = KUrl::fromPath(filePath);
    m_filePath = url.directory(KUrl::ObeyTrailingSlash);
}

void HistoryImageId::setUniqueHash(const QString& uniqueHash, int fileSize)
{
    m_uniqueHash = uniqueHash;
    m_fileSize   = fileSize;
}

bool HistoryImageId::isValid() const
{
    return (m_type != InvalidType)
        && (!m_uuid.isEmpty() || !m_fileName.isEmpty());
}

bool HistoryImageId::operator==(const HistoryImageId& other) const
{
    return m_uuid         == other.m_uuid
       &&  m_type         == other.m_type
       &&  m_fileName     == other.m_fileName
       &&  m_filePath     == other.m_filePath
       &&  m_creationDate == other.m_creationDate
       &&  m_uniqueHash   == other.m_uniqueHash
       &&  m_fileSize     == other.m_fileSize
       &&  m_originalUUID == other.m_originalUUID;
}

} // namespace Digikam
