//////////////////////////////////////////////////////////////////////////////
//
//    ALBUMSETTINGS.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// Qt includes.
 
#include <qstring.h>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>

// Local includes.

#include "thumbnailsize.h"
#include "albumsettings.h"


class AlbumSettingsPrivate 
{
public:

    KConfig *config;

    QString     albumLibraryPath;
    QStringList albumCollectionNames;
    QString     imageFilefilter;
    QString     movieFilefilter;
    QString     audioFilefilter;
    QString     rawFilefilter;

    int            thumbnailSize;
    
    AlbumSettings::AlbumSortOrder albumSortOrder;

    bool iconShowMime;
    bool iconShowSize;
    bool iconShowDate;
    bool iconShowComments;
    bool iconShowFileComments;
    bool iconShowResolution;
    bool saveExifComments;
    bool exifRotate;
    bool exifSetOrientation;
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
                                          
    d->imageFilefilter = "*.png *.jpg *.jpeg *.tif *.tiff *.gif *.bmp *.xpm *.ppm *.xcf *.pcx";
    d->movieFilefilter = "*.mpeg *.mpg *.avi *.mov";
    d->audioFilefilter = "*.ogg *.mp3 *.wma *.wav";
    d->rawFilefilter =   "*.crw *.nef *.raf *.mrw *.orf";
    d->thumbnailSize = ThumbnailSize::Medium;

    d->iconShowMime = false;
    d->iconShowSize = true;
    d->iconShowDate = false;
    d->iconShowComments = true;
    d->iconShowFileComments = false;
    d->iconShowResolution = false;
    d->saveExifComments = true;
    d->exifRotate = true;
    d->exifSetOrientation = true;
}

void AlbumSettings::readSettings()
{
    KConfig* config = d->config;

    config->setGroup("Album Settings");
    
    d->albumLibraryPath = 
            config->readPathEntry("Album Path",
                              QString::null);

    QStringList collectionList =
        config->readListEntry("Album Collections");
    if (!collectionList.isEmpty())
        d->albumCollectionNames = collectionList;

    d->albumSortOrder =
        AlbumSettings::AlbumSortOrder (config->readNumEntry("Album Sort Order",
                                                            (int)AlbumSettings::ByCollection));
    
    if (config->hasKey("File Filter"))
        d->imageFilefilter =
            config->readEntry("File Filter",
                              d->imageFilefilter);

    if (config->hasKey("Movie File Filter"))
        d->movieFilefilter =
            config->readEntry("Movie File Filter",
                              d->movieFilefilter);

    if (config->hasKey("Audio File Filter"))
        d->audioFilefilter =
            config->readEntry("Audio File Filter",
                              d->audioFilefilter);
                              
    if (config->hasKey("Raw File Filter"))
        d->rawFilefilter =
            config->readEntry("Raw File Filter",
                              d->rawFilefilter);
                              
    if (config->hasKey("Default Icon Size"))
        d->thumbnailSize =
            config->readNumEntry("Default Icon Size",
                                 ThumbnailSize::Medium);

    if (config->hasKey("Icon Show Resolution"))
        d->iconShowResolution =
            config->readBoolEntry("Icon Show Resolution",
                                  false);                                 

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

    if (config->hasKey("Icon Show File Comments"))
        d->iconShowFileComments =
            config->readBoolEntry("Icon Show File Comments",
                                  true);

    config->setGroup("EXIF Settings");

    if (config->hasKey("Save EXIF Comments"))
        d->saveExifComments =
            config->readBoolEntry("Save EXIF Comments",
                                  true);
    
    if (config->hasKey("EXIF Rotate"))
        d->exifRotate =
            config->readBoolEntry("EXIF Rotate",
                                  true);
    
    if (config->hasKey("EXIF Set Orientation"))
        d->exifSetOrientation =
            config->readBoolEntry("EXIF Set Orientation",
                                  true);
}

void AlbumSettings::saveSettings()
{
    KConfig* config = d->config;

    config->setGroup("Album Settings");

    config->writePathEntry("Album Path", d->albumLibraryPath);

    config->writeEntry("Album Collections",
                       d->albumCollectionNames);

    config->writeEntry("Album Sort Order",
                       (int)d->albumSortOrder);
    
    config->writeEntry("File Filter",
                       d->imageFilefilter);

    config->writeEntry("Movie File Filter",
                       d->movieFilefilter);
    
    config->writeEntry("Audio File Filter",
                       d->audioFilefilter);
                           
    config->writeEntry("Raw File Filter",
                       d->rawFilefilter);
    
    config->writeEntry("Default Icon Size",
                       QString::number(d->thumbnailSize));
    
    config->writeEntry("Icon Show Resolution",
                       d->iconShowResolution);
                                                      
    config->writeEntry("Icon Show Mime",
                       d->iconShowMime);
                       
    config->writeEntry("Icon Show Size",
                       d->iconShowSize);
                       
    config->writeEntry("Icon Show Date",
                       d->iconShowDate);
                       
    config->writeEntry("Icon Show Comments",
                       d->iconShowComments);
                       
    config->writeEntry("Icon Show File Comments",
                       d->iconShowFileComments);
                       
    config->setGroup("EXIF Settings");

    config->writeEntry("Save EXIF Comments",
                       d->saveExifComments);
                       
    config->writeEntry("EXIF Rotate",
                       d->exifRotate);
                       
    config->writeEntry("EXIF Set Orientation",
                       d->exifSetOrientation);
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

void AlbumSettings::setImageFileFilter(const QString& filter)
{
    d->imageFilefilter = filter;    
}

QString AlbumSettings::getImageFileFilter() const
{
    return d->imageFilefilter;
}

void AlbumSettings::setMovieFileFilter(const QString& filter)
{
    d->movieFilefilter = filter;    
}

QString AlbumSettings::getMovieFileFilter() const
{
    return d->movieFilefilter;
}

void AlbumSettings::setAudioFileFilter(const QString& filter)
{
    d->audioFilefilter = filter;    
}

QString AlbumSettings::getAudioFileFilter() const
{
    return d->audioFilefilter;
}

void AlbumSettings::setRawFileFilter(const QString& filter)
{
    d->rawFilefilter = filter;    
}

QString AlbumSettings::getRawFileFilter() const
{
    return d->rawFilefilter;
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

void AlbumSettings::setIconShowFileComments(bool val)
{
    d->iconShowFileComments = val;
}

bool AlbumSettings::getIconShowFileComments() const
{
    return d->iconShowFileComments;
}

void AlbumSettings::setIconShowResolution(bool val)
{
    d->iconShowResolution = val;
}

bool AlbumSettings::getIconShowResolution() const
{
    return d->iconShowResolution;
}

void AlbumSettings::setSaveExifComments(bool val)
{
    d->saveExifComments = val;
}

bool AlbumSettings::getSaveExifComments() const
{
    return d->saveExifComments;
}

void AlbumSettings::setExifRotate(bool val)
{
    d->exifRotate = val;
}

bool AlbumSettings::getExifRotate() const
{
    return d->exifRotate;
}

void AlbumSettings::setExifSetOrientation(bool val)
{
    d->exifSetOrientation = val;
}

bool AlbumSettings::getExifSetOrientation() const
{
    return d->exifSetOrientation;
}

void AlbumSettings::setIconShowDate(bool val)
{
    d->iconShowDate = val;
}

bool AlbumSettings::getIconShowDate() const
{
    return d->iconShowDate;
}
