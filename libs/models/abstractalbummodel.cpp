/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-23
 * Description : Qt Model for Albums
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// #include "abstractalbummodel.h"
#include "abstractalbummodel.moc"
#include "abstractalbummodelpriv.h"

// KDE includes

#include <klocale.h>
#include <kglobal.h>

// Local includes

#include "albummanager.h"
#include "albumthumbnailloader.h"
#include "albummodeldragdrophandler.h"

namespace Digikam
{

AbstractAlbumModel::AbstractAlbumModel(Album::Type albumType, Album *rootAlbum, RootAlbumBehavior rootBehavior,
                                       QObject *parent)
                  : QAbstractItemModel(parent), d(new AlbumModelPriv)
{
    d->type = albumType;
    d->rootAlbum = rootAlbum;
    d->rootBehavior = rootBehavior;

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAboutToBeAdded(Album*, Album*, Album*)),
            this, SLOT(slotAlbumAboutToBeAdded(Album*, Album*, Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAboutToBeDeleted(Album*)),
            this, SLOT(slotAlbumAboutToBeDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumHasBeenDeleted(void *)),
            this, SLOT(slotAlbumHasBeenDeleted(void *)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));
}

AbstractAlbumModel::~AbstractAlbumModel()
{
    delete d;
}

QVariant AbstractAlbumModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Album *a = static_cast<Album*>(index.internalPointer());

    return albumData(a, role);
}

QVariant AbstractAlbumModel::albumData(Album *a, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
            return a->title();
        case Qt::ToolTipRole:
            return a->title();
        case Qt::DecorationRole:
            // reimplement in subclasses
            return decorationRole(a);
        case AlbumTypeRole:
            return a->type();
        case AlbumPointerRole:
            return QVariant::fromValue(a);
    }
    return QVariant();
}

QVariant AbstractAlbumModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    if (section == 0 && role == Qt::DisplayRole)
        return columnHeader();
    return QVariant();
}

int AbstractAlbumModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        Album *a = static_cast<Album*>(parent.internalPointer());
        return d->numberOfChildren(a);
    }
    else
    {
        if (!d->rootAlbum)
            return 0;

        if (d->rootBehavior == IncludeRootAlbum)
            return 1;
        else
            return d->numberOfChildren(d->rootAlbum);
    }
}

int AbstractAlbumModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

Qt::ItemFlags AbstractAlbumModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    Album *a = static_cast<Album*>(index.internalPointer());
    return itemFlags(a);
}

bool AbstractAlbumModel::hasChildren(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        Album *a = static_cast<Album*>(parent.internalPointer());
        return a->firstChild();
    }
    else
    {
        if (!d->rootAlbum)
            return false;

        if (d->rootBehavior == IncludeRootAlbum)
            return 1;
        else
            return d->rootAlbum->firstChild();
    }
}

QModelIndex AbstractAlbumModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column != 0)
        return QModelIndex();

    if (parent.isValid())
    {
        Album *parentAlbum = static_cast<Album*>(parent.internalPointer());
        Album *a = d->findNthChild(parentAlbum, row);
        if (a)
            return createIndex(row, column, a);
    }
    else
    {
        if (!d->rootAlbum)
            return QModelIndex();

        if (d->rootBehavior == IncludeRootAlbum)
        {
            if (row == 0)
                return createIndex(0, 0, d->rootAlbum);
        }
        else
        {
            Album *a = d->findNthChild(d->rootAlbum, row);
            if (a)
                return createIndex(row, column, a);
        }
    }
    return QModelIndex();
}

QModelIndex AbstractAlbumModel::parent(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Album *a = static_cast<Album*>(index.internalPointer());
        return indexForAlbum(a->parent());
    }
    return QModelIndex();
}

Qt::DropActions AbstractAlbumModel::supportedDropActions() const
{
    return Qt::CopyAction|Qt::MoveAction;
}

QStringList AbstractAlbumModel::mimeTypes() const
{
    if (d->dragDropHandler)
        return d->dragDropHandler->mimeTypes();
    return QStringList();
}

bool AbstractAlbumModel::dropMimeData(const QMimeData *, Qt::DropAction, int, int, const QModelIndex &)
{
    // we require custom solutions
    return false;
}

QMimeData *AbstractAlbumModel::mimeData(const QModelIndexList& indexes) const
{
    if (!d->dragDropHandler)
        return 0;

    QList<Album*> albums;
    foreach (const QModelIndex& index, indexes)
    {
        Album *a = albumForIndex(index);
        if (a)
            albums << a;
    }
    return d->dragDropHandler->createMimeData(albums);
}

void AbstractAlbumModel::setEnableDrag(bool enable)
{
    d->itemDrag = enable;
}

