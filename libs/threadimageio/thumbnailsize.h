/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-07
 * Description : thumbnails size interface
 *
 * Copyright (C) 2004 by Renchi Raju <renchi dot raju at gmail dot com>
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

#ifndef THUMBNAILSIZE_H
#define THUMBNAILSIZE_H

namespace Digikam
{

class ThumbnailSize
{

public:

    enum Size
    {
        Step      = 8,
        Tiny      = 32,
        VerySmall = 64,
        Small     = 80,
        Medium    = 96,
        Large     = 160,
        Huge      = 256
    };

    ThumbnailSize()
    {
        m_Size = Medium;
    }

    ThumbnailSize(int size)
    {
        m_Size = size;
    }

    ThumbnailSize(const ThumbnailSize& thumbsize)
    {
        m_Size = thumbsize.m_Size;
    }

    ~ThumbnailSize()
    {
    }

    ThumbnailSize& operator=(const ThumbnailSize& thumbsize)
    {
        m_Size = thumbsize.m_Size;
        return *this;
    }

    bool operator==(const ThumbnailSize& thumbsize) const
    {
        return m_Size == thumbsize.m_Size;
    }

    bool operator!=(const ThumbnailSize& thumbsize) const
    {
        return m_Size != thumbsize.m_Size;
    }

    int size() const
    {
        return m_Size;
    }

private:

    int m_Size;
};

}  // namespace Digikam

#endif // THUMBNAILSIZE_H
