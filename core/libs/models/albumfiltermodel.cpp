/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-24
 * Description : Qt Model for Albums - filter model
 *
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009      by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2014-2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "albumfiltermodel.h"

// Qt includes

#include <QSortFilterProxyModel>
#include <QCollator>
#include <QHeaderView>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "albummodel.h"
#include "applicationsettings.h"

namespace Digikam
{

AlbumFilterModel::AlbumFilterModel(QObject* const parent)
    : QSortFilterProxyModel(parent), m_parent(parent)
{
    m_filterBehavior = FullFiltering;
    m_chainedModel   = 0;
    setSortRole(AbstractAlbumModel::AlbumSortRole);
    setSortCaseSensitivity(Qt::CaseInsensitive);

    // sorting may have changed when the string comparison is different
    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(invalidate()));

    // dynamicSortFilter does not work well for us: a dataChange may, because of our way of filtering,
    // also affect parents and children of the changed index, which is not handled by QSortFilterProxyModel.
    setDynamicSortFilter(false);
    // Instead, we listen directly to AlbumManager's relevant change signals
    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsUpdated(int)),
            this, SLOT(slotAlbumsHaveBeenUpdated(int)));
}

void AlbumFilterModel::setFilterBehavior(FilterBehavior behavior)
{
    if (behavior == m_filterBehavior)
    {
        return;
    }

    m_filterBehavior = behavior;
    invalidateFilter();
}

void AlbumFilterModel::setSearchTextSettings(const SearchTextSettings& settings)
{
    if (!sourceModel())
    {
        return;
    }

    // don't use isFiltering here because it may be reimplemented
    bool wasSearching = settingsFilter(m_settings);
    bool willSearch   = settingsFilter(settings);
    emit searchTextSettingsAboutToChange(wasSearching, willSearch);

    m_settings = settings;
    invalidateFilter();
    emit filterChanged();

    emit searchTextSettingsChanged(wasSearching, willSearch);

    if (sourceAlbumModel()->albumType() == Album::PHYSICAL)
    {
        // find out if this setting has some results or not
        int validRows = 0;

        // for every collection we got
        for (int i = 0; i < rowCount(rootAlbumIndex()); ++i)
        {
            QModelIndex collectionIndex = index(i, 0, rootAlbumIndex());
            // count the number of rows
            validRows                   += rowCount(collectionIndex);
        }

        bool hasResult = validRows > 0;
        qCDebug(DIGIKAM_GENERAL_LOG) << "new search text settings: " << settings.text
                 << ": hasResult = " << hasResult << ", validRows = "
                 << validRows;
        emit hasSearchResult(hasResult);
    }
    else
    {
        QModelIndex head = rootAlbumIndex(); // either root, or invalid, thus toplevel
        emit hasSearchResult(rowCount(head));
    }
}

bool AlbumFilterModel::settingsFilter(const SearchTextSettings& settings) const
{
    return !settings.text.isEmpty();
}

bool AlbumFilterModel::isFiltering() const
{
    return settingsFilter(m_settings);
}

SearchTextSettings AlbumFilterModel::searchTextSettings() const
{
    return m_settings;
}

void AlbumFilterModel::setSourceAlbumModel(AbstractAlbumModel* source)
{
    if (m_chainedModel)
    {
        m_chainedModel->setSourceAlbumModel(source);
    }
    else
    {
        if (source != sourceModel())
        {
            setSourceModel(source);
        }
    }
}

void AlbumFilterModel::setSourceFilterModel(AlbumFilterModel* source)
{
    if (source)
    {
        AbstractAlbumModel* model = sourceAlbumModel();

        if (model)
        {
            source->setSourceAlbumModel(model);
        }
    }

    if (m_chainedModel != source || sourceModel() != source)
    {
        m_chainedModel = source;
        setSourceModel(source);
    }
}

void AlbumFilterModel::setSourceModel(QAbstractItemModel* model)
{
    // made it protected, only setSourceAlbumModel is public
    QSortFilterProxyModel::setSourceModel(model);
}

AbstractAlbumModel* AlbumFilterModel::sourceAlbumModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->sourceAlbumModel();
    }

    return static_cast<AbstractAlbumModel*>(sourceModel());
}

AlbumFilterModel* AlbumFilterModel::sourceFilterModel() const
{
    return m_chainedModel;
}

QModelIndex AlbumFilterModel::mapToSourceAlbumModel(const QModelIndex& index) const
{
    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceAlbumModel(mapToSource(index));
    }

    return mapToSource(index);
}

QModelIndex AlbumFilterModel::mapFromSourceAlbumModel(const QModelIndex& albummodel_index) const
{
    if (m_chainedModel)
    {
        return mapFromSource(m_chainedModel->mapFromSourceAlbumModel(albummodel_index));
    }

    return mapFromSource(albummodel_index);
}

