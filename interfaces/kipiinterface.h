////////////////////////////////////////////////////////////////////////////////
//
//    KIPIINTERFACE.H
//
//    Copyright (C) 2004 Gilles Caulier <caulier dot gilles at free.fr>
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


#ifndef DIGIKAM_KIPIINTERFACE_H
#define DIGIKAM_KIPIINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_KIPI

// Qt includes.

#include <qvaluelist.h>

// KDE includes.

#include <kurl.h>

// KIPI Includes.

#include <libkipi/interface.h>
#include <libkipi/imagecollection.h>
#include <libkipi/imageinfo.h>
#include <libkipi/imageinfoshared.h>
#include <libkipi/imagecollectionshared.h>

namespace Digikam
{
class AlbumManager;
class AlbumInfo;
}

namespace KIPI
{
class Interface;
class ImageCollection;
class ImageInfo;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

class DigikamImageInfo :public KIPI::ImageInfoShared
{
public:
    DigikamImageInfo( KIPI::Interface* interface, const KURL& url );
    ~DigikamImageInfo();
    
    virtual QString title();
    virtual void setTitle( const QString& );

    virtual QString description();
    virtual void setDescription( const QString& );

    virtual QMap<QString,QVariant> attributes();
    virtual void clearAttributes();
    virtual void addAttributes( const QMap<QString,QVariant>& );

    virtual int angle();
    virtual void setAngle( int );

    virtual QDateTime time( KIPI::TimeSpec what );
    virtual void setTime( const QDateTime& time, KIPI::TimeSpec spec );
    virtual bool isTimeExact();

    virtual void cloneData( ImageInfoShared* other );

private:
    QString imageName_;
    QString albumName_;
    QString imageComments_;
    Digikam::AlbumInfo *album_;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////

class DigikamImageCollection :public KIPI::ImageCollectionShared
{
public:
    enum Type { CurrentAlbum, CurrentSelection, SubClass };

    DigikamImageCollection( Type tp );
    ~DigikamImageCollection();
    
    virtual QString name();
    virtual QString comment();
    virtual KURL::List images();
    virtual KURL path();
    virtual KURL uploadPath();
    virtual KURL uploadRoot();

protected:
    KURL commonRoot();

private:
    Type _tp;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////

class KipiInterface : public KIPI::Interface
{
    Q_OBJECT

public:
    KipiInterface( QObject *parent, const char *name=0);
    ~KipiInterface();
    
    virtual KIPI::ImageCollection currentAlbum();
    virtual KIPI::ImageCollection currentSelection();
    virtual QValueList<KIPI::ImageCollection> allAlbums();
    virtual KIPI::ImageInfo info( const KURL& );
    virtual bool addImage( const KURL&, QString& errmsg );
    virtual void delImage( const KURL& );
    virtual void refreshImages( const KURL::List& urls );
    virtual int features() const;

protected:
    QString askForCategory();
    
    Digikam::AlbumManager *m_albumManager;

signals:
    void imagesChanged( const KURL::List& );
};

#endif  // HAVE_KIPI

#endif  // DIGIKAM_KIPIINTERFACE_H
