/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-23
 * Description : Qt Model for Albums - abstract base classes private header
 *
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ABSTRACTALBUMMODELPRIVATE_H
#define ABSTRACTALBUMMODELPRIVATE_H

namespace Digikam
{

class AbstractAlbumModel::Private
{
public:

    Private()
    {
        rootAlbum       = 0;
        type            = Album::PHYSICAL;
        rootBehavior    = AbstractAlbumModel::IncludeRootAlbum;
        addingAlbum     = 0;
        removingAlbum   = 0;
        itemDrag        = true;
        itemDrop        = true;
        dragDropHandler = 0;
    }

    Album*                                rootAlbum;
    Album::Type                           type;
    AbstractAlbumModel::RootAlbumBehavior rootBehavior;
    bool                                  itemDrag;
    bool                                  itemDrop;
    AlbumModelDragDropHandler*            dragDropHandler;

    Album*                                addingAlbum;
    quintptr                              removingAlbum;

public:

    Album* findNthChild(Album* const parent, int n) const
    {
        // return the n-th of the children of parent, or 0
        Album* a = parent->firstChild();

        if (!a)
        {
            return 0;
        }

        for (int i = 0 ; i < n ; ++i)
        {
            a = a->next();

            if (!a)
            {
                return 0;
            }
        }

        return a;
    }

    int findIndexAsChild(Album* const child) const
    {
        // return index of child in the list of children of its parent
        Album* const parent = child->parent();

        if (!parent)
        {
            return 0;
        }

        Album* a = parent->firstChild();
        int i    = 0;

        while (a != child)
        {
            a = a->next();

            if (!a)
            {
                return -1;
            }

            ++i;
        }

        return i;
    }

    int numberOfChildren(Album* const parent) const
    {
        Album* a  = parent->firstChild();
        int count = 0;

        while (a)
        {
            ++count;
            a = a->next();
        }

        return count;
    }
};

} // namespace Digikam

#endif // ABSTRACTALBUMMODELPRIVATE_H