void AbstractAlbumModel::setEnableDrop(bool enable)
{
    d->itemDrop = enable;
}

void AbstractAlbumModel::setDragDropHandler(AlbumModelDragDropHandler *handler)
{
    d->dragDropHandler = handler;
}

AlbumModelDragDropHandler *AbstractAlbumModel::dragDropHandler() const
{
    return d->dragDropHandler;
}

QModelIndex AbstractAlbumModel::indexForAlbum(Album *a) const
{
    if (!a)
        return QModelIndex();

    if (!filterAlbum(a))
        return QModelIndex();

    // a is root album? Decide on root behavior
    if (a == d->rootAlbum)
    {
        if (d->rootBehavior == IncludeRootAlbum)
            // create top-level index
            return createIndex(0, 0, a);
        else
            // with this behavior, root album has no valid index
            return QModelIndex();
    }

    // Normal album. Get its row.
    int row = d->findIndexAsChild(a);
    return createIndex(row, 0, a);
}

Album *AbstractAlbumModel::albumForIndex(const QModelIndex& index) const
{
    return static_cast<Album*>(index.internalPointer());
}

Album *AbstractAlbumModel::rootAlbum() const
{
    return d->rootAlbum;
}

QModelIndex AbstractAlbumModel::rootAlbumIndex() const
{
    return indexForAlbum(d->rootAlbum);
}

Album::Type AbstractAlbumModel::albumType() const
{
    return d->type;
}

QVariant AbstractAlbumModel::decorationRole(Album *) const
{
    return QVariant();
}

QString AbstractAlbumModel::columnHeader() const
{
    return i18n("Album");
}

Qt::ItemFlags AbstractAlbumModel::itemFlags(Album *) const
{
    Qt::ItemFlags f = Qt::ItemIsSelectable|Qt::ItemIsEnabled;
    if (d->itemDrag)
        f |= Qt::ItemIsDragEnabled;
    if (d->itemDrop)
        f |= Qt::ItemIsDropEnabled;
    return f;
}

bool AbstractAlbumModel::filterAlbum(Album *album) const
{
    return album && album->type() == d->type;
}

void AbstractAlbumModel::slotAlbumAboutToBeAdded(Album *album, Album *parent, Album *prev)
{
    if (!filterAlbum(album))
        return;

    // start inserting operation
    int row = prev ? d->findIndexAsChild(prev)+1 : 0;
    QModelIndex parentIndex = indexForAlbum(parent);
    beginInsertRows(parentIndex, row, row);

    // The root album will become available in time
    // when the model is instantiated before albums are initialized.
    // Set d->rootAlbum only after
    if (album->isRoot() && !d->rootAlbum)
        d->rootAlbum = album;

    // store album for slotAlbumAdded
    d->addingAlbum = album;
}

void AbstractAlbumModel::slotAlbumAdded(Album *album)
{
    if (d->addingAlbum == album)
    {

        bool isRoot = d->addingAlbum == d->rootAlbum;
        d->addingAlbum = 0;
        endInsertRows();
        if (isRoot)
            emit rootAlbumAvailable();
    }
}

void AbstractAlbumModel::slotAlbumAboutToBeDeleted(Album *album)
{
    if (!filterAlbum(album))
        return;

    // begin removing operation
    int row = d->findIndexAsChild(album);
    QModelIndex parent = indexForAlbum(album->parent());
    albumCleared(album);
    beginRemoveRows(parent, row, row);

    // store album for slotAlbumHasBeenDeleted
    d->removingAlbum = album;
}

void AbstractAlbumModel::slotAlbumHasBeenDeleted(void *p)
{
    if (d->removingAlbum == p)
    {
        d->removingAlbum = 0;
        endRemoveRows();
    }
}

void AbstractAlbumModel::slotAlbumsCleared()
{
    d->rootAlbum = 0;
    reset();
    allAlbumsCleared();
}

void AbstractAlbumModel::slotAlbumIconChanged(Album* album)
{
    QModelIndex index = indexForAlbum(album);
    emit dataChanged(index, index);
}

void AbstractAlbumModel::slotAlbumRenamed(Album *album)
{
    QModelIndex index = indexForAlbum(album);
    emit dataChanged(index, index);
}

// ------------------------------------------------------------------

AbstractSpecificAlbumModel::AbstractSpecificAlbumModel(Album::Type albumType,
                                                       Album *rootAlbum,
                                                       RootAlbumBehavior rootBehavior,
                                                       QObject *parent)
                          : AbstractAlbumModel(albumType, rootAlbum, rootBehavior, parent)
{
}