Album* AlbumFilterModel::albumForIndex(const QModelIndex& index) const
{
    return AbstractAlbumModel::retrieveAlbum(index);
}

QModelIndex AlbumFilterModel::indexForAlbum(Album* album) const
{
    AbstractAlbumModel* model = sourceAlbumModel();

    if (!model)
    {
        return QModelIndex();
    }

    return mapFromSourceAlbumModel(model->indexForAlbum(album));
}

QModelIndex AlbumFilterModel::rootAlbumIndex() const
{
    AbstractAlbumModel* model = sourceAlbumModel();

    if (!model)
    {
        return QModelIndex();
    }

    return mapFromSourceAlbumModel(model->rootAlbumIndex());
}

QVariant AlbumFilterModel::dataForCurrentSortRole(Album* album) const
{
    if (album)
    {
        if (album->type() == Album::PHYSICAL)
        {
            PAlbum* const a = static_cast<PAlbum*>(album);

            ApplicationSettings::AlbumSortRole sortRole = ApplicationSettings::instance()->getAlbumSortRole();

            switch (sortRole)
            {
                case ApplicationSettings::ByFolder:
                    return a->title();
                case ApplicationSettings::ByDate:
                    return a->date();
                default:
                    return a->category();
            }
        }
        else if (album->type() == Album::TAG)
        {
            return static_cast<TAlbum*>(album)->title();
        }
        else
        {
            return static_cast<DAlbum*>(album)->date();
        }
    }

    return QVariant();
}

bool AlbumFilterModel::matches(Album* album) const
{
    // We want to work on the visual representation, so we use model data with AlbumTitleRole,
    // not a direct Album method.
    // We use direct source's index, not our index,
    // because if the item is currently filtered out, we won't have an index for this album.
    QModelIndex source_index = sourceAlbumModel()->indexForAlbum(album);

    if (m_chainedModel)
    {
        source_index = m_chainedModel->mapFromSourceAlbumModel(source_index);
    }

    QString displayTitle = source_index.data(AbstractAlbumModel::AlbumTitleRole).toString();
    return displayTitle.contains(m_settings.text, m_settings.caseSensitive);
}

AlbumFilterModel::MatchResult AlbumFilterModel::matchResult(const QModelIndex& index) const
{
    return matchResult(albumForIndex(index));
}

AlbumFilterModel::MatchResult AlbumFilterModel::matchResult(Album* album) const
{
    if (!album)
    {
        return NoMatch;
    }

    PAlbum* const palbum = dynamic_cast<PAlbum*>(album);

    if (album->isRoot() || (palbum && palbum->isAlbumRoot()))
    {
        return SpecialMatch;
    }

    TAlbum* const talbum = dynamic_cast<TAlbum*>(album);

    if (talbum && talbum->isInternalTag())
    {
        return NoMatch;
    }

    if (matches(album))
    {
        return DirectMatch;
    }

    if (m_filterBehavior == SimpleFiltering)
    {
        return NoMatch;
    }

    if (m_filterBehavior == FullFiltering)
    {
        // check if any of the parents match the search
        Album* parent         = album->parent();
        PAlbum* const pparent = palbum ? static_cast<PAlbum*>(parent) : 0;

        while (parent && !(parent->isRoot() || (pparent && pparent->isAlbumRoot()) ) )
        {
            if (matches(parent))
            {
                return ParentMatch;
            }

            parent = parent->parent();
        }
    }

    // check if any of the children match the search
    AlbumIterator it(album);

    while (it.current())
    {
        if (matches(*it))
        {
            return ChildMatch;
        }

        ++it;
    }

    return NoMatch;
}

bool AlbumFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index  = sourceModel()->index(source_row, 0, source_parent);
    Album* const album = AbstractAlbumModel::retrieveAlbum(index);
    MatchResult result = matchResult(album);
    return result;
}

