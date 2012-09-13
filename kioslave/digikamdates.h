/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a kio-slave to process date query on
 *               digiKam albums.
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

#ifndef DIGIKAMDATES_H
#define DIGIKAMDATES_H

// KDE includes

#include <kio/slavebase.h>

class kio_digikamdates : public KIO::SlaveBase
{

public:

    kio_digikamdates(const QByteArray& pool_socket, const QByteArray& app_socket);
    virtual ~kio_digikamdates();

    void special(const QByteArray& data);
};

#endif /* DIGIKAMDATES_H */
