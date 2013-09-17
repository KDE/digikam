/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-09-17
 * Description : Wrapper class that will provide get/set methods to
 *               communicate with Nepomuk. Since Nepomuk can change it's api
 *               please keep all Nepomuk related code in this class.
 *               DkNepomukService should be as clean as possible.
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include "dknepomukwrap.h"

#include <Nepomuk2/Resource>
#include <Nepomuk2/Variant>

#include <Soprano/Vocabulary/NAO>

// QList<int> metatype defined in Nepomuk/Variant
#define DATABASECHANGESETS_H_NO_QLIST_METATYPE_DECLARATION
#include "albumdb.h"
#include "databaseaccess.h"
#include "tagscache.h"
#include "imageinfo.h"

using namespace Soprano::Vocabulary;

namespace Digikam
{

DkNepomukWrap::DkNepomukWrap()
{

}

Nepomuk2::Tag DkNepomukWrap::digikamToNepomukTag(int tagId)
{
    if (tagId <= 0)
    {
        return Nepomuk2::Tag();
    }

    if (TagsCache::instance()->isInternalTag(tagId))
    {
        return Nepomuk2::Tag();
    }

    QString tagName = TagsCache::instance()->tagName(tagId);

    if (tagName.isEmpty())
    {
        return Nepomuk2::Tag();
    }

    Nepomuk2::Tag tag(tagName);

    if (!tag.exists())
    {
        // from dolphin's panels/information/newtagdialog.cpp
        tag.setLabel(tagName);
        tag.addIdentifier(tagName);

        TagInfo info = DatabaseAccess().db()->getTagInfo(tagId);

        if (!(info.icon.isNull()))
        {

            tag.addSymbol(info.icon);
        }
    }

    return tag;
}

void DkNepomukWrap::renameNepomukTag(QString oldName, QString newName)
{
    Nepomuk2::Tag nTag(oldName);
    Nepomuk2::Variant value( newName );

    nTag.setProperty( NAO::identifier(), value );
    nTag.setProperty( NAO::prefLabel(), value );
}

void DkNepomukWrap::setUnsetTag(Nepomuk2::Resource res, Nepomuk2::Tag tag, bool toSet)
{
    if (toSet)
    {
        res.addTag(tag);
    }
    else
    {
        res.removeProperty(Soprano::Vocabulary::NAO::hasTag(), tag.uri());
    }
}

}
