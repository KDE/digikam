/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : header list view item
 *
 * Copyright (C) 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef DBHEADERLISTITEM_H
#define DBHEADERLISTITEM_H

// Qt includes

#include <QObject>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace Digikam
{

class DbHeaderListItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT

public:

    DbHeaderListItem(QTreeWidget* parent, const QString& key);
    ~DbHeaderListItem();

private Q_SLOTS:

    void slotThemeChanged();

private:

    DbHeaderListItem(const DbHeaderListItem&);
    DbHeaderListItem& operator=(const DbHeaderListItem&);
};

}  // namespace Digikam

#endif /* DBHEADERLISTITEM_H */
