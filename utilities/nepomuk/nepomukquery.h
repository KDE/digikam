/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-09-24
 * Description : Nepomuk Query class provide Nepomuk Api based implementation
 *               to query for images rating, asigned tags and comments.
 *               It also query Tags.
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

#ifndef DKNEPOMUKQUERY_H
#define DKNEPOMUKQUERY_H

#include <QObject>

namespace Nepomuk2
{
    namespace Query
    {
        class Query;
    }
}

namespace Digikam
{

class DkNepomukService;

class NepomukQuery : public QObject
{
    Q_OBJECT
public:
    NepomukQuery(DkNepomukService* const service);

    void queryImagesProperties();

    void queryTags();

private:

    Nepomuk2::Query::Query getImagePropertiesQuery();
    DkNepomukService* service;
};

} // namespace Digikam

#endif // DKNEPOMUKQUERY_H