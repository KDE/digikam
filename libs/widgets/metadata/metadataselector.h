/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-16
 * Description : metadata selector.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATA_SELECTOR_H
#define METADATA_SELECTOR_H

// Qt includes

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QStringList>

// Local includes

#include "digikam_export.h"
#include "dmetadata.h"

namespace Digikam
{

class MdKeyListViewItem;

class DIGIKAM_EXPORT MetadataSelectorItem : public QTreeWidgetItem
{

public:

    MetadataSelectorItem(MdKeyListViewItem* parent, const QString& key, const QString& title, const QString& desc);
    virtual ~MetadataSelectorItem();

    QString key() const;

private:

    QString m_key;
};

// ------------------------------------------------------------------------------------

class DIGIKAM_EXPORT MetadataSelector : public QTreeWidget
{

public:

    MetadataSelector(QWidget* parent);
    virtual ~MetadataSelector();

    void setTagsMap(const DMetadata::TagsMap& map);

    void setcheckedTagsList(const QStringList& list);
    QStringList checkedTagsList();
};

}  // namespace Digikam

#endif /* METADATA_SELECTOR_H */
