/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2003-16-10
 * Description : 
 * 
 * Copyright 2003-2004 by Renchi Raju and Gilles Caulier
 * Copyright 2005-2006 by Gilles Caulier
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

#ifndef ALBUMSETTINGS_H
#define ALBUMSETTINGS_H

// Qt includes.

#include <qstringlist.h>
#include <qstring.h>

namespace Digikam
{

class AlbumSettingsPrivate;

class AlbumSettings 
{
public:

    enum AlbumSortOrder
    {
        ByFolder = 0,
        ByCollection,
        ByDate
    };

    enum ImageSortOrder
    {
        ByIName = 0,
        ByIPath,
        ByIDate,
        ByISize,
        ByIRating
    };
    
    AlbumSettings();
    ~AlbumSettings();

    void readSettings();
    void saveSettings();

    void setAlbumLibraryPath(const QString& path);
    QString getAlbumLibraryPath() const;

    void setShowSplashScreen(bool val);
    bool getShowSplashScreen() const;

    void setScanAtStart(bool val);
    bool getScanAtStart() const;

    void setAlbumCollectionNames(const QStringList& list);
    QStringList getAlbumCollectionNames();

    bool addAlbumCollectionName(const QString& name);
    bool delAlbumCollectionName(const QString& name);
    
    void setAlbumSortOrder(const AlbumSortOrder order);
    AlbumSortOrder getAlbumSortOrder() const;

    void setImageSortOrder(const ImageSortOrder order);
    ImageSortOrder getImageSortOrder() const;

    void setImageFileFilter(const QString& filter);
    QString getImageFileFilter() const;
    
    void setMovieFileFilter(const QString& filter);
    QString getMovieFileFilter() const;

    void setAudioFileFilter(const QString& filter);
    QString getAudioFileFilter() const;
            
    void setRawFileFilter(const QString& filter);
    QString getRawFileFilter() const;

    bool    addImageFileExtension(const QString& ext);
    QString getAllFileFilter() const;
    
    void setDefaultIconSize(int val);
    int  getDefaultIconSize() const;

    void setIconShowName(bool val);
    bool getIconShowName() const;
    
    void setIconShowSize(bool val);
    bool getIconShowSize() const;

    void setIconShowComments(bool val);
    bool getIconShowComments() const;

    void setIconShowResolution(bool val);
    bool getIconShowResolution() const;

    void setIconShowTags(bool val);
    bool getIconShowTags() const;
    
    void setIconShowDate(bool val);
    bool getIconShowDate() const;

    void setIconShowModDate(bool val);
    bool getIconShowModDate() const;

    void setIconShowRating(bool val);
    bool getIconShowRating() const;
    
    void setExifRotate(bool val);
    bool getExifRotate() const;

    void setExifSetOrientation(bool val);
    bool getExifSetOrientation() const;

    void setSaveIptcTags(bool val);
    bool getSaveIptcTags() const;

    void setSaveIptcRating(bool val);
    bool getSaveIptcRating() const;

    void setSaveIptcPhotographerId(bool val);
    bool getSaveIptcPhotographerId() const;

    void setSaveIptcCredits(bool val);
    bool getSaveIptcCredits() const;

    void setIptcAuthor(const QString& author);
    QString getIptcAuthor() const;

    void setIptcAuthorTitle(const QString& authorTitle);
    QString getIptcAuthorTitle() const;

    void setIptcCredit(const QString& credit);
    QString getIptcCredit() const;

    void setIptcSource(const QString& source);
    QString getIptcSource() const;

    void setIptcCopyright(const QString& copyright);
    QString getIptcCopyright() const;

    void setSaveComments(bool val);
    bool getSaveComments() const;

    void setSaveDateTime(bool val);
    bool getSaveDateTime() const;
    
    void setShowToolTips(bool val);
    bool getShowToolTips() const;
    
    void    setCurrentTheme(const QString& theme);
    QString getCurrentTheme() const;

    void    setUseTrash(bool val);
    bool    getUseTrash() const;

    static AlbumSettings *instance();

private:

    static AlbumSettings* instance_;
    
    void init();

    AlbumSettingsPrivate* d;
};

}  // namespace Digikam

#endif  // ALBUMSETTINGS_H
