/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at free.fr>
 *          Ralf Holzer <ralf at well.com>
 *          Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date   : 2004-08-02
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes.

#include <qvaluelist.h>
#include <qstring.h>
#include <qmap.h>

// KDE includes.

#include <kurl.h>
#include <kio/job.h>

// KIPI Includes.

#include <libkipi/interface.h>
#include <libkipi/imagecollection.h>
#include <libkipi/imageinfo.h>
#include <libkipi/imageinfoshared.h>
#include <libkipi/imagecollectionshared.h>

class QDateTime;

class AlbumManager;
class Album;
class PAlbum;
class TAlbum;
class AlbumDB;
class AlbumSettings;

namespace KIPI
{
class Interface;
class ImageCollection;
class ImageInfo;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

class DigikamImageInfo : public KIPI::ImageInfoShared
{
public:
    
    DigikamImageInfo( KIPI::Interface* interface, const KURL& url );
    ~DigikamImageInfo();
    
    virtual QString title();
    virtual void setTitle( const QString& );

    virtual QString description();
    virtual void setDescription( const QString& );

    virtual void cloneData( ImageInfoShared* other );

    virtual void setTime( const QDateTime& time, KIPI::TimeSpec spec = KIPI::FromInfo );
    
    virtual QMap<QString,QVariant> attributes();                    
    virtual void clearAttributes();
    virtual void addAttributes( const QMap<QString,QVariant>& );
    
    virtual int  angle();
    virtual void setAngle( int angle );
    
private:

    PAlbum *parentAlbum();

    PAlbum *palbum_;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////

class DigikamImageCollection : public KIPI::ImageCollectionShared
{
    
public:

    enum Type { AllItems, SelectedItems };

    DigikamImageCollection( Type tp, Album *album,
                            const QString& filter );
    ~DigikamImageCollection();
    
    virtual QString name();
    virtual QString comment();
    virtual QString category();
    virtual QDate date();
    virtual KURL::List images();
    virtual KURL path();
    virtual KURL uploadPath();
    virtual KURL uploadRoot();
    virtual QString uploadRootName();
    virtual bool    isDirectory();
    
private:

    KURL::List imagesFromPAlbum(PAlbum* album) const;
    KURL::List imagesFromTAlbum(TAlbum* album) const;
    
    Type    tp_;
    Album  *album_;
    QString imgFilter_;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////

class DigikamKipiInterface : public KIPI::Interface
{
    Q_OBJECT

public:

    DigikamKipiInterface( QObject *parent, const char *name=0);
    ~DigikamKipiInterface();

    virtual KIPI::ImageCollection currentAlbum();
    virtual KIPI::ImageCollection currentSelection();
    virtual QValueList<KIPI::ImageCollection> allAlbums();
    virtual KIPI::ImageInfo info( const KURL& );
    virtual bool addImage( const KURL&, QString& errmsg );
    virtual void delImage( const KURL& );
    virtual void refreshImages( const KURL::List& urls );
    virtual int features() const;
    virtual QString fileExtensions();

public slots:

    void slotSelectionChanged( bool b );
    void slotCurrentAlbumChanged( Album *palbum );
    
private:
    
    AlbumManager *albumManager_;
    AlbumDB      *albumDB_;
};


#endif  // DIGIKAM_KIPIINTERFACE_H

