/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : configuration view for external BQM plugin
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dpluginbqmconfview.h"

// Qt include

#include <QList>
#include <QHeaderView>

// Local includes

#include "dpluginbqm.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPluginBqmCB : public QTreeWidgetItem
{
public:

    explicit DPluginBqmCB(DPluginBqm* const plugin, QTreeWidget* const parent)
        : QTreeWidgetItem(parent),
          m_plugin(plugin)
    {
        setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        setDisabled(false);

        // Name + Icon + Selector
        setText(0, m_plugin->name());
        setIcon(0, m_plugin->icon());
        setToolTip(0, m_plugin->details());

        // Categories
        QStringList list = m_plugin->batchToolGroup();
        setText(1, list.join(QString::fromLatin1(", ")));

        // Number of actions
        setText(2, QString::number(m_plugin->toolCount()));
        
        // Description
        setText(3, m_plugin->description());

        // Authors
        list = m_plugin->pluginAuthors();
        setText(4, list.join(QString::fromLatin1(", ")));
    };

    ~DPluginBqmCB()
    {
    };

    bool contains(const QString& txt, Qt::CaseSensitivity cs) const
    {
        return (text(0).contains(txt, cs) ||
                text(1).contains(txt, cs) ||
                text(2).contains(txt, cs) ||
                text(3).contains(txt, cs) ||
                text(4).contains(txt, cs));
    };

public:

    DPluginBqm* m_plugin;
};

// ---------------------------------------------------------------------

class Q_DECL_HIDDEN DPluginBqmConfView::Private
{
public:

    explicit Private()
    {
    };

    QString              filter;
    QList<DPluginBqmCB*> bqmBoxes;
};

DPluginBqmConfView::DPluginBqmConfView(QWidget* const parent)
    : QTreeWidget(parent),
      d(new Private)
{
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setSortingEnabled(true);
    setColumnCount(5);

    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(3, QHeaderView::Stretch);
    header()->setSectionResizeMode(4, QHeaderView::Interactive);
    header()->setSortIndicatorShown(true);

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);

    DPluginLoader* const loader = DPluginLoader::instance();

    if (loader)
    {
        foreach (DPlugin* const tool, loader->allPlugins())
        {
            DPluginBqm* const bqm = dynamic_cast<DPluginBqm*>(tool);

            if (bqm)
            {
                d->bqmBoxes.append(new DPluginBqmCB(bqm, this));
            }
        }
    }

    // Sort items by plugin names.
    sortItems(0, Qt::AscendingOrder);
}

DPluginBqmConfView::~DPluginBqmConfView()
{
    delete d;
}

DPluginBqm* DPluginBqmConfView::plugin(QTreeWidgetItem* const item) const
{
    if (item)
    {
        DPluginBqmCB* const cb = dynamic_cast<DPluginBqmCB*>(item);

        if (cb)
        {
            return cb->m_plugin;
        }
    }

    return 0;
}

int DPluginBqmConfView::count() const
{
    return d->bqmBoxes.count();
}

int DPluginBqmConfView::visible() const
{
    int visible = 0;

    foreach (DPluginBqmCB* const item, d->bqmBoxes)
    {
        if (!item->isHidden())
            ++visible;
    }

    return visible;
}

void DPluginBqmConfView::setFilter(const QString& filter, Qt::CaseSensitivity cs)
{
    d->filter  = filter;
    bool query = false;

    foreach (DPluginBqmCB* const item, d->bqmBoxes)
    {
        if (item->contains(filter, cs))
        {
            query = true;
            item->setHidden(false);
        }
        else
        {
            item->setHidden(true);
        }
    }

    emit signalSearchResult(query);
}

QString DPluginBqmConfView::filter() const
{
    return d->filter;
}

} // namespace Digikam
