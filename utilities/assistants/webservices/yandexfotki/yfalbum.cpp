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

#include "yfalbum.h"

namespace Digikam
{

YandexFotkiAlbum::YandexFotkiAlbum()
{
    // nothing
}

YandexFotkiAlbum::YandexFotkiAlbum(const QString& urn,
                                   const QString& author,
                                   const QString& title,
                                   const QString& summary,
                                   const QString& apiEditUrl,
                                   const QString& apiSelfUrl,
                                   const QString& apiPhotosUrl,
                                   const QDateTime& publishedDate,
                                   const QDateTime& editedDate,
                                   const QDateTime& updatedDate,
                                   const QString& password
                                  )
    :   m_urn(urn),
        m_author(author),
        m_title(title),
        m_summary(summary),
        m_apiEditUrl(apiEditUrl),
        m_apiSelfUrl(apiSelfUrl),
        m_apiPhotosUrl(apiPhotosUrl),
        m_publishedDate(publishedDate),
        m_editedDate(editedDate),
        m_updatedDate(updatedDate),
        m_password(password)
{
    // nothing
}

YandexFotkiAlbum::YandexFotkiAlbum(const YandexFotkiAlbum& album)
    :   m_urn(album.urn()),
        m_author(album.author()),
        m_title(album.title()),
        m_summary(album.summary()),
        m_apiEditUrl(album.m_apiEditUrl),
        m_apiSelfUrl(album.m_apiSelfUrl),
        m_apiPhotosUrl(album.m_apiPhotosUrl),
        m_publishedDate(album.publishedDate()),
        m_editedDate(album.editedDate()),
        m_updatedDate(album.updatedDate()),
        m_password(album.m_password)
{
    //nothing
}

/*
YandexFotkiAlbum& YandexFotkiAlbum::operator=(const YandexFotkiAlbum& album) {
    m_urn = album.urn();
    m_author = album.author();
    m_title = album.title();
    m_summary = album.summary();
    m_apiEditUrl = album.m_apiEditUrl;
    m_apiSelfUrl = album.m_apiSelfUrl;
    m_apiPhotosUrl = album.m_apiPhotosUrl;
    m_publishedDate = album.publishedDate();
    m_editedDate = album.editedDate();
    m_updatedDate = album.updatedDate();
    m_password = album.m_password;
    return *this;
}
*/

QDebug operator<<(QDebug d, const YandexFotkiAlbum& a)
{
    d.nospace() << "YandexFotkiAlbum(\n";

    d.space() << "urn:" << a.urn() << ",\n";
    d.space() << "author:" << a.author() << ",\n";
    d.space() << "title:" << a.title() << ",\n";
    d.space() << "summary:" << a.summary() << ",\n";
    d.space() << "apiEditUrl:" << a.m_apiEditUrl << ",\n";
    d.space() << "apiSelfUrl:" << a.m_apiSelfUrl << ",\n";
    d.space() << "apiPhotoUrl:" << a.m_apiPhotosUrl << ",\n";
    d.space() << "publishedDate:" << a.publishedDate() << ",\n";
    d.space() << "editedDate:" << a.editedDate() << ",\n";
    d.space() << "updatedDate:" << a.updatedDate() << ",\n";
    d.space() << "password:" << !a.m_password.isNull() << "" << a.m_password << "\n";

    d.nospace() << ")";
    return d;
}

} // namespace Digikam
