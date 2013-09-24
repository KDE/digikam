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

#include "nepomukquery.h"
#include "digikamnepomukservice.h"

#include <kdebug.h>

#include <Nepomuk2/Query/ResourceTypeTerm>
#include <Nepomuk2/Query/ComparisonTerm>
#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Query/OrTerm>
#include <Nepomuk2/Query/Term>
#include <Nepomuk2/Query/LiteralTerm>

#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/ResultIterator>

#include <Nepomuk2/Vocabulary/NFO>
#include <Soprano/Vocabulary/NAO>
#include <Nepomuk2/Types/Property>
#include <Nepomuk2/Variant>

using namespace Nepomuk2;

namespace Digikam
{

NepomukQuery::NepomukQuery(DkNepomukService* const service)
{
    this->service = service;
}

Query::Query buildImagePropertiesQuery()
{

    // Query structure: (isImage() && (hasTag() || hasRating() || hasComment()))

    Query::ResourceTypeTerm imgType(Vocabulary::NFO::Image());

    Query::ComparisonTerm hasTag(Soprano::Vocabulary::NAO::hasTag(),
                                            Nepomuk2::Query::Term());

    Query::ComparisonTerm hasRating(Soprano::Vocabulary::NAO::numericRating(),
                                               Nepomuk2::Query::Term());

    Query::ComparisonTerm hasComment(Soprano::Vocabulary::NAO::description(),
                                               Nepomuk2::Query::Term());
    Query::OrTerm  hasProperties;
    hasProperties.addSubTerm(hasTag);
    hasProperties.addSubTerm(hasRating);
    hasProperties.addSubTerm(hasComment);

    Query::AndTerm finalTerm;

    finalTerm.addSubTerm(imgType);
    finalTerm.addSubTerm(hasProperties);

    Query::Query query(finalTerm);

    return query;

}
void NepomukQuery::queryImagesProperties()
{
    Query::Query query = buildImagePropertiesQuery();

    Query::ResultIterator it(query);

    int ind = 0;
    while(it.next())
    {
        Resource res = it.current().resource();

        Variant tag = res.property(Soprano::Vocabulary::NAO::hasTag());
        if(tag.isValid())
            kDebug() << "Image " << ind << " Have Tags";

        Variant rating = res.property(Soprano::Vocabulary::NAO::numericRating());
        if(rating.isValid())
            kDebug() << "Image " << ind << " Have Rating";

        Variant comment = res.property(Soprano::Vocabulary::NAO::description());
        if(comment.isValid())
            kDebug() << "Image " << ind << " Have Comments";

        ++ind;
    }

}


void NepomukQuery::queryTags()
{

}
} // namespace Digikam