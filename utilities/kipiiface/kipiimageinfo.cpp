/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to get/set image information/properties
 *               in a digiKam album.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kipiimageinfo.h"

// KDE includes

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "imageattributeswatch.h"
#include "imagecomments.h"
#include "imageposition.h"
#include "globals.h"
#include "tagscache.h"

namespace Digikam
{

KipiImageInfo::KipiImageInfo(KIPI::Interface* const interface, const KUrl& url)
    : KIPI::ImageInfoShared(interface, url)
{
    m_info = ImageInfo(url);
}

KipiImageInfo::~KipiImageInfo()
{
}

PAlbum* KipiImageInfo::parentAlbum() const
{
    return AlbumManager::instance()->findPAlbum(m_info.albumId());
}

#if KIPI_VERSION >= 0x010200
void KipiImageInfo::cloneData( ImageInfoShared* const other )
#else
void KipiImageInfo::cloneData( ImageInfoShared* other )
#endif
{
    addAttributes( other->attributes() );
}

QMap<QString, QVariant> KipiImageInfo::attributes()
{
    QMap<QString, QVariant> res;

    PAlbum* p = parentAlbum();

    if (p)
    {
        // Get item name property from database
        res["name"]          = m_info.name();

        // Get default title property from database
        res["comment"]       = m_info.comment();

        // Get default title property from database
        res["title"]         = m_info.title();

        // Get date property from database
        res["date"]          = m_info.dateTime();
        res["isexactdate"]   = true;

        // Get orientation property from database
        res["orientation"]   = m_info.orientation();
        res["angle"]         = m_info.orientation();           // NOTE: for compatibility. Deprecated and replaced by "orientation".

        // Get digiKam Tags Path list of picture from database.
        // Ex.: "City/Paris/Monuments/Notre Dame"
        QList<int> tagIds    = m_info.tagIds();
        QStringList tagspath = AlbumManager::instance()->tagPaths(tagIds, false);
        res["tagspath"]      = tagspath;

        // Get digiKam Tags name (keywords) list of picture from database.
        // Ex.: "Notre Dame"
        QStringList tags     = AlbumManager::instance()->tagNames(tagIds);
        res["keywords"]      = tags;
        res["tags"]          = tags;                          // NOTE: for compatibility. Deprecated and replaced by "keywords".

        // Get digiKam Rating of picture from database.
        int rating           = m_info.rating();
        res["rating"]        = rating;

        // Get digiKam Color Label of picture from database.
        int color            = m_info.colorLabel();
        res["colorlabel"]    = color;

        // Get digiKam Pick Label of picture from database.
        int pick             = m_info.pickLabel();
        res["picklabel"]     = pick;

        // Get GPS location of picture from database.
        ImagePosition pos    = m_info.imagePosition();

        if (!pos.isEmpty())
        {
            double lat       = pos.latitudeNumber();
            double lng       = pos.longitudeNumber();
            double alt       = pos.altitude();
            res["latitude"]  = lat;
            res["longitude"] = lng;
            res["altitude"]  = alt;
        }

        // Get file size from database.
        qlonglong size       = m_info.fileSize();
        res["filesize"]      = size;

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database
    }

    return res;
}

void KipiImageInfo::addAttributes(const QMap<QString, QVariant>& res)
{
    PAlbum* p = parentAlbum();

    if (p)
    {
        QMap<QString, QVariant> attributes = res;

        // Here we get informed that a plugin has renamed item
        if (attributes.contains("name"))
        {
            PAlbum* p       = parentAlbum();
            QString newName = attributes["name"].toString();

            if (p && !newName.isEmpty())
            {
                DatabaseAccess().db()->moveItem(p->id(), _url.fileName(), p->id(), newName);
                _url = _url.upUrl();
                _url.addPath(newName);
            }
            attributes.remove("name");
        }

        // Set digiKam default Title of picture into database.
        if (attributes.contains("comment"))
        {
            QString comment        = attributes["comment"].toString();
            DatabaseAccess access;
            ImageComments comments = m_info.imageComments(access);
            // we set a comment with default language, author and date null
            comments.addComment(comment);
            attributes.remove("comment");
        }

        // Set digiKam date of picture into database.
        if (attributes.contains("date"))
        {
            QDateTime date = attributes["date"].toDateTime();
            m_info.setDateTime(date);
            attributes.remove("date");
        }

        // Set digiKam orientation of picture into database.
        if (attributes.contains("angle") ||                 // NOTE: for compatibility. Deprecated and replaced by "orientation".
            attributes.contains("orientation"))
        {
            int angle = attributes["orientation"].toInt();
            m_info.setOrientation(angle);
            attributes.remove("angle");
            attributes.remove("orientation");
        }

        // Set digiKam default Title of picture into database.
        if (attributes.contains("title"))
        {
            QString title          = attributes["title"].toString();
            DatabaseAccess access;
            ImageComments comments = m_info.imageComments(access);
            // we set a comment with default language, author and date null
            comments.addTitle(title);
            attributes.remove("title");
        }

        // Set digiKam Tags Path list of picture into database.
        // Ex.: "City/Paris/Monuments/Notre Dame"
        if (attributes.contains("tagspath"))
        {
            QStringList tagspaths = attributes["tagspath"].toStringList();

            QList<int> tagIds     = TagsCache::instance()->getOrCreateTags(tagspaths);
            DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << m_info.id(), tagIds);
            attributes.remove("tagspath");
        }

        // Set digiKam Rating of picture into database.
        if (attributes.contains("rating"))
        {
            int rating = attributes["rating"].toInt();

            if (rating >= RatingMin && rating <= RatingMax)
            {
                m_info.setRating(rating);
            }
            attributes.remove("rating");
        }

        // Set digiKam Color Label of picture into database.
        if (attributes.contains("colorlabel"))
        {
            int color = attributes["colorlabel"].toInt();

            if (color >= NoColorLabel && color <= WhiteLabel)
            {
                m_info.setColorLabel(color);
            }
            attributes.remove("colorlabel");
        }

        // Set digiKam Pick Label of picture into database.
        if (attributes.contains("picklabel"))
        {
            int pick = attributes["picklabel"].toInt();

            if (pick >= NoPickLabel && pick <= AcceptedLabel)
            {
                m_info.setPickLabel(pick);
            }
            attributes.remove("picklabel");
        }

        // GPS location management from plugins.

        if (attributes.contains("latitude")  ||
            attributes.contains("longitude") ||
            attributes.contains("altitude"))
        {
            ImagePosition position = m_info.imagePosition();

            // Set GPS latitude location of picture into database.
            if (attributes.contains("latitude"))
            {
                double lat = attributes["latitude"].toDouble();

                if (lat >= -90.0 && lat <= 90.0)
                {
                    position.setLatitude(lat);
                }
                attributes.remove("latitude");
            }

            // Set GPS longitude location of picture into database.
            if (attributes.contains("longitude"))
            {
                double lng = attributes["longitude"].toDouble();

                if (lng >= -180.0 && lng <= 180.0)
                {
                    position.setLongitude(lng);
                }
                attributes.remove("longitude");
            }

            // Set GPS altitude location of picture into database.
            if (attributes.contains("altitude"))
            {
                double alt = attributes["altitude"].toDouble();
                position.setAltitude(alt);
                attributes.remove("altitude");
            }

            position.apply();
        }

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database

        if (!attributes.isEmpty())
            kWarning() << "These attributes are not yet managed by digiKam KIPI interface: " << attributes;
    }

