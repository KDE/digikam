/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ALBUMTREEVIEW_H
#define ALBUMTREEVIEW_H

// Qt includes

#include <QTreeView>

// Local includes

#include "albummodel.h"
#include "albumfiltermodel.h"

namespace Digikam
{

class AbstractAlbumTreeView : public QTreeView
{
    Q_OBJECT

public:

    AbstractAlbumTreeView(AbstractSpecificAlbumModel *model, QWidget *parent = 0);

    AbstractSpecificAlbumModel *albumModel() const;
    AlbumFilterModel *albumFilterModel() const;

public Q_SLOTS:

    void setSearchTextSettings(const SearchTextSettings &settings);

Q_SIGNALS:

    void filteringDone(bool atLeastOneMatch);

protected Q_SLOTS:

    void slotFilterChanged();
    virtual void slotRootAlbumAvailable();

protected:

    bool checkExpandedState(const QModelIndex &index);

    AbstractSpecificAlbumModel *m_albumModel;
    AlbumFilterModel           *m_albumFilterModel;
};

class AbstractCountingAlbumTreeView : public AbstractAlbumTreeView
{
    Q_OBJECT

public:

    AbstractCountingAlbumTreeView(AbstractCountingAlbumModel *model, QWidget *parent = 0);

private Q_SLOTS:

    void slotCollapsed(const QModelIndex &index);
    void slotExpanded(const QModelIndex &index);
    void slotSetShowCount();
    void slotRowsInserted(const QModelIndex &parent, int start, int end);
    void updateShowCountState(const QModelIndex &index, bool recurse);
};

class AlbumTreeView : public AbstractCountingAlbumTreeView
{
public:

    AlbumTreeView(QWidget *parent = 0);
};

class TagTreeView : public AbstractCountingAlbumTreeView
{
public:

    TagTreeView(QWidget *parent = 0);
};

class SearchTreeView : public AbstractAlbumTreeView
{
public:

    SearchTreeView(QWidget *parent = 0);
};

class DateAlbumTreeView : public AbstractCountingAlbumTreeView
{
public:

    DateAlbumTreeView(QWidget *parent = 0);
};



}

#endif

