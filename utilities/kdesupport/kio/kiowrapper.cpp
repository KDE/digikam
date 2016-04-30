/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-10
 * Description : A wrapper to isolate KIO Jobs calls
 *
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2015-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kiowrapper.h"

// Qt includes

#include <QPair>
#include <QPointer>
#include <QMimeDatabase>

// KDE includes

#include <kio_version.h>
#include <krun.h>

namespace Digikam
{

bool KIOWrapper::run(const KService& service, const QList<QUrl>& urls, QWidget* const window)
{
#if KIO_VERSION < QT_VERSION_CHECK(5,6,0)
    return KRun::run(service, urls, window);
#else
    return KRun::runService(service, urls, window);
#endif
}

bool KIOWrapper::run(const QString& exec, const QList<QUrl>& urls, QWidget* const window)
{
    return KRun::run(exec, urls, window);
}

bool KIOWrapper::run(const QUrl& url, QWidget* const window)
{
    return KRun::runUrl(url, QMimeDatabase().mimeTypeForUrl(url).name(), window);
}

} // namespace Digikam
