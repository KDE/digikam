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
 * Copyright (C) 2004-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include <klocalizedstring.h>

// LibKipi includes

#include <KIPI/Interface>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "coredb.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "coredbaccess.h"
#include "fileactionmngr.h"
#include "imageattributeswatch.h"
#include "imagecomments.h"
#include "imageinfo.h"
#include "imageposition.h"
#include "imagecopyright.h"
#include "digikam_globals.h"
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

KipiImageInfo::KipiImageInfo(Interface* const interface, const QUrl& url)
    : ImageInfoShared(interface, url) , d(new Private)
{
    d->info = ScanController::instance()->scannedInfo(url.toLocalFile());

    if (d->info.isNull())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DB Info is null (" << url.path() << ")";
    }
}

KipiImageInfo::~KipiImageInfo()
{
    delete d;
}

void KipiImageInfo::cloneData(ImageInfoShared* const other)
{
    QUrl otherUrl = other->url();

    ImageInfo parentInf = ImageInfo::fromUrl(otherUrl);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Clone DB Info from" << parentInf.fileUrl().path() << "to" << _url.path();
    FileActionMngr::instance()->copyAttributes(parentInf, _url.toLocalFile());
}

QMap<QString, QVariant> KipiImageInfo::attributes()
{
    QMap<QString, QVariant> res;

    PAlbum* p = d->parentAlbum();

    if (p)
    {
        // Get item name property from database
        res[QLatin1String("name")]          = d->info.name();

        // Get default title property from database
        res[QLatin1String("comment")]       = d->info.comment();

        // Get default title property from database
        res[QLatin1String("title")]         = d->info.title();

        // Get date property from database
        res[QLatin1String("date")]          = d->info.dateTime();
        res[QLatin1String("isexactdate")]   = true;

        // Get orientation property from database
        res[QLatin1String("orientation")]   = d->info.orientation();
        res[QLatin1String("angle")]         = d->info.orientation();           // NOTE: for compatibility. Deprecated and replaced by "orientation".

        // Get digiKam Tags Path list of picture from database.
        // Ex.: "City/Paris/Monuments/Notre Dame"
        QList<int> tagIds    = d->info.tagIds();
        QStringList tagspath = AlbumManager::instance()->tagPaths(tagIds, false);
        res[QLatin1String("tagspath")]      = tagspath;

        // Get digiKam Tags name (keywords) list of picture from database.
        // Ex.: "Notre Dame"
        QStringList tags     = AlbumManager::instance()->tagNames(tagIds);
        res[QLatin1String("keywords")]      = tags;
        res[QLatin1String("tags")]          = tags;                          // NOTE: for compatibility. Deprecated and replaced by "keywords".

        // Get digiKam Rating of picture from database.
        int rating           = d->info.rating();
        res[QLatin1String("rating")]        = rating;

        // Get digiKam Color Label of picture from database.
        int color            = d->info.colorLabel();
        res[QLatin1String("colorlabel")]    = color;

        // Get digiKam Pick Label of picture from database.
        int pick             = d->info.pickLabel();
        res[QLatin1String("picklabel")]     = pick;

        // Get GPS location of picture from database.
        ImagePosition pos    = d->info.imagePosition();

        if (!pos.isEmpty())
        {
            double lat       = pos.latitudeNumber();
            double lng       = pos.longitudeNumber();
            double alt       = pos.altitude();
            res[QLatin1String("latitude")]  = lat;
            res[QLatin1String("longitude")] = lng;
            res[QLatin1String("altitude")]  = alt;
        }

        // Get file size from database.
        qlonglong size       = d->info.fileSize();
        res[QLatin1String("filesize")]      = size;

        // Get copyright information of picture from database.
        ImageCopyright rights = d->info.imageCopyright();
        res[QLatin1String("creators")]       = rights.creator();
        res[QLatin1String("credit")]         = rights.credit();
        res[QLatin1String("rights")]         = rights.rights();
        res[QLatin1String("source")]         = rights.source();

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database
    }

    return res;
}

