/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-17-04
 * Description : time adjust images list.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (c) 2018      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_TIME_ADJUST_LIST_H
#define DIGIKAM_TIME_ADJUST_LIST_H

// Qt includes

#include <QMap>
#include <QDateTime>
#include <QUrl>

// Local includes

#include "dimageslist.h"
#include "timeadjustsettings.h"

namespace Digikam
{

class TimeAdjustList : public DImagesList
{
    Q_OBJECT

public:

    /* The different columns in a list. */
    enum FieldType
    {
        TIMESTAMP_USED     = DImagesListView::User1,
        TIMESTAMP_UPDATED  = DImagesListView::User2,
        STATUS             = DImagesListView::User3
    };

    enum ProcessingStatus
    {
        NOPROCESS_ERROR = 1 << 0,
        META_TIME_ERROR = 1 << 1,
        FILE_TIME_ERROR = 1 << 2
    };

public:

    explicit TimeAdjustList(QWidget* const parent);
    ~TimeAdjustList();

    void setItemDates(const QMap<QUrl, QDateTime>& map, FieldType type);
    void setStatus(const QMap<QUrl, int>& status);
};

}  // namespace Digikam

#endif // DIGIKAM_TIME_ADJUST_LIST_H
