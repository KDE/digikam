////////////////////////////////////////////////////////////////////////////////
//
//    KIPIINTERFACE.H
//
//    Copyright (C) 2004 Gilles Caulier <caulier dot gilles at free.fr>
//                       Ralf Holzer <ralf at well.com>
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

// Qt includes.

#include <qvaluelist.h>

// KDE includes.

#include <kurl.h>
#include <kio/job.h>

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

class DigikamImageInfo : public KIPI::ImageInfoShared
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
    
    virtual void setTime( const QDateTime& time, KIPI::TimeSpec spec = KIPI::FromInfo );
    
    virtual int DigikamImageInfo::angle();
    virtual void setAngle( int angle );
       
private:
    QString             imageName_;
    QString             imageUrl_;
    QString             albumName_;
    QString             imageComments_;
    Digikam::AlbumInfo *album_;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////

class DigikamImageCollection : public KIPI::ImageCollectionShared
{
public:
    enum Type { AllAlbumItems, AlbumItemsSelection };

    DigikamImageCollection( Type tp, Digikam::AlbumInfo *album=0 );
    ~DigikamImageCollection();
    
    virtual QString name();
    virtual QString comment();
    virtual KURL::List images();
    virtual KURL path();
    virtual KURL uploadPath();
    virtual KURL uploadRoot();
    virtual QString uploadRootName();
    
protected:
    Digikam::AlbumInfo *album_;
    
    KURL commonRoot();
    
private:
    Type _tp;
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
    virtual KIPI::ImageCollection currentScope();
    virtual QValueList<KIPI::ImageCollection> allAlbums();
    virtual KIPI::ImageInfo info( const KURL& );
    virtual bool addImage( const KURL&, QString& errmsg );
    virtual void delImage( const KURL& );
    virtual void refreshImages( const KURL::List& urls );
    virtual int features() const;

protected slots:
    void slot_onAddImageFinished(KIO::Job* job);

public slots:
    void slotSelectionChanged( bool );    

protected:
    QString askForCategory();
    
    // For Add images operations.
    Digikam::AlbumInfo *m_sourceAlbum;
    Digikam::AlbumInfo *m_targetAlbum;
    QString             m_imageFileName;
    
    Digikam::AlbumManager *albumManager_;
};

#endif  // DIGIKAM_KIPIINTERFACE_H
