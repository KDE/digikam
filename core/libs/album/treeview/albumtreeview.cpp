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

#include "albumtreeview.h"
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

// ----------------------------------------------------------------------------------------------------

AlbumTreeView::AlbumTreeView(QWidget* const parent, Flags flags)
    : AbstractCheckableAlbumTreeView(parent, flags)
{
    setRootIsDecorated(false);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setAutoExpandDelay(AUTOEXPANDDELAY);

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new AlbumModel(AlbumModel::IncludeRootAlbum, this));
    }
}

AlbumTreeView::~AlbumTreeView()
{
}

void AlbumTreeView::setAlbumModel(AlbumModel* const model)
{
    // changing model is not implemented
    if (m_albumModel)
    {
        return;
    }

    AbstractCheckableAlbumTreeView::setAlbumModel(model);

    m_dragDropHandler = albumModel()->dragDropHandler();

    if (!m_dragDropHandler)
    {
        m_dragDropHandler = new AlbumDragDropHandler(albumModel());

        model->setDragDropHandler(m_dragDropHandler);
    }
}

void AlbumTreeView::setAlbumFilterModel(CheckableAlbumFilterModel* const filterModel)
{
    AbstractCheckableAlbumTreeView::setAlbumFilterModel(filterModel);
}

AlbumModel* AlbumTreeView::albumModel() const
{
    return dynamic_cast<AlbumModel*>(m_albumModel);
}

PAlbum* AlbumTreeView::currentAlbum() const
{
    return dynamic_cast<PAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

PAlbum* AlbumTreeView::albumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<PAlbum*> (m_albumFilterModel->albumForIndex(index));
}

void AlbumTreeView::setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager)
{
    AbstractCheckableAlbumTreeView::setCurrentAlbums(albums, selectInAlbumManager);
}

void AlbumTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    PAlbum* const album = AlbumManager::instance()->findPAlbum(albumId);
    setCurrentAlbums(QList<Album*>() << album, selectInAlbumManager);
}

// -------------------------------------------------------------------------------------------------------

TagTreeView::TagTreeView(QWidget* const parent, Flags flags)
    : AbstractCheckableAlbumTreeView(parent, flags),
      m_filteredModel(0)
{
    m_modificationHelper = new TagModificationHelper(this, this);
    setRootIsDecorated(true);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setAutoExpandDelay(AUTOEXPANDDELAY);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new TagModel(TagModel::IncludeRootAlbum, this));
    }

    if (flags & CreateDefaultFilterModel) // must set again!
    {
        setAlbumFilterModel(new TagPropertiesFilterModel(this), albumFilterModel());
    }
}

TagTreeView::~TagTreeView()
{
}

void TagTreeView::setAlbumFilterModel(TagPropertiesFilterModel* const filteredModel, CheckableAlbumFilterModel* const filterModel)
{
    m_filteredModel = filteredModel;
    AbstractCheckableAlbumTreeView::setAlbumFilterModel(filterModel);
    // hook in: source album model -> filtered model -> album filter model
    albumFilterModel()->setSourceFilterModel(m_filteredModel);
}

void TagTreeView::setAlbumModel(TagModel* const model)
{
    // changing model is not implemented
    if (m_albumModel)
    {
        return;
    }

    AbstractCheckableAlbumTreeView::setAlbumModel(model);

    if (m_filteredModel)
    {
        m_filteredModel->setSourceAlbumModel(model);
    }

    m_dragDropHandler = albumModel()->dragDropHandler();

    if (!m_dragDropHandler)
    {
        m_dragDropHandler = new TagDragDropHandler(albumModel());
        albumModel()->setDragDropHandler(m_dragDropHandler);

        connect(albumModel()->dragDropHandler(), SIGNAL(assignTags(QList<qlonglong>,QList<int>)),
                FileActionMngr::instance(), SLOT(assignTags(QList<qlonglong>,QList<int>)));
    }

    if (m_albumModel && (m_albumModel->rootAlbumBehavior() == AbstractAlbumModel::IncludeRootAlbum))
    {
        setRootIsDecorated(false);
    }

    if (m_albumFilterModel)
    {
        expand(m_albumFilterModel->rootAlbumIndex());
    }
}

TagModel* TagTreeView::albumModel() const
{
    return static_cast<TagModel*>(m_albumModel);
}

TagPropertiesFilterModel* TagTreeView::filteredModel() const
{
    return m_filteredModel;
}

