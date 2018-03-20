/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-07
 * Description : thumbnails size interface
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbnailsize.h"

namespace Digikam
{

static const QString s_configUseLargeThumbsEntry(QLatin1String("Use Large Thumbs"));
static bool          s_useLargeThumbs = false;

ThumbnailSize::ThumbnailSize()
{
    m_Size = Medium;
}

ThumbnailSize::ThumbnailSize(int size)
{
    m_Size = size;
}

ThumbnailSize::ThumbnailSize(const ThumbnailSize& thumbsize)
{
    m_Size = thumbsize.m_Size;
}

ThumbnailSize::~ThumbnailSize()
{
}

ThumbnailSize& ThumbnailSize::operator=(const ThumbnailSize& thumbsize)
{
    m_Size = thumbsize.m_Size;
    return *this;
}

bool ThumbnailSize::operator==(const ThumbnailSize& thumbsize) const
{
    return m_Size == thumbsize.m_Size;
}

bool ThumbnailSize::operator!=(const ThumbnailSize& thumbsize) const
{
    return m_Size != thumbsize.m_Size;
}

int ThumbnailSize::size() const
{
    return m_Size;
}

// -- Static methods ---------------------------------------------------------

void ThumbnailSize::setUseLargeThumbs(bool val)
{
    s_useLargeThumbs = val;
}

bool ThumbnailSize::getUseLargeThumbs()
{
    return s_useLargeThumbs;
}

void ThumbnailSize::readSettings(const KConfigGroup& group)
{
    setUseLargeThumbs(group.readEntry(s_configUseLargeThumbsEntry, false));
}

void ThumbnailSize::saveSettings(KConfigGroup& group, bool val)
{
    group.writeEntry(s_configUseLargeThumbsEntry, val);
}

int ThumbnailSize::maxThumbsSize()
{
    if (s_useLargeThumbs)
        return ThumbnailSize::HD;

    return ThumbnailSize::Huge;
}

}  // namespace Digikam
