/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - baloo helper.
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemscanner_p.h"

#ifdef HAVE_KFILEMETADATA
#   include "baloowrap.h"
#endif

namespace Digikam
{

void ItemScanner::scanBalooInfo()
{

#ifdef HAVE_KFILEMETADATA

    BalooWrap* const baloo = BalooWrap::instance();

    if (!baloo->getSyncToDigikam())
    {
        return;
    }

    BalooInfo bInfo = baloo->getSemanticInfo(QUrl::fromLocalFile(d->fileInfo.absoluteFilePath()));

    if (!bInfo.tags.isEmpty())
    {
        // get tag ids, create if necessary
        QList<int> tagIds = TagsCache::instance()->getOrCreateTags(bInfo.tags);
        d->commit.tagIds += tagIds;
    }

    if (bInfo.rating != -1)
    {
        if (!d->commit.imageInformationFields.testFlag(DatabaseFields::Rating))
        {
            d->commit.imageInformationFields |= DatabaseFields::Rating;
            d->commit.imageInformationInfos.insert(0, QVariant(bInfo.rating));
        }
    }

    if (!bInfo.comment.isEmpty())
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Comment " << bInfo.comment;

        if (!d->commit.captions.contains(QLatin1String("x-default")))
        {
            CaptionValues val;
            val.caption                   = bInfo.comment;
            d->commit.commitImageComments = true;
            d->commit.captions.insert(QLatin1String("x-default"), val);
        }
    }

#endif // HAVE_KFILEMETADATA

}

} // namespace Digikam
