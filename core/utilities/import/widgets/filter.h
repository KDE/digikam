/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Filter for filter combobox
 *
 * Copyright (C) 2010-2011 by Petri Damstén <petri dot damsten at iki dot fi>
 * Copyright (C) 2014      by Teemu Rytilahti <tpr@iki.fi>
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

#ifndef DIGIKAM_FILTER_H
#define DIGIKAM_FILTER_H

// Qt includes

#include <QHash>
#include <QString>
#include <QList>
#include <QStringList>
#include <QRegExp>

// Local includes

#include "camiteminfo.h"

namespace Digikam
{

class Filter
{
public:

    explicit Filter();
    ~Filter();

    QString toString();
    void    fromString(const QString& filter);
    bool    match(const QStringList& wildcards, const QString& name);
    const   QRegExp& regexp(const QString& wildcard);
    const   QStringList& mimeWildcards(const QString& mime);
    bool    matchesCurrentFilter(const CamItemInfo& item);

public:

    QString                     name;
    bool                        onlyNew;
    QStringList                 fileFilter;
    QStringList                 pathFilter;
    QString                     mimeFilter;
    QHash<QString, QRegExp>     filterHash;
    QHash<QString, QStringList> mimeHash;
};

typedef QList<Filter*> FilterList;

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::Filter*)

#endif // DIGIKAM_FILTER_H
