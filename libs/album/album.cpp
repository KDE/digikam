/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : digiKam album types
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "album.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "albummanager.h"
#include "collectionmanager.h"
#include "coredbaccess.h"
#include "coredburl.h"
#include "tagscache.h"

namespace Digikam
{

Album::Album(Album::Type type, int id, bool root)
{
    m_parent           = 0;
    m_next             = 0;
    m_prev             = 0;
    m_firstChild       = 0;
    m_lastChild        = 0;
    m_clearing         = false;
    m_type             = type;
    m_id               = id;
    m_root             = root;
    m_usedByLabelsTree = false;
}

Album::~Album()
{
    if (m_parent)
    {
        m_parent->removeChild(this);
    }

    clear();
    AlbumManager::internalInstance->notifyAlbumDeletion(this);
}

void Album::setParent(Album* const parent)
{
    if (parent)
    {
        m_parent = parent;
        parent->insertChild(this);
    }
}

Album* Album::parent() const
{
    return m_parent;
}

Album* Album::firstChild() const
{
    return m_firstChild;
}

Album* Album::lastChild() const
{
    return m_lastChild;
}

Album* Album::next() const
{
    return m_next;
}

Album* Album::prev() const
{
    return m_prev;
}

AlbumList Album::childAlbums(bool recursive)
{
    AlbumList childList;

    for (Album* child = this->firstChild(); child; child = child->next())
    {
        childList += child;

        if (recursive)
        {
            childList += child->childAlbums(recursive);
        }
    }

    return childList;
}

QList< int > Album::childAlbumIds(bool recursive)
{
    QList <int> ids;

    AlbumList childList = this->childAlbums(recursive);

    QListIterator<Album*> it(childList);

    while (it.hasNext())
    {
        ids += it.next()->id();
    }

    return ids;
}

void Album::insertChild(Album* const child)
{
    if (!child)
    {
        return;
    }

    if (!m_firstChild)
    {
        m_firstChild  = child;
        m_lastChild   = child;
        child->m_next = 0;
        child->m_prev = 0;
    }
    else
    {
        m_lastChild->m_next = child;
        child->m_prev       = m_lastChild;
        child->m_next       = 0;
        m_lastChild         = child;
    }
}

void Album::removeChild(Album* const child)
{
    if (!child || m_clearing)
    {
        return;
    }

    if (child == m_firstChild)
    {
        m_firstChild = m_firstChild->m_next;

        if (m_firstChild)
        {
            m_firstChild->m_prev = 0;
        }
        else
        {
            m_firstChild = m_lastChild = 0;
        }
    }
    else if (child == m_lastChild)
    {
        m_lastChild = m_lastChild->m_prev;

        if (m_lastChild)
        {
            m_lastChild->m_next = 0;
        }
        else
        {
            m_firstChild = m_lastChild = 0;
        }
    }
    else
    {
        Album* c = child;

        if (c->m_prev)
        {
            c->m_prev->m_next = c->m_next;
        }

        if (c->m_next)
        {
            c->m_next->m_prev = c->m_prev;
        }
    }
}

void Album::clear()
{
    m_clearing       = true;
    Album* child     = m_firstChild;
    Album* nextChild = 0;

    while (child)
    {
        nextChild = child->m_next;
        delete child;
        child     = nextChild;
    }

    m_firstChild = 0;
    m_lastChild  = 0;
    m_clearing   = false;
}

int Album::globalID() const
{
    return globalID(m_type, m_id);
}

int Album::globalID(Type type, int id)
{
    switch (type)
    {
        // Use the upper bits to create unique ids.
        case (PHYSICAL):
            return id;

        case (TAG):
            return id | (1 << 28);

        case (DATE):
            return id | (1 << 29);

        case (SEARCH):
            return id | (1 << 30);

        case (FACE):
            return id | (1 << 31);

        default:
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unknown album type";
            return -1;
    }
}

int Album::id() const
{
    return m_id;
}

void Album::setTitle(const QString& title)
{
    m_title = title;
}

QString Album::title() const
{
    return m_title;
}

Album::Type Album::type() const
{
    return m_type;
}

void Album::setExtraData(const void* const key, void* const value)
{
    m_extraMap.insert(key, value);
}

void Album::removeExtraData(const void* const key)
{
    m_extraMap.remove(key);
}

void* Album::extraData(const void* const key) const
{
    typedef QMap<const void*, void*> Map;
    Map::const_iterator it = m_extraMap.constFind(key);

    if (it == m_extraMap.constEnd())
    {
        return 0;
    }

    return it.value();
}

bool Album::isRoot() const
{
    return m_root;
}

bool Album::isAncestorOf(Album* const album) const
{
    bool val = false;
    Album* a = album;

    while (a && !a->isRoot())
    {
        if (a == this)
        {
            val = true;
            break;
        }

        a = a->parent();
    }

    return val;
}

bool Album::isUsedByLabelsTree() const
{
    return m_usedByLabelsTree;
}

bool Album::isTrashAlbum() const
{
    if (m_id < -1 && m_type == PHYSICAL)
    {
        return true;
    }

    return false;
}

void Album::setUsedByLabelsTree(bool isUsed)
{
    m_usedByLabelsTree = isUsed;
}

// ------------------------------------------------------------------------------

int PAlbum::m_uniqueTrashId = -2;

PAlbum::PAlbum(const QString& title)
    : Album(Album::PHYSICAL, 0, true),
      m_iconId(0)
{
    setTitle(title);
    m_isAlbumRootAlbum = false;
    m_albumRootId      = -1;
    m_parentPath       = QLatin1Char('/');
    m_path.clear();
}

PAlbum::PAlbum(int albumRoot, const QString& label)
    : Album(Album::PHYSICAL, -1, false),
      m_iconId(0)
{
    // set the id to -1 (line above). AlbumManager may change that later.
    setTitle(label);
    m_albumRootId      = albumRoot;
    m_isAlbumRootAlbum = true;
    m_parentPath       = QLatin1Char('/');
    m_path.clear();
}

PAlbum::PAlbum(int albumRoot, const QString& parentPath, const QString& title, int id)
    : Album(Album::PHYSICAL, id, false),
      m_iconId(0)
{
    // If path is /holidays/2007, title is only "2007", path is "/holidays"
    setTitle(title);
    m_albumRootId      = albumRoot;
    m_isAlbumRootAlbum = false;
    m_parentPath       = parentPath + QLatin1Char('/');
    m_path             = title;
    m_date             = QDate::currentDate();
}

PAlbum::PAlbum(const QString& parentPath, int albumRoot)
    : Album(Album::PHYSICAL, m_uniqueTrashId--, false),
      m_iconId(0)
{
    setTitle(i18n("Trash"));

    m_albumRootId      = albumRoot;
    m_isAlbumRootAlbum = false;
    m_parentPath       = parentPath + QLatin1Char('/');
    m_path             = QLatin1String("Trash");
}

PAlbum::~PAlbum()
{
}

bool PAlbum::isAlbumRoot() const
{
    return m_isAlbumRootAlbum;
}

void PAlbum::setCaption(const QString& caption)
{
    m_caption = caption;

    CoreDbAccess access;
    access.db()->setAlbumCaption(id(), m_caption);
}

void PAlbum::setCategory(const QString& category)
{
    m_category = category;

    CoreDbAccess access;
    access.db()->setAlbumCategory(id(), m_category);
}

void PAlbum::setDate(const QDate& date)
{
    m_date = date;

    CoreDbAccess access;
    access.db()->setAlbumDate(id(), m_date);
}

QString PAlbum::albumRootPath() const
{
    return CollectionManager::instance()->albumRootPath(m_albumRootId);
}

QString PAlbum::albumRootLabel() const
{
    return CollectionManager::instance()->albumRootLabel(m_albumRootId);
}

int PAlbum::albumRootId() const
{
    return m_albumRootId;
}

QString PAlbum::caption() const
{
    return m_caption;
}

QString PAlbum::category() const
{
    return m_category;
}

QDate PAlbum::date() const
{
    return m_date;
}

QString PAlbum::albumPath() const
{
    return m_parentPath + m_path;
}

CoreDbUrl PAlbum::databaseUrl() const
{
    return CoreDbUrl::fromAlbumAndName(QString(), albumPath(),
                                       QUrl::fromLocalFile(albumRootPath()), m_albumRootId);
}

QString PAlbum::prettyUrl() const
{
    QString u = i18n("Albums") + QLatin1Char('/') + albumRootLabel() + albumPath();

    if (u.endsWith(QLatin1Char('/')))
        u.truncate(u.length() - 1);

    return u;
}

qlonglong PAlbum::iconId() const
{
    return m_iconId;
}

QUrl PAlbum::fileUrl() const
{
    return databaseUrl().fileUrl();
}

QString PAlbum::folderPath() const
{
    return databaseUrl().fileUrl().toLocalFile();
}

// --------------------------------------------------------------------------

TAlbum::TAlbum(const QString& title, int id, bool root)
    : Album(Album::TAG, id, root), m_pid(0),
      m_iconId(0)
{
    setTitle(title);
}

TAlbum::~TAlbum()
{
}

QString TAlbum::tagPath(bool leadingSlash) const
{
    if (isRoot())
    {
        return leadingSlash ? QLatin1String("/") : QLatin1String("");
    }

    QString u;

    if (parent())
    {
        u = (static_cast<TAlbum*>(parent()))->tagPath(leadingSlash);

        if (!parent()->isRoot())
        {
            u += QLatin1Char('/');
        }
    }

    u += title();

    return u;
}

QString TAlbum::prettyUrl() const
{
    return i18n("Tags") + tagPath(true);
}

CoreDbUrl TAlbum::databaseUrl() const
{
    return CoreDbUrl::fromTagIds(tagIDs());
}

QList<int> TAlbum::tagIDs() const
{
    if (isRoot())
    {
        return QList<int>();
    }
    else if (parent())
    {
        return static_cast<TAlbum*>(parent())->tagIDs() << id();
    }
    else
    {
        return QList<int>() << id();
    }
}

QString TAlbum::icon() const
{
    return m_icon;
}

bool TAlbum::isInternalTag() const
{
    return TagsCache::instance()->isInternalTag(id());
}

qlonglong TAlbum::iconId() const
{
    return m_iconId;
}

bool TAlbum::hasProperty(const QString& key) const
{
    return TagsCache::instance()->hasProperty(id(), key);
}

QString TAlbum::property(const QString& key) const
{
    return TagsCache::instance()->propertyValue(id(), key);
}

QMap<QString, QString> TAlbum::properties() const
{
    return TagsCache::instance()->properties(id());
}

// --------------------------------------------------------------------------

int DAlbum::m_uniqueID = 0;

DAlbum::DAlbum(const QDate& date, bool root, Range range)
    : Album(Album::DATE, root ? 0 : ++m_uniqueID, root)
{
    m_date  = date;
    m_range = range;

    // Set the name of the date album
    QString dateTitle;

    if (m_range == Month)
    {
        dateTitle = m_date.toString(QLatin1String("MMMM yyyy"));
    }
    else
    {
        dateTitle = m_date.toString(QLatin1String("yyyy"));
    }

    setTitle(dateTitle);
}

DAlbum::~DAlbum()
{
}

QDate DAlbum::date() const
{
    return m_date;
}

DAlbum::Range DAlbum::range() const
{
    return m_range;
}

CoreDbUrl DAlbum::databaseUrl() const
{
    if (m_range == Month)
    {
        return CoreDbUrl::fromDateForMonth(m_date);
    }

    return CoreDbUrl::fromDateForYear(m_date);
}

// --------------------------------------------------------------------------

SAlbum::SAlbum(const QString& title, int id, bool root)
    : Album(Album::SEARCH, id, root),
      m_searchType(DatabaseSearch::UndefinedType)
{
    setTitle(title);
}

SAlbum::~SAlbum()
{
}

void SAlbum::setSearch(DatabaseSearch::Type type, const QString& query)
{
    m_searchType = type;
    m_query      = query;
}

CoreDbUrl SAlbum::databaseUrl() const
{
    return CoreDbUrl::searchUrl(id());
}

QString SAlbum::query() const
{
    return m_query;
}

DatabaseSearch::Type SAlbum::searchType() const
{
    return m_searchType;
}

bool SAlbum::isNormalSearch() const
{
    switch (m_searchType)
    {
        case DatabaseSearch::KeywordSearch:
        case DatabaseSearch::AdvancedSearch:
        case DatabaseSearch::LegacyUrlSearch:
            return true;

        default:
            return false;
    }
}

bool SAlbum::isAdvancedSearch() const
{
    return m_searchType == DatabaseSearch::AdvancedSearch;
}

bool SAlbum::isKeywordSearch() const
{
    return m_searchType == DatabaseSearch::KeywordSearch;
}

bool SAlbum::isTimelineSearch() const
{
    return m_searchType == DatabaseSearch::TimeLineSearch;
}

bool SAlbum::isHaarSearch() const
{
    return m_searchType == DatabaseSearch::HaarSearch;
}

bool SAlbum::isMapSearch() const
{
    return m_searchType == DatabaseSearch::MapSearch;
}

bool SAlbum::isDuplicatesSearch() const
{
    return m_searchType == DatabaseSearch::DuplicatesSearch;
}

bool SAlbum::isTemporarySearch() const
{

    if (isHaarSearch())
    {
        return (title() == getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch)) ||
                title() == getTemporaryHaarTitle(DatabaseSearch::HaarSketchSearch);
    }

