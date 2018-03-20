/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : image info task splitter
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "imageinfotasksplitter.h"
#include "parallelworkers.h"

namespace Digikam
{

ImageInfoTaskSplitter::ImageInfoTaskSplitter(const FileActionImageInfoList& list)
    : FileActionImageInfoList(list)
{
    int parts = ParallelWorkers::optimalWorkerCount();
    m_n       = qMax(1, list.size() / parts);
}

ImageInfoTaskSplitter::~ImageInfoTaskSplitter()
{
}

FileActionImageInfoList ImageInfoTaskSplitter::next()
{
    QList<ImageInfo> list;

    if (size() <= m_n)
    {
        list = *this;
        clear();
    }
    else
    {
        list.reserve(m_n);

        // qCopy does not work with QList
        for (int i = 0;  i < m_n ; i++)
            list << at(i);

        erase(begin(), begin() + m_n);
    }

    return FileActionImageInfoList::continueTask(list, progress());
}

bool ImageInfoTaskSplitter::hasNext() const
{
    return !isEmpty();
}

} // namespace Digikam
