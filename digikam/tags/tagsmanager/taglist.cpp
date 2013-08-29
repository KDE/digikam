/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-07-31
 * Description : Tag List implementation as Quick Acess for various
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

#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVariant>

#include <kdebug.h>

#include "taglist.h"
#include "albumtreeview.h"
#include "tagmngrtreeview.h"
#include "tagmngrlistmodel.h"
#include "tagmngrlistview.h"
#include "tagmngrlistitem.h"

namespace Digikam {

class TagList::TagListPriv
{
public:
    TagListPriv()
    {
        addButton       = 0;
        tagList         = 0;
        tagListModel    = 0;
        treeView        = 0;
    }

    QPushButton*        addButton;
    TagMngrListView*    tagList;
    TagMngrListModel*   tagListModel;
    TagMngrTreeView*      treeView;
    QMap<int, QList<ListItem*> > tagMap;

};

TagList::TagList(TagMngrTreeView* treeView, QWidget* parent)
        : QWidget(parent), d(new TagListPriv())
{
    d->treeView     = treeView;

    QVBoxLayout* layout = new QVBoxLayout();
    d->addButton    = new QPushButton(i18n("Add to List"));
    d->addButton->setToolTip(i18n("Add selected tags to Quick Access List"));
    d->tagList      = new TagMngrListView(this);
    d->tagListModel = new TagMngrListModel();

    d->tagList->setModel(d->tagListModel);
    d->tagList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->tagList->setDragEnabled(true);
    d->tagList->setAcceptDrops(true);
    d->tagList->setDropIndicatorShown(true);
    d->tagList->setSpacing(3);

    layout->addWidget(d->addButton);
    layout->addWidget(d->tagList);

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddPressed()));

    connect(d->tagList->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,
            SLOT(slotSelectionChanged()));

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
    KConfig conf("digikam_tagsmanagerrc");
    conf.deleteGroup("List Content");

    KConfigGroup group = conf.group("List Content");

    QList<ListItem*> currentItems = d->tagListModel->allItems();
    group.writeEntry("Size", currentItems.count()-1);

    for(int it = 1; it < currentItems.size(); it++)
    {
        QList<int> ids = currentItems.at(it)->getTagIds();
        QString saveData;
        for(int jt = 0; jt < ids.size(); jt++)
        {
            saveData.append(QString::number(ids.at(jt)) + " ");
        }
        group.writeEntry(QString("item%1").arg(it-1),
                         saveData);
    }

}

void TagList::restoreSettings()
{
    KConfig conf("digikam_tagsmanagerrc");
    KConfigGroup group = conf.group("List Content");
    QStringList items;

    int size = group.readEntry("Size", -1);

    /**
     * If config is empty add generic All Tags
     */
    d->tagListModel->addItem(QList<QVariant>()
                             << QBrush(Qt::cyan, Qt::Dense2Pattern));
    if(size == 0)
    {
        return;
    }

    for(int it = 0; it < size; it++)
    {
        QString data = group.readEntry(QString("item%1").arg(it), "");
        if(data.isEmpty())
            continue;

        QStringList ids = data.split(" ", QString::SkipEmptyParts);
        TAlbum* item = AlbumManager::instance()->findTAlbum(ids.first().toInt());
        QList<QVariant> itemData;
        itemData << QBrush(Qt::cyan, Qt::Dense2Pattern);
        itemData << item->id();

        ListItem* listItem = d->tagListModel->addItem(itemData);
        d->tagMap[item->id()].append(listItem);
    }
    /** All Tags item should be selected **/
    QModelIndex rootIndex = d->tagList->model()->index(0,0);
    d->tagList->setCurrentIndex(rootIndex);
}

void TagList::slotAddPressed()
{
    QModelIndexList selected = d->treeView->selectionModel()->selectedIndexes();
    /**
     * Figure out how to implement multiple selection
     */
    if(selected.isEmpty())
        return;

    TAlbum* album = static_cast<TAlbum*>(d->treeView->albumForIndex(selected.first()));

    QList<QVariant> itemData;
    itemData << QBrush(Qt::cyan, Qt::Dense2Pattern);
    itemData << album->id();
    d->tagListModel->addItem(itemData);

}

void TagList::slotSelectionChanged()
{
    QModelIndex index = d->tagList->currentIndex();

    ListItem* item = static_cast<ListItem*>(index.internalPointer());

    TagsManagerFilterModel* filterModel = d->treeView->getFilterModel();

    filterModel->setQuickListTags(item->getTagIds());

}

}
