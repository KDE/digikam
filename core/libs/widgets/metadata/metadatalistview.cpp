/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view widget to
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

#include "metadatalistview.h"

// Qt includes

#include <QHeaderView>
#include <QTimer>
#include <QPalette>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "mdkeylistviewitem.h"
#include "metadatalistviewitem.h"

namespace Digikam
{

MetadataListView::MetadataListView(QWidget* const parent)
    : QTreeWidget(parent)
{
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setColumnCount(2);
    header()->setSectionResizeMode(QHeaderView::Stretch);
    header()->hide();

    QStringList labels;
    labels.append(QLatin1String("Name"));        // no i18n here: hidden header
    labels.append(QLatin1String("Value"));       // no i18n here: hidden header
    setHeaderLabels(labels);

    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);

    m_parent = dynamic_cast<MetadataWidget*>(parent);

    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(slotSelectionChanged(QTreeWidgetItem*, int)));
}

MetadataListView::~MetadataListView()
{
}

QString MetadataListView::getCurrentItemKey()
{
    if (currentItem() && (currentItem()->flags() & Qt::ItemIsSelectable))
    {
        MetadataListViewItem* item = static_cast<MetadataListViewItem*>(currentItem());
        return item->getKey();
    }

    return QString();
}

void MetadataListView::setCurrentItemByKey(const QString& itemKey)
{
    if (itemKey.isNull())
    {
        return;
    }

    int i                 = 0;
    QTreeWidgetItem* item = 0;

    do
    {
        item = topLevelItem(i);

        if (item && (item->flags() & Qt::ItemIsSelectable))
        {
            MetadataListViewItem* const lvItem = dynamic_cast<MetadataListViewItem*>(item);

            if (lvItem)
            {
                if (lvItem->getKey() == itemKey)
                {
                    setCurrentItem(item);
                    scrollToItem(item);
                    m_selectedItemKey = itemKey;
                    return;
                }
            }
        }

        ++i;
    }
    while (item);
}

void MetadataListView::slotSelectionChanged(QTreeWidgetItem* item, int)
{
    if (!item)
    {
        return;
    }

    MetadataListViewItem* const viewItem = static_cast<MetadataListViewItem*>(item);
    m_selectedItemKey                    = viewItem->getKey();
    QString tagValue                     = viewItem->getValue().simplified();
    QString tagTitle                     = m_parent->getTagTitle(m_selectedItemKey);
    QString tagDesc                      = m_parent->getTagDescription(m_selectedItemKey);

    if (tagValue.length() > 128)
    {
        tagValue.truncate(128);
        tagValue.append(QLatin1String("..."));
    }

    this->setWhatsThis(i18n("<b>Title: </b><p>%1</p>"
                            "<b>Value: </b><p>%2</p>"
                            "<b>Description: </b><p>%3</p>",
                            tagTitle, tagValue, tagDesc));
}

void MetadataListView::setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& tagsFilter)
{
    clear();

    uint               subItems      = 0;
    MdKeyListViewItem* parentifDItem = 0;
    QStringList        filters       = tagsFilter;
    QString            ifDItemName;

    for (DMetadata::MetaDataMap::const_iterator it = ifds.constBegin(); it != ifds.constEnd(); ++it)
    {
        // We checking if we have changed of ifDName
        QString currentIfDName = it.key().section(QLatin1Char('.'), 1, 1);

        if ( currentIfDName != ifDItemName )
        {
            ifDItemName = currentIfDName;

            // Check if the current IfD have any items. If no remove it before to toggle to the next IfD.
            if ( subItems == 0 && parentifDItem)
            {
                delete parentifDItem;
            }

            parentifDItem = new MdKeyListViewItem(this, currentIfDName);
            subItems      = 0;
        }

        if (filters.isEmpty())
        {
            QString tagTitle = m_parent->getTagTitle(it.key());
            new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
            ++subItems;
        }
        else if (!it.key().section(QLatin1Char('.'), 2, 2).startsWith(QLatin1String("0x")))
        {
            // We ignore all unknown tags if necessary.

            if (filters.contains(QLatin1String("FULL")))
            {
                // We don't filter the output (Photo Mode)

                QString tagTitle = m_parent->getTagTitle(it.key());
                new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                ++subItems;
            }
            else if (!filters.isEmpty())
            {
                // We using the filter to make a more user friendly output (Custom Mode)

                // Filter is not a list of complete tag keys
                if (!filters.at(0).contains(QLatin1String(".")) && filters.contains(it.key().section(QLatin1Char('.'), 2, 2)))
                {
                    QString tagTitle = m_parent->getTagTitle(it.key());
                    new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                    ++subItems;
                    filters.removeAll(it.key());
                }
                else if (filters.contains(it.key()))
                {
                    QString tagTitle = m_parent->getTagTitle(it.key());
                    new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                    ++subItems;
                    filters.removeAll(it.key());
                }
            }
        }
    }

    // To check if the last IfD have any items...
    if ( subItems == 0 && parentifDItem)
    {
        delete parentifDItem;
    }

    // Add not found tags from filter as grey items.
    if (!filters.isEmpty() && filters.at(0) != QLatin1String("FULL") && filters.at(0).contains(QLatin1String(".")))
    {
        foreach(const QString& key, filters)
        {
            MdKeyListViewItem* pitem = findMdKeyItem(key);

            if (!pitem)
            {
                pitem = new MdKeyListViewItem(this, key.section(QLatin1Char('.'), 1, 1));
            }

            QString tagTitle = m_parent->getTagTitle(key);
            new MetadataListViewItem(pitem, key, tagTitle);
        }
    }

    setCurrentItemByKey(m_selectedItemKey);
    update();
}

