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

#ifndef KIOWRAPPER_H
#define KIOWRAPPER_H

// Qt includes

#include <QObject>
#include <QUrl>
#include <QStringList>
#include <QList>
#include <QPixmap>
#include <QWidget>

// Local includes

#include "digikam_export.h"

class KJob;
class KService;
class KFileItem;

namespace Digikam
{

class DIGIKAM_EXPORT KIOWrapper : public QObject
{
    Q_OBJECT

public:

    KIOWrapper();

    void move(const QUrl& src, const QUrl& dest);
    void del(const QUrl& url);
    void trash(const QUrl& url);

public:

    static bool fileCopy(const QUrl& src, const QUrl& dest, bool withKJobWidget = false, QWidget* const widget = 0);
    static bool fileMove(const QUrl& src, const QUrl& dest);
    static bool mkdir(const QUrl& url, bool withKJobWidget = false, QWidget* const widget = 0);
    static bool rename(const QUrl& oldUrl, const QUrl& newUrl);

    static bool run(const KService& service, const QList<QUrl>& urls, QWidget* const window);
    static bool run(const QString& exec, const QList<QUrl>& urls, QWidget* const window);
    static bool run(const QUrl& url, QWidget* const window);

Q_SIGNALS:

    // Internal use only : used by slotKioJobResult()
    void error(const QString& errMsg);

private Q_SLOTS:

    // Used by move, del, trash methods
    void slotKioJobResult(KJob* job);
};

} // namespace Digikam

#endif // KIOWRAPPER_H
