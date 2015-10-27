/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
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

// Qt includes

#include <QMimeType>
#include <QMimeDatabase>

// Local includes

#include "camiteminfo.h"
#include "filter.h"

namespace Digikam
{

Filter::Filter()
    : onlyNew(false)
{
}

Filter::~Filter()
{
}

QString Filter::toString()
{
    return QString::fromUtf8("%1|%2|%3|%4|%5")
           .arg(name)
           .arg(onlyNew ? QLatin1String("true") : QLatin1String("false"))
           .arg(fileFilter.join(QLatin1String(";")))
           .arg(pathFilter.join(QLatin1String(";")))
           .arg(mimeFilter);
}

void Filter::fromString(const QString& filter)
{
    QStringList s = filter.split(QLatin1Char('|'));
    name          = s.value(0);
    onlyNew       = (s.value(1) == QLatin1String("true"));

    if (!s.value(2).isEmpty())
    {
        fileFilter = s.value(2).split(QLatin1Char(';'));
    }

    if (!s.value(3).isEmpty())
    {
        pathFilter = s.value(3).split(QLatin1Char(';'));
    }

    if (!s.value(4).isEmpty())
    {
        mimeFilter = s.value(4);
    }
}

const QRegExp& Filter::regexp(const QString& wildcard)
{
    if (!filterHash.contains(wildcard))
    {
        QRegExp rx(wildcard.toLower());
        rx.setPatternSyntax(QRegExp::Wildcard);
        filterHash[wildcard] = rx;
    }

    return filterHash[wildcard];
}

bool Filter::match(const QStringList& wildcards, const QString& name)
{
    bool match = false;

    foreach(const QString& wildcard, wildcards)
    {
        match = regexp(wildcard).exactMatch(name);
        //qCDebug(DIGIKAM_IMPORTUI_LOG) << "**" << wildcard << name << match;

        if (match)
        {
            break;
        }
    }

    return match;
}

const QStringList& Filter::mimeWildcards(const QString& mime)
{
    if (!mimeHash.contains(mime))
    {
        QStringList& wc  = mimeHash[mime];
        QStringList list = mime.split(QLatin1Char(';'));

        foreach(const QString& m, list)
        {
            QMimeType mime = QMimeDatabase().mimeTypeForName(m);

            if (!mime.isValid())
            {
                continue;
            }

            foreach(const QString& pattern, mime.globPatterns())
            {
                wc.append(pattern);
            }
        }
    }

    return mimeHash[mime];
}

bool Filter::matchesCurrentFilter(const CamItemInfo& item)
{
    if (onlyNew)
    {
        if (item.downloaded == CamItemInfo::DownloadedYes)
        {
            return false;
        }
    }

    QString folder = item.folder.toLower();
    QString name   = item.name.toLower();

    if (!fileFilter.isEmpty())
    {
        if (!match(fileFilter, name))
        {
            return false;
        }
    }

    if (!pathFilter.isEmpty())
    {
        if (!match(pathFilter, folder))
        {
            return false;
        }
    }

    if (!mimeFilter.isEmpty())
    {
        if (!match(mimeWildcards(mimeFilter), name))
        {
            return false;
        }
    }

    return true;
}

} // namespace Digikam