    // To update sidebar content. Some kipi-plugins use this way to refresh sidebar
    // using an empty QMap().
    ImageAttributesWatch::instance()->fileMetadataChanged(_url);
}

void KipiImageInfo::delAttributes(const QStringList& res)
{
    PAlbum* p = parentAlbum();

    if (p)
    {
        QStringList attributes = res;

        // Remove all Comments.
        if (attributes.contains("comment"))
        {
            DatabaseAccess access;
            ImageComments comments = m_info.imageComments(access);
            comments.removeAll(DatabaseComment::Comment);
            attributes.removeAll("comment");
        }

        // Remove date.
        if (attributes.contains("date"))
        {
            m_info.setDateTime(QDateTime());
            attributes.removeAll("date");
        }

        // Remove orientation.
        if (attributes.contains("angle") ||                 // NOTE: for compatibility. Deprecated and replaced by "orientation".
            attributes.contains("orientation"))
        {
            m_info.setOrientation(0);
            attributes.removeAll("angle");
            attributes.removeAll("orientation");
        }

        // Remove all Titles.
        if (attributes.contains("title"))
        {
            DatabaseAccess access;
            ImageComments comments = m_info.imageComments(access);
            comments.removeAll(DatabaseComment::Title);
            attributes.removeAll("title");
        }

        // Remove all tags of a picture from database.
        if (attributes.contains("tags") ||                   // NOTE: for compatibility. Deprecated and replaced by "tagspath".
            attributes.contains("tagspath")
        )
        {
            m_info.removeAllTags();
            attributes.removeAll("tags");
            attributes.removeAll("tagspath");
        }

        // Remove digiKam Rating of picture from database.
        if (attributes.contains("rating"))
        {
            m_info.setRating(RatingMin);
            attributes.removeAll("rating");
        }

        // Remove digiKam Color Label of picture from database.
        if (attributes.contains("colorlabel"))
        {
            m_info.setColorLabel(NoColorLabel);
            attributes.removeAll("colorlabel");
        }

        // Remove digiKam Pick Label of picture from database.
        if (attributes.contains("picklabel"))
        {
            m_info.setPickLabel(NoPickLabel);
            attributes.removeAll("picklabel");
        }

        // GPS location management from plugins.

        if (attributes.contains("gpslocation"))
        {
            ImagePosition position = m_info.imagePosition();
            position.remove();
            position.apply();
            attributes.removeAll("gpslocation");
        }

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database

        if (!attributes.isEmpty())
            kWarning() << "These attributes are not yet managed by digiKam KIPI interface: " << attributes;
    }

    // To update sidebar content. Some kipi-plugins use this way to refresh sidebar
    // using an empty QMap().
    ImageAttributesWatch::instance()->fileMetadataChanged(_url);
}

