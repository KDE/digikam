/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : A model to hold information about image tags.
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
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

#ifndef TREEBRANCH_H
#define TREEBRANCH_H

// Qt includes

#include <QPersistentModelIndex>
#include <QList>

namespace Digikam
{

class TreeBranch
{
public:

    TreeBranch()
        : sourceIndex(),
          parent(0),
          data(),
          type(),
          oldChildren(),
          spacerChildren()
    {
    }

    ~TreeBranch()
    {
        qDeleteAll(oldChildren);
    }

public:

    QPersistentModelIndex sourceIndex;
    TreeBranch*           parent;
    QString               data;
    Type                  type;
    QList<TreeBranch*>    oldChildren;
    QList<TreeBranch*>    spacerChildren;
    QList<TreeBranch*>    newChildren;
};

} // namespace Digikam

#endif // TREEBRANCH_H
