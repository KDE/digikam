/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-18
 * Description : a tool to export items to YandexFotki web service
 *
 * Copyright (C) 2010      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "yfphoto.h"

// Qt includes

#ifndef KDE_NO_DEBUG_OUTPUT
#include <QDebug>
#endif // KDE_NO_DEBUG_OUTPUT

namespace Digikam
{

YFPhoto::YFPhoto(const QString& urn,
                 const QString& author,
                 const QString& title,
                 const QString& summary,
                 const QString& apiEditUrl,
                 const QString& apiSelfUrl,
                 const QString& apiMediaUrl,
                 const QString& apiAlbumUrl,
                 const QDateTime& publishedDate,
                 const QDateTime& editedDate,
                 const QDateTime& updatedDate,
                 const QDateTime& createdDate,
                 Access access,
                 bool hideOriginal,
                 bool disableComments,
                 bool adult,
                 const QString& remoteUrl)
    : m_urn(urn),
      m_author(author),
      m_title(title),
      m_summary(summary),
      m_apiEditUrl(apiEditUrl),
      m_apiSelfUrl(apiSelfUrl),
      m_apiMediaUrl(apiMediaUrl),
      m_apiAlbumUrl(apiAlbumUrl),
      m_publishedDate(publishedDate),
      m_editedDate(editedDate),
      m_updatedDate(updatedDate),
      m_createdDate(createdDate),
      m_access(access),
      m_hideOriginal(hideOriginal),
      m_disableComments(disableComments),
      m_adult(adult),
      m_remoteUrl(remoteUrl)
{
    // nothing
}

YFPhoto::YFPhoto(const YFPhoto& photo)
    : m_urn(photo.urn()),
      m_author(photo.author()),
      m_title(photo.title()),
      m_summary(photo.summary()),
      m_apiEditUrl(photo.m_apiEditUrl),
      m_apiSelfUrl(photo.m_apiSelfUrl),
      m_apiMediaUrl(photo.m_apiMediaUrl),
      m_apiAlbumUrl(photo.m_apiAlbumUrl),
      m_publishedDate(photo.publishedDate()),
      m_editedDate(photo.editedDate()),
      m_updatedDate(photo.updatedDate()),
      m_createdDate(photo.createdDate()),
      m_access(photo.access()),
      m_hideOriginal(photo.isHideOriginal()),
      m_disableComments(photo.isDisableComments()),
      m_adult(photo.isAdult()),
      m_remoteUrl(photo.remoteUrl()),
      m_localUrl(photo.localUrl()),
      m_originalUrl(photo.originalUrl())
{
    //nothing
}

#ifndef KDE_NO_DEBUG_OUTPUT
QDebug operator<<(QDebug d, const YFPhoto& p)
{
    d.nospace() << "YFPhoto(\n";

    d.space() << "urn:" << p.urn() << ",\n";
    d.space() << "author:" << p.author() << ",\n";
    d.space() << "title:" << p.title() << ",\n";
    d.space() << "summary:" << p.summary() << ",\n";
    d.space() << "apiEditUrl:" << p.m_apiEditUrl << ",\n";
    d.space() << "apiSelfUrl:" << p.m_apiSelfUrl << ",\n";
    d.space() << "apiMediaUrl:" << p.m_apiMediaUrl << ",\n";
    d.space() << "apiAlbumUrl:" << p.m_apiAlbumUrl << ",\n";
    d.space() << "publishedDate:" << p.publishedDate() << ",\n";
    d.space() << "editedDate:" << p.editedDate() << ",\n";
    d.space() << "updatedDate:" << p.updatedDate() << ",\n";
    d.space() << "createdDate:" << p.createdDate() << ",\n";
    d.space() << "access:" << p.access() << ",\n";
    d.space() << "hideOriginal:" << p.isHideOriginal() << ",\n";
    d.space() << "disableComments:" << p.isDisableComments() << ",\n";
    d.space() << "adult:" << p.isAdult() << ",\n";
    d.space() << "remoteUrl:" << p.remoteUrl() << ",\n";
    d.space() << "localUrl:" << p.localUrl() << ",\n";
    d.space() << "originalUrl:" << p.originalUrl() << ",\n";
    d.space() << "tags:" << "\n";

    foreach(const QString& t, p.tags)
    {
        d.space() << t << ",";
    }

    d.space() << "\n";

    d.nospace() << ")";
    return d;
}
#endif // KDE_NO_DEBUG_OUTPUT

} // namespace Digikam
