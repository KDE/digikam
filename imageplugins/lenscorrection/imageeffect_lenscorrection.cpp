/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix lens errors
 * 
 * Copyright (C) 2008 Adrian Schroeter <adrian@suse.de>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes.

#include <cstring>
#include <cmath>
#include <cstdlib>

// Qt includes. 
 
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QGridLayout>
#include <QTabWidget>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kcombobox.h>
#include <kglobal.h>

// LibKExiv2 includes.

#include <libkexiv2/kexiv2.h>

// Local includes.

#include "klensfun.h"
#include "version.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imageeffect_lenscorrection.h"
#include "imageeffect_lenscorrection.moc"

namespace DigikamLensCorrectionImagesPlugin
{

ImageEffect_LensCorrection::ImageEffect_LensCorrection(QWidget* parent)
                          : Digikam::CtrlPanelDlg(parent, i18n("Lens Error Correction"),
                                                   "lensfx", false, true, true,
                                                   Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Lens Error Correction"), 
                                       digikam_version,
                                       ki18n("A digiKam image plugin to fix errors caused by lens."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2008, Adrian Schroeter\n"
                                       "(c) 2008, Gilles Caulier"), 
                                       KLocalizedString(),
                                       "http://www.digikam.org");

    about->addAuthor(ki18n("Adrian Schroeter"), ki18n("Author and maintainer"),
                     "adrian@suse.de");

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("provided base classes"),
                     "caulier dot gilles at gmail dot com");

    setAboutData(about);

    // -------------------------------------------------------------

    m_mainTab = new QTabWidget( m_imagePreviewWidget );
    m_imagePreviewWidget->setUserAreaWidget(m_mainTab);

    QWidget *firstPage            = new QWidget(m_mainTab);
    QGridLayout *firstPageLayout  = new QGridLayout( firstPage );
    m_mainTab->addTab( firstPage, i18n("Camera and Lens") );
    QWidget *secondPage           = new QWidget(m_mainTab);
    QGridLayout *secondPageLayout = new QGridLayout( secondPage );
    m_mainTab->addTab( secondPage, i18n("Use Filters") );

#if 0
    m_maskPreviewLabel = new QLabel( m_mainTab );
    m_maskPreviewLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    m_maskPreviewLabel->setWhatsThis( i18n("<p>You can see here a thumbnail preview of the "
                                           "correction applied to a cross pattern.") );
#endif

    // -------------------------------------------------------------

    m_cameraSelector = new KLFDeviceSelector(m_mainTab);

    connect(m_cameraSelector, SIGNAL(lensSelected()),
            this, SLOT(slotLensChanged()));            

//    firstPageLayout->addWidget(m_maskPreviewLabel, 0, 0, 1, 2 );
    firstPageLayout->addWidget(m_cameraSelector, 0, 0, 1, 2 );
    firstPageLayout->setMargin(spacingHint());
    firstPageLayout->setSpacing(spacingHint());

    // -------------------------------------------------------------

    m_filterCCA  = new QCheckBox(i18n("Chromatic Aberration"), m_mainTab);
    m_filterVig  = new QCheckBox(i18n("Vignetting"), m_mainTab);
    m_filterCCI  = new QCheckBox(i18n("Color Correction"), m_mainTab);
    m_filterDist = new QCheckBox(i18n("Distortion"), m_mainTab);
    m_filterGeom = new QCheckBox(i18n("Geometry"), m_mainTab);

    secondPageLayout->addWidget(m_filterCCA,  0, 0, 1, 2 );
    secondPageLayout->addWidget(m_filterVig,  1, 0, 1, 2 );
    secondPageLayout->addWidget(m_filterCCI,  2, 0, 1, 2 );
    secondPageLayout->addWidget(m_filterDist, 3, 0, 1, 2 );
    secondPageLayout->addWidget(m_filterGeom, 4, 0, 1, 2 );
    secondPageLayout->setMargin(spacingHint());
    secondPageLayout->setSpacing(spacingHint());

    connect(m_filterCCA,  SIGNAL(stateChanged (int)), 
            this, SLOT(setFilters()));

    connect(m_filterVig,  SIGNAL(stateChanged (int)), 
            this, SLOT(setFilters()));

    connect(m_filterCCI,  SIGNAL(stateChanged (int)), 
            this, SLOT(setFilters()));

    connect(m_filterDist, SIGNAL(stateChanged (int)), 
            this, SLOT(setFilters()));

    connect(m_filterGeom, SIGNAL(stateChanged (int)), 
            this, SLOT(setFilters()));

    // -------------------------------------------------------------

    slotInit();
};

ImageEffect_LensCorrection::~ImageEffect_LensCorrection()
{
}

