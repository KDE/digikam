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
#include "tagfolderview.h"
#include "tagmngrlistmodel.h"
#include "tagmngrlistview.h"
#include <tagmngrlistitem.h>

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
    TagFolderView*      treeView;

};

TagList::TagList(TagFolderView* treeView, QWidget* parent)
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
    group.writeEntry("Size", currentItems.count());

    for(int it = 0; it < currentItems.size(); it++)
    {
        group.writeEntry(QString("item%1").arg(it),
                         currentItems.at(it)->data(Qt::DisplayRole));
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
    if(size < 2)
    {
        d->tagListModel->addItem(QVariant("All Tags"));
        return;
    }

    for(int it = 0; it < size; it++)
    {
        QString data = group.readEntry(QString("item%1").arg(it), "");
        if(data.isEmpty())
            continue;

        d->tagListModel->addItem(QVariant(data));
    }
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

    d->tagListModel->addItem(QVariant(album->title()));

}

}
