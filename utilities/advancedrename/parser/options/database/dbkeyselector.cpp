/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : database key selector.
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

#include "dbkeyselector.h"

// Qt includes

#include <QTreeWidget>
#include <QHeaderView>
#include <QGridLayout>
#include <QString>
#include <QMap>
#include <QPushButton>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ditemtooltip.h"
#include "dbkeyscollection.h"
#include "dbheaderlistitem.h"

namespace Digikam
{

DbKeySelectorItem::DbKeySelectorItem(DbHeaderListItem* const parent, const QString& title, const QString& desc)
    : QTreeWidgetItem(parent),
      m_key(title),
      m_description(desc)
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

DbKeySelectorItem::~DbKeySelectorItem()
{
}

QString DbKeySelectorItem::key() const
{
    return m_key;
}

QString DbKeySelectorItem::description() const
{
    return m_description;
}

// ------------------------------------------------------------------------------------

DbKeySelector::DbKeySelector(QWidget* const parent)
    : QTreeWidget(parent)
{
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setColumnCount(2);

    QStringList labels;
    labels.append(i18n("Key"));
    labels.append(i18n("Description"));
    setHeaderLabels(labels);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
}

DbKeySelector::~DbKeySelector()
{
}

void DbKeySelector::setKeysMap(const DbOptionKeysMap& map)
{
    clear();
    setSortingEnabled(true);

    QMap<QString, DbHeaderListItem*> headers;

    for (DbOptionKeysMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        if (!headers.contains(it.value()->collectionName()))
        {
            headers.insert(it.value()->collectionName(), new DbHeaderListItem(this, it.value()->collectionName()));
        }

        new DbKeySelectorItem(headers.value(it.value()->collectionName()),
                              it.key(),
                              it.value()->ids().value(it.key()));
    }

    sortByColumn(0, Qt::AscendingOrder);
    setSortingEnabled(false);
}

QStringList DbKeySelector::checkedKeysList()
{
    QStringList list;
    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Checked);

    while (*it)
    {
        DbKeySelectorItem* const item = dynamic_cast<DbKeySelectorItem*>(*it);

        if (item)
        {
            list.append(item->key());
        }

        ++it;
    }

    return list;
}

// ------------------------------------------------------------------------------------

class DbKeySelectorView::Private
{
public:

    Private() :
        selector(0),
        searchBar(0)
    {
    }

    DbKeySelector* selector;
    SearchTextBar* searchBar;
};

DbKeySelectorView::DbKeySelectorView(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid = new QGridLayout(this);
    d->selector             = new DbKeySelector(this);
    d->searchBar            = new SearchTextBar(this, QLatin1String("DbKeySelectorView"));

    grid->addWidget(d->selector,  0, 0, 1, 1);
    grid->addWidget(d->searchBar, 1, 0, 1, 1);
    grid->setColumnStretch(0, 10);
    grid->setRowStretch(0, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));
}

DbKeySelectorView::~DbKeySelectorView()
{
    delete d;
}

void DbKeySelectorView::setKeysMap(const DbOptionKeysMap& map)
{
    d->selector->setKeysMap(map);
}

QStringList DbKeySelectorView::checkedKeysList() const
{
    d->searchBar->clear();
    return d->selector->checkedKeysList();
}

void DbKeySelectorView::slotSearchTextChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    // Restore all DbKey items.
    QTreeWidgetItemIterator it2(d->selector);

    while (*it2)
    {
        DbHeaderListItem* const item = dynamic_cast<DbHeaderListItem*>(*it2);

        if (item)
        {
            item->setHidden(false);
        }

        ++it2;
    }

    QTreeWidgetItemIterator it(d->selector);

    while (*it)
    {
        DbKeySelectorItem* const item = dynamic_cast<DbKeySelectorItem*>(*it);

        if (item)
        {
            bool match = item->description().contains(search, settings.caseSensitive) ||
                         item->key().contains(search, settings.caseSensitive);

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

    removeChildlessHeaders();
    d->searchBar->slotSearchResult(atleastOneMatch);
}


void DbKeySelectorView::removeChildlessHeaders()
{
    QTreeWidgetItemIterator it(d->selector);

    while (*it)
    {
        DbHeaderListItem* const item = dynamic_cast<DbHeaderListItem*>(*it);

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

}  // namespace Digikam
