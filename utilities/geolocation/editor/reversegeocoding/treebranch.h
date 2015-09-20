/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-05-12
 * @brief  A model to hold information about image tags.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gabriel Voicu
 *         <a href="mailto:ping dot gabi at gmail dot com">ping dot gabi at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
