/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_MEDIAWIKI_JOB_P_H
#define DIGIKAM_MEDIAWIKI_JOB_P_H

#include "mediawiki_iface.h"

namespace MediaWiki
{

class Q_DECL_HIDDEN JobPrivate
{
public:

    explicit JobPrivate(Iface& MediaWiki)
        : MediaWiki(MediaWiki),
          manager(MediaWiki.manager()),
          reply(nullptr)
    {
    }

    Iface&                       MediaWiki;
    QNetworkAccessManager* const manager;
    QNetworkReply*               reply;
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_JOB_P_H
