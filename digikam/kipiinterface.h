/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : digiKam kipi library interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_KIPIINTERFACE_H
#define DIGIKAM_KIPIINTERFACE_H

// Qt includes.

#include <QList>
#include <QString>
#include <QPixmap>
#include <QMap>

// KDE includes.

#include <kurl.h>
#include <kio/job.h>

// libKipi Includes.

#include <libkipi/interface.h>
#include <libkipi/imagecollection.h>
#include <libkipi/imageinfo.h>
#include <libkipi/imageinfoshared.h>
#include <libkipi/imagecollectionshared.h>

// Local includes

#include "loadingdescription.h"
#include "imageinfo.h"

class QDateTime;

namespace KIPI
{
class Interface;
class ImageCollection;
class ImageInfo;
}

namespace Digikam
{

class ThumbnailLoadThread;
class AlbumManager;
class Album;
class PAlbum;
class TAlbum;

/** DigikamImageInfo: class to get/set image information/properties in a digiKam album. */

class DigikamImageInfo : public KIPI::ImageInfoShared
{
public:
    
    DigikamImageInfo( KIPI::Interface* interface, const KUrl& url );
    ~DigikamImageInfo();
    
    virtual QString title();
    virtual void setTitle( const QString& );

    virtual QString description();
    virtual void setDescription( const QString& );

    virtual void cloneData( ImageInfoShared* other );

    virtual QDateTime time( KIPI::TimeSpec spec );
    virtual void setTime( const QDateTime& time, KIPI::TimeSpec spec = KIPI::FromInfo );
    
    virtual QMap<QString, QVariant> attributes();                    
    virtual void addAttributes(const QMap<QString, QVariant>& res);
    virtual void delAttributes(const QMap<QString, QVariant>& res);
    virtual void clearAttributes();
    
    virtual int  angle();
    virtual void setAngle( int angle );
    
private:

    PAlbum* parentAlbum();

private:

    ImageInfo m_info;
};


/** DigikamImageCollection: class to get/set image collection information/properties in a digiKam 
    album database. */

class DigikamImageCollection : public KIPI::ImageCollectionShared
{
    
public:

    enum Type 
    { 
        AllItems, 
        SelectedItems 
    };

public:

    DigikamImageCollection( Type tp, Album *album, const QString& filter );
    ~DigikamImageCollection();
    
    virtual QString name();
    virtual QString comment();
    virtual QString category();
    virtual QDate date();
    virtual KUrl::List images();
    virtual KUrl path();
    virtual KUrl uploadPath();
    virtual KUrl uploadRoot();
    virtual QString uploadRootName();
    virtual bool isDirectory();
    virtual bool operator==(ImageCollectionShared&);
    
private:

    KUrl::List imagesFromPAlbum(PAlbum* album) const;
    KUrl::List imagesFromTAlbum(TAlbum* album) const;
    
private:

    QString  m_imgFilter;

    Type     m_tp;
    Album   *m_album;
};


/** DigikamKipiInterface: class to interface digiKam with kipi library. */

class DigikamKipiInterface : public KIPI::Interface
{
    Q_OBJECT

public:

    DigikamKipiInterface( QObject *parent, const char *name=0);
    ~DigikamKipiInterface();

    KIPI::ImageCollection currentAlbum();
    KIPI::ImageCollection currentSelection();
    QList<KIPI::ImageCollection> allAlbums();
    KIPI::ImageInfo info( const KUrl& );

    bool addImage( const KUrl&, QString& errmsg );
    void delImage( const KUrl& );
    void refreshImages( const KUrl::List& urls );

    int features() const;
    QString fileExtensions();

    void thumbnail( const KUrl& url, int size );
    void thumbnails( const KUrl::List& list, int size );

public slots:

    void slotSelectionChanged( bool b );
    void slotCurrentAlbumChanged( Album *palbum );

private slots:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    AlbumManager        *m_albumManager;
    ThumbnailLoadThread *m_thumbLoadThread;
};

}  // namespace Digikam

#endif  // DIGIKAM_KIPIINTERFACE_H
