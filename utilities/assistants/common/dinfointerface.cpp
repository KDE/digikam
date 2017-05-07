/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : template interface to image informations.
 *               This class do not depend of digiKam database library
 *               to permit to re-use tools on Showfoto.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dinfointerface.h"

namespace Digikam
{

DInfoInterface::DInfoInterface(QObject* const parent)
    : QObject(parent)
{
}

DInfoInterface::~DInfoInterface()
{
}

QList<QUrl> DInfoInterface::currentSelectedItems() const
{
    return QList<QUrl>();
}

QList<QUrl> DInfoInterface::currentAlbumItems() const
{
    return QList<QUrl>();
}

QList<QUrl> DInfoInterface::allAlbumItems() const
{
    return QList<QUrl>();
}

int DInfoInterface::currentAlbum() const
{
    return 0;
}

QList<QUrl> DInfoInterface::albumItems(int) const
{
    return QList<QUrl>();
}

DInfoInterface::DInfoMap DInfoInterface::albumInfo(int) const
{
    return DInfoMap();
}

DInfoInterface::DInfoMap DInfoInterface::itemInfo(const QUrl&) const
{
    return DInfoMap();
}

QList<QUrl> DInfoInterface::albumsItems(const DAlbumIDs&) const
{
    return QList<QUrl>();
}

QWidget* DInfoInterface::albumChooser(QWidget* const) const
{
    return 0;
}

DInfoInterface::DAlbumIDs DInfoInterface::albumChooserItems() const
{
    return DAlbumIDs();
}

// -----------------------------------------------------------------

DItemInfo::DItemInfo(const DInfoInterface::DInfoMap& info)
    : m_info(info)
{
}

DItemInfo::~DItemInfo()
{
}

QString DItemInfo::name() const
{
    QString ret;
    DInfoInterface::DInfoMap::const_iterator it = m_info.find(QLatin1String("name"));

    if (it != m_info.end())
    {
        ret = it.value().toString();
    }

    return ret;
}

QString DItemInfo::comment() const
{
    QString ret;
    DInfoInterface::DInfoMap::const_iterator it = m_info.find(QLatin1String("comment"));

    if (it != m_info.end())
    {
        ret = it.value().toString();
    }

    return ret;
}

int DItemInfo::orientation() const
{
    int ret;
    DInfoInterface::DInfoMap::const_iterator it = m_info.find(QLatin1String("orientation"));

    if (it != m_info.end())
    {
        ret = it.value().toInt();
    }

    return ret;
}

QDateTime DItemInfo::dateTime() const
{
    QDateTime ret;
    DInfoInterface::DInfoMap::const_iterator it = m_info.find(QLatin1String("datetime"));

    if (it != m_info.end())
    {
        ret = it.value().toDateTime();
    }

    return ret;
}

// -----------------------------------------------------------------

DAlbumInfo::DAlbumInfo(const DInfoInterface::DInfoMap& info)
    : m_info(info)
{
}

DAlbumInfo::~DAlbumInfo()
{
}

QString DAlbumInfo::title() const
{
    QString ret;
    DInfoInterface::DInfoMap::const_iterator it = m_info.find(QLatin1String("title"));

    if (it != m_info.end())
    {
        ret = it.value().toString();
    }

    return ret;
}

QString DAlbumInfo::caption() const
{
    QString ret;
    DInfoInterface::DInfoMap::const_iterator it = m_info.find(QLatin1String("comment"));

    if (it != m_info.end())
    {
        ret = it.value().toString();
    }

    return ret;
}

QDate DAlbumInfo::date() const
{
    QDate ret;
    DInfoInterface::DInfoMap::const_iterator it = m_info.find(QLatin1String("date"));

    if (it != m_info.end())
    {
        ret = it.value().toDate();
    }

    return ret;
}

}  // namespace Digikam