void ImageEffect_LensCorrection::slotInit()
{
    resetValues(); // for checking Exif data
    readUserSettings(); 
}

void ImageEffect_LensCorrection::slotLensChanged()
{
    m_filterCCA->setEnabled(m_cameraSelector->getKLFObject()->supportsCCA());
    m_filterVig->setEnabled(m_cameraSelector->getKLFObject()->supportsVig());
    m_filterCCI->setEnabled(m_cameraSelector->getKLFObject()->supportsVig());
    m_filterDist->setEnabled(m_cameraSelector->getKLFObject()->supportsDistortion());
    m_filterGeom->setEnabled(m_cameraSelector->getKLFObject()->supportsDistortion());
    setFilters();
};

void ImageEffect_LensCorrection::readUserSettings(void)
{
    m_mainTab->blockSignals(true);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("Lens Correction Tool Dialog");

    m_filterCCA->setCheckState(group.readEntry("CCA", true)         ? Qt::Checked : Qt::Unchecked);
    m_filterVig->setCheckState(group.readEntry("Vignetting", true)  ? Qt::Checked : Qt::Unchecked);
    m_filterCCI->setCheckState(group.readEntry("CCI", true)         ? Qt::Checked : Qt::Unchecked);
    m_filterDist->setCheckState(group.readEntry("Distortion", true) ? Qt::Checked : Qt::Unchecked);
    m_filterGeom->setCheckState(group.readEntry("Geometry", true)   ? Qt::Checked : Qt::Unchecked);

    setFilters();
    slotEffect();
    m_mainTab->blockSignals(false);
}

void ImageEffect_LensCorrection::writeUserSettings(void)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("Lens Correction Tool Dialog");
    if ( m_filterCCA->isEnabled() )
        group.writeEntry("CCA", (m_filterCCA->checkState() == Qt::Checked) ? true : false);
    if ( m_filterVig->isEnabled() )
        group.writeEntry("Vignetting", (m_filterVig->checkState() == Qt::Checked) ? true : false);
    if ( m_filterCCI->isEnabled() )
        group.writeEntry("CCI", (m_filterCCI->checkState() == Qt::Checked) ? true : false);
    if ( m_filterDist->isEnabled() )
        group.writeEntry("Distortion", (m_filterDist->checkState() == Qt::Checked) ? true : false);
    if ( m_filterGeom->isEnabled() )
        group.writeEntry("Geometry", (m_filterGeom->checkState() == Qt::Checked) ? true : false);
    group.sync();
}

void ImageEffect_LensCorrection::setFilters()
{
    m_cameraSelector->getKLFObject()->setCorrection(
        (m_filterCCA->checkState()  == Qt::Checked && m_filterCCA->isEnabled())  ? true : false,
        (m_filterVig->checkState()  == Qt::Checked && m_filterVig->isEnabled())  ? true : false,
        (m_filterCCI->checkState()  == Qt::Checked && m_filterCCI->isEnabled())  ? true : false,
        (m_filterDist->checkState() == Qt::Checked && m_filterDist->isEnabled()) ? true : false,
        (m_filterGeom->checkState() == Qt::Checked && m_filterGeom->isEnabled()) ? true : false
     );
}

void ImageEffect_LensCorrection::resetValues()
{
    m_mainTab->blockSignals(true);

    // read from Exif data ...
    Digikam::ImageIface iface(0, 0);
    Digikam::DImg *i = iface.getOriginalImg();
    if ( i ) 
    {
        KExiv2Iface::KExiv2 meta;
        meta.setExif( i->getExif() );
        m_cameraSelector->findFromExif( meta );
    }

    m_mainTab->blockSignals(false);
} 

void ImageEffect_LensCorrection::prepareEffect()
{
    m_mainTab->setEnabled(false);

    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                       (new KLensFunFilter(&image, this, m_cameraSelector->getKLFObject()));

}

void ImageEffect_LensCorrection::prepareFinal()
{
    m_mainTab->setEnabled(false);

    Digikam::ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    Digikam::DImg originalImage(iface.originalWidth(), iface.originalHeight(),
                                iface.originalSixteenBit(), iface.originalHasAlpha(), data);

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                       (new KLensFunFilter( &originalImage, this, m_cameraSelector->getKLFObject()));

    delete [] data;
}

void ImageEffect_LensCorrection::putPreviewData()
{
    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImage(imDest);
}

void ImageEffect_LensCorrection::putFinalData()
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Lens Correction"), m_threadedFilter->getTargetImage().bits());
}

void ImageEffect_LensCorrection::renderingFinished()
{
    m_mainTab->setEnabled(true);
}

}  // NameSpace DigikamLensCorrectionImagesPlugin
