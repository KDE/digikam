/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ABSTRACT_COUNTING_ALBUM_TREE_VIEW_H
#define DIGIKAM_ABSTRACT_COUNTING_ALBUM_TREE_VIEW_H

// Qt includes

#include <QTreeView>

// Local includes

#include "abstractalbumtreeview.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "albumpointer.h"
#include "statesavingobject.h"

namespace Digikam
{

class AbstractCountingAlbumTreeView : public AbstractAlbumTreeView
{
    Q_OBJECT

public:

    explicit AbstractCountingAlbumTreeView(QWidget* const parent, Flags flags);

protected:

    void setAlbumModel(AbstractCountingAlbumModel* const model);
    void setAlbumFilterModel(AlbumFilterModel* const filterModel);

    virtual void rowsInserted(const QModelIndex& parent, int start, int end);

private Q_SLOTS:

    void slotCollapsed(const QModelIndex& index);
    void slotExpanded(const QModelIndex& index);
    void setShowCountFromSettings();
    void updateShowCountState(const QModelIndex& index, bool recurse);

private:

    void init();
};

} // namespace Digikam

#endif // DIGIKAM_ABSTRACT_COUNTING_ALBUM_TREE_VIEW_H
