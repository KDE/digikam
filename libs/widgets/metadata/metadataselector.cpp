/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-16
 * Description : metadata selector.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QGridLayout>
#include <QApplication>
#include <QPushButton>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ditemtooltip.h"
#include "mdkeylistviewitem.h"

namespace Digikam
{

MetadataSelectorItem::MetadataSelectorItem(MdKeyListViewItem* const parent, const QString& key,
                                           const QString& title, const QString& desc)
    : QTreeWidgetItem(parent),
      m_key(key),
      m_parent(parent)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
    setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

    setText(0, title);

    QString descVal = desc.simplified();

    if (descVal.length() > 512)
    {
        descVal.truncate(512);
        descVal.append(QLatin1String("..."));
    }

    setText(1, descVal);

    DToolTipStyleSheet cnt;
    setToolTip(1, QLatin1String("<qt><p>") + cnt.breakString(descVal) + QLatin1String("</p></qt>"));
}

MetadataSelectorItem::~MetadataSelectorItem()
{
}

QString MetadataSelectorItem::key() const
{
    return m_key;
}

QString MetadataSelectorItem::mdKeyTitle() const
{
    return (m_parent ? m_parent->text(0) : QString());
}

// ------------------------------------------------------------------------------------

MetadataSelector::MetadataSelector(QWidget* const parent)
    : QTreeWidget(parent)
{
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setColumnCount(2);

    QStringList labels;
    labels.append(i18n("Name"));
    labels.append(i18n("Description"));
    setHeaderLabels(labels);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
}

MetadataSelector::~MetadataSelector()
{
}

void MetadataSelector::setTagsMap(const DMetadata::TagsMap& map)
{
    clear();

    uint                    subItems = 0;
    QString                 ifDItemName, currentIfDName;
    MdKeyListViewItem*      parentifDItem = 0;
    QList<QTreeWidgetItem*> toplevelItems;

    for (DMetadata::TagsMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        // We checking if we have changed of ifDName
        currentIfDName = it.key().section(QLatin1Char('.'), 1, 1);

        if ( currentIfDName != ifDItemName )
        {
            ifDItemName = currentIfDName;

            // Check if the current IfD have any items. If not, remove it before to toggle to the next IfD.
            if ( subItems == 0 && parentifDItem)
            {
                delete parentifDItem;
            }

            parentifDItem = new MdKeyListViewItem(0, currentIfDName);
            toplevelItems << parentifDItem;
            subItems      = 0;
        }

        // We ignore all unknown tags if necessary.
        if (!it.key().section(QLatin1Char('.'), 2, 2).startsWith(QLatin1String("0x")))
        {
            new MetadataSelectorItem(parentifDItem, it.key(), it.value().at(0), it.value().at(2));
            ++subItems;
        }
    }

    addTopLevelItems(toplevelItems);

    // We need to call setFirstColumnSpanned() in here again because the widgets were added parentless and therefore
    // no layout information was present at construction time. Now that all items have a parent, we need to trigger the
    // method again.
    for (QList<QTreeWidgetItem*>::const_iterator it = toplevelItems.constBegin(); it != toplevelItems.constEnd(); ++it)
    {
        if (*it)
        {
            (*it)->setFirstColumnSpanned(true);
        }
    }
    expandAll();
}

void MetadataSelector::setcheckedTagsList(const QStringList& list)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

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
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item)
        {
            list.append(item->key());
        }

        ++it;
    }

    return list;
}

void MetadataSelector::clearSelection()
{
    collapseAll();

    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Checked);

    while (*it)
    {
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item)
        {
            item->setCheckState(0, Qt::Unchecked);
        }

        ++it;
    }

    expandAll();
}

void MetadataSelector::selectAll()
{
    collapseAll();

    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::NotChecked);

    while (*it)
    {
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item)
        {
            item->setCheckState(0, Qt::Checked);
        }

        ++it;
    }

    expandAll();
}

// ------------------------------------------------------------------------------------

class MetadataSelectorView::Private
{
public:

    Private()
    {
        selectAllBtn        = 0;
        clearSelectionBtn   = 0;
        defaultSelectionBtn = 0;
        selector            = 0;
        searchBar           = 0;
    }

    QStringList       defaultFilter;

    QPushButton*      selectAllBtn;
    QPushButton*      clearSelectionBtn;
    QPushButton*      defaultSelectionBtn;

    MetadataSelector* selector;

    SearchTextBar*    searchBar;
};

