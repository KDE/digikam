/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : item info task splitter
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

#ifndef DIGIKAM_ITEM_INFO_TASK_SPLITTER_H
#define DIGIKAM_ITEM_INFO_TASK_SPLITTER_H

// Local includes

#include "fileactionimageinfolist.h"

namespace Digikam
{

class ItemInfoTaskSplitter : public FileActionItemInfoList
{
public:

    explicit ItemInfoTaskSplitter(const FileActionItemInfoList& list);
    ~ItemInfoTaskSplitter();

    FileActionItemInfoList next();
    bool hasNext() const;

protected:

    int m_n;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_INFO_TASK_SPLITTER_H
