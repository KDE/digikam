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
#include <qpixmap.h>
#include <qcombobox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kurl.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kfileitem.h>
#include <kio/previewjob.h>

// LibKExif includes.

#include <libkexif/kexifwidget.h>

// Local includes.

#include "imagepropertiesexif.h"

ImagePropertiesEXIF::ImagePropertiesEXIF(QWidget* page)
{
    QVBoxLayout* vlay = new QVBoxLayout(page, 5, 5);
    QHBoxLayout* hlay = new QHBoxLayout(vlay);

    m_labelThumb = new QLabel( page );
    m_labelThumb->setFixedHeight( 48 );
    hlay->addWidget(m_labelThumb);
    hlay->addStretch();

    QLabel* levelLabel  = new QLabel(i18n("Select level of detail:"), page);
    m_levelCombo        = new QComboBox(page);
    hlay->addWidget(levelLabel);
    hlay->addWidget(m_levelCombo);

    QWhatsThis::add( m_levelCombo, i18n("<p>Select here the Exif information level to display<p>"
                                        "<b>Simple</b>: display general information about the photograph "
                                        " (default).<p>"
                                        "<b>Full</b>: display all EXIF sections.") );  
    
    m_exifWidget = new KExifWidget(page);
    vlay->addWidget(m_exifWidget);

    m_levelCombo->insertItem(i18n("Simple"));
    m_levelCombo->insertItem(i18n("Full"));
    connect(m_levelCombo, SIGNAL(activated(int)),
            SLOT(slotLevelChanged(int)));

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Image Properties Dialog");
    m_levelCombo->setCurrentItem(config->readNumEntry("EXIF Level", 0));
    m_currItem = config->readEntry("Current EXIF Item", QString());
    slotLevelChanged(0);
}

ImagePropertiesEXIF::~ImagePropertiesEXIF()
{
    KConfig* config = kapp->config();
    config->setGroup("Image Properties Dialog");
    config->writeEntry("EXIF Level", m_levelCombo->currentItem());
    config->writeEntry("Current EXIF Item", m_currItem);
}

void ImagePropertiesEXIF::setCurrentURL(const KURL& url)
{
    KIO::PreviewJob* job = KIO::filePreview(url, 48);
    
    connect(job, SIGNAL(gotPreview(const KFileItem *, const QPixmap &)),
            this, SLOT(slotGotThumbnail(const KFileItem *, const QPixmap &)));
    connect(job, SIGNAL(failed(const KFileItem *)),
            this, SLOT(slotFailedThumbnail(const KFileItem *)));
    
    // ----------------------------------------------------------------

    if (!m_exifWidget->getCurrentItemName().isNull())
        m_currItem = m_exifWidget->getCurrentItemName();
    
    m_exifWidget->loadFile(url.path());

    m_exifWidget->setCurrentItem(m_currItem);
}

void ImagePropertiesEXIF::slotLevelChanged(int)
{
    if (m_levelCombo->currentText() == i18n("Simple"))
        m_exifWidget->setMode(KExifWidget::SIMPLE);
    else
        m_exifWidget->setMode(KExifWidget::FULL);
}

void ImagePropertiesEXIF::slotGotThumbnail(const KFileItem *, const QPixmap &pix)
{
    if (!pix.isNull())
        m_labelThumb->setPixmap(pix);
    else
        m_labelThumb->clear();
}

void ImagePropertiesEXIF::slotFailedThumbnail(const KFileItem *)
{
    m_labelThumb->clear();
}

#include "imagepropertiesexif.moc"
