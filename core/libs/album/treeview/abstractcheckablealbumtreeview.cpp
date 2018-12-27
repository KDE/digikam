/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * Copyright (C) 2014      by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "abstractcheckablealbumtreeview.h"
#include "abstractalbumtreeview_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN AbstractCheckableAlbumTreeView::Private
{
public:

    explicit Private()
    {
    }

    static const QString configCheckedAlbumsEntry;
    static const QString configPartiallyCheckedAlbumsEntry;
    static const QString configRestoreCheckedEntry;

    QList<int>           checkedAlbumIds;
    QList<int>           partiallyCheckedAlbumIds;
};

const QString AbstractCheckableAlbumTreeView::Private::configCheckedAlbumsEntry(QLatin1String("Checked"));
const QString AbstractCheckableAlbumTreeView::Private::configPartiallyCheckedAlbumsEntry(QLatin1String("PartiallyChecked"));
const QString AbstractCheckableAlbumTreeView::Private::configRestoreCheckedEntry(QLatin1String("RestoreChecked"));

// --------------------------------------------------------

AbstractCheckableAlbumTreeView::AbstractCheckableAlbumTreeView(QWidget* const parent, Flags flags)
    : AbstractCountingAlbumTreeView(parent, flags & ~CreateDefaultFilterModel),
      d(new Private)
{
    m_checkOnMiddleClick = true;
    m_restoreCheckState  = false;

    if (flags & CreateDefaultFilterModel)
    {
        setAlbumFilterModel(new CheckableAlbumFilterModel(this));
    }
}

AbstractCheckableAlbumTreeView::~AbstractCheckableAlbumTreeView()
{
    delete d;
}

AbstractCheckableAlbumModel* AbstractCheckableAlbumTreeView::albumModel() const
{
    return dynamic_cast<AbstractCheckableAlbumModel*>(m_albumModel);
}

CheckableAlbumFilterModel* AbstractCheckableAlbumTreeView::albumFilterModel() const
{
    return dynamic_cast<CheckableAlbumFilterModel*> (m_albumFilterModel);
}

void AbstractCheckableAlbumTreeView::setCheckOnMiddleClick(bool doThat)
{
    m_checkOnMiddleClick = doThat;
}

void AbstractCheckableAlbumTreeView::middleButtonPressed(Album* a)
{
    AbstractCheckableAlbumModel* const model = static_cast<AbstractCheckableAlbumModel*>(m_albumModel);

    if (!model)
    {
        return;
    }

    if (model->isCheckable())
    {
        if (model->isTristate())
        {
            switch (model->checkState(a))
            {
                case Qt::Unchecked:
                    model->setCheckState(a, Qt::PartiallyChecked);
                    break;
                case Qt::PartiallyChecked:
                    model->setCheckState(a, Qt::Checked);
                    break;
                case Qt::Checked:
                    model->setCheckState(a, Qt::Unchecked);
                    break;
            }
        }
        else
        {
            model->toggleChecked(a);
        }
    }
}

bool AbstractCheckableAlbumTreeView::isRestoreCheckState() const
{
    return m_restoreCheckState;
}

void AbstractCheckableAlbumTreeView::setRestoreCheckState(bool restore)
{
    m_restoreCheckState = restore;
}

void AbstractCheckableAlbumTreeView::doLoadState()
{
    AbstractCountingAlbumTreeView::doLoadState();

    KConfigGroup group = getConfigGroup();

    if (!m_restoreCheckState)
    {
        m_restoreCheckState = group.readEntry(entryName(d->configRestoreCheckedEntry), false);
    }

    if (!m_restoreCheckState || !checkableModel()->isCheckable())
    {
        return;
    }

    const QStringList checkedAlbums = group.readEntry(entryName(d->configCheckedAlbumsEntry), QStringList());

    d->checkedAlbumIds.clear();

    foreach (const QString& albumId, checkedAlbums)
    {
        bool ok;
        const int id = albumId.toInt(&ok);

        if (ok)
        {
            d->checkedAlbumIds << id;
        }
    }

    const QStringList partiallyCheckedAlbums = group.readEntry(entryName(d->configPartiallyCheckedAlbumsEntry), QStringList());
    d->partiallyCheckedAlbumIds.clear();

    foreach (const QString& albumId, partiallyCheckedAlbums)
    {
        bool ok;
        const int id = albumId.toInt(&ok);

        if (ok)
        {
            d->partiallyCheckedAlbumIds << id;
        }
    }

    // initially sync with the albums that are already in the model
    restoreCheckStateForHierarchy(QModelIndex());
    horizontalScrollBar()->setValue(0);
}

void AbstractCheckableAlbumTreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    AbstractCountingAlbumTreeView::rowsInserted(parent, start, end);

    if (!d->checkedAlbumIds.isEmpty())
    {
        for (int i = start ; i <= end ; ++i)
        {
            const QModelIndex child = checkableModel()->index(i, 0, parent);
            restoreCheckState(child);
        }
    }
}

void AbstractCheckableAlbumTreeView::restoreCheckStateForHierarchy(const QModelIndex& index)
{
    // recurse children
    for (int i = 0 ; i < checkableModel()->rowCount(index) ; ++i)
    {
        const QModelIndex child = checkableModel()->index(i, 0, index);
        restoreCheckState(child);
        restoreCheckStateForHierarchy(child);
    }
}

void AbstractCheckableAlbumTreeView::restoreCheckState(const QModelIndex& index)
{
    Album* const album = checkableModel()->albumForIndex(index);

    if (!album || !(album->id()))
    {
        return;
    }

    if (d->checkedAlbumIds.contains(album->id()))
    {
        checkableModel()->setCheckState(album, Qt::Checked);
        d->checkedAlbumIds.removeOne(album->id());
    }

    if (d->partiallyCheckedAlbumIds.contains(album->id()))
    {
        checkableModel()->setCheckState(album, Qt::PartiallyChecked);
        d->partiallyCheckedAlbumIds.removeOne(album->id());
    }
}

void AbstractCheckableAlbumTreeView::doSaveState()
{
    AbstractCountingAlbumTreeView::doSaveState();

    KConfigGroup group = getConfigGroup();

    group.writeEntry(entryName(d->configRestoreCheckedEntry), m_restoreCheckState);

    if (!m_restoreCheckState || !checkableModel()->isCheckable())
    {
        return;
    }

    const QList<Album*> checkedAlbums = checkableModel()->checkedAlbums();
    QStringList checkedIds;

    foreach (Album* const album, checkedAlbums)
    {
        checkedIds << QString::number(album->id());
    }

    group.writeEntry(entryName(d->configCheckedAlbumsEntry), checkedIds);

    if (!checkableModel()->isTristate())
    {
        return;
    }

    const QList<Album*> partiallyCheckedAlbums = checkableModel()->partiallyCheckedAlbums();
    QStringList partiallyCheckedIds;

    foreach (Album* const album, partiallyCheckedAlbums)
    {
        partiallyCheckedIds << QString::number(album->id());
    }

    group.writeEntry(entryName(d->configPartiallyCheckedAlbumsEntry), partiallyCheckedIds);
}

} // namespace Digikam
