/* ============================================================
 * File  : albumsettings.h
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

#ifndef ALBUMSETTINGS_H
#define ALBUMSETTINGS_H

#include <qstringlist.h>
#include <qstring.h>

class AlbumSettingsPrivate;

class AlbumSettings {

public:

    enum AlbumSortOrder
    {
        ByCollection = 0,
        ByDate,
        Flat
    };

    AlbumSettings();
    ~AlbumSettings();

    void readSettings();
    void saveSettings();

    void setAlbumLibraryPath(const QString& path);
    QString getAlbumLibraryPath() const;

    void setAlbumCollectionNames(const QStringList& list);
    QStringList getAlbumCollectionNames();

    bool addAlbumCollectionName(const QString& name);
    bool delAlbumCollectionName(const QString& name);
    
    void setAlbumSortOrder(const AlbumSortOrder order);
    AlbumSortOrder getAlbumSortOrder();

    void setFileFilter(const QString& filter);
    QString getFileFilter() const;
    
    void setDefaultIconSize(int val);
    int  getDefaultIconSize() const;

    void setIconShowMime(bool val);
    bool getIconShowMime() const;

    void setIconShowSize(bool val);
    bool getIconShowSize() const;

    void setIconShowComments(bool val);
    bool getIconShowComments() const;

    void setIconShowDate(bool val);
    bool getIconShowDate() const;

    static AlbumSettings *instance();

private:

    static AlbumSettings* instance_;
    
    void init();

    AlbumSettingsPrivate* d;

};

#endif
