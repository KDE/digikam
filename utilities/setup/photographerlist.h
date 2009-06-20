/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : Photographers list container.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PHOTOGRAPHERLIST_H
#define PHOTOGRAPHERLIST_H

// Qt includes

#include <QList>
#include <QObject>

class QString;
class QDateTime;

class KAction;

namespace Digikam
{

class Photographer;
class PhotographerListPrivate;

class PhotographerList : public QObject
{
    Q_OBJECT

public:

    PhotographerList(QObject *parent, const QString& file);
    ~PhotographerList();

    bool load();
    bool save();
    void clear();

    void insert(Photographer* photographer);
    void remove(Photographer* photographer);

    Photographer* find(const QString& author) const;

    QList<Photographer*>* photographerList();

    static PhotographerList* defaultList();

Q_SIGNALS:

    void signalPhotographerAdded(Photographer*);
    void signalPhotographerRemoved(Photographer*);

private:

    void insertPrivate(Photographer* photographer);
    void removePrivate(Photographer* photographer);

private:

    static PhotographerList *m_defaultList;
    PhotographerListPrivate* const d;
};

}  // namespace Digikam

#endif /* PHOTOGRAPHERLIST_H */
