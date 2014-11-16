/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to get/set image information/properties
 *               in a digiKam album.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well dot com>
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

// Qt includes

#include <QDir>
#include <QDateTime>

// KDE includes

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>

// LibKipi includes

#include <libkipi/interface.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "databaseaccess.h"
#include "fileactionmngr.h"
#include "imageattributeswatch.h"
#include "imagecomments.h"
#include "imageinfo.h"
#include "imageposition.h"
#include "imagecopyright.h"
#include "globals.h"
#include "scancontroller.h"
#include "tagscache.h"

namespace Digikam
{

class KipiImageInfo::Private
{
public:

    Private()
    {
    }

    PAlbum* parentAlbum() const
    {
        return AlbumManager::instance()->findPAlbum(info.albumId());
    }

public:

    ImageInfo info;
};

KipiImageInfo::KipiImageInfo(Interface* const interface, const KUrl& url)
    : ImageInfoShared(interface, url) , d(new Private)
{
    d->info = ScanController::instance()->scannedInfo(url.toLocalFile());

    if (d->info.isNull())
    {
        kDebug() << "DB Info is null (" << url.path() << ")";
    }
}

KipiImageInfo::~KipiImageInfo()
{
    delete d;
}

void KipiImageInfo::cloneData(ImageInfoShared* const other)
{
    KUrl otherUrl = other->url();

    ImageInfo parentInf = ImageInfo::fromUrl(otherUrl);
    kDebug() << "Clone DB Info from" << parentInf.fileUrl().path() << "to" << _url.path();
    FileActionMngr::instance()->copyAttributes(parentInf, _url.toLocalFile());
}

QMap<QString, QVariant> KipiImageInfo::attributes()
{
    QMap<QString, QVariant> res;

    PAlbum* p = d->parentAlbum();

    if (p)
    {
        // Get item name property from database
        res["name"]          = d->info.name();

        // Get default title property from database
        res["comment"]       = d->info.comment();

        // Get default title property from database
        res["title"]         = d->info.title();

        // Get date property from database
        res["date"]          = d->info.dateTime();
        res["isexactdate"]   = true;

        // Get orientation property from database
        res["orientation"]   = d->info.orientation();
        res["angle"]         = d->info.orientation();           // NOTE: for compatibility. Deprecated and replaced by "orientation".

        // Get digiKam Tags Path list of picture from database.
        // Ex.: "City/Paris/Monuments/Notre Dame"
        QList<int> tagIds    = d->info.tagIds();
        QStringList tagspath = AlbumManager::instance()->tagPaths(tagIds, false);
        res["tagspath"]      = tagspath;

        // Get digiKam Tags name (keywords) list of picture from database.
        // Ex.: "Notre Dame"
        QStringList tags     = AlbumManager::instance()->tagNames(tagIds);
        res["keywords"]      = tags;
        res["tags"]          = tags;                          // NOTE: for compatibility. Deprecated and replaced by "keywords".

        // Get digiKam Rating of picture from database.
        int rating           = d->info.rating();
        res["rating"]        = rating;

        // Get digiKam Color Label of picture from database.
        int color            = d->info.colorLabel();
        res["colorlabel"]    = color;

        // Get digiKam Pick Label of picture from database.
        int pick             = d->info.pickLabel();
        res["picklabel"]     = pick;

        // Get GPS location of picture from database.
        ImagePosition pos    = d->info.imagePosition();

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
        qlonglong size       = d->info.fileSize();
        res["filesize"]      = size;

        // Get copyright information of picture from database.
        ImageCopyright rights = d->info.imageCopyright();
        res["creators"]       = rights.creator();
        res["credit"]         = rights.credit();
        res["rights"]         = rights.rights();
        res["source"]         = rights.source();

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database
    }

    return res;
}

void KipiImageInfo::addAttributes(const QMap<QString, QVariant>& res)
{
    PAlbum* p = d->parentAlbum();

    if (p)
    {
        QMap<QString, QVariant> attributes = res;

        // Here we get informed that a plugin has renamed item
        if (attributes.contains("name"))
        {
            PAlbum* p       = d->parentAlbum();
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
            ImageComments comments = d->info.imageComments(access);
            // we set a comment with default language, author and date null
            comments.addComment(comment);
            attributes.remove("comment");
        }

        // Set digiKam date of picture into database.
        if (attributes.contains("date"))
        {
            QDateTime date = attributes["date"].toDateTime();
            d->info.setDateTime(date);
            attributes.remove("date");
        }

        // Set digiKam orientation of picture into database.
        if (attributes.contains("angle") ||                 // NOTE: for compatibility. Deprecated and replaced by "orientation".
            attributes.contains("orientation"))
        {
            int angle = attributes["orientation"].toInt();
            d->info.setOrientation(angle);
            attributes.remove("angle");
            attributes.remove("orientation");
        }

        // Set digiKam default Title of picture into database.
        if (attributes.contains("title"))
        {
            QString title          = attributes["title"].toString();
            DatabaseAccess access;
            ImageComments comments = d->info.imageComments(access);
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
            DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << d->info.id(), tagIds);
            attributes.remove("tagspath");
        }

        // Set digiKam Rating of picture into database.
        if (attributes.contains("rating"))
        {
            int rating = attributes["rating"].toInt();

            if (rating >= RatingMin && rating <= RatingMax)
            {
                d->info.setRating(rating);
            }

            attributes.remove("rating");
        }

        // Set digiKam Color Label of picture into database.
        if (attributes.contains("colorlabel"))
        {
            int color = attributes["colorlabel"].toInt();

            if (color >= NoColorLabel && color <= WhiteLabel)
            {
                d->info.setColorLabel(color);
            }

            attributes.remove("colorlabel");
        }

        // Set digiKam Pick Label of picture into database.
        if (attributes.contains("picklabel"))
        {
            int pick = attributes["picklabel"].toInt();

            if (pick >= NoPickLabel && pick <= AcceptedLabel)
            {
                d->info.setPickLabel(pick);
            }

            attributes.remove("picklabel");
        }

        // GPS location management from plugins.

        if (attributes.contains("latitude")  ||
            attributes.contains("longitude") ||
            attributes.contains("altitude"))
        {
            ImagePosition position = d->info.imagePosition();

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

        // Copyright information management from plugins.

        if (attributes.contains("creators") ||
            attributes.contains("credit")   ||
            attributes.contains("rights")   ||
            attributes.contains("source"))
        {
            ImageCopyright rights = d->info.imageCopyright();

            if (attributes.contains("creators"))
            {
                QStringList list = attributes["creators"].toStringList();
                if (!list.isEmpty())
                {
                    rights.removeCreators();
                    foreach(const QString& val, list)
                    {
                        rights.setCreator(val, ImageCopyright::AddEntryToExisting);
                    }
                }
                attributes.remove("creators");
            }

            if (attributes.contains("credit"))
            {
                rights.setCredit(attributes["credit"].toString());
                attributes.remove("credit");
            }

            if (attributes.contains("rights"))
            {
                rights.setRights(attributes["rights"].toString());
                attributes.remove("rights");
            }

            if (attributes.contains("source"))
            {
                rights.setSource(attributes["source"].toString());
                attributes.remove("source");
            }
        }

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database

        // Remove read-only attributes.

        attributes.remove("filesize");
        attributes.remove("isexactdate");
        attributes.remove("keywords");
        attributes.remove("tags");

        // Check if others attributes are not yet managed here...
        if (!attributes.isEmpty())
        {
            kWarning() << "These attributes are not yet managed by digiKam KIPI interface: " << attributes;
        }
    }

    // To update sidebar content. Some kipi-plugins use this way to refresh sidebar
    // using an empty QMap().
    ImageAttributesWatch::instance()->fileMetadataChanged(_url);
}

void KipiImageInfo::delAttributes(const QStringList& res)
{
    PAlbum* p = d->parentAlbum();

    if (p)
    {
        QStringList attributes = res;

        // Remove all Comments.
        if (attributes.contains("comment"))
        {
            DatabaseAccess access;
            ImageComments comments = d->info.imageComments(access);
            comments.removeAll(DatabaseComment::Comment);
            attributes.removeAll("comment");
        }

        // Remove date.
        if (attributes.contains("date"))
        {
            d->info.setDateTime(QDateTime());
            attributes.removeAll("date");
        }

        // Remove orientation.
        if (attributes.contains("angle") ||                 // NOTE: for compatibility. Deprecated and replaced by "orientation".
            attributes.contains("orientation"))
        {
            d->info.setOrientation(0);
            attributes.removeAll("angle");
            attributes.removeAll("orientation");
        }

        // Remove all Titles.
        if (attributes.contains("title"))
        {
            DatabaseAccess access;
            ImageComments comments = d->info.imageComments(access);
            comments.removeAll(DatabaseComment::Title);
            attributes.removeAll("title");
        }

        // Remove all tags of a picture from database.
        if (attributes.contains("tags") ||                   // NOTE: for compatibility. Deprecated and replaced by "tagspath".
            attributes.contains("tagspath")
           )
        {
            d->info.removeAllTags();
            attributes.removeAll("tags");
            attributes.removeAll("tagspath");
        }

        // Remove digiKam Rating of picture from database.
        if (attributes.contains("rating"))
        {
            d->info.setRating(RatingMin);
            attributes.removeAll("rating");
        }

        // Remove digiKam Color Label of picture from database.
        if (attributes.contains("colorlabel"))
        {
            d->info.setColorLabel(NoColorLabel);
            attributes.removeAll("colorlabel");
        }

        // Remove digiKam Pick Label of picture from database.
        if (attributes.contains("picklabel"))
        {
            d->info.setPickLabel(NoPickLabel);
            attributes.removeAll("picklabel");
        }

        // Remove GPS location management from database.
        if (attributes.contains("gpslocation"))
        {
            ImagePosition position = d->info.imagePosition();
            position.remove();
            position.apply();
            attributes.removeAll("gpslocation");
        }

        // Remove copyrights information from database.
        if (attributes.contains("copyrights"))
        {
            ImageCopyright rights = d->info.imageCopyright();
            rights.removeAll();
            attributes.removeAll("copyrights");
        }

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database

        if (!attributes.isEmpty())
        {
            kWarning() << "These attributes are not yet managed by digiKam KIPI interface: " << attributes;
        }
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
    attr.append("copyrights");

    // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database

    delAttributes(attr);
}

}  // namespace Digikam