void AbstractSpecificAlbumModel::setupThumbnailLoading()
{
    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
            this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));

    connect(loader, SIGNAL(signalFailed(Album *)),
            this, SLOT(slotThumbnailLost(Album *)));

    connect(loader, SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotReloadThumbnails()));
}

QString AbstractSpecificAlbumModel::columnHeader() const
{
    return m_columnHeader;
}

void AbstractSpecificAlbumModel::setColumnHeader(const QString& header)
{
    m_columnHeader = header;
    emit headerDataChanged(Qt::Horizontal, 0, 0);
}

void AbstractSpecificAlbumModel::slotGotThumbnailFromIcon(Album *album, const QPixmap&)
{
    // see decorationRole() method of subclasses

    if (!filterAlbum(album))
        return;

    QModelIndex index = indexForAlbum(album);
    emit dataChanged(index, index);
}

void AbstractSpecificAlbumModel::slotThumbnailLost(Album *)
{
    // ignore, use default thumbnail
}

void AbstractSpecificAlbumModel::slotReloadThumbnails()
{
    // emit dataChanged() for all albums
    emitDataChangedForChildren(rootAlbum());
}

void AbstractSpecificAlbumModel::emitDataChangedForChildren(Album *album)
{
    if (!album)
        return;

    for (Album *child = album->firstChild(); child; child = child->next())
    {
        if (filterAlbum(child))
        {
            // recurse to children of children
            emitDataChangedForChildren(child);

            // emit signal for child
            QModelIndex index = indexForAlbum(child);
            emit dataChanged(index, index);
        }
    }
}

// ------------------------------------------------------------------

AbstractCountingAlbumModel::AbstractCountingAlbumModel(Album::Type albumType, Album *rootAlbum,
                                                       RootAlbumBehavior rootBehavior,
                                                       QObject *parent)
                           : AbstractSpecificAlbumModel(albumType, rootAlbum, rootBehavior, parent)
{
    m_showCount = false;
}

void AbstractCountingAlbumModel::setShowCount(bool show)
{
    if (m_showCount != show)
    {
        m_showCount = show;
        emitDataChangedForChildren(rootAlbum());
    }
}

bool AbstractCountingAlbumModel::showCount() const
{
    return m_showCount;
}

void AbstractCountingAlbumModel::excludeChildrenCount(const QModelIndex& index)
{
    Album *album = albumForIndex(index);
    if (!album)
        return;
    m_includeChildrenAlbums.remove(album->id());
    updateCount(album);
}

void AbstractCountingAlbumModel::includeChildrenCount(const QModelIndex& index)
{
    Album *album = albumForIndex(index);
    if (!album)
        return;
    m_includeChildrenAlbums << album->id();
    updateCount(album);
}

void AbstractCountingAlbumModel::setCountMap(const QMap<int, int>& idCountMap)
{
    m_countMap = idCountMap;
    QMap<int, int>::const_iterator it = m_countMap.constBegin();
    for (; it != m_countMap.constEnd(); ++it)
        updateCount(albumForId(it.key()));
}

void AbstractCountingAlbumModel::updateCount(Album *album)
{
    if (!album)
        return;

    QHash<int, int>::iterator includeIt = m_countHashReady.find(album->id());
    bool changed = false;

    // get count for album without children
    int count = m_countMap.value(album->id());

    // if wanted, add up children's counts
    if (m_includeChildrenAlbums.contains(album->id()))
    {
        AlbumIterator it(album);
        while ( it.current() )
        {
            count += m_countMap.value((*it)->id());
            ++it;
        }
    }

    // insert or update
    if (includeIt == m_countHashReady.end())
    {
        changed = true;
        m_countHashReady[album->id()] = count;
    }
    else
    {
        changed = (includeIt.value() != count);
        includeIt.value() = count;
    }

    // notify views
    if (changed)
    {
        QModelIndex index = indexForAlbum(album);
        emit dataChanged(index, index);
    }
}

void AbstractCountingAlbumModel::setCount(Album *album, int count)
{
    if (!album)
        return;

    QHash<int, int>::iterator includeIt = m_countHashReady.find(album->id());
    bool changed = false;

    // insert or update
    if (includeIt == m_countHashReady.end())
    {
        changed = true;
        m_countHashReady[album->id()] = count;
    }
    else
    {
        changed = (includeIt.value() != count);
        includeIt.value() = count;
    }

    // notify views
    if (changed)
    {
        QModelIndex index = indexForAlbum(album);
        emit dataChanged(index, index);
    }
}

QVariant AbstractCountingAlbumModel::albumData(Album *album, int role) const
{
    if (role == Qt::DisplayRole && m_showCount && !album->isRoot())
    {
        QHash<int, int>::const_iterator it = m_countHashReady.constFind(album->id());
        if (it != m_countHashReady.constEnd())
            return QString("%1 (%2)").arg(albumName(album)).arg(it.value());
    }
    return AbstractSpecificAlbumModel::albumData(album, role);
}

