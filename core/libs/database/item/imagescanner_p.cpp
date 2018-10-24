/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - private containers.
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

#include "imagescanner_p.h"

namespace Digikam
{

ImageScannerCommit::ImageScannerCommit()
    : operation(NoOp),
      copyImageAttributesId(-1),
      commitImageInformation(false),
      commitImageMetadata(false),
      commitVideoMetadata(false),
      commitImagePosition(false),
      commitImageComments(false),
      commitImageCopyright(false),
      commitFaces(false),
      commitIPTCCore(false),
      hasColorTag(false),
      hasPickTag(false)
{
}

// ---------------------------------------------------------------------------------------

lessThanByProximityToSubject::lessThanByProximityToSubject(const ImageInfo& subject)
    : subject(subject)
{
}

bool lessThanByProximityToSubject::operator()(const ImageInfo& a, const ImageInfo& b)
{
    if (a.isNull() || b.isNull())
    {
        // both null: false
        // only a null: a greater than b (null infos at end of list)
        //  (a && b) || (a && !b) = a
        // only b null: a less than b
        if (a.isNull())
        {
            return false;
        }

        return true;
    }

    if (a == b)
    {
        return false;
    }

    // same collection
    if (a.albumId() != b.albumId())
    {
        // same album
        if (a.albumId() == subject.albumId())
        {
            return true;
        }

        if (b.albumId() == subject.albumId())
        {
            return false;
        }

        if (a.albumRootId() != b.albumRootId())
        {
            // different collection
            if (a.albumRootId() == subject.albumRootId())
            {
                return true;
            }

            if (b.albumRootId() == subject.albumRootId())
            {
                return false;
            }
        }
    }

    if (a.modDateTime() != b.modDateTime())
    {
        return a.modDateTime() < b.modDateTime();
    }

    if (a.name() != b.name())
    {
        return qAbs(a.name().compare(subject.name())) < qAbs(b.name().compare(subject.name()));
    }

    // last resort
    return (a.id() < b.id());
}

// ---------------------------------------------------------------------------

ImageScanner::Private::Private()
    : hasImage(false),
      hasMetadata(false),
      loadedFromDisk(false),
      scanMode(ModifiedScan),
      hasHistoryToResolve(false)
{
    time.start();
}

} // namespace Digikam