void KipiImageInfo::addAttributes(const QMap<QString, QVariant>& res)
{
    PAlbum* const p = d->parentAlbum();

    if (p)
    {
        QMap<QString, QVariant> attributes = res;

        // Here we get informed that a plugin has renamed item
        if (attributes.contains(QLatin1String("name")))
        {
            QString newName = attributes[QLatin1String("name")].toString();

            if (p && !newName.isEmpty())
            {
                CoreDbAccess().db()->moveItem(p->id(), _url.fileName(), p->id(), newName);

                QUrl u(_url);
                u    = u.adjusted(QUrl::StripTrailingSlash);
                u.adjusted(QUrl::RemoveFilename);

                _url = u;
                _url.setPath(newName);
            }

            attributes.remove(QLatin1String("name"));
        }

        // Set digiKam default Title of picture into database.
        if (attributes.contains(QLatin1String("comment")))
        {
            QString comment        = attributes[QLatin1String("comment")].toString();
            CoreDbAccess access;
            ImageComments comments = d->info.imageComments(access);
            // we set a comment with default language, author and date null
            comments.addComment(comment);
            attributes.remove(QLatin1String("comment"));
        }

        // Set digiKam date of picture into database.
        if (attributes.contains(QLatin1String("date")))
        {
            QDateTime date = attributes[QLatin1String("date")].toDateTime();
            d->info.setDateTime(date);
            attributes.remove(QLatin1String("date"));
        }

        // Set digiKam orientation of picture into database.
        if (attributes.contains(QLatin1String("angle")) ||                 // NOTE: for compatibility. Deprecated and replaced by "orientation".
            attributes.contains(QLatin1String("orientation")))
        {
            int angle = attributes[QLatin1String("orientation")].toInt();
            d->info.setOrientation(angle);
            attributes.remove(QLatin1String("angle"));
            attributes.remove(QLatin1String("orientation"));
        }

        // Set digiKam default Title of picture into database.
        if (attributes.contains(QLatin1String("title")))
        {
            QString title          = attributes[QLatin1String("title")].toString();
            CoreDbAccess access;
            ImageComments comments = d->info.imageComments(access);
            // we set a comment with default language, author and date null
            comments.addTitle(title);
            attributes.remove(QLatin1String("title"));
        }

        // Set digiKam Tags Path list of picture into database.
        // Ex.: "City/Paris/Monuments/Notre Dame"
        if (attributes.contains(QLatin1String("tagspath")))
        {
            QStringList tagspaths = attributes[QLatin1String("tagspath")].toStringList();

            QList<int> tagIds     = TagsCache::instance()->getOrCreateTags(tagspaths);
            CoreDbAccess().db()->addTagsToItems(QList<qlonglong>() << d->info.id(), tagIds);
            attributes.remove(QLatin1String("tagspath"));
        }

        // Set digiKam Rating of picture into database.
        if (attributes.contains(QLatin1String("rating")))
        {
            int rating = attributes[QLatin1String("rating")].toInt();

            if (rating >= RatingMin && rating <= RatingMax)
            {
                d->info.setRating(rating);
            }

            attributes.remove(QLatin1String("rating"));
        }

        // Set digiKam Color Label of picture into database.
        if (attributes.contains(QLatin1String("colorlabel")))
        {
            int color = attributes[QLatin1String("colorlabel")].toInt();

            if (color >= NoColorLabel && color <= WhiteLabel)
            {
                d->info.setColorLabel(color);
            }

            attributes.remove(QLatin1String("colorlabel"));
        }

        // Set digiKam Pick Label of picture into database.
        if (attributes.contains(QLatin1String("picklabel")))
        {
            int pick = attributes[QLatin1String("picklabel")].toInt();

            if (pick >= NoPickLabel && pick <= AcceptedLabel)
            {
                d->info.setPickLabel(pick);
            }

            attributes.remove(QLatin1String("picklabel"));
        }

        // GPS location management from plugins.

        if (attributes.contains(QLatin1String("latitude"))  ||
            attributes.contains(QLatin1String("longitude")) ||
            attributes.contains(QLatin1String("altitude")))
        {
            ImagePosition position = d->info.imagePosition();

            // Set GPS latitude location of picture into database.
            if (attributes.contains(QLatin1String("latitude")))
            {
                double lat = attributes[QLatin1String("latitude")].toDouble();

                if (lat >= -90.0 && lat <= 90.0)
                {
                    position.setLatitude(lat);
                }

                attributes.remove(QLatin1String("latitude"));
            }

            // Set GPS longitude location of picture into database.
            if (attributes.contains(QLatin1String("longitude")))
            {
                double lng = attributes[QLatin1String("longitude")].toDouble();

                if (lng >= -180.0 && lng <= 180.0)
                {
                    position.setLongitude(lng);
                }

                attributes.remove(QLatin1String("longitude"));
            }

            // Set GPS altitude location of picture into database.
            if (attributes.contains(QLatin1String("altitude")))
            {
                double alt = attributes[QLatin1String("altitude")].toDouble();
                position.setAltitude(alt);
                attributes.remove(QLatin1String("altitude"));
            }

            position.apply();
        }

        // Copyright information management from plugins.

        if (attributes.contains(QLatin1String("creators")) ||
            attributes.contains(QLatin1String("credit"))   ||
            attributes.contains(QLatin1String("rights"))   ||
            attributes.contains(QLatin1String("source")))
        {
            ImageCopyright rights = d->info.imageCopyright();

            if (attributes.contains(QLatin1String("creators")))
            {
                QStringList list = attributes[QLatin1String("creators")].toStringList();

                if (!list.isEmpty())
                {
                    rights.removeCreators();

                    foreach(const QString& val, list)
                    {
                        rights.setCreator(val, ImageCopyright::AddEntryToExisting);
                    }
                }
                attributes.remove(QLatin1String("creators"));
            }

            if (attributes.contains(QLatin1String("credit")))
            {
                rights.setCredit(attributes[QLatin1String("credit")].toString());
                attributes.remove(QLatin1String("credit"));
            }

            if (attributes.contains(QLatin1String("rights")))
            {
                rights.setRights(attributes[QLatin1String("rights")].toString());
                attributes.remove(QLatin1String("rights"));
            }

            if (attributes.contains(QLatin1String("source")))
            {
                rights.setSource(attributes[QLatin1String("source")].toString());
                attributes.remove(QLatin1String("source"));
            }
        }

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database

        // Remove read-only attributes.

        attributes.remove(QLatin1String("filesize"));
        attributes.remove(QLatin1String("isexactdate"));
        attributes.remove(QLatin1String("keywords"));
        attributes.remove(QLatin1String("tags"));

        // Check if others attributes are not yet managed here...
        if (!attributes.isEmpty())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "These attributes are not yet managed by digiKam KIPI interface: " << attributes;
        }
    }

    // To update sidebar content. Some kipi-plugins use this way to refresh sidebar
    // using an empty QMap().
    ImageAttributesWatch::instance()->fileMetadataChanged(_url);
}

