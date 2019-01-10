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

// Local includes

#include "dplugingeneric.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPluginGenericCB : public QTreeWidgetItem
{
public:

    explicit DPluginGenericCB(DPluginGeneric* const plugin, QTreeWidget* const parent)
        : QTreeWidgetItem(parent),
          m_plugin(plugin)
    {
        setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        setDisabled(false);

        // Name + Icon + Selector
        setText(0, m_plugin->name());
        setIcon(0, m_plugin->icon());
        setCheckState(0, m_plugin->shouldLoaded() ? Qt::Checked : Qt::Unchecked);
        setToolTip(0, m_plugin->details());

        // Categories
        QStringList list = m_plugin->actionCategories();
        setText(1, list.join(QString::fromLatin1(", ")));

        // Number of actions
        setText(2, QString::number(m_plugin->actionCount()));
        
        // Description
        setText(3, m_plugin->description());

        // Authors
        list = m_plugin->pluginAuthors();
        setText(4, list.join(QString::fromLatin1(", ")));
    };

    ~DPluginGenericCB()
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

    DPluginGeneric* m_plugin;
};

// ---------------------------------------------------------------------

class Q_DECL_HIDDEN DPluginConfView::Private
{
public:

    explicit Private()
    {
    };

    QString                  filter;
    QList<DPluginGenericCB*> geneBoxes;
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
            DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(tool);

            if (gene)
            {
                d->geneBoxes.append(new DPluginGenericCB(gene, this));
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

        foreach (DPluginGenericCB* const item, d->geneBoxes)
        {
            bool load = (item->checkState(0) == Qt::Checked);
            group.writeEntry(item->m_plugin->iid(), load);
            item->m_plugin->setVisible(load);
            item->m_plugin->setShouldLoaded(load);
        }

        config->sync();
    }
}

void DPluginConfView::selectAll()
{
    foreach (DPluginGenericCB* const item, d->geneBoxes)
    {
        item->setCheckState(0, Qt::Checked);
    }
}

void DPluginConfView::clearAll()
{
    foreach (DPluginGenericCB* const item, d->geneBoxes)
    {
        item->setCheckState(0, Qt::Unchecked);
    }
}

int DPluginConfView::count() const
{
    return d->geneBoxes.count();
}

int DPluginConfView::actived() const
{
    int actived = 0;

    foreach (DPluginGenericCB* const item, d->geneBoxes)
    {
        if (item->checkState(0) == Qt::Checked)
            ++actived;
    }

    return actived;
}

int DPluginConfView::visible() const
{
    int visible = 0;

    foreach (DPluginGenericCB* const item, d->geneBoxes)
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

    foreach (DPluginGenericCB* const item, d->geneBoxes)
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
