/* ============================================================
 * File  : albumsettings.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-12
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <qstring.h>
#include <kconfig.h>
#include <klocale.h>

#include "thumbnailsize.h"
#include "albumsettings.h"

class AlbumSettingsPrivate {

public:

    KConfig *config;

    QString     albumLibraryPath;
    QStringList albumCollectionNames;
    QString     fileFilter;

    int            thumbnailSize;
    AlbumSettings::AlbumSortOrder albumSortOrder;

    bool iconShowMime;
    bool iconShowSize;
    bool iconShowDate;
    bool iconShowComments;
    bool iconShowResolution;

};


AlbumSettings* AlbumSettings::instance_ = 0;

AlbumSettings * AlbumSettings::instance()
{
    return instance_;
}

AlbumSettings::AlbumSettings()
{
    d = new AlbumSettingsPrivate;
    d->config = new KConfig("digikamrc");

    instance_ = this;
    
    init();
}

AlbumSettings::~AlbumSettings()
{
    delete d->config;
    delete d;
    instance_ = 0;
}

void AlbumSettings::init()
{
    d->albumCollectionNames.clear();
    d->albumCollectionNames.append(i18n("Family"));
    d->albumCollectionNames.append(i18n("Travel"));
    d->albumCollectionNames.append(i18n("Holidays"));
    d->albumCollectionNames.append(i18n("Friends"));
    d->albumCollectionNames.append(i18n("Nature"));
    d->albumCollectionNames.append(i18n("Party"));
    d->albumCollectionNames.append(i18n("Todo"));
    d->albumCollectionNames.append(i18n("Miscellaneous"));

    d->albumSortOrder = AlbumSettings::ByCollection;
                                          
    d->fileFilter = "*.jpg *.jpeg *.tif *.tiff *.gif *.png *.bmp *.xpm *.ppm *.crw";
    d->thumbnailSize = ThumbnailSize::Medium;

    d->iconShowMime = false;
    d->iconShowSize = true;
    d->iconShowDate = false;
    d->iconShowComments = true;
    d->iconShowResolution = false;
}

void AlbumSettings::readSettings()
{
    KConfig* config = d->config;

    config->setGroup("Album Settings");
    
    d->albumLibraryPath = 
            config->readEntry("Album Path",
                              QString::null);

    QStringList collectionList =
        config->readListEntry("Album Collections");
    if (!collectionList.isEmpty())
        d->albumCollectionNames = collectionList;

    d->albumSortOrder =
        AlbumSettings::AlbumSortOrder (config->readNumEntry("Album Sort Order",
                                                            (int)AlbumSettings::ByCollection));
    
    if (config->hasKey("File Filter"))
        d->fileFilter =
            config->readEntry("File Filter",
                              d->fileFilter);

    if (config->hasKey("Default Icon Size"))
        d->thumbnailSize =
            config->readNumEntry("Default Icon Size",
                                 ThumbnailSize::Medium);


    if (config->hasKey("Icon Show Mime"))
        d->iconShowMime =
            config->readBoolEntry("Icon Show Mime",
                                  false);

    if (config->hasKey("Icon Show Size"))
        d->iconShowSize =
            config->readBoolEntry("Icon Show Size",
                                  true);

    if (config->hasKey("Icon Show Date"))
        d->iconShowDate =
            config->readBoolEntry("Icon Show Date",
                                  false);

    if (config->hasKey("Icon Show Comments"))
        d->iconShowComments =
            config->readBoolEntry("Icon Show Comments",
                                  true);

}

void AlbumSettings::saveSettings()
{
    KConfig* config = d->config;

    config->setGroup("Album Settings");

    config->writeEntry("Album Path", d->albumLibraryPath);

    config->writeEntry("Album Collections",
                       d->albumCollectionNames);

    config->writeEntry("Album Sort Order",
                       (int)d->albumSortOrder);
    
    config->writeEntry("File Filter",
                       d->fileFilter);
    
    config->writeEntry("Default Icon Size",
                       QString::number(d->thumbnailSize));

    config->writeEntry("Icon Show Mime",
                       d->iconShowMime);
    config->writeEntry("Icon Show Size",
                       d->iconShowSize);
    config->writeEntry("Icon Show Date",
                       d->iconShowDate);
    config->writeEntry("Icon Show Comments",
                       d->iconShowComments);
    config->sync();

}

void AlbumSettings::setAlbumLibraryPath(const QString& path)
{
    d->albumLibraryPath = path;    
}

QString AlbumSettings::getAlbumLibraryPath() const
{
    return d->albumLibraryPath;
}

void AlbumSettings::setAlbumCollectionNames(const QStringList& list)
{
    d->albumCollectionNames = list;    
}

QStringList AlbumSettings::getAlbumCollectionNames()
{
    return d->albumCollectionNames;
}


bool AlbumSettings::addAlbumCollectionName(const QString& name)
{
    if (d->albumCollectionNames.contains(name))
        return false;

    d->albumCollectionNames.append(name);
    return true;
}

bool AlbumSettings::delAlbumCollectionName(const QString& name)
{
    uint count = d->albumCollectionNames.remove(name);
    return (count > 0) ? true : false; 
}

void AlbumSettings::setAlbumSortOrder(const AlbumSettings::AlbumSortOrder order)
{
    d->albumSortOrder = order;
}

AlbumSettings::AlbumSortOrder AlbumSettings::getAlbumSortOrder()
{
    return d->albumSortOrder;
}

void AlbumSettings::setFileFilter(const QString& filter)
{
    d->fileFilter = filter;    
}

QString AlbumSettings::getFileFilter() const
{
    return d->fileFilter;
}

void AlbumSettings::setDefaultIconSize(int val)
{
    d->thumbnailSize = val;
}

int AlbumSettings::getDefaultIconSize() const
{
    return d->thumbnailSize;
}


void AlbumSettings::setIconShowMime(bool val)
{
    d->iconShowMime = val;
}

bool AlbumSettings::getIconShowMime() const
{
    return d->iconShowMime;
}

void AlbumSettings::setIconShowSize(bool val)
{
    d->iconShowSize = val;
}

bool AlbumSettings::getIconShowSize() const
{
    return d->iconShowSize;
}

void AlbumSettings::setIconShowComments(bool val)
{
    d->iconShowComments = val;
}

bool AlbumSettings::getIconShowComments() const
{
    return d->iconShowComments;
}

void AlbumSettings::setIconShowDate(bool val)
{
    d->iconShowDate = val;
}

bool AlbumSettings::getIconShowDate() const
{
    return d->iconShowDate;
}
