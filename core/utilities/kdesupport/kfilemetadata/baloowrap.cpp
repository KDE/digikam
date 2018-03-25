/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-08-05
 * Description : KDE file indexer interface.
 *
 * Copyright (C) 2014 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "baloowrap.h"

// Qt includes

#include <QUrl>

// KDE includes

#include <klocalizedstring.h>
#include <kfilemetadata/usermetadata.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class BalooWrap::Private
{
public:

    Private()
    {
        syncFromBalooToDigikam = false;
        syncFromDigikamToBaloo = false;
    }

    bool syncFromDigikamToBaloo;
    bool syncFromBalooToDigikam;
};

QPointer<BalooWrap> BalooWrap::internalPtr = QPointer<BalooWrap>();

BalooWrap::BalooWrap(QObject* const parent)
    : QObject(parent),
      d(new BalooWrap::Private)
{
}

BalooWrap::~BalooWrap()
{
    delete d;
}

bool BalooWrap::isCreated()
{
    return (!internalPtr.isNull());
}

BalooWrap* BalooWrap::instance()
{
    if (BalooWrap::internalPtr.isNull())
    {
        BalooWrap::internalPtr = new BalooWrap();
    }

    return BalooWrap::internalPtr;
}


void BalooWrap::setTags(const QUrl& url, QStringList* const tags)
{
    setAllData(url,tags, NULL, -1);
}

void BalooWrap::setComment(const QUrl& url, QString* const comment)
{
    setAllData(url, NULL, comment, -1);
}

void BalooWrap::setRating(const QUrl& url, int rating)
{
    setAllData(url, NULL, NULL, rating);
}

void BalooWrap::setAllData(const QUrl& url, QStringList* const tags, QString* const comment, int rating)
{
    if (!d->syncFromDigikamToBaloo)
    {
        return;
    }

    KFileMetaData::UserMetaData md(url.toLocalFile());

    if (tags != NULL)
    {
        md.setTags(*tags);
    }

    if (comment != NULL)
    {
        md.setUserComment(*comment);
    }

    if (rating != -1)
    {
        // digiKam store rating as value form 0 to 5
        // while baloo store it as value from 0 to 10
        md.setRating(rating * 2);
    }
}

BalooInfo BalooWrap::getSemanticInfo(const QUrl& url) const
{
    if (!d->syncFromBalooToDigikam)
    {
        return BalooInfo();
    }

    KFileMetaData::UserMetaData md(url.toLocalFile());

    //Baloo::File file = job->file();
    BalooInfo bInfo;

    // Baloo have rating from 0 to 10, while digiKam have only from 0 to 5
    bInfo.rating  = md.rating() / 2;
    bInfo.comment = md.userComment();

    foreach(QString tag, md.tags().toSet())
    {
        bInfo.tags.append(i18n("BalooTags/") + tag);
    }

    return bInfo;
}


void BalooWrap::setSyncToBaloo(bool value)
{
    d->syncFromDigikamToBaloo = value;
}

bool BalooWrap::getSyncToBaloo() const
{
    return d->syncFromDigikamToBaloo;
}

bool BalooWrap::getSyncToDigikam() const
{
    return d->syncFromBalooToDigikam;
}

void BalooWrap::setSyncToDigikam(bool value)
{
    d->syncFromBalooToDigikam = value;
}

} // namespace Digikam
