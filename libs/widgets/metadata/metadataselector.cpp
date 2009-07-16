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

#include "metadataselector.h"

// Qt includes

#include <QTreeWidget>
#include <QHeaderView>

// Local includes

#include "mdkeylistviewitem.h"

namespace Digikam
{

MetadataSelectorItem::MetadataSelectorItem(MdKeyListViewItem *parent, const QString& key,
                                           const QString& title, const QString& desc)
                    : QTreeWidgetItem(parent)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
    setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    setText(0, title);
    setText(1, desc);
    m_key = key;
}

MetadataSelectorItem::~MetadataSelectorItem()
{
}

QString MetadataSelectorItem::key() const
{
    return m_key;
}

// ------------------------------------------------------------------------------------

MetadataSelector::MetadataSelector(QWidget* parent)
                : QTreeWidget(parent)
{
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setColumnCount(2);
    header()->setResizeMode(QHeaderView::Stretch);
    header()->hide();

    QStringList labels;
    labels.append( "Name" );              // no i18n here: hidden header
    labels.append( "Description" );       // no i18n here: hidden header
    setHeaderLabels(labels);
}

MetadataSelector::~MetadataSelector()
{
}

void MetadataSelector::setTagsMap(const DMetadata::TagsMap& map)
{
    clear();

    uint               subItems = 0;
    QString            ifDItemName;
    MdKeyListViewItem *parentifDItem = 0;

    for (DMetadata::TagsMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        // We checking if we have changed of ifDName
        QString currentIfDName = it.key().section('.', 1, 1);

        if ( currentIfDName != ifDItemName )
        {
            ifDItemName = currentIfDName;

            // Check if the current IfD have any items. If no remove it before to toggle to the next IfD.
            if ( subItems == 0 && parentifDItem)
                delete parentifDItem;

            parentifDItem = new MdKeyListViewItem(this, currentIfDName);
            subItems = 0;
        }

        // We ignore all unknown tags if necessary.
        if (!it.key().section('.', 2, 2).startsWith(QLatin1String("0x")))
        {
            new MetadataSelectorItem(parentifDItem, it.key(), it.value()[0], it.value()[2]);
            ++subItems;
        }
    }
}

void MetadataSelector::setcheckedTagsList(const QStringList& list)
{
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        MetadataSelectorItem *item = dynamic_cast<MetadataSelectorItem*>(*it);
        if (item && list.contains(item->key()))
        {
            item->setCheckState(0, Qt::Checked);
        }
        ++it;
    }
}

QStringList MetadataSelector::checkedTagsList()
{
    QStringList list;
    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Checked);
    while (*it)
    {
        MetadataSelectorItem *item = dynamic_cast<MetadataSelectorItem*>(*it);
        if (item)
        {
            list.append(item->key());
        }
        ++it;
    }
    return list;
}

}  // namespace Digikam
