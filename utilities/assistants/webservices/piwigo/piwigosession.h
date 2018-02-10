/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * Copyright (C) 2010-2014 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PIWIGO_SESSION_H
#define PIWIGO_SESSION_H

// Qt includes

#include <QString>

namespace Digikam
{

class PiwigoSession
{

public:

    explicit PiwigoSession();
    ~PiwigoSession();

public:

    QString url()      const;
    QString username() const;
    QString password() const;

    void setUrl(const QString& url);
    void setUsername(const QString& username);
    void setPassword(const QString& password);

public:

    void save();

private:

    void load();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PIWIGO_SESSION_H
