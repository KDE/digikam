/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : image info task splitter
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef IMAGEINFOTASKSPLITTER_H
#define IMAGEINFOTASKSPLITTER_H

// Qt includes

#include <QList>

// Local includes

#include "imageinfo.h"

namespace Digikam
{

class ImageInfoTaskSplitter : public QList<ImageInfo>
{
public:

    ImageInfoTaskSplitter(const QList<ImageInfo>& list);
    ~ImageInfoTaskSplitter();

    QList<ImageInfo> next();
    bool hasNext() const;

protected:

    int m_n;
};

} // namespace Digikam

#endif /* IMAGEINFOTASKSPLITTER_H */
