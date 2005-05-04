/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
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

#include <qlayout.h>
#include <qlabel.h>

#include <ksqueezedtextlabel.h>
#include <kseparator.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kfileitem.h>
#include <kdeversion.h>
#include <kfilemetainfo.h>
#include <kglobal.h>

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "thumbnailjob.h"
#include "imagepropertiesgeneral.h"

ImagePropertiesGeneral::ImagePropertiesGeneral(QWidget* page)
{
    QVBoxLayout *vlay = new QVBoxLayout( page, 0, 5 );

    KSeparator*  sep;
    QGridLayout* gridLay;
    QLabel*      label;

    // -- Setup thumbnail -----------------------------------------------
    
    m_thumbLabel = new QLabel( page );
    m_thumbLabel->setFixedHeight( 128 );
    vlay->addWidget(m_thumbLabel, 0, Qt::AlignHCenter);

    // -- Setup File properties infos -----------------------------------

    sep = new KSeparator(Horizontal, page);
    vlay->addWidget(sep);
    
    gridLay = new QGridLayout(3, 3);
    vlay->addLayout( gridLay );
    
    label      = new QLabel( i18n("Name:"), page );
    m_filename = new KSqueezedTextLabel( page );
    label->setBuddy( m_filename );
    gridLay->addMultiCellWidget( label, 0, 0, 0, 0 );
    gridLay->addMultiCellWidget( m_filename, 0, 0, 1, 2  );
    
    label      = new QLabel( i18n("Type:"), page );
    m_filetype = new KSqueezedTextLabel(page);
    label->setBuddy( m_filetype );
    gridLay->addMultiCellWidget( label, 1, 1, 0, 0 );
    gridLay->addMultiCellWidget( m_filetype, 1, 1, 1, 2  );
    
    label     = new QLabel( i18n("Dimensions:"), page);
    m_filedim = new KSqueezedTextLabel(page);
    label->setBuddy( m_filedim );
    gridLay->addMultiCellWidget( label, 2, 2, 0, 0 );
    gridLay->addMultiCellWidget( m_filedim, 2, 2, 1, 2  );

    sep = new KSeparator( Horizontal, page );
    vlay->addWidget( sep );
    
    // -- Setup FileSystem infos -----------------------------------

    gridLay = new QGridLayout(5, 3);
    vlay->addLayout( gridLay );
    
    label = new QLabel( i18n("Modified:"), page);
    m_filedate = new KSqueezedTextLabel(page);
    label->setBuddy( m_filedate );
    gridLay->addMultiCellWidget( label, 1, 1, 0, 0 );
    gridLay->addMultiCellWidget( m_filedate, 1, 1, 1, 2  );
    
    label = new QLabel( i18n("Size:"), page);
    m_filesize = new KSqueezedTextLabel(page);
    label->setBuddy( m_filesize );
    gridLay->addMultiCellWidget( label, 2, 2, 0, 0 );
    gridLay->addMultiCellWidget( m_filesize, 2, 2, 1, 2  );

    label       = new QLabel( i18n("Owner:"), page);
    m_fileowner = new KSqueezedTextLabel(page);
    label->setBuddy( m_fileowner );
    gridLay->addMultiCellWidget( label, 3, 3, 0, 0 );
    gridLay->addMultiCellWidget( m_fileowner, 3, 3, 1, 2  );

    label             = new QLabel( i18n("Permissions:"), page);
    m_filepermissions = new KSqueezedTextLabel(page);
    label->setBuddy( m_filepermissions );
    gridLay->addMultiCellWidget( label, 4, 4, 0, 0 );
    gridLay->addMultiCellWidget( m_filepermissions, 4, 4, 1, 2  );
        
    sep = new KSeparator (Horizontal, page);
    vlay->addWidget(sep);

    // -- Setup digiKam infos -----------------------------------
    
    gridLay = new QGridLayout(3, 3);
    vlay->addLayout( gridLay );
    
    label       = new QLabel( i18n("Album:"), page);
    m_filealbum = new KSqueezedTextLabel(page);
    label->setBuddy( m_filealbum );
    gridLay->addMultiCellWidget( label, 0, 0, 0, 0 );
    gridLay->addMultiCellWidget( m_filealbum, 0, 0, 1, 2  );

    label          = new QLabel( i18n("Comments:"), page);
    m_filecomments = new KSqueezedTextLabel(page);
    label->setBuddy( m_filecomments );
    gridLay->addMultiCellWidget( label, 1, 1, 0, 0 );
    gridLay->addMultiCellWidget( m_filecomments, 1, 1, 1, 2  );
    
    label      = new QLabel( i18n("Tags:"), page);
    m_filetags = new KSqueezedTextLabel(page);
    label->setBuddy( m_filetags );
    gridLay->addMultiCellWidget( label, 2, 2, 0, 0 );
    gridLay->addMultiCellWidget( m_filetags, 2, 2, 1, 2  );

    // -----------------------------------------------------------
    
    vlay->addStretch(1);
}

