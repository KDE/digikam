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

    int         thumbnailSize;
    
    AlbumSettings::AlbumSortOrder albumSortOrder;
    AlbumSettings::ImageSortOrder  imageSortOrder;

    bool recurseTags;
    bool showToolTips;
    bool showSplash;
    bool useTrash;
    
    bool iconShowName;
    bool iconShowSize;
    bool iconShowDate;
    bool iconShowComments;
    bool iconShowFileComments;
    bool iconShowResolution;
    bool iconShowTags;
    bool saveExifComments;
    bool exifRotate;
    bool exifSetOrientation;

    QString currentTheme;
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
    d->albumCollectionNames.sort();

    d->albumSortOrder = AlbumSettings::ByFolder;
    d->imageSortOrder  = AlbumSettings::ByIName;
                                          
    d->imageFilefilter = "*.png *.jpg *.jpeg *.tif *.tiff *.gif *.bmp *.xpm *.ppm *.pnm *.xcf *.pcx";
    d->movieFilefilter = "*.mpeg *.mpg *.avi *.mov";
    d->audioFilefilter = "*.ogg *.mp3 *.wma *.wav";
    d->rawFilefilter =   "*.crw *.nef *.raf *.mrw *.orf";
    d->thumbnailSize = ThumbnailSize::Medium;

    d->recurseTags  = false;
    d->showToolTips = true;
    d->showSplash   = true;
    d->useTrash     = true;
    
    d->iconShowName = false;
    d->iconShowSize = false;
    d->iconShowDate = true;
    d->iconShowComments = true;
    d->iconShowFileComments = false;
    d->iconShowResolution = false;
    d->iconShowTags = true;
    d->saveExifComments = false;
    d->exifRotate = false;
    d->exifSetOrientation = false;
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
    {
        collectionList.sort();
        d->albumCollectionNames = collectionList;
    }

    d->albumSortOrder =
        AlbumSettings::AlbumSortOrder(config->readNumEntry("Album Sort Order",
                                                           (int)AlbumSettings::ByFolder));

    d->imageSortOrder =
        AlbumSettings::ImageSortOrder(config->readNumEntry("Image Sort Order",
                                                          (int)AlbumSettings::ByIName));
    
    d->imageFilefilter = config->readEntry("File Filter",
                                           d->imageFilefilter);

    d->movieFilefilter = config->readEntry("Movie File Filter",
                                           d->movieFilefilter);

    d->audioFilefilter = config->readEntry("Audio File Filter",
                                           d->audioFilefilter);
                              
    d->rawFilefilter = config->readEntry("Raw File Filter",
                                         d->rawFilefilter);
                              
    d->thumbnailSize = config->readNumEntry("Default Icon Size",
                             ThumbnailSize::Medium);

    d->recurseTags   = config->readBoolEntry("Recurse Tags", false);

    d->showToolTips   = config->readBoolEntry("Show ToolTips", true);
    
    d->iconShowName = config->readBoolEntry("Icon Show Name", false); 

    d->iconShowResolution = config->readBoolEntry("Icon Show Resolution",
                                                  false);                                 

    d->iconShowSize = config->readBoolEntry("Icon Show Size",
                              false);

    d->iconShowDate = config->readBoolEntry("Icon Show Date",
                                            true);

    d->iconShowComments = config->readBoolEntry("Icon Show Comments",
                                                true);

    d->iconShowFileComments = config->readBoolEntry("Icon Show File Comments",
                                                    false);

    d->iconShowTags = config->readBoolEntry("Icon Show Tags", true);

    d->currentTheme = config->readEntry("Theme", i18n("Default"));
    
    config->setGroup("EXIF Settings");

    d->saveExifComments = config->readBoolEntry("Save EXIF Comments",
                                                false);
    
    d->exifRotate = config->readBoolEntry("EXIF Rotate",
                                          false);
    
    d->exifSetOrientation = config->readBoolEntry("EXIF Set Orientation",
                                                  false);

    config->setGroup("General Settings");
    
    d->showSplash =
        config->readBoolEntry("Show Splash", true);

    d->useTrash =
        config->readBoolEntry("Use Trash", true);
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

    config->writeEntry("Image Sort Order",
                       (int)d->imageSortOrder);
    
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

    config->writeEntry("Recurse Tags", d->recurseTags);

    config->writeEntry("Show ToolTips", d->showToolTips);
    
    config->writeEntry("Icon Show Name",
                       d->iconShowName);

    config->writeEntry("Icon Show Resolution",
                       d->iconShowResolution);
                                                      
    config->writeEntry("Icon Show Size",
                       d->iconShowSize);
                       
    config->writeEntry("Icon Show Date",
                       d->iconShowDate);
                       
    config->writeEntry("Icon Show Comments",
                       d->iconShowComments);
                       
    config->writeEntry("Icon Show File Comments",
                       d->iconShowFileComments);
                       
    config->writeEntry("Icon Show Tags",
                       d->iconShowTags);

    config->writeEntry("Theme", d->currentTheme);
    
    config->setGroup("EXIF Settings");

    config->writeEntry("Save EXIF Comments",
                       d->saveExifComments);
                       
    config->writeEntry("EXIF Rotate",
                       d->exifRotate);
                       
    config->writeEntry("EXIF Set Orientation",
                       d->exifSetOrientation);

    config->setGroup("General Settings");
    
    config->writeEntry("Show Splash", d->showSplash);

    config->writeEntry("Use Trash", d->useTrash);
    
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

void AlbumSettings::setShowSplashScreen(bool val)
{
    d->showSplash = val;    
}

bool AlbumSettings::getShowSplashScreen() const
{
    return d->showSplash;
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

AlbumSettings::AlbumSortOrder AlbumSettings::getAlbumSortOrder() const
{
    return d->albumSortOrder;
}

void AlbumSettings::setImageSortOder(const ImageSortOrder order)
{
    d->imageSortOrder = order;    
}

AlbumSettings::ImageSortOrder AlbumSettings::getImageSortOrder() const
{
    return d->imageSortOrder;
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

void AlbumSettings::setIconShowName(bool val)
{
    d->iconShowName = val;
}

bool AlbumSettings::getIconShowName() const
{
    return d->iconShowName;
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

void AlbumSettings::setIconShowTags(bool val)
{
    d->iconShowTags = val;    
}

bool AlbumSettings::getIconShowTags() const
{
    return d->iconShowTags;
}

void AlbumSettings::setIconShowDate(bool val)
{
    d->iconShowDate = val;
}

bool AlbumSettings::getIconShowDate() const
{
    return d->iconShowDate;
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

void AlbumSettings::setRecurseTags(bool val)
{
    d->recurseTags = val;
}

bool AlbumSettings::getRecurseTags() const
{
    return d->recurseTags;
}

void AlbumSettings::setShowToolTips(bool val)
{
    d->showToolTips = val;
}

bool AlbumSettings::getShowToolTips() const
{
    return d->showToolTips;
}

void AlbumSettings::setCurrentTheme(const QString& theme)
{
    d->currentTheme = theme;    
}

QString AlbumSettings::getCurrentTheme() const
{
    return d->currentTheme;
}

void AlbumSettings::setUseTrash(bool val)
{
    d->useTrash = val;
}

bool AlbumSettings::getUseTrash() const
{
    return d->useTrash;
}