    return (title() == getTemporaryTitle(m_searchType));
}

QString SAlbum::displayTitle() const
{
    if (isTemporarySearch())
    {
        switch (m_searchType)
        {
            case DatabaseSearch::TimeLineSearch:
                return i18n("Current Timeline Search");

            case DatabaseSearch::HaarSearch:
            {
                if (title() == getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch))
                {
                    return i18n("Current Fuzzy Image Search");
                }
                else if (title() == getTemporaryHaarTitle(DatabaseSearch::HaarSketchSearch))
                {
                    return i18n("Current Fuzzy Sketch Search");
                }

                break;
            }

            case DatabaseSearch::MapSearch:
                return i18n("Current Map Search");

            case DatabaseSearch::KeywordSearch:
            case DatabaseSearch::AdvancedSearch:
            case DatabaseSearch::LegacyUrlSearch:
                return i18n("Current Search");

            case DatabaseSearch::DuplicatesSearch:
                return i18n("Current Duplicates Search");

            case DatabaseSearch::UndefinedType:
                break;
        }
    }

    return title();
}

QString SAlbum::getTemporaryTitle(DatabaseSearch::Type type, DatabaseSearch::HaarSearchType haarType)
{
    switch (type)
    {
        case DatabaseSearch::TimeLineSearch:
            return QLatin1String("_Current_Time_Line_Search_");

        case DatabaseSearch::HaarSearch:
            return getTemporaryHaarTitle(haarType);

        case DatabaseSearch::MapSearch:
            return QLatin1String("_Current_Map_Search_");

        case DatabaseSearch::KeywordSearch:
        case DatabaseSearch::AdvancedSearch:
        case DatabaseSearch::LegacyUrlSearch:
            return QLatin1String("_Current_Search_View_Search_");

        case DatabaseSearch::DuplicatesSearch:
            return QLatin1String("_Current_Duplicates_Search_");

        default:
            qCDebug(DIGIKAM_GENERAL_LOG) << "Untreated temporary search type " << type;
            return QLatin1String("_Current_Unknown_Search_");
    }
}

