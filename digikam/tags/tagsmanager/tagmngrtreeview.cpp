/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-08-05
 * Description : Tag Manager Tree View derived from TagsFolderView to implement
 *               a custom context menu and some batch view options, such as
 *               expanding multiple items
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

#include <QModelIndex>

#include "tagmngrtreeview.h"

namespace Digikam {

class TagMngrTreeView::TagMngrTreeViewPriv
{

public:
    TagMngrTreeViewPriv()
    {
        expandLevel = 0;
    }

    int expandLevel;
};

TagMngrTreeView::TagMngrTreeView(QWidget* parent, TagModel* model)
                : TagFolderView(parent, model), d(new TagMngrTreeViewPriv())
{

}

TagMngrTreeView::~TagMngrTreeView()
{

}

void TagMngrTreeView::slotExpandSelected()
{
    QModelIndexList list = selectionModel()->selectedIndexes();
    foreach(QModelIndex index, list)
    {
        expand(index);
    }
}

void TagMngrTreeView::slotExpandTree()
{
    d->expandLevel++;
    expandToDepth(d->expandLevel);
}

} // namespace Digikam