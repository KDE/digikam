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

#include "dbkeyselector.moc"

// Qt includes

#include <QTreeWidget>
#include <QHeaderView>
#include <QGridLayout>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <kpushbutton.h>

// Local includes

#include "ditemtooltip.h"
#include "dboptionkey.h"

namespace Digikam
{

DbKeySelectorItem::DbKeySelectorItem(QTreeWidget *parent, const QString& title, const QString& desc)
                 : QTreeWidgetItem(parent), m_key(title), m_description(desc)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
    setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

    setText(0, title);

    QString descVal = desc.simplified();
    if (descVal.length() > 512)
    {
        descVal.truncate(512);
        descVal.append("...");
    }

    setText(1, descVal);

    DToolTipStyleSheet cnt;
    setToolTip(1, "<qt><p>" + cnt.breakString(descVal) + "</p></qt>");
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

DbKeySelector::DbKeySelector(QWidget* parent)
             : QTreeWidget(parent)
{
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setColumnCount(2);

    QStringList labels;
    labels.append( i18n("Key") );
    labels.append( i18n("Description") );
    setHeaderLabels(labels);
    header()->setResizeMode(0, QHeaderView::Stretch);
    header()->setResizeMode(1, QHeaderView::Stretch);
}

DbKeySelector::~DbKeySelector()
{
}

void DbKeySelector::setKeysMap(const DbOptionKeysMap& map)
{
    clear();

    for (DbOptionKeysMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        new DbKeySelectorItem(this, it.key(), it.value()->ids().value(it.key()));
    }
}

QStringList DbKeySelector::checkedKeysList()
{
    QStringList list;
    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Checked);
    while (*it)
    {
        DbKeySelectorItem *item = dynamic_cast<DbKeySelectorItem*>(*it);
        if (item)
            list.append(item->key());

        ++it;
    }
    return list;
}

// ------------------------------------------------------------------------------------

class DbKeySelectorViewPriv
{
public:

    DbKeySelectorViewPriv()
    {
        selector  = 0;
        searchBar = 0;
    }

    DbKeySelector* selector;
    SearchTextBar* searchBar;
};

DbKeySelectorView::DbKeySelectorView(QWidget* parent)
                 : QWidget(parent), d(new DbKeySelectorViewPriv)
{
    QGridLayout *grid = new QGridLayout(this);
    d->selector       = new DbKeySelector(this);
    d->searchBar      = new SearchTextBar(this, "DbKeySelectorView");

    grid->addWidget(d->selector,  0, 0, 1, 1);
    grid->addWidget(d->searchBar, 1, 0, 1, 1);
    grid->setColumnStretch(0, 10);
    grid->setRowStretch(0, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SLOT(slotSearchTextChanged(const SearchTextSettings&)));
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
        DbKeySelectorItem *item = dynamic_cast<DbKeySelectorItem*>(*it2);
        if (item)
            item->setHidden(false);
        ++it2;
    }

    QTreeWidgetItemIterator it(d->selector);
    while (*it)
    {
        DbKeySelectorItem *item = dynamic_cast<DbKeySelectorItem*>(*it);
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

    d->searchBar->slotSearchResult(atleastOneMatch);
}

}  // namespace Digikam
