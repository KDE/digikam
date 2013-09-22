/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-09-18
 * Description : Nepomuk Watcher class that keep tracks of changes in Nepomuk
 *               and apply them into digiKam database
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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
#ifndef NEPOMUKWATCHER_H
#define NEPOMUKWATCHER_H

#include <Nepomuk2/Resource>
#include <Nepomuk2/Types/Property>

class QVariant;
namespace Digikam
{
class DkNepomukService;

class NepomukWatcher : public QObject
{
    Q_OBJECT
public:
    NepomukWatcher(DkNepomukService* parent);
    ~NepomukWatcher();

private Q_SLOTS:
    void slotPropertyAdded(Nepomuk2::Resource res,
                           Nepomuk2::Types::Property prop, QVariant var);
    void slotPropertyRemoved(Nepomuk2::Resource res,
                             Nepomuk2::Types::Property prop, QVariant var);

    void slotResAdded(Nepomuk2::Resource res, QList<QUrl> types);

    void slotResRemoved(QUrl url, QList<QUrl> types);

private:
    class NepomukWatcherPriv;
    NepomukWatcherPriv* d;
};

}
#endif // NEPOMUKWATCHER_H