QString SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarSearchType haarType)
{
    switch (haarType)
    {
        case DatabaseSearch::HaarImageSearch:
            return QLatin1String("_Current_Fuzzy_Image_Search_");

        case DatabaseSearch::HaarSketchSearch:
            return QLatin1String("_Current_Fuzzy_Sketch_Search_");

        default:
            qCDebug(DIGIKAM_GENERAL_LOG) << "Untreated temporary haar search type " << haarType;
            return QLatin1String("_Current_Unknown_Haar_Search_");
    }
}

// --------------------------------------------------------------------------

AlbumIterator::AlbumIterator(Album* const album)
{
    m_root    = album;
    m_current = album ? album->firstChild() : 0;
}

AlbumIterator::~AlbumIterator()
{
}

AlbumIterator& AlbumIterator::operator++()
{
    if (!m_current)
    {
        return *this;
    }

    Album* album = m_current->firstChild();

    if (!album)
    {
        while ((album = m_current->next()) == 0)
        {
            m_current = m_current->parent();

            if (m_current == m_root)
            {
                // we have reached the root.
                // that means no more children
                m_current = 0;
                break;
            }

            if (m_current == 0)
            {
                break;
            }
        }
    }

    m_current = album;

    return *this;
}

Album* AlbumIterator::operator*()
{
    return m_current;
}

Album* AlbumIterator::current() const
{
    return m_current;
}

}  // namespace Digikam
