/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : digiKam album types
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes.

#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "albummanager.h"
#include "albumdb.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "album.h"

namespace Digikam
{

Album::Album(Album::Type type, int id, bool root)
{
    m_parent     = 0;
    m_next       = 0;
    m_prev       = 0;
    m_firstChild = 0;
    m_lastChild  = 0;
    m_clearing   = false;
    m_type       = type;
    m_id         = id;
    m_root       = root;
}

Album::~Album()
{
    if (m_parent)
        m_parent->removeChild(this);
    clear();
}

void Album::setParent(Album* parent)
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

void Album::insertChild(Album* child)
{
    if (!child)
        return;

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

void Album::removeChild(Album* child)
{
    if (!child || m_clearing)
        return;

    if (child == m_firstChild)
    {
        m_firstChild = m_firstChild->m_next;
        if (m_firstChild)
            m_firstChild->m_prev = 0;
        else
            m_firstChild = m_lastChild = 0;
    }
    else if (child == m_lastChild)
    {
        m_lastChild = m_lastChild->m_prev;
        if (m_lastChild)
            m_lastChild->m_next = 0;
        else
            m_firstChild = m_lastChild = 0;
    }
    else
    {
        Album* c = child;
        if (c->m_prev)
            c->m_prev->m_next = c->m_next;
        if (c->m_next)
            c->m_next->m_prev = c->m_prev;
    }
}

void Album::clear()
{
    m_clearing = true;

    Album* child = m_firstChild;
    Album* nextChild;
    while (child)
    {
        nextChild = child->m_next;
        delete child;
        child = nextChild;
    }

    m_firstChild = 0;
    m_lastChild  = 0;
    m_clearing   = false;
}

int Album::globalID() const
{
    switch (m_type)
    {
        // Use the upper bits to create unique ids.
        case (PHYSICAL):
            return m_id;
        case(TAG):
            return m_id | (1 << 28);
        case(DATE):
            return m_id | (1 << 29);
        case(SEARCH):
            return m_id | (1 << 30);
        default:
            DError() << "Unknown album type" << endl;
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

void Album::setExtraData(const void* key, void* value)
{
    m_extraMap.insert(key, value);
}

void Album::removeExtraData(const void* key)
{
    m_extraMap.remove(key);
}

void* Album::extraData(const void* key) const
{
    typedef QMap<const void*, void*> Map;
    Map::const_iterator it = m_extraMap.find(key);
    if (it == m_extraMap.end())
        return 0;

    return it.value();
}

bool Album::isRoot() const
{
    return m_root;
}

bool Album::isAncestorOf(Album* album) const
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

// ------------------------------------------------------------------------------

PAlbum::PAlbum(const QString& title)
      : Album(Album::PHYSICAL, 0, true)
{
    setTitle(title);
    m_albumRootId = -1;
    m_parentPath   = "/";
    m_date        = QDate::currentDate();
}

PAlbum::PAlbum(int albumRoot, const QString &parentPath, const QString& title, int id)
      : Album(Album::PHYSICAL, id, false)
{
    // If path is /holidays/2007, title is only "2007", path is "/holidays"
    setTitle(title);
    m_albumRootId = albumRoot;
    m_parentPath   = parentPath + "/";
    m_date        = QDate::currentDate();
}

PAlbum::~PAlbum()
{
}

void PAlbum::setCaption(const QString& caption)
{
    m_caption = caption;

    DatabaseAccess access;
    access.db()->setAlbumCaption(id(), m_caption);
}

void PAlbum::setCollection(const QString& collection)
{
    m_collection = collection;

    DatabaseAccess access;
    access.db()->setAlbumCollection(id(), m_collection);
}

void PAlbum::setDate(const QDate& date)
{
    m_date = date;

    DatabaseAccess access;
    access.db()->setAlbumDate(id(), m_date);
}

QString PAlbum::albumRootPath() const
{
    return CollectionManager::instance()->albumRootPath(m_albumRootId);
}

int PAlbum::albumRootId() const
{
    return m_albumRootId;
}

QString PAlbum::caption() const
{
    return m_caption;
}

QString PAlbum::collection() const
{
    return m_collection;
}

QDate PAlbum::date() const
{
    return m_date;
}

QString PAlbum::url() const
{
    return albumPath();
}

QString PAlbum::albumPath() const
{
    return m_parentPath + title();
}

DatabaseUrl PAlbum::kurl() const
{
    return databaseUrl();
}

DatabaseUrl PAlbum::databaseUrl() const
{
    return DatabaseUrl::fromAlbumAndName(QString(), albumPath(), albumRootPath(), m_albumRootId);
}

QString PAlbum::prettyUrl() const
{
    QString u = i18n("My Albums") + albumPath();
    return u;
}

QString PAlbum::icon() const
{
    return m_icon;
}

KUrl PAlbum::iconKURL() const
{
    KUrl u;
    u.setPath( m_icon );
    return u;
}

KUrl PAlbum::fileUrl() const
{
    return databaseUrl().fileUrl();
}

QString PAlbum::folderPath() const
{
    return databaseUrl().fileUrl().path();
}

// --------------------------------------------------------------------------

TAlbum::TAlbum(const QString& title, int id, bool root)
      : Album(Album::TAG, id, root)
{
    setTitle(title);
}

TAlbum::~TAlbum()
{
}

QString TAlbum::tagPath(bool leadingSlash) const
{
    if (isRoot())
        return leadingSlash ? "/" : "";

    QString u;

    if (parent())
    {
        u = ((TAlbum*)parent())->tagPath(leadingSlash);
        if (!parent()->isRoot())
            u += '/';
    }

    u += title();

    return u;
}

QString TAlbum::prettyUrl() const
{
    QString u = i18n("My Tags") + tagPath(true);
    return u;
}

DatabaseUrl TAlbum::kurl() const
{
    return DatabaseUrl::fromTagIds(tagIDs());
}

Q3ValueList<int> TAlbum::tagIDs() const
{
    if (isRoot())
    {
        return Q3ValueList<int>();
    }
    else if (parent())
    {
        return static_cast<TAlbum*>(parent())->tagIDs() << id();
    }
    else
    {
        return Q3ValueList<int>() << id();
    }
}

QString TAlbum::icon() const
{
    return m_icon;
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
        dateTitle = m_date.toString("MMMM yyyy");
    else 
        dateTitle = m_date.toString("yyyy");

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

DatabaseUrl DAlbum::kurl() const
{
    if (m_range == Month)
        return DatabaseUrl::fromDateForMonth(m_date);

    return DatabaseUrl::fromDateForYear(m_date);
}

// --------------------------------------------------------------------------

SAlbum::SAlbum(const QString &title, int id, bool root)
      : Album(Album::SEARCH, id, root),
        m_type(DatabaseSearch::UndefinedType)
{
    setTitle(title);
}

SAlbum::~SAlbum()
{
}

void SAlbum::setSearch(DatabaseSearch::Type type, const QString &query)
{
    m_type  = type;
    m_query = query;
}

DatabaseUrl SAlbum::kurl() const
{
    return DatabaseUrl::searchUrl(id());
}

QString SAlbum::query() const
{
    return m_query;
}

DatabaseSearch::Type SAlbum::type() const
{
    return m_type;
}

bool SAlbum::isNormalSearch() const
{
    switch (m_type)
    {
        case DatabaseSearch::KeywordSearch:
        case DatabaseSearch::AdvancedSearch:
        case DatabaseSearch::LegacyUrlSearch:
            return true;
        default:
            return false;
    }
}

bool SAlbum::isKeywordSearch() const
{
    return m_type == DatabaseSearch::KeywordSearch;
}

bool SAlbum::isTimelineSearch() const
{
    return m_type == DatabaseSearch::TimeLineSearch;
}

// --------------------------------------------------------------------------

AlbumIterator::AlbumIterator(Album *album)
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
        return *this;

    Album *album = m_current->firstChild();
    if ( !album )
    {
        while ( (album = m_current->next()) == 0  )
        {
            m_current = m_current->parent();

            if ( m_current == m_root )
            {
                // we have reached the root.
                // that means no more children
                m_current = 0;
                break;
            }

            if ( m_current == 0 )
                break;
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
