/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-07-31
 * Description : Tag List implementation as Quick Access for various
 *               subtrees in Tag Manager
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "taglist.h"

// Qt includes

#include <QVBoxLayout>
#include <QPushButton>
#include <QVariant>

// Local includes

#include "digikam_debug.h"
#include "albumtreeview.h"
#include "tagmngrtreeview.h"
#include "tagmngrlistmodel.h"
#include "tagmngrlistview.h"
#include "tagmngrlistitem.h"

namespace Digikam
{

class TagList::Private
{
public:

    Private()
    {
        addButton       = 0;
        tagList         = 0;
        tagListModel    = 0;
        treeView        = 0;
    }

    QPushButton*                 addButton;
    TagMngrListView*             tagList;
    TagMngrListModel*            tagListModel;
    TagMngrTreeView*             treeView;
    QMap<int, QList<ListItem*> > tagMap;
};

TagList::TagList(TagMngrTreeView* const treeView, QWidget* const parent)
        : QWidget(parent), d(new Private())
{
    d->treeView               = treeView;
    QVBoxLayout* const layout = new QVBoxLayout();
    d->addButton              = new QPushButton(i18n("Add to List"));
    d->addButton->setToolTip(i18n("Add selected tags to Quick Access List"));
    d->tagList                = new TagMngrListView(this);
    d->tagListModel           = new TagMngrListModel(this);

    d->tagList->setModel(d->tagListModel);
    d->tagList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->tagList->setDragEnabled(true);
    d->tagList->setAcceptDrops(true);
    d->tagList->setDropIndicatorShown(true);

    layout->addWidget(d->addButton);
    layout->addWidget(d->tagList);

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddPressed()));

    connect(d->tagList->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this,
            SLOT(slotSelectionChanged()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotTagDeleted(Album*)));

    restoreSettings();
    this->setLayout(layout);
}

TagList::~TagList()
{
    delete d->tagList;
    delete d->tagListModel;
    delete d;
}

void TagList::saveSettings()
{
    KConfig conf(QLatin1String("digikam_tagsmanagerrc"));
    conf.deleteGroup(QLatin1String("List Content"));

    KConfigGroup group = conf.group(QLatin1String("List Content"));

    QList<ListItem*> currentItems = d->tagListModel->allItems();
    group.writeEntry(QLatin1String("Size"), currentItems.count()-1);

    for (int it = 1; it < currentItems.size(); it++)
    {
        QList<int> ids = currentItems.at(it)->getTagIds();
        QString saveData;

        for (int jt = 0; jt < ids.size(); jt++)
        {
            saveData.append(QString::number(ids.at(jt)) + QLatin1String(" "));
        }

        group.writeEntry(QString::fromUtf8("item%1").arg(it-1), saveData);
    }
}

void TagList::restoreSettings()
{
    KConfig conf(QLatin1String("digikam_tagsmanagerrc"));
    KConfigGroup group = conf.group(QLatin1String("List Content"));
    QStringList items;

    int size = group.readEntry(QLatin1String("Size"), -1);

    /**
     * If config is empty add generic All Tags
     */
    d->tagListModel->addItem(QList<QVariant>() << QBrush(Qt::cyan, Qt::Dense2Pattern));

    if (size == 0 || size < 0)
    {
        return;
    }

    for(int it = 0; it < size; it++)
    {
        QString data = group.readEntry(QString::fromUtf8("item%1").arg(it), "");

        if(data.isEmpty())
            continue;

        QStringList ids = data.split(QLatin1String(" "), QString::SkipEmptyParts);
        QList<QVariant> itemData;
        itemData << QBrush(Qt::cyan, Qt::Dense2Pattern);

        foreach(const QString& tagId, ids)
        {
            TAlbum* const item = AlbumManager::instance()->findTAlbum(tagId.toInt());

            if(item)
            {
                itemData << item->id();
            }
        }

        ListItem* const listItem = d->tagListModel->addItem(itemData);

        /** Use this map to find all List Items that contain specific tag
         *  usually to remove deleted tag
         */
        foreach(int tagId, listItem->getTagIds())
        {
            d->tagMap[tagId].append(listItem);
        }
    }

    /** "All Tags" item should be selected **/
    QModelIndex rootIndex = d->tagList->model()->index(0,0);
    d->tagList->setCurrentIndex(rootIndex);
}

void TagList::slotAddPressed()
{
    QModelIndexList selected = d->treeView->selectionModel()->selectedIndexes();

    if (selected.isEmpty())
    {
        return;
    }

    QList<QVariant> itemData;
    itemData << QBrush(Qt::cyan, Qt::Dense2Pattern);

    foreach(const QModelIndex& index, selected)
    {
        TAlbum* const album = static_cast<TAlbum*>(d->treeView->albumForIndex(index));
        itemData << album->id();
    }

    ListItem* listItem = d->tagListModel->addItem(itemData);

    /** Use this map to find all List Items that contain specific tag
     *  usually to remove deleted tag
     */
    foreach(int tagId, listItem->getTagIds())
    {
        d->tagMap[tagId].append(listItem);
    }
}

void TagList::slotSelectionChanged()
{
    QModelIndexList indexList = d->tagList->mySelectedIndexes();
    QSet<int> mySet;

    foreach(const QModelIndex& index, indexList)
    {
        ListItem* const item = static_cast<ListItem*>(index.internalPointer());

        if (item->getTagIds().isEmpty())
        {
            mySet.clear();
            break;
        }

        foreach(int tagId, item->getTagIds())
        {
            mySet.insert(tagId);
        }
    }

    TagsManagerFilterModel* const filterModel = d->treeView->getFilterModel();
    filterModel->setQuickListTags(QList<int>::fromSet(mySet));
}

void TagList::slotTagDeleted(Album* album)
{
    TAlbum* const talbum = dynamic_cast<TAlbum*>(album);

    if (!talbum)
    {
        return;
    }

    int delId = talbum->id();

    QList<ListItem*> items = d->tagMap[delId];

    foreach(ListItem* const item, items)
    {
        item->removeTagId(delId);

        if (item->getTagIds().isEmpty())
        {
            d->tagListModel->deleteItem(item);
            d->tagMap[delId].removeOne(item);
            d->treeView->getFilterModel()->setQuickListTags(QList<int>());
        }
    }
}

void TagList::slotDeleteSelected()
{
    QModelIndexList sel = d->tagList->selectionModel()->selectedIndexes();

    if (sel.isEmpty())
    {
        return;
    }

    foreach(const QModelIndex& index, sel)
    {
        ListItem* const item = static_cast<ListItem*>(index.internalPointer());
        d->tagListModel->deleteItem(item);
    }

    d->tagList->selectionModel()->select(d->tagList->model()->index(0,0),
                                         QItemSelectionModel::SelectCurrent);

}

void TagList::enableAddButton(bool value)
{
    d->addButton->setEnabled(value);
}

} // namespace Digikam