bool AlbumFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    Album* leftAlbum  = albumForIndex(left);
    Album* rightAlbum = albumForIndex(right);

    if (!leftAlbum || !rightAlbum)
    {
        return QSortFilterProxyModel::lessThan(left, right);
    }

    if (leftAlbum->isTrashAlbum() != rightAlbum->isTrashAlbum())
    {
        // trash albums go to the bottom, regardless of sort role
        return (sortOrder() == Qt::AscendingOrder) ? !leftAlbum->isTrashAlbum() : leftAlbum->isTrashAlbum();
    }

    QVariant valLeft  = dataForCurrentSortRole(leftAlbum);
    QVariant valRight = dataForCurrentSortRole(rightAlbum);

    ApplicationSettings::AlbumSortRole role = ApplicationSettings::instance()->getAlbumSortRole();

    if ((role == ApplicationSettings::ByDate || role == ApplicationSettings::ByCategory)&&(valLeft == valRight))
    {
        return QSortFilterProxyModel::lessThan(left, right);
    }

    bool natural = ApplicationSettings::instance()->isStringTypeNatural();

    if ((valLeft.type() == QVariant::String) && (valRight.type() == QVariant::String))
    {
        QCollator collator;
        collator.setNumericMode(natural);
        collator.setIgnorePunctuation(false);
        collator.setCaseSensitivity(sortCaseSensitivity());
        return (collator.compare(valLeft.toString(), valRight.toString()) < 0);
    }
    else if ((valLeft.type() == QVariant::Date) && (valRight.type() == QVariant::Date))
    {
        return compareByOrder(valLeft.toDate(),valRight.toDate(),Qt::AscendingOrder) < 0;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

void AlbumFilterModel::slotAlbumRenamed(Album* album)
{
    if (album)
    {
        slotAlbumsHaveBeenUpdated(album->type());
    }
}

void AlbumFilterModel::slotAlbumsHaveBeenUpdated(int type)
{
    if (sourceAlbumModel() && sourceAlbumModel()->albumType() == type)
    {
        invalidate();
    }
}

// -----------------------------------------------------------------------------

CheckableAlbumFilterModel::CheckableAlbumFilterModel(QObject* const parent) :
    AlbumFilterModel(parent),
    m_filterChecked(false),
    m_filterPartiallyChecked(false)
{
}

void CheckableAlbumFilterModel::setSourceAlbumModel(AbstractCheckableAlbumModel* source)
{
    AlbumFilterModel::setSourceAlbumModel(source);
}

void CheckableAlbumFilterModel::setSourceFilterModel(CheckableAlbumFilterModel* source)
{
    AlbumFilterModel::setSourceFilterModel(source);
}

AbstractCheckableAlbumModel* CheckableAlbumFilterModel::sourceAlbumModel() const
{
    return static_cast<AbstractCheckableAlbumModel*>(AlbumFilterModel::sourceAlbumModel());
}

void CheckableAlbumFilterModel::setFilterChecked(bool filter)
{
    m_filterChecked = filter;
    invalidateFilter();
    emit filterChanged();
}

void CheckableAlbumFilterModel::setFilterPartiallyChecked(bool filter)
{
    m_filterPartiallyChecked = filter;
    invalidateFilter();
    emit filterChanged();
}

bool CheckableAlbumFilterModel::isFiltering() const
{
    return (AlbumFilterModel::isFiltering() || m_filterChecked || m_filterPartiallyChecked);
}

bool CheckableAlbumFilterModel::matches(Album* album) const
{
    bool accepted = AlbumFilterModel::matches(album);

    if (!m_filterChecked && !m_filterPartiallyChecked)
    {
        return accepted;
    }

    Qt::CheckState state = sourceAlbumModel()->checkState(album);

    bool stateAccepted = false;

    if (m_filterPartiallyChecked)
    {
        stateAccepted |= state == Qt::PartiallyChecked;
    }

    if (m_filterChecked)
    {
        stateAccepted |= state == Qt::Checked;
    }

    return accepted && stateAccepted;
}

// -----------------------------------------------------------------------------

SearchFilterModel::SearchFilterModel(QObject* const parent)
    : CheckableAlbumFilterModel(parent),
      m_searchType(-1),
      m_listTemporary(false)
{
}

void SearchFilterModel::setSourceSearchModel(SearchModel* source)
{
    setSourceAlbumModel(source);
}

SearchModel* SearchFilterModel::sourceSearchModel() const
{
    return dynamic_cast<SearchModel*> (sourceModel());
}

void SearchFilterModel::setFilterSearchType(DatabaseSearch::Type type)
{
    setTypeFilter(type);
}

void SearchFilterModel::listNormalSearches()
{
    setTypeFilter(-1);
}

void SearchFilterModel::listAllSearches()
{
    setTypeFilter(-2);
}

void SearchFilterModel::listTimelineSearches()
{
    setTypeFilter(DatabaseSearch::TimeLineSearch);
}

void SearchFilterModel::listHaarSearches()
{
    setTypeFilter(DatabaseSearch::HaarSearch);
}

void SearchFilterModel::listMapSearches()
{
    setTypeFilter(DatabaseSearch::MapSearch);
}

void SearchFilterModel::listDuplicatesSearches()
{
    setTypeFilter(DatabaseSearch::DuplicatesSearch);
}

void SearchFilterModel::setTypeFilter(int type)
{
    m_searchType = type;
    invalidateFilter();
    emit filterChanged();
}

void SearchFilterModel::setListTemporarySearches(bool list)
{
    m_listTemporary = list;
    invalidateFilter();
    emit filterChanged();
}

bool SearchFilterModel::isFiltering() const
{
    return (m_searchType != -2 || !m_listTemporary);
}

bool SearchFilterModel::matches(Album* album) const
{
    if (!AlbumFilterModel::matches(album))
    {
        return false;
    }

    SAlbum* const salbum = static_cast<SAlbum*>(album);

    if (m_searchType == -1)
    {
        if (!salbum->isNormalSearch())
        {
            return false;
        }
    }
    else if (m_searchType == -2)
    {
    }
    else
    {
        if (salbum->searchType() != (DatabaseSearch::Type)m_searchType)
        {
            return false;
        }
    }

    if (!m_listTemporary && salbum->isTemporarySearch())
    {
        return false;
    }

    return true;
}

void SearchFilterModel::setSourceAlbumModel(AbstractAlbumModel* source)
{
    AlbumFilterModel::setSourceAlbumModel(source);
}

// -----------------------------------------------------------------------------

TagPropertiesFilterModel::TagPropertiesFilterModel(QObject* const parent)
    : CheckableAlbumFilterModel(parent)
{
    connect(AlbumManager::instance(), SIGNAL(signalTagPropertiesChanged(TAlbum*)),
            this, SLOT(tagPropertiesChanged(TAlbum*)));
}

void TagPropertiesFilterModel::setSourceAlbumModel(TagModel* source)
{
    CheckableAlbumFilterModel::setSourceAlbumModel(source);
}

TagModel* TagPropertiesFilterModel::sourceTagModel() const
{
    return dynamic_cast<TagModel*>(sourceModel());
}

void TagPropertiesFilterModel::listOnlyTagsWithProperty(const QString& property)
{
    if (m_propertiesWhiteList.contains(property))
    {
        return;
    }

    m_propertiesWhiteList << property;
    invalidateFilter();
    emit filterChanged();
}

void TagPropertiesFilterModel::removeListOnlyProperty(const QString& property)
{
    if (!m_propertiesWhiteList.contains(property))
    {
        return;
    }

    m_propertiesWhiteList.remove(property);
    invalidateFilter();
    emit filterChanged();
}

void TagPropertiesFilterModel::doNotListTagsWithProperty(const QString& property)
{
    if (m_propertiesBlackList.contains(property))
    {
        return;
    }

    m_propertiesBlackList << property;
    invalidateFilter();
    emit filterChanged();
}

void TagPropertiesFilterModel::removeDoNotListProperty(const QString& property)
{
    if (!m_propertiesBlackList.contains(property))
    {
        return;
    }

    m_propertiesBlackList.remove(property);
    invalidateFilter();
    emit filterChanged();
}

bool TagPropertiesFilterModel::isFiltering() const
{
    return !m_propertiesWhiteList.isEmpty() || !m_propertiesBlackList.isEmpty();
}

void TagPropertiesFilterModel::tagPropertiesChanged(TAlbum*)
{
    // I do not expect batch changes. Otherwise we'll need a timer.
    if (isFiltering())
    {
        invalidateFilter();
    }
}

bool TagPropertiesFilterModel::matches(Album* album) const
{
    if (!AlbumFilterModel::matches(album))
    {
        return false;
    }

    TAlbum* const talbum = static_cast<TAlbum*>(album);

    foreach(const QString& prop, m_propertiesBlackList)
    {
        if (talbum->hasProperty(prop))
        {
            return false;
        }
    }

    foreach(const QString& prop, m_propertiesWhiteList)
    {
        if (!talbum->hasProperty(prop))
        {
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------

TagsManagerFilterModel::TagsManagerFilterModel(QObject* const parent)
    : TagPropertiesFilterModel(parent)
{
}

void TagsManagerFilterModel::setQuickListTags(QList<int> tags)
{
    m_keywords.clear();

    foreach(int tag, tags)
    {
        m_keywords << tag;
    }

    invalidateFilter();
    emit filterChanged();
}

bool TagsManagerFilterModel::matches(Album* album) const
{
    if (!TagPropertiesFilterModel::matches(album))
    {
        return false;
    }

    if (m_keywords.isEmpty())
    {
        return true;
    }

    bool dirty = false;

    for (QSet<int>::const_iterator it = m_keywords.begin(); it != m_keywords.end(); ++it)
    {
        TAlbum* const talbum = AlbumManager::instance()->findTAlbum(*it);

        if (!talbum)
        {
            continue;
        }

        if (talbum->title().compare(album->title()) == 0)
        {
            dirty = true;
        }
    }

    return dirty;
}

} // namespace Digikam
