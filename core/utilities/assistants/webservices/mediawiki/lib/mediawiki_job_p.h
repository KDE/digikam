/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MEDIAWIKI_JOB_P_H
#define MEDIAWIKI_JOB_P_H

#include "mediawiki_iface.h"

namespace mediawiki
{

class JobPrivate
{
public:

    explicit JobPrivate(Iface& mediawiki)
        : mediawiki(mediawiki),
          manager(mediawiki.manager()),
          reply(0)
    {
    }

    Iface&                   mediawiki;
    QNetworkAccessManager* const manager;
    QNetworkReply*               reply;
};

} // namespace mediawiki

#endif // MEDIAWIKI_JOB_P_H