void KipiImageInfo::clearAttributes()
{
    QStringList attr;
    attr.append("comment");
    attr.append("date");
    attr.append("title");
    attr.append("orientation");
    attr.append("tagspath");
    attr.append("rating");
    attr.append("colorlabel");
    attr.append("picklabel");
    attr.append("gpslocation");

    // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database

    delAttributes(attr);
}

/// DEPRECATED METHODS with libkipi 1.5.0. Use attributes()/addAttributes() methods instead.

#if KIPI_VERSION < 0x010500

#if KIPI_VERSION >= 0x010300

QString KipiImageInfo::name()
{
    QMap<QString, QVariant> map = attributes();
    if (!map.isEmpty()) return map.value("name", QString()).toString();
    return QString();
}

void KipiImageInfo::setName(const QString& newName)
{
    QMap<QString, QVariant> map;
    map.insert("name", newName);
    addAttributes(map);
}

#else // KIPI_VERSION >= 0x010300

QString KipiImageInfo::title()
{
    QMap<QString, QVariant> map = attributes();
    if (!map.isEmpty()) return map.value("name", QString()).toString();
    return QString();
}

void KipiImageInfo::setTitle(const QString& newName)
{
    QMap<QString, QVariant> map;
    map.insert("name", newName);
    addAttributes(map);
}

#endif // KIPI_VERSION >= 0x010300

QString KipiImageInfo::description()
{
    QMap<QString, QVariant> map = attributes();
    if (!map.isEmpty()) return map.value("comment", QString()).toString();
    return QString();
}

void KipiImageInfo::setDescription( const QString& description )
{
    QMap<QString, QVariant> map;
    map.insert("comment", description);
    addAttributes(map);
}

int KipiImageInfo::angle()
{
    QMap<QString, QVariant> map = attributes();
    if (!map.isEmpty()) return map.value("angle", 0).toInt();
    return 0;
}

void KipiImageInfo::setAngle(int orientation)
{
    QMap<QString, QVariant> map;
    map.insert("angle", orientation);
    addAttributes(map);
}

QDateTime KipiImageInfo::time(KIPI::TimeSpec)
{
    QMap<QString, QVariant> map = attributes();
    if (!map.isEmpty()) return map.value("date", QDateTime()).toDateTime();
    return QDateTime();
}

void KipiImageInfo::setTime(const QDateTime& date, KIPI::TimeSpec)
{
    if ( !date.isValid() )
    {
        kWarning() << "Invalid datetime specified";
        return;
    }

    QMap<QString, QVariant> map;
    map.insert("date", date);
    addAttributes(map);
}

#endif // KIPI_VERSION < 0x010500

}  // namespace Digikam
