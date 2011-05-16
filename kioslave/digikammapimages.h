/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-20
 * Description : a kio-slave to process map search
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DIGIKAMMAPIMAGES_H
#define DIGIKAMMAPIMAGES_H

// KDE includes

#include <kio/slavebase.h>

class kio_digikammapimages : public KIO::SlaveBase
{

public:

    kio_digikammapimages(const QByteArray& pool_socket, const QByteArray& app_socket);
    virtual ~kio_digikammapimages();

    void special(const QByteArray& data);
};

#endif /* DIGIKAMMAPIMAGES_H */
