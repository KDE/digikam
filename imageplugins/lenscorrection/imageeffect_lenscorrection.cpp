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
#include <kseparator.h>
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

#include "version.h"
#include "ddebug.h"
#include "imageiface.h"
#include "klensfun.h"
#include "imageeffect_lenscorrection.h"
#include "imageeffect_lenscorrection.moc"

namespace DigikamLensCorrectionImagesPlugin
{

ImageEffect_LensCorrection::ImageEffect_LensCorrection(QWidget* parent)
                          : Digikam::ImageGuideDlg(parent, i18n("Lens Error Correction"),
                                                   "lensfx", false, true, true,
                                                   Digikam::ImageGuideWidget::HVGuideMode,
                                                   true)
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

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Developer"),
                     "caulier dot gilles at gmail dot com");

    setAboutData(about);

    // -------------------------------------------------------------

    m_settingsWidget  = new QWidget(mainWidget());
    QGridLayout *grid = new QGridLayout(m_settingsWidget);
//    m_mainTab->addTab(firstPage, i18n("Camera and Lens"));

#if 0
    m_maskPreviewLabel = new QLabel(firstPage);
    m_maskPreviewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_maskPreviewLabel->setWhatsThis(i18n("<p>You can see here a thumbnail preview of the "
                                          "correction applied to a cross pattern."));
#endif

    m_cameraSelector = new KLFDeviceSelector(m_settingsWidget);
    KSeparator *line = new KSeparator(Qt::Horizontal, m_settingsWidget);

    // -------------------------------------------------------------

//    m_mainTab->addTab(secondPage, i18n("Use Filters"));

    m_filterCCA  = new QCheckBox(i18n("Chromatic Aberration"), m_settingsWidget);
    m_filterVig  = new QCheckBox(i18n("Vignetting"), m_settingsWidget);
    m_filterCCI  = new QCheckBox(i18n("Color Correction"), m_settingsWidget);
    m_filterDist = new QCheckBox(i18n("Distortion"), m_settingsWidget);
    m_filterGeom = new QCheckBox(i18n("Geometry"), m_settingsWidget);

//    grid->addWidget(m_maskPreviewLabel, 0, 0, 1, 2);
    grid->addWidget(m_cameraSelector, 0, 0, 1, 2);
    grid->addWidget(line       ,      1, 0, 1, 2);
    grid->addWidget(m_filterCCA,      2, 0, 1, 2);
    grid->addWidget(m_filterVig,      3, 0, 1, 2);
    grid->addWidget(m_filterCCI,      4, 0, 1, 2);
    grid->addWidget(m_filterDist,     5, 0, 1, 2);
    grid->addWidget(m_filterGeom,     6, 0, 1, 2);
    grid->setMargin(spacingHint());
    grid->setSpacing(spacingHint());

    setUserAreaWidget(m_settingsWidget);

    // -------------------------------------------------------------

    connect(m_cameraSelector, SIGNAL(signalLensSelected()),
            this, SLOT(slotLensChanged()));

    connect(m_filterCCA,  SIGNAL(stateChanged(int)), 
            this, SLOT(setFilters()));

    connect(m_filterVig,  SIGNAL(stateChanged(int)), 
            this, SLOT(setFilters()));

    connect(m_filterCCI,  SIGNAL(stateChanged(int)), 
            this, SLOT(setFilters()));

    connect(m_filterDist, SIGNAL(stateChanged(int)), 
            this, SLOT(setFilters()));

    connect(m_filterGeom, SIGNAL(stateChanged(int)), 
            this, SLOT(setFilters()));
}

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
}

void ImageEffect_LensCorrection::readUserSettings()
{
    m_settingsWidget->blockSignals(true);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Lens Correction Tool Dialog");

    m_filterCCA->setCheckState(group.readEntry("CCA", true)         ? Qt::Checked : Qt::Unchecked);
    m_filterVig->setCheckState(group.readEntry("Vignetting", true)  ? Qt::Checked : Qt::Unchecked);
    m_filterCCI->setCheckState(group.readEntry("CCI", true)         ? Qt::Checked : Qt::Unchecked);
    m_filterDist->setCheckState(group.readEntry("Distortion", true) ? Qt::Checked : Qt::Unchecked);
    m_filterGeom->setCheckState(group.readEntry("Geometry", true)   ? Qt::Checked : Qt::Unchecked);

    m_settingsWidget->blockSignals(false);
    setFilters();
}

void ImageEffect_LensCorrection::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Lens Correction Tool Dialog");
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

    slotEffect();
}

void ImageEffect_LensCorrection::resetValues()
{
    m_settingsWidget->blockSignals(true);

    // Read Exif informations ...
    Digikam::DImg *i = m_imagePreviewWidget->imageIface()->getOriginalImg();
    KExiv2Iface::KExiv2 meta;
    meta.setExif(i->getExif());
    m_cameraSelector->findFromExif(meta);

    m_settingsWidget->blockSignals(false);
} 

void ImageEffect_LensCorrection::prepareEffect()
{
    m_settingsWidget->setEnabled(false);

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    uchar *data                = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg image(w, h, sb, false, data);

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                       (new KLensFunFilter(&image, this, m_cameraSelector->getKLFObject()));

}

void ImageEffect_LensCorrection::prepareFinal()
{
    m_settingsWidget->setEnabled(false);

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
    m_imagePreviewWidget->imageIface()->putPreviewImage(imDest.bits());
    m_imagePreviewWidget->updatePreview();
}

void ImageEffect_LensCorrection::putFinalData()
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Lens Correction"), m_threadedFilter->getTargetImage().bits());
}

void ImageEffect_LensCorrection::renderingFinished()
{
    m_settingsWidget->setEnabled(true);
}

}  // NameSpace DigikamLensCorrectionImagesPlugin
