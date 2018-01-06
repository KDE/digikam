/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view item widget to
 *               display metadata
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATALISTVIEWITEM_H
#define METADATALISTVIEWITEM_H

// Qt includes

#include <QString>
#include <QTreeWidgetItem>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MetadataListViewItem : public QTreeWidgetItem
{
public:

    MetadataListViewItem(QTreeWidgetItem* const parent, const QString& key,
                         const QString& title, const QString& value);
    MetadataListViewItem(QTreeWidgetItem* const parent, const QString& key,
                         const QString& title);
    ~MetadataListViewItem();

    QString getKey();
    QString getTitle();
    QString getValue();

private:

    QString m_key;
};

} // namespace Digikam

#endif // METADATALISTVIEWITEM_H
