/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Container for image info objects
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEINFOLIST_H
#define IMAGEINFOLIST_H

// Qt includes

#include <QList>

// Local includes

#include "imageinfo.h"
#include "digikam_export.h"
#include "digikam_config.h"

namespace Digikam
{

class ImageInfo;

class DIGIKAM_DATABASE_EXPORT ImageInfoList : public QList<ImageInfo>
{

public:

    ImageInfoList()
    {
    }

    explicit ImageInfoList(const QList<qlonglong>& idList);

    ImageInfoList(const QList<ImageInfo>& list)
        : QList<ImageInfo>(list)
    {
    }

    QList<qlonglong> toImageIdList()  const;
    QList<QUrl>      toImageUrlList() const;

    void loadGroupImageIds() const;
    void loadTagIds() const;

    bool static namefileLessThan(const ImageInfo& d1, const ImageInfo& d2);

    /**
     * @brief singleGroupMainItem
     * @return If the list contains of items of only one group including the
     * main item, this main item is returned, otherwise a null ImageInfo.
     */
    ImageInfo singleGroupMainItem() const;
};

typedef ImageInfoList::iterator ImageInfoListIterator;

}  // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImageInfoList)

#endif // IMAGEINFOLIST_H
