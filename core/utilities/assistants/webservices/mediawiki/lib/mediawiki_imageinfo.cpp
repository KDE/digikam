/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Ludovic Delfau <ludovicdelfau at gmail dot com>
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

#include "mediawiki_imageinfo.h"

// C++ includes

#include <algorithm>

namespace mediawiki
{

class Q_DECL_HIDDEN Imageinfo::Private
{
public:

    QDateTime                timestamp;
    QString                  user;
    QString                  comment;
    QUrl                     url;
    QUrl                     descriptionUrl;
    QUrl                     thumbUrl;
    qint64                   thumbWidth;
    qint64                   thumbHeight;
    qint64                   size;
    qint64                   width;
    qint64                   height;
    QString                  sha1;
    QString                  mime;
    QHash<QString, QVariant> metadata;
};

Imageinfo::Imageinfo()
    : d(new Private())
{
    d->thumbWidth  = -1;
    d->thumbHeight = -1;
    d->size        = -1;
    d->width       = -1;
    d->height      = -1;
}

Imageinfo::Imageinfo(const Imageinfo& other)
    : d(new Private(*(other.d)))
{}

Imageinfo::~Imageinfo()
{
    delete d;
}

Imageinfo& Imageinfo::operator=(Imageinfo other)
{
    *d = *other.d;
    return *this;
}

bool Imageinfo::operator==(const Imageinfo& other) const
{
    return timestamp()      == other.timestamp()      &&
           user()           == other.user()           &&
           comment()        == other.comment()        &&
           url()            == other.url()            &&
           descriptionUrl() == other.descriptionUrl() &&
           thumbUrl()       == other.thumbUrl()       &&
           thumbWidth()     == other.thumbWidth()     &&
           thumbHeight()    == other.thumbHeight()    &&
           size()           == other.size()           &&
           width()          == other.width()          &&
           height()         == other.height()         &&
           sha1()           == other.sha1()           &&
           mime()           == other.mime()           &&
           metadata()       == other.metadata();
}

QDateTime Imageinfo::timestamp() const
{
    return d->timestamp;
}

void Imageinfo::setTimestamp(const QDateTime& timestamp)
{
    d->timestamp = timestamp;
}

QString Imageinfo::user() const
{
    return d->user;
}

void Imageinfo::setUser(const QString& user)
{
    d->user = user;
}

QString Imageinfo::comment() const
{
    return d->comment;
}

void Imageinfo::setComment(const QString& comment)
{
    d->comment = comment;
}

QUrl Imageinfo::url() const
{
    return d->url;
}

void Imageinfo::setUrl(const QUrl& url)
{
    d->url = url;
}

QUrl Imageinfo::descriptionUrl() const
{
    return d->url;
}

void Imageinfo::setDescriptionUrl(const QUrl& descriptionUrl)
{
    d->descriptionUrl = descriptionUrl;
}

QUrl Imageinfo::thumbUrl() const
{
    return d->thumbUrl;
}

void Imageinfo::setThumbUrl(const QUrl& thumbUrl)
{
    d->thumbUrl = thumbUrl;
}

qint64 Imageinfo::thumbWidth() const
{
    return d->thumbWidth;
}

void Imageinfo::setThumbWidth(qint64 thumbWidth)
{
    d->thumbWidth = thumbWidth;
}

qint64 Imageinfo::thumbHeight() const
{
    return d->thumbHeight;
}

void Imageinfo::setThumbHeight(qint64 thumbHeight)
{
    d->thumbHeight = thumbHeight;
}

qint64 Imageinfo::size() const
{
    return d->size;
}

void Imageinfo::setSize(qint64 size)
{
    d->size = size;
}

qint64 Imageinfo::width() const
{
    return d->width;
}

void Imageinfo::setWidth(qint64 width)
{
    d->width = width;
}

qint64 Imageinfo::height() const
{
    return d->height;
}

void Imageinfo::setHeight(qint64 height)
{
    d->height = height;
}

QString Imageinfo::sha1() const
{
    return d->sha1;
}

void Imageinfo::setSha1(const QString& sha1)
{
    d->sha1 = sha1;
}

QString Imageinfo::mime() const
{
    return d->mime;
}

void Imageinfo::setMime(const QString& mime)
{
    d->mime = mime;
}

const QHash<QString, QVariant>& Imageinfo::metadata() const
{
    return d->metadata;
}

QHash<QString, QVariant>& Imageinfo::metadata()
{
    return d->metadata;
}

void Imageinfo::setMetadata(const QHash<QString, QVariant>& metadata)
{
     d->metadata = metadata;
}

} // namespace mediawiki
