/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-15
 * Description :
 *
 * Copyright 2004 by Renchi Raju

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
#include "album.h"

namespace Digikam
{

Album::Album(Album::Type type, int id, bool root)
{
    m_parent = 0;
    m_next   = 0;
    m_prev   = 0;
    m_firstChild = 0;
    m_lastChild  = 0;
    m_clearing   = false;

    m_type  = type;
    m_id    = id;
    m_root  = root;
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
        m_firstChild = child;
        m_lastChild  = child;
        child->m_next = 0;
        child->m_prev = 0;
    }
    else
    {
        m_lastChild->m_next = child;
        child->m_prev  = m_lastChild;
        child->m_next  = 0;
        m_lastChild    = child;
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

    m_clearing = false;
}

int Album::globalID() const
{
    switch (m_type)
    {
    case (PHYSICAL):
        return 10000 + m_id;
    case(TAG):
        return 20000 + m_id;
    case(DATE):
        return 30000 + m_id;
    case(SEARCH):
        return 40000 + m_id;
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
    m_extraMap.replace(key, value);
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

    return it.data();
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

PAlbum::PAlbum(const QString& title, int id,  bool root)
    : Album(Album::PHYSICAL, id, root)
{
    setTitle(title);
    m_caption    = "";
    m_collection = "";
    m_date = QDate::currentDate();
}

PAlbum::~PAlbum()
{

}

void PAlbum::setCaption(const QString& caption)
{
    m_caption = caption;

    AlbumDB* db = AlbumManager::instance()->albumDB();
    db->setAlbumCaption(id(), m_caption);
}

void PAlbum::setCollection(const QString& collection)
{
    m_collection = collection;
    AlbumDB* db = AlbumManager::instance()->albumDB();
    db->setAlbumCollection(id(), m_collection);
}

void PAlbum::setDate(const QDate& date)
{
    m_date = date;

    AlbumDB* db = AlbumManager::instance()->albumDB();
    db->setAlbumDate(id(), m_date);
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
    QString u("");
    if (isRoot())
    {
        return "/";
    }
    else if (parent())
    {
        u = ((PAlbum*)parent())->url();
        if (!u.endsWith("/"))
            u += '/';
    }
    u += title();
    return u;
}

KURL PAlbum::kurl() const
{
    KURL u;
    u.setProtocol("digikamalbums");
    u.setUser(AlbumManager::instance()->getLibraryPath());
    // add an empty host. KURLDrag will eat away the user
    // attribute if a host is not present. probably a URL
    // specification
    u.setHost(" ");
    u.setPath(url());
    return u;
}

QString PAlbum::prettyURL() const
{
    QString u = i18n("My Albums") + url();
    return u;
}

QString PAlbum::icon() const
{
    return m_icon;
}

KURL PAlbum::iconKURL() const
{
    KURL u;
    u.setPath( m_icon );
    return u;
}

QString PAlbum::folderPath() const
{
    KURL u(AlbumManager::instance()->getLibraryPath());
    u.addPath(url());
    return u.path();
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

QString TAlbum::url() const
{
    if (isRoot())
    {
        return "/";
    }

    QString u;
    if (parent())
    {
        u = ((TAlbum*)parent())->url();
        if (!u.endsWith("/"))
            u += '/';
    }
    u += title();
    return u;
}

QString TAlbum::prettyURL() const
{
    QString u = i18n("My Tags") + url();
    return u;
}

KURL TAlbum::kurl() const
{
    KURL url;
    url.setProtocol("digikamtags");

    if (isRoot())
    {
        url.setPath("/");
    }
    else if (parent())
    {
        TAlbum *p = static_cast<TAlbum*>(parent());
        url.setPath(p->kurl().path(1));
        url.addPath(QString::number(id()));
    }
    else
    {
        url = KURL();
    }
    return url;
}


QString TAlbum::icon() const
{
    return m_icon;
}

// --------------------------------------------------------------------------

int DAlbum::m_uniqueID = 0;

DAlbum::DAlbum(const QDate& date, bool root)
    : Album(Album::DATE, root ? 0 : ++m_uniqueID, root),
      m_date(date)
{
}

DAlbum::~DAlbum()
{
}

QDate DAlbum::date() const
{
    return m_date;
}

KURL DAlbum::kurl() const
{
    KURL u;
    u.setProtocol("digikamdates");
    u.setPath(QString("/%1/%2")
              .arg(m_date.year())
              .arg(m_date.month()));

    return u;
}

// --------------------------------------------------------------------------

SAlbum::SAlbum(int id, const KURL& url, bool simple, bool root)
    : Album(Album::SEARCH, id, root),
      m_kurl(url), m_simple(simple)
{
    setTitle(url.queryItem("name"));
}

SAlbum::~SAlbum()
{

}

KURL SAlbum::kurl() const
{
    return m_kurl;
}

bool SAlbum::isSimple() const
{
    return m_simple;
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

