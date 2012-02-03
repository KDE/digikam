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

#if KIPI_VERSION >= 0x010300
QString KipiImageInfo::name()
{
    return m_info.name();
}

void KipiImageInfo::setName(const QString& newName)
{
    // Here we get informed that a plugin has renamed the file

    PAlbum* p = parentAlbum();

    if ( p && !newName.isEmpty() )
    {
        DatabaseAccess().db()->moveItem(p->id(), _url.fileName(), p->id(), newName);
        _url = _url.upUrl();
        _url.addPath(newName);
    }
}
#else
QString KipiImageInfo::title()
{
    return m_info.name();
}

void KipiImageInfo::setTitle(const QString& newName)
{
    // Here we get informed that a plugin has renamed the file

    PAlbum* p = parentAlbum();

    if ( p && !newName.isEmpty() )
    {
        DatabaseAccess().db()->moveItem(p->id(), _url.fileName(), p->id(), newName);
        _url = _url.upUrl();
        _url.addPath(newName);
    }
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

QDateTime KipiImageInfo::time( KIPI::TimeSpec /*spec*/ )
{
    return m_info.dateTime();
}

void KipiImageInfo::setTime(const QDateTime& time, KIPI::TimeSpec)
{
    if ( !time.isValid() )
    {
        kWarning() << "Invalid datetime specified";
        return;
    }

    m_info.setDateTime(time);
}

int KipiImageInfo::angle()
{
    return m_info.orientation();
}

void KipiImageInfo::setAngle(int angle)
{
    m_info.setOrientation(angle);
}

#if KIPI_VERSION >= 0x010200
void KipiImageInfo::cloneData( ImageInfoShared* const other )
#else
void KipiImageInfo::cloneData( ImageInfoShared* other )
#endif
{
    setTime( other->time(KIPI::FromInfo), KIPI::FromInfo );
    addAttributes( other->attributes() );
}

QMap<QString, QVariant> KipiImageInfo::attributes()
{
    QMap<QString, QVariant> res;

    PAlbum* p = parentAlbum();

    if (p)
    {
        // Get default title property from database
        res["comment"]       = m_info.comment();;

        // Get default title property from database
        res["title"]         = m_info.title();

        // Get date property from database
        res["date"]          = time(KIPI::FromInfo);

        // Get angle property from database
        res["angle"]          = angle();

        QList<int> tagIds    = m_info.tagIds();
        // Get digiKam Tags Path list of picture from database.
        // Ex.: "City/Paris/Monuments/Notre Dame"

        QStringList tagspath = AlbumManager::instance()->tagPaths(tagIds, false);
        res["tagspath"]      = tagspath;

        // Get digiKam Tags name list of picture from database.
        // Ex.: "Notre Dame"
        QStringList tags     = AlbumManager::instance()->tagNames(tagIds);
        res["tags"]          = tags;

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

        // Set digiKam default Title of picture into database.
        if (attributes.contains("comment"))
        {
            QString comment = attributes["comment"].toString();
            DatabaseAccess access;
            ImageComments comments = m_info.imageComments(access);
            // we set a comment with default language, author and date null
            comments.addComment(comment);
        }

        // Set digiKam date of picture into database.
        if (attributes.contains("date"))
        {
            QDateTime date = attributes["date"].toDateTime();
            setTime(date);
        }

        // Set digiKam angle of picture into database.
        if (attributes.contains("angle"))
        {
            int angle = attributes["angle"].toInt();
            setAngle(angle);
        }

        // Set digiKam default Title of picture into database.
        if (attributes.contains("title"))
        {
            QString title          = attributes["title"].toString();
            DatabaseAccess access;
            ImageComments comments = m_info.imageComments(access);
            // we set a comment with default language, author and date null
            comments.addTitle(title);
        }

        // Set digiKam Tags Path list of picture into database.
        // Ex.: "City/Paris/Monuments/Notre Dame"
        if (attributes.find("tagspath") != attributes.end())
        {
            QStringList tagspaths = attributes["tagspath"].toStringList();

            QList<int> tagIds     = TagsCache::instance()->getOrCreateTags(tagspaths);
            DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << m_info.id(), tagIds);
        }

        // Set digiKam Rating of picture into database.
        if (attributes.contains("rating"))
        {
            int rating = attributes["rating"].toInt();

            if (rating >= RatingMin && rating <= RatingMax)
            {
                m_info.setRating(rating);
            }
        }

        // Set digiKam Color Label of picture into database.
        if (attributes.contains("colorlabel"))
        {
            int color = attributes["colorlabel"].toInt();

            if (color >= NoColorLabel && color <= WhiteLabel)
            {
                m_info.setColorLabel(color);
            }
        }

        // Set digiKam Pick Label of picture into database.
        if (attributes.contains("picklabel"))
        {
            int pick = attributes["picklabel"].toInt();

            if (pick >= NoPickLabel && pick <= AcceptedLabel)
            {
                m_info.setPickLabel(pick);
            }
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
            }

            // Set GPS longitude location of picture into database.
            if (attributes.contains("longitude"))
            {
                double lng = attributes["longitude"].toDouble();

                if (lng >= -180.0 && lng <= 180.0)
                {
                    position.setLongitude(lng);
                }
            }

            // Set GPS altitude location of picture into database.
            if (attributes.contains("altitude"))
            {
                double alt = attributes["altitude"].toDouble();
                position.setAltitude(alt);
            }

            position.apply();
        }

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database
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
        // Remove all Comments.
        if (res.contains("comment"))
        {
            DatabaseAccess access;
            ImageComments comments = m_info.imageComments(access);
            comments.removeAll(DatabaseComment::Comment);
        }

        // Remove date.
        if (res.contains("date"))
        {
            m_info.setDateTime(QDateTime());
        }

        // Remove angle.
        if (res.contains("angle"))
        {
            setAngle(0);
        }

        // Remove all Titles.
        if (res.contains("title"))
        {
            DatabaseAccess access;
            ImageComments comments = m_info.imageComments(access);
            comments.removeAll(DatabaseComment::Title);
        }

        // Remove all tags of a picture from database.
        if (res.contains("tags"))
        {
            m_info.removeAllTags();
        }

        // Remove digiKam Rating of picture from database.
        if (res.contains("rating"))
        {
            m_info.setRating(RatingMin);
        }

        // Remove digiKam Color Label of picture from database.
        if (res.contains("colorlabel"))
        {
            m_info.setColorLabel(NoColorLabel);
        }

        // Remove digiKam Pick Label of picture from database.
        if (res.contains("picklabel"))
        {
            m_info.setPickLabel(NoPickLabel);
        }

        // GPS location management from plugins.

        if (res.contains("gpslocation"))
        {
            ImagePosition position = m_info.imagePosition();
            position.remove();
            position.apply();
        }

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database
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
    attr.append("angle");
    attr.append("tags");
    attr.append("rating");
    attr.append("colorlabel");
    attr.append("picklabel");
    attr.append("gpslocation");

    // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database

    delAttributes(attr);
}

}  // namespace Digikam
