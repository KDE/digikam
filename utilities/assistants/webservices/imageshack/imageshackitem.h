/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#ifndef IMAGESHACK_ITEM_H
#define IMAGESHACK_ITEM_H

namespace Digikam
{

class ImageShackGallery
{

public:

    explicit ImageShackGallery()
    {
    }

    QString m_user;
    QString m_server;
    QString m_name;
    QString m_title;
    QString m_url;
};

// -------------------------------------------------------------------

class ImageShackPhoto
{

public:

    explicit ImageShackPhoto()
    {
    }

    QString m_server;
    QString m_name;
    QString m_bucket;
};

} // namespace Digikam

#endif // IMAGESHACK_ITEM_H