void MetadataListView::setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& keysFilter,
                                  const QStringList& tagsFilter)
{
    clear();

    QStringList        filters       = tagsFilter;
    uint               subItems      = 0;
    MdKeyListViewItem* parentifDItem = 0;

    if (ifds.count() == 0)
    {
        return;
    }

    for (QStringList::const_iterator itKeysFilter = keysFilter.constBegin();
         itKeysFilter != keysFilter.constEnd();
         ++itKeysFilter)
    {
        subItems      = 0;
        parentifDItem = new MdKeyListViewItem(this, *itKeysFilter);

        DMetadata::MetaDataMap::const_iterator it = ifds.constEnd();

        while (it != ifds.constBegin())
        {
            --it;

            if ( *itKeysFilter == it.key().section(QLatin1Char('.'), 1, 1) )
            {
                if (filters.isEmpty())
                {
                    QString tagTitle = m_parent->getTagTitle(it.key());
                    new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                    ++subItems;
                }
                else if (!it.key().section(QLatin1Char('.'), 2, 2).startsWith(QLatin1String("0x")))
                {
                    // We ignore all unknown tags if necessary.

                    if (filters.contains(QLatin1String("FULL")))
                    {
                        // We don't filter the output (Photo Mode)

                        QString tagTitle = m_parent->getTagTitle(it.key());
                        new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                        ++subItems;
                    }
                    else if (!filters.isEmpty())
                    {
                        // We using the filter to make a more user friendly output (Custom Mode)

                        // Filter is not a list of complete tag keys
                        if (!filters.at(0).contains(QLatin1String(".")) && filters.contains(it.key().section(QLatin1Char('.'), 2, 2)))
                        {
                            QString tagTitle = m_parent->getTagTitle(it.key());
                            new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                            ++subItems;
                            filters.removeAll(it.key());
                        }
                        else if (filters.contains(it.key()))
                        {
                            QString tagTitle = m_parent->getTagTitle(it.key());
                            new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                            ++subItems;
                            filters.removeAll(it.key());
                        }
                    }
                }
            }
        }

        // We checking if the last IfD have any items. If no, we remove it.
        if ( subItems == 0 && parentifDItem)
        {
            delete parentifDItem;
        }
    }

    // Add not found tags from filter as grey items.
    if (!filters.isEmpty() && filters.at(0) != QLatin1String("FULL") && filters.at(0).contains(QLatin1String(".")))
    {
        foreach(const QString& key, filters)
        {
            MdKeyListViewItem* pitem = findMdKeyItem(key);

            if (!pitem)
            {
                pitem = new MdKeyListViewItem(this, key.section(QLatin1Char('.'), 1, 1));
            }

            QString tagTitle = m_parent->getTagTitle(key);
            new MetadataListViewItem(pitem, key, tagTitle);
        }
    }

    setCurrentItemByKey(m_selectedItemKey);
    update();
}

void MetadataListView::slotSearchTextChanged(const SearchTextSettings& settings)
{
    bool query     = false;
    QString search = settings.text;

    // Restore all MdKey items.
    QTreeWidgetItemIterator it2(this);

    while (*it2)
    {
        MdKeyListViewItem* const item = dynamic_cast<MdKeyListViewItem*>(*it2);

        if (item)
        {
            item->setHidden(false);
        }

        ++it2;
    }

    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        MetadataListViewItem* const item = dynamic_cast<MetadataListViewItem*>(*it);

        if (item)
        {
            if (item->text(0).contains(search, settings.caseSensitive) ||
                item->text(1).contains(search, settings.caseSensitive))
            {
                query = true;
                item->setHidden(false);
            }
            else
            {
                item->setHidden(true);
            }
        }

        ++it;
    }

    // If we found MdKey items alone, we hide it...
    cleanUpMdKeyItem();

    emit signalTextFilterMatch(query);
}

void MetadataListView::cleanUpMdKeyItem()
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        MdKeyListViewItem* const item = dynamic_cast<MdKeyListViewItem*>(*it);

        if (item)
        {
            int children = item->childCount();
            int visibles = 0;

            for (int i = 0 ; i < children; ++i)
            {
                QTreeWidgetItem* const citem = (*it)->child(i);

                if (!citem->isHidden())
                {
                    ++visibles;
                }
            }

            if (!children || !visibles)
            {
                item->setHidden(true);
            }
        }

        ++it;
    }
}

MdKeyListViewItem* MetadataListView::findMdKeyItem(const QString& key)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        MdKeyListViewItem* const item = dynamic_cast<MdKeyListViewItem*>(*it);

        if (item)
        {
            if (key.section(QLatin1Char('.'), 1, 1) == item->getKey())
            {
                return item;
            }
        }

        ++it;
    }

    return 0;
}

}  // namespace Digikam
