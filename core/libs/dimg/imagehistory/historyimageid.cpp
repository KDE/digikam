/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class holding properties of referenced files used in non-dest. editing
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QFileInfo>
#include <QUrl>

namespace Digikam
{

HistoryImageId::HistoryImageId()
    : m_type(InvalidType), m_fileSize(0)
{
}

HistoryImageId::HistoryImageId(const QString& uuid, Type type)
    : m_type(type), m_uuid(uuid), m_fileSize(0)
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
    QUrl url = QUrl::fromLocalFile(filePath);
    m_filePath = url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile() + QLatin1Char('/');
}

void HistoryImageId::setPath(const QString& path)
{
    m_filePath = path;

    if (!m_filePath.endsWith(QLatin1Char('/')))
    {
        m_filePath += QLatin1Char('/');
    }
}

void HistoryImageId::setUniqueHash(const QString& uniqueHash, qlonglong fileSize)
{
    m_uniqueHash = uniqueHash;
    m_fileSize   = fileSize;
}

bool HistoryImageId::isValid() const
{
    return (m_type != InvalidType)
           && (!m_uuid.isEmpty() || !m_fileName.isEmpty());
}

HistoryImageId::Type HistoryImageId::type() const
{
    return m_type;
}

QString HistoryImageId::path() const
{
    return m_filePath;
}

QString HistoryImageId::filePath() const
{
    return m_filePath + m_fileName;
}

bool HistoryImageId::hasFileOnDisk() const
{
    return !m_filePath.isEmpty() && !m_fileName.isEmpty();
}

bool HistoryImageId::hasFileName() const
{
    return !m_fileName.isEmpty();
}

QString HistoryImageId::fileName() const
{
    return m_fileName;
}

bool HistoryImageId::hasUuid() const
{
    return !m_uuid.isEmpty();
}

QString HistoryImageId::uuid() const
{
    return m_uuid;
}

bool HistoryImageId::hasCreationDate() const
{
    return m_creationDate.isValid();
}

QDateTime HistoryImageId::creationDate() const
{
    return m_creationDate;
}

bool HistoryImageId::hasUniqueHashIdentifier() const
{
    return !m_uniqueHash.isEmpty() && m_fileSize;
}

QString HistoryImageId::uniqueHash() const
{
    return m_uniqueHash;
}

qlonglong HistoryImageId::fileSize() const
{
    return m_fileSize;
}

QString HistoryImageId::originalUuid() const
{
    return m_originalUUID;
}

bool HistoryImageId::operator==(const HistoryImageId& other) const
{
    return m_uuid         == other.m_uuid           &&
           m_type         == other.m_type           &&
           m_fileName     == other.m_fileName       &&
           m_filePath     == other.m_filePath       &&
           m_creationDate == other.m_creationDate   &&
           m_uniqueHash   == other.m_uniqueHash     &&
           m_fileSize     == other.m_fileSize       &&
           m_originalUUID == other.m_originalUUID;
}

} // namespace Digikam
