/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004-2005 by Gilles Caulier
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

// Qt includes.
 
#include <qlayout.h>
#include <qlabel.h>

// KDE includes.

#include <ksqueezedtextlabel.h>
#include <kseparator.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kfileitem.h>
#include <kio/previewjob.h>

// Local includes.

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

    // -----------------------------------------------------------
    
    vlay->addStretch(1);
}

ImagePropertiesGeneral::~ImagePropertiesGeneral()
{
}

void ImagePropertiesGeneral::setCurrentURL(const KURL& url)
{
    KIO::PreviewJob* job = KIO::filePreview(url, 128);
    
    connect(job, SIGNAL(gotPreview(const KFileItem *, const QPixmap &)),
            this, SLOT(slotGotThumbnail(const KFileItem *, const QPixmap &)));
    connect(job, SIGNAL(failed(const KFileItem *)),
            this, SLOT(slotFailedThumbnail(const KFileItem *)));

    // ------------------------------------------------------------------------------
    
    m_filename->clear();
    m_filetype->clear();
    m_filedim->clear();
    m_filedate->clear();
    m_filesize->clear();
    m_fileowner->clear();
    m_filepermissions->clear();

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
}

void ImagePropertiesGeneral::slotGotThumbnail(const KFileItem *, const QPixmap &pix)
{
    m_thumbLabel->setPixmap(pix);
}

void ImagePropertiesGeneral::slotFailedThumbnail(const KFileItem *)
{
    m_thumbLabel->clear();
    m_thumbLabel->setText(i18n("Thumbnail unavailable"));
}

#include "imagepropertiesgeneral.moc"