QString AbstractCountingAlbumModel::albumName(Album *album) const
{
    return album->title();
}

void AbstractCountingAlbumModel::albumCleared(Album *album)
{
    m_countMap.remove(album->id());
    m_countHashReady.remove(album->id());
    m_includeChildrenAlbums.remove(album->id());
}

void AbstractCountingAlbumModel::allAlbumsCleared()
{
    m_countMap.clear();
    m_countHashReady.clear();
    m_includeChildrenAlbums.clear();
}

// ------------------------------------------------------------------

AbstractCheckableAlbumModel::AbstractCheckableAlbumModel(Album::Type albumType, Album *rootAlbum,
                                                         RootAlbumBehavior rootBehavior,
                                                         QObject *parent)
                           : AbstractCountingAlbumModel(albumType, rootAlbum, rootBehavior, parent)
{
    m_extraFlags = 0;
}

void AbstractCheckableAlbumModel::setCheckable(bool isCheckable)
{
    if (isCheckable)
        m_extraFlags |= Qt::ItemIsUserCheckable;
    else
    {
        m_extraFlags &= ~Qt::ItemIsUserCheckable;
        resetCheckedAlbums();
    }
}

bool AbstractCheckableAlbumModel::isCheckable() const
{
    return m_extraFlags & Qt::ItemIsUserCheckable;
}

void AbstractCheckableAlbumModel::setTristate(bool isTristate)
{
    if (isTristate)
        m_extraFlags |= Qt::ItemIsTristate;
    else
        m_extraFlags &= ~Qt::ItemIsTristate;
}

bool AbstractCheckableAlbumModel::isTristate() const
{
    return m_extraFlags & Qt::ItemIsTristate;
}

bool AbstractCheckableAlbumModel::isChecked(Album *album) const
{
    return m_checkedAlbums.value(album, Qt::Unchecked) == Qt::Checked;
}

Qt::CheckState AbstractCheckableAlbumModel::checkState(Album *album) const
{
    return m_checkedAlbums.value(album, Qt::Unchecked);
}

void AbstractCheckableAlbumModel::setChecked(Album *album, bool isChecked)
{
    setData(indexForAlbum(album), isChecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
}

void AbstractCheckableAlbumModel::setCheckState(Album *album, Qt::CheckState state)
{
    setData(indexForAlbum(album), state, Qt::CheckStateRole);
}

void AbstractCheckableAlbumModel::toggleChecked(Album *album)
{
    setChecked(album, !isChecked(album));
}

QList<Album *> AbstractCheckableAlbumModel::checkedAlbums() const
{
    // return a list with all keys with value Qt::Checked
    return m_checkedAlbums.keys(Qt::Checked);
}

void AbstractCheckableAlbumModel::resetCheckedAlbums()
{
    QList<Album *> oldChecked = m_checkedAlbums.keys();
    m_checkedAlbums.clear();
    foreach(Album *album, oldChecked)
    {
        QModelIndex index = indexForAlbum(album);
        emit dataChanged(index, index);
        emit checkStateChanged(album, Qt::Unchecked);
    }
}

QVariant AbstractCheckableAlbumModel::albumData(Album *a, int role) const
{
    if (role == Qt::CheckStateRole)
    {
        if (m_extraFlags & Qt::ItemIsUserCheckable)
        {
            // with Qt::Unchecked as default, albums not in the hash (initially all)
            // are simply regarded as unchecked
            return m_checkedAlbums.value(a, Qt::Unchecked);
        }
    }

    return AbstractCountingAlbumModel::albumData(a, role);
}

Qt::ItemFlags AbstractCheckableAlbumModel::flags(const QModelIndex& index) const
{
    return AbstractCountingAlbumModel::flags(index) | m_extraFlags;
}

bool AbstractCheckableAlbumModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole)
    {
        Qt::CheckState state = (Qt::CheckState)value.toInt();
        Album *album = albumForIndex(index);
        if (!album)
            return false;
        m_checkedAlbums.insert(album, state);
        emit dataChanged(index, index);
        emit checkStateChanged(album, state);
        return true;
    }
    else
        return AbstractCountingAlbumModel::setData(index, value, role);
}

void AbstractCheckableAlbumModel::albumCleared(Album *album)
{
    m_checkedAlbums.remove(album);
    AbstractCountingAlbumModel::albumCleared(album);
}

void AbstractCheckableAlbumModel::allAlbumsCleared()
{
    m_checkedAlbums.clear();
    AbstractCountingAlbumModel::allAlbumsCleared();
}

} // namespace Digikam
