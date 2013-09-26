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

    /**
     * @brief NepomukWatcher      - Will set up two resourceWatchers,
     *                              one for image properties and other for
     *                              tags.
     */
    NepomukWatcher(DkNepomukService* parent);
    ~NepomukWatcher();

private Q_SLOTS:

    /**
     * @brief slotPropertyAdded   - a tag, a rating or a comment was added
     *                              to Nepomuk image resource
     * @param res                 - Nepomuk image resource
     *
     * @param prop                - property that was added, only NAO::hasTag()
     *                              NAO::numericRating() and NAO::description()
     *                              are watched
     * @param var                 - value of property that was added
     */
    void slotPropertyAdded(Nepomuk2::Resource res,
                           Nepomuk2::Types::Property prop, QVariant var);
    /**
     * @brief slotPropertyRemoved - a tag, a rating or a comment was removed
     *                              from Nepomuk image resource
     * @param res                 - Nepomuk image resource
     *
     * @param prop                - property that was removed, only NAO::hasTag()
     *                              NAO::numericRating() and NAO::description()
     *                              are watched
     * @param var                 - value of property that was added
     */
    void slotPropertyRemoved(Nepomuk2::Resource res,
                             Nepomuk2::Types::Property prop, QVariant var);

    /**
     * @brief slotResAdded         - a tag resource was added to Nepomuk and
     *                               it will be added to digiKam
     * @param res                  - tag resource that was added
     *
     * @param types                - should contain NAO::Tag()
     */
    void slotResAdded(Nepomuk2::Resource res, QList<QUrl> types);

    /**
     * @brief slotResRemoved       - a tag resource was removed to Nepomuk and
     *                               it will be removed to digiKam
     * @param res                  - tag resource that was removed
     *
     * @param types                - should contain NAO::Tag()
     */
    void slotResRemoved(QUrl url, QList<QUrl> types);

private:
    class NepomukWatcherPriv;
    NepomukWatcherPriv* d;
};

}
#endif // NEPOMUKWATCHER_H