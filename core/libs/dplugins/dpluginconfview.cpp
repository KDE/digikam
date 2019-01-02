/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : configuration view for external plugin
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

#include "dpluginconfview.h"

// Qt include

#include <QList>
#include <QHeaderView>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

namespace Digikam
{

class Q_DECL_HIDDEN DPluginCheckBox : public QTreeWidgetItem
{
public:

    explicit DPluginCheckBox(DPlugin* const tool, QTreeWidget* const parent)
        : QTreeWidgetItem(parent),
          m_tool(tool)
    {
        setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        setDisabled(false);

        // Name + Icon + Selector
        setText(0, m_tool->name());
        setIcon(0, m_tool->icon());
        setCheckState(0, m_tool->isLoaded() ? Qt::Checked : Qt::Unchecked);

        // Categories
        QStringList list = m_tool->pluginCategories();
        setText(1, list.join(QString::fromLatin1(", ")));

        // Version
        setText(2, m_tool->version());

        // Description
        setText(3, m_tool->description());

        // Authors
        list = m_tool->pluginAuthors();
        setText(4, list.join(QString::fromLatin1(", ")));
    };

    ~DPluginCheckBox()
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

    DPlugin* m_tool;
};

// ---------------------------------------------------------------------

class Q_DECL_HIDDEN DPluginConfView::Private
{
public:

    explicit Private()
    {
    };

    QString                 filter;
    QList<DPluginCheckBox*> boxes;
};

DPluginConfView::DPluginConfView(QWidget* const parent)
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
            if (tool)
            {
                d->boxes.append(new DPluginCheckBox(tool, this));
            }
        }
    }

    // Sort items by plugin names.
    sortItems(0, Qt::AscendingOrder);
}

DPluginConfView::~DPluginConfView()
{
    delete d;
}

void DPluginConfView::apply()
{
    DPluginLoader* const loader = DPluginLoader::instance();

    if (loader)
    {
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup group      = config->group(loader->configGroupName());

        foreach (DPluginCheckBox* const item, d->boxes)
        {
            bool load = (item->checkState(0) == Qt::Checked);
            group.writeEntry(item->m_tool->id(), load);
        }

        config->sync();
    }
}

void DPluginConfView::selectAll()
{
    foreach (DPluginCheckBox* const item, d->boxes)
    {
        item->setCheckState(0, Qt::Checked);
    }
}

void DPluginConfView::clearAll()
{
    foreach (DPluginCheckBox* const item, d->boxes)
    {
        item->setCheckState(0, Qt::Unchecked);
    }
}

int DPluginConfView::count() const
{
    return d->boxes.count();
}

int DPluginConfView::actived() const
{
    int actived = 0;

    foreach (DPluginCheckBox* const item, d->boxes)
    {
        if (item->checkState(0) == Qt::Checked)
            ++actived;
    }

    return actived;
}

int DPluginConfView::visible() const
{
    int visible = 0;

    foreach (DPluginCheckBox* const item, d->boxes)
    {
        if (!item->isHidden())
            ++visible;
    }

    return visible;
}

void DPluginConfView::setFilter(const QString& filter, Qt::CaseSensitivity cs)
{
    d->filter  = filter;
    bool query = false;

    foreach (DPluginCheckBox* const item, d->boxes)
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

QString DPluginConfView::filter() const
{
    return d->filter;
}

} // namespace Digikam
