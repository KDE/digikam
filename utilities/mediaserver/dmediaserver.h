/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : a media server to export collections through DLNA.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DMEDIA_SERVER_H
#define DMEDIA_SERVER_H

// Qt includes

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QList>

namespace Digikam
{

typedef QMap<QString, QList<QUrl> > MediaServerMap;
    
class DMediaServer : public QObject
{
    Q_OBJECT

public:

    explicit DMediaServer(QObject* const parent = 0);
    virtual ~DMediaServer();

    void addImagesOnServer(const QList<QUrl>&);
    void addAlbumsOnServer(const MediaServerMap&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DMEDIA_SERVER_H
