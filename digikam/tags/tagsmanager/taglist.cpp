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
    d->addButton    = new QPushButton("(+)");
    d->tagList      = new TagMngrListView(this);
    d->tagListModel = new TagMngrListModel(QString("All Tags"));

    d->tagList->setModel(d->tagListModel);
    d->tagList->setSpacing(3);

    layout->addWidget(d->addButton);
    layout->addWidget(d->tagList);

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddPressed()));

    this->setLayout(layout);
}

TagList::~TagList()
{
    delete d;
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
