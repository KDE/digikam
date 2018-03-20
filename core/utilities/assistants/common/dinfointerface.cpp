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
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QtGlobal>

// Local includes

#include "metaengine.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

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

bool DInfoInterface::supportAlbums() const
{
    return false;
}

QWidget* DInfoInterface::uploadWidget(QWidget* const) const
{
    return 0;
}

QUrl DInfoInterface::uploadUrl() const
{
    return QUrl();
}

// -----------------------------------------------------------------

DItemInfo::DItemInfo(const DInfoInterface::DInfoMap& info)
    : m_info(info)
{
    //qCDebug(DIGIKAM_GENERAL_LOG) << m_info;
}

DItemInfo::~DItemInfo()
{
}

QVariant DItemInfo::parseInfoMap(const QString& key) const
{
    QVariant ret;
    DInfoInterface::DInfoMap::const_iterator it = m_info.find(key);

    if (it != m_info.end())
    {
        ret = it.value();
    }

    return ret;
}

QString DItemInfo::name() const
{
    QVariant val = parseInfoMap(QLatin1String("name"));
    return !val.isNull() ? val.toString() : QString();
}

QString DItemInfo::comment() const
{
    QVariant val = parseInfoMap(QLatin1String("comment"));
    return !val.isNull() ? val.toString() : QString();
}

QString DItemInfo::title() const
{
    QVariant val = parseInfoMap(QLatin1String("title"));
    return !val.isNull() ? val.toString() : QString();
}

int DItemInfo::orientation() const
{
    QVariant val = parseInfoMap(QLatin1String("orientation"));
    return !val.isNull() ? val.toInt() : MetaEngine::ORIENTATION_UNSPECIFIED;
}

QSize DItemInfo::dimensions() const
{
    QVariant val = parseInfoMap(QLatin1String("dimensions"));
    return !val.isNull() ? val.toSize() : QSize();
}

QDateTime DItemInfo::dateTime() const
{
    QVariant val = parseInfoMap(QLatin1String("datetime"));
    return !val.isNull() ? val.toDateTime() : QDateTime();
}

QStringList DItemInfo::keywords() const
{
    QVariant val = parseInfoMap(QLatin1String("keywords"));
    return !val.isNull() ? val.toStringList() : QStringList();
}

QStringList DItemInfo::tagsPath() const
{
    QVariant val = parseInfoMap(QLatin1String("tagspath"));
    return !val.isNull() ? val.toStringList() : QStringList();
}

int DItemInfo::rating() const
{
    QVariant val = parseInfoMap(QLatin1String("rating"));
    return !val.isNull() ? val.toInt() : RatingMin;
}

int DItemInfo::colorLabel() const
{
    QVariant val = parseInfoMap(QLatin1String("colorlabel"));
    return !val.isNull() ? val.toInt() : NoColorLabel;
}

int DItemInfo::pickLabel() const
{
    QVariant val = parseInfoMap(QLatin1String("picklabel"));
    return !val.isNull() ? val.toInt() : NoPickLabel;
}

double DItemInfo::latitude() const
{
    QVariant val = parseInfoMap(QLatin1String("latitude"));
    return !val.isNull() ? val.toDouble() : qQNaN();
}

double DItemInfo::longitude() const
{
    QVariant val = parseInfoMap(QLatin1String("longitude"));
    return !val.isNull() ? val.toDouble() : qQNaN();
}

double DItemInfo::altitude() const
{
    QVariant val = parseInfoMap(QLatin1String("altitude"));
    return !val.isNull() ? val.toDouble() : qQNaN();
}

qlonglong DItemInfo::fileSize() const
{
    QVariant val = parseInfoMap(QLatin1String("filesize"));
    return !val.isNull() ? val.toLongLong() : 0;
}

QStringList DItemInfo::creators() const
{
    QVariant val = parseInfoMap(QLatin1String("creators"));
    return !val.isNull() ? val.toStringList() : QStringList();
}

QString DItemInfo::credit() const
{
    QVariant val = parseInfoMap(QLatin1String("credit"));
    return !val.isNull() ? val.toString() : QString();
}

QString DItemInfo::rights() const
{
    QVariant val = parseInfoMap(QLatin1String("rights"));
    return !val.isNull() ? val.toString() : QString();
}

QString DItemInfo::source() const
{
    QVariant val = parseInfoMap(QLatin1String("source"));
    return !val.isNull() ? val.toString() : QString();
}

QString DItemInfo::exposureTime() const
{
    QVariant val = parseInfoMap(QLatin1String("exposuretime"));
    return !val.isNull() ? val.toString() : QString();
}

QString DItemInfo::sensitivity() const
{
    QVariant val = parseInfoMap(QLatin1String("sensitivity"));
    return !val.isNull() ? val.toString() : QString();
}

QString DItemInfo::aperture() const
{
    QVariant val = parseInfoMap(QLatin1String("aperture"));
    return !val.isNull() ? val.toString() : QString();
}

QString DItemInfo::focalLength() const
{
    QVariant val = parseInfoMap(QLatin1String("focallength"));
    return !val.isNull() ? val.toString() : QString();
}

bool DItemInfo::hasGeolocationInfo() const
{
    // NOTE: GPS position without altitude is a valid geolocation.
    return (!qIsNaN(latitude()) && !qIsNaN(longitude()));
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

QString DAlbumInfo::path() const
{
    QString ret;
    DInfoInterface::DInfoMap::const_iterator it = m_info.find(QLatin1String("path"));

    if (it != m_info.end())
    {
        ret = it.value().toString();
    }

    return ret;
}

} // namespace Digikam