MetadataSelectorView::MetadataSelectorView(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid = new QGridLayout(this);
    d->selector             = new MetadataSelector(this);
    d->searchBar            = new SearchTextBar(this, QLatin1String("MetadataSelectorView"));
    d->selectAllBtn         = new QPushButton(i18n("Select All"),this);
    d->clearSelectionBtn    = new QPushButton(i18n("Clear"),this);
    d->defaultSelectionBtn  = new QPushButton(i18n("Default"),this);

    grid->addWidget(d->selector,            0, 0, 1, 5);
    grid->addWidget(d->searchBar,           1, 0, 1, 1);
    grid->addWidget(d->selectAllBtn,        1, 2, 1, 1);
    grid->addWidget(d->clearSelectionBtn,   1, 3, 1, 1);
    grid->addWidget(d->defaultSelectionBtn, 1, 4, 1, 1);
    grid->setColumnStretch(0, 10);
    grid->setRowStretch(0, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    setControlElements(SearchBar | SelectAllBtn | DefaultBtn | ClearBtn);

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(d->selectAllBtn, SIGNAL(clicked()),
            this, SLOT(slotSelectAll()));

    connect(d->defaultSelectionBtn, SIGNAL(clicked()),
            this, SLOT(slotDeflautSelection()));

    connect(d->clearSelectionBtn, SIGNAL(clicked()),
            this, SLOT(slotClearSelection()));
}

MetadataSelectorView::~MetadataSelectorView()
{
    delete d;
}

void MetadataSelectorView::setTagsMap(const DMetadata::TagsMap& map)
{
    d->selector->setTagsMap(map);
}

void MetadataSelectorView::setcheckedTagsList(const QStringList& list)
{
    d->selector->setcheckedTagsList(list);
}

void MetadataSelectorView::setDefaultFilter(const QStringList& list)
{
    d->defaultFilter = list;
}

QStringList MetadataSelectorView::defaultFilter() const
{
    return d->defaultFilter;
}

int MetadataSelectorView::itemsCount() const
{
    return d->selector->model()->rowCount();
}

QStringList MetadataSelectorView::checkedTagsList() const
{
    d->searchBar->clear();
    return d->selector->checkedTagsList();
}

void MetadataSelectorView::slotSearchTextChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    // Restore all MdKey items.
    QTreeWidgetItemIterator it2(d->selector);

    while (*it2)
    {
        MdKeyListViewItem* const item = dynamic_cast<MdKeyListViewItem*>(*it2);

        if (item)
        {
            item->setHidden(false);
        }

        ++it2;
    }

    QTreeWidgetItemIterator it(d->selector);

    while (*it)
    {
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item)
        {
            bool match = item->text(0).contains(search, settings.caseSensitive) ||
                         item->mdKeyTitle().contains(search, settings.caseSensitive);

            if (match)
            {
                atleastOneMatch = true;
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

    d->searchBar->slotSearchResult(atleastOneMatch);
}

void MetadataSelectorView::cleanUpMdKeyItem()
{
    QTreeWidgetItemIterator it(d->selector);

    while (*it)
    {
        MdKeyListViewItem* const item = dynamic_cast<MdKeyListViewItem*>(*it);

        if (item)
        {
            int children   = item->childCount();
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

void MetadataSelectorView::slotDeflautSelection()
{
    slotClearSelection();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->selector->collapseAll();

    QTreeWidgetItemIterator it(d->selector);

    while (*it)
    {
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item)
        {
            if (d->defaultFilter.contains(item->text(0)))
            {
                item->setCheckState(0, Qt::Checked);
            }
        }

        ++it;
    }

    d->selector->expandAll();
    QApplication::restoreOverrideCursor();
}

void MetadataSelectorView::slotSelectAll()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->selector->selectAll();
    QApplication::restoreOverrideCursor();
}

void MetadataSelectorView::slotClearSelection()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->selector->clearSelection();
    QApplication::restoreOverrideCursor();
}

void MetadataSelectorView::setControlElements(ControlElements controllerMask)
{
    d->searchBar->setVisible(controllerMask & SearchBar);
    d->selectAllBtn->setVisible(controllerMask & SelectAllBtn);
    d->clearSelectionBtn->setVisible(controllerMask & ClearBtn);
    d->defaultSelectionBtn->setVisible(controllerMask & DefaultBtn);
}

void MetadataSelectorView::clearSelection()
{
    slotClearSelection();
}

void MetadataSelectorView::selectAll()
{
    slotSelectAll();
}

void MetadataSelectorView::selectDefault()
{
    slotDeflautSelection();
}

}  // namespace Digikam