TAlbum* TagTreeView::currentAlbum() const
{
    return dynamic_cast<TAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

QList<Album*> TagTreeView::selectedTags()
{
    return selectedAlbums<Album>(selectionModel(),m_filteredModel);
}

QList<TAlbum*> TagTreeView::selectedTagAlbums()
{
    return selectedAlbums<TAlbum>(selectionModel(),m_filteredModel);
}

TAlbum* TagTreeView::albumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<TAlbum*> (m_albumFilterModel->albumForIndex(index));
}

TagModificationHelper* TagTreeView::tagModificationHelper() const
{
    return m_modificationHelper;
}

void TagTreeView::setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager)
{
    AbstractCheckableAlbumTreeView::setCurrentAlbums(albums, selectInAlbumManager);
}

void TagTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    TAlbum* const album = AlbumManager::instance()->findTAlbum(albumId);
    setCurrentAlbums(QList<Album*>() << album, selectInAlbumManager);
}

// --------------------------------------------------------------------------------------

SearchTreeView::SearchTreeView(QWidget* const parent, Flags flags)
    : AbstractCheckableAlbumTreeView(parent, flags),
      m_filteredModel(0)
{
    setRootIsDecorated(false);

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new SearchModel(this));
    }

    if (flags & CreateDefaultFilterModel) // must set again!
    {
        setAlbumFilterModel(new SearchFilterModel(this), albumFilterModel());
    }
}

SearchTreeView::~SearchTreeView()
{
}

void SearchTreeView::setAlbumModel(SearchModel* const model)
{
    AbstractCheckableAlbumTreeView::setAlbumModel(model);

    if (m_filteredModel)
    {
        m_filteredModel->setSourceSearchModel(model);
    }
}

SearchModel* SearchTreeView::albumModel() const
{
    return static_cast<SearchModel*>(m_albumModel);
}

void SearchTreeView::setAlbumFilterModel(SearchFilterModel* const filteredModel, CheckableAlbumFilterModel* const filterModel)
{
    m_filteredModel = filteredModel;
    AbstractCheckableAlbumTreeView::setAlbumFilterModel(filterModel);
    // hook in: source album model -> filtered model -> album filter model
    albumFilterModel()->setSourceFilterModel(m_filteredModel);
}

SearchFilterModel* SearchTreeView::filteredModel() const
{
    return m_filteredModel;
}

SAlbum* SearchTreeView::currentAlbum() const
{
    return dynamic_cast<SAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

void SearchTreeView::setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager)
{
    AbstractCheckableAlbumTreeView::setCurrentAlbums(albums, selectInAlbumManager);
}

void SearchTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    SAlbum* const album = AlbumManager::instance()->findSAlbum(albumId);
    setCurrentAlbums(QList<Album*>() << album, selectInAlbumManager);
}

// ---------------------------------------------------------------------------------------------------

DateAlbumTreeView::DateAlbumTreeView(QWidget* const parent, Flags flags)
    : AbstractCountingAlbumTreeView(parent, flags)
{
    // this view should always show the inclusive counts
    disconnect(this, SIGNAL(expanded(QModelIndex)),
               this, SLOT(slotExpanded(QModelIndex)));

    disconnect(this, SIGNAL(collapsed(QModelIndex)),
               this, SLOT(slotCollapsed(QModelIndex)));

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new DateAlbumModel(this));
    }
}

void DateAlbumTreeView::setAlbumModel(DateAlbumModel* const model)
{
    AbstractCountingAlbumTreeView::setAlbumModel(model);
}

DateAlbumModel* DateAlbumTreeView::albumModel() const
{
    return static_cast<DateAlbumModel*>(m_albumModel);
}

void DateAlbumTreeView::setAlbumFilterModel(AlbumFilterModel* const filterModel)
{
    AbstractCountingAlbumTreeView::setAlbumFilterModel(filterModel);
}

DAlbum* DateAlbumTreeView::currentAlbum() const
{
    return dynamic_cast<DAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

DAlbum* DateAlbumTreeView::albumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<DAlbum*> (m_albumFilterModel->albumForIndex(index));
}

void DateAlbumTreeView::setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager)
{
    AbstractCountingAlbumTreeView::setCurrentAlbums(albums, selectInAlbumManager);
}

void DateAlbumTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    DAlbum* const album = AlbumManager::instance()->findDAlbum(albumId);
    setCurrentAlbums(QList<Album*>() << album, selectInAlbumManager);
}

} // namespace Digikam
