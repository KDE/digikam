/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Ludovic Delfau <ludovicdelfau at gmail dot com>
 * Copyright (C) 2011      by Paolo de Vathaire <paolo dot devathaire at gmail dot com>
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

#include "mediawiki_image.h"

// C++ includes

#include <algorithm>

namespace MediaWiki
{

class Q_DECL_HIDDEN Image::Private
{
public:

    qint64  namespaceId;
    QString title;
};

Image::Image()
    : d(new Private())
{
    d->namespaceId = -1;
}

Image::Image(const Image& other)
    : d(new Private(*(other.d)))
{
}

Image::~Image()
{
    delete d;
}

Image& Image::operator=(Image other)
{
    *d = *other.d;
    return *this;
}

bool Image::operator==(const Image& other) const
{
    return namespaceId() == other.namespaceId() &&
           title()       == other.title();
}

qint64 Image::namespaceId() const
{
    return d->namespaceId;
}

void Image::setNamespaceId(qint64 namespaceId)
{
    d->namespaceId = namespaceId;
}

QString Image::title() const
{
    return d->title;
}

void Image::setTitle(const QString& title)
{
    d->title = title;
}

} // namespace MediaWiki
