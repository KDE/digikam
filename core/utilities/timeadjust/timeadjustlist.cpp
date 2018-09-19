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

#include "timeadjustlist.h"

// Qt includes

#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

TimeAdjustList::TimeAdjustList(QWidget* const parent)
    : DImagesList(parent)
{
    setControlButtonsPlacement(DImagesList::NoControlButtons);
    listView()->setColumn(static_cast<Digikam::DImagesListView::ColumnType>(TIMESTAMP_USED),
                          i18n("Timestamp Used"), true);
    listView()->setColumn(static_cast<Digikam::DImagesListView::ColumnType>(TIMESTAMP_UPDATED),
                          i18n("Timestamp Updated"), true);
    listView()->setColumn(static_cast<Digikam::DImagesListView::ColumnType>(STATUS),
                          i18n("Status"), true);
}

TimeAdjustList::~TimeAdjustList()
{
}

void TimeAdjustList::setItemDates(const QMap<QUrl, QDateTime>& map, FieldType type)
{
    QString dateTimeFormat = QLocale().dateFormat(QLocale::ShortFormat);

    if (!dateTimeFormat.contains(QLatin1String("yyyy")))
    {
        dateTimeFormat.replace(QLatin1String("yy"),
                               QLatin1String("yyyy"));
    }

    dateTimeFormat.append(QLatin1String(" hh:mm:ss"));

    foreach (const QUrl& url, map.keys())
    {
        DImagesListViewItem* const item = listView()->findItem(url);

        if (item)
        {
            QDateTime dt = map.value(url);

            if (dt.isValid())
            {
                item->setText(type, dt.toString(dateTimeFormat));
            }
            else
            {
                item->setText(type, i18n("not valid"));
            }
        }
    }
}

void TimeAdjustList::setStatus(const QMap<QUrl, int>& status)
{
    foreach (const QUrl& url, status.keys())
    {
        DImagesListViewItem* const item = listView()->findItem(url);

        if (item)
        {
            QStringList errors;
            int         flags = status.value(url);

            if (flags & META_TIME_ERROR)
            {
                errors << i18n("Failed to update metadata timestamp");
            }

            if (flags & FILE_TIME_ERROR)
            {
                errors << i18n("Failed to update file timestamp");
            }

            if (errors.isEmpty())
            {
                item->setText(STATUS, i18n("Processed without error"));
            }
            else
            {
                item->setText(STATUS, errors.join(QLatin1String(" | ")));
            }
        }
    }
}

}  // namespace Digikam