void KipiImageInfo::delAttributes(const QStringList& res)
{
    PAlbum* const p = d->parentAlbum();

    if (p)
    {
        QStringList attributes = res;

        // Remove all Comments.
        if (attributes.contains(QLatin1String("comment")))
        {
            CoreDbAccess access;
            ImageComments comments = d->info.imageComments(access);
            comments.removeAll(DatabaseComment::Comment);
            attributes.removeAll(QLatin1String("comment"));
        }

        // Remove date.
        if (attributes.contains(QLatin1String("date")))
        {
            d->info.setDateTime(QDateTime());
            attributes.removeAll(QLatin1String("date"));
        }

        // Remove orientation.
        if (attributes.contains(QLatin1String("angle")) ||                 // NOTE: for compatibility. Deprecated and replaced by "orientation".
            attributes.contains(QLatin1String("orientation")))
        {
            d->info.setOrientation(0);
            attributes.removeAll(QLatin1String("angle"));
            attributes.removeAll(QLatin1String("orientation"));
        }

        // Remove all Titles.
        if (attributes.contains(QLatin1String("title")))
        {
            CoreDbAccess access;
            ImageComments comments = d->info.imageComments(access);
            comments.removeAll(DatabaseComment::Title);
            attributes.removeAll(QLatin1String("title"));
        }

        // Remove all tags of a picture from database.
        if (attributes.contains(QLatin1String("tags")) ||                   // NOTE: for compatibility. Deprecated and replaced by "tagspath".
            attributes.contains(QLatin1String("tagspath"))
           )
        {
            d->info.removeAllTags();
            attributes.removeAll(QLatin1String("tags"));
            attributes.removeAll(QLatin1String("tagspath"));
        }

        // Remove digiKam Rating of picture from database.
        if (attributes.contains(QLatin1String("rating")))
        {
            d->info.setRating(RatingMin);
            attributes.removeAll(QLatin1String("rating"));
        }

        // Remove digiKam Color Label of picture from database.
        if (attributes.contains(QLatin1String("colorlabel")))
        {
            d->info.setColorLabel(NoColorLabel);
            attributes.removeAll(QLatin1String("colorlabel"));
        }

        // Remove digiKam Pick Label of picture from database.
        if (attributes.contains(QLatin1String("picklabel")))
        {
            d->info.setPickLabel(NoPickLabel);
            attributes.removeAll(QLatin1String("picklabel"));
        }

        // Remove GPS location management from database.
        if (attributes.contains(QLatin1String("gpslocation")))
        {
            ImagePosition position = d->info.imagePosition();
            position.remove();
            position.apply();
            attributes.removeAll(QLatin1String("gpslocation"));
        }

        // Remove copyrights information from database.
        if (attributes.contains(QLatin1String("copyrights")))
        {
            ImageCopyright rights = d->info.imageCopyright();
            rights.removeAll();
            attributes.removeAll(QLatin1String("copyrights"));
        }

        // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database

        if (!attributes.isEmpty())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "These attributes are not yet managed by digiKam KIPI interface: " << attributes;
        }
    }

    // To update sidebar content. Some kipi-plugins use this way to refresh sidebar
    // using an empty QMap().
    ImageAttributesWatch::instance()->fileMetadataChanged(_url);
}

void KipiImageInfo::clearAttributes()
{
    QStringList attr;
    attr.append(QLatin1String("comment"));
    attr.append(QLatin1String("date"));
    attr.append(QLatin1String("title"));
    attr.append(QLatin1String("orientation"));
    attr.append(QLatin1String("tagspath"));
    attr.append(QLatin1String("rating"));
    attr.append(QLatin1String("colorlabel"));
    attr.append(QLatin1String("picklabel"));
    attr.append(QLatin1String("gpslocation"));
    attr.append(QLatin1String("copyrights"));

    // NOTE: add here a kipi-plugins access to future picture attributes stored by digiKam database

    delAttributes(attr);
}

}  // namespace Digikam