ImagePropertiesGeneral::~ImagePropertiesGeneral()
{
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();
}

void ImagePropertiesGeneral::setCurrentURL(const KURL& url)
{
    // ------------------------------------------------------------------------------

    if (!m_thumbJob.isNull())
        m_thumbJob->kill();
    
    m_thumbJob = new ThumbnailJob(url, 128);
    connect(m_thumbJob, SIGNAL(signalThumbnailMetaInfo(const KURL&,
                                                       const QPixmap&,
                                                       const KFileMetaInfo*)),
            SLOT(slotGotThumbnail(const KURL&,
                                         const QPixmap&,
                                         const KFileMetaInfo*)));

    connect(m_thumbJob, SIGNAL(signalFailed(const KURL&)),
            SLOT(slotFailedThumbnail(const KURL&)));       

    // ------------------------------------------------------------------------------
    
    m_filename->clear();
    m_filetype->clear();
    m_filedim->clear();
    m_filedate->clear();
    m_filesize->clear();
    m_fileowner->clear();
    m_filepermissions->clear();
    m_filealbum->clear();
    m_filecomments->clear();
    m_filetags->clear();

    // -- File system informations ---------------------------------------------------
    
    KFileItem* fi = new KFileItem(KFileItem::Unknown,
                                  KFileItem::Unknown,
                                  url);
    m_filename->setText( url.fileName() );
    m_filetype->setText( KMimeType::findByURL(url)->comment() );

#if KDE_IS_VERSION(3,2,0)
    KFileMetaInfo meta(url);
#else
    KFileMetaInfo meta(url.path());
#endif

    if (meta.isValid())
    {
        QSize dims;
        
        if (meta.containsGroup("Jpeg EXIF Data"))
            dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
        else if (meta.containsGroup("General"))
            dims = meta.group("General").item("Dimensions").value().toSize();
        else if (meta.containsGroup("Technical"))
            dims = meta.group("Technical").item("Dimensions").value().toSize();
        
        m_filedim->setText( QString("%1 x %2 %3").arg(dims.width())
                            .arg(dims.height()).arg(i18n("pixels")) );
    }

    QDateTime dateurl;
    dateurl.setTime_t(fi->time(KIO::UDS_MODIFICATION_TIME));
    m_filedate->setText( KGlobal::locale()->formatDateTime(dateurl, true, true) );
    m_filesize->setText( i18n("%1 (%2)").arg(KIO::convertSize(fi->size()))
                                        .arg(KGlobal::locale()->formatNumber(fi->size(), 0)) );
    m_fileowner->setText( i18n("%1 - %2").arg(fi->user()).arg(fi->group()) );
    m_filepermissions->setText( fi->permissionsString() );

    // -- digiKam metadata ---------------------------------------------------

    AlbumManager* man = AlbumManager::instance();    
    KURL u            = url.upURL();
    PAlbum* album     = man->findPAlbum(u);
    
    if (album)
    {
        AlbumDB* db = AlbumManager::instance()->albumDB();

        QStringList tagPaths;

        IntList tagIDs(db->getItemTagIDs(album, url.fileName()));
        
        for (IntList::iterator it = tagIDs.begin(); it != tagIDs.end(); ++it)
        {
            TAlbum* ta = man->findTAlbum(*it);
            if (ta)
            {
                tagPaths.append(ta->getURL().remove(0,1));
            }
        }
        
        m_filealbum->setText( album->getURL().remove(0,1) );
        m_filecomments->setText( db->getItemCaption( album, url.filename() ) );
        m_filetags->setText( tagPaths.join(", "));        
    }
}

void ImagePropertiesGeneral::slotGotThumbnail(const KURL&, const QPixmap& pix,
                                              const KFileMetaInfo*)
{
    m_thumbLabel->setPixmap(pix);
}

void ImagePropertiesGeneral::slotFailedThumbnail(const KURL&)
{
    m_thumbLabel->clear();
    m_thumbLabel->setText(i18n("Thumbnail unavailable"));
}

#include "imagepropertiesgeneral.moc"
