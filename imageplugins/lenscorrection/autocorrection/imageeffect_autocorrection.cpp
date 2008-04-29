/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automaticaly camera lens aberrations
 * 
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
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

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dmetadata.h"
#include "imageiface.h"
#include "klensfun.h"
#include "imageeffect_autocorrection.h"
#include "imageeffect_autocorrection.moc"

namespace DigikamAutoCorrectionImagesPlugin
{

ImageEffect_AutoCorrection::ImageEffect_AutoCorrection(QWidget* parent)
                          : Digikam::ImageGuideDlg(parent, i18n("Lens Auto-Correction"),
                                                   "lensautocorrection", false, true, true,
                                                   Digikam::ImageGuideWidget::HVGuideMode,
                                                   true)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Lens Auto-Correction"), 
                                       digikam_version,
                                       ki18n("A digiKam image plugin to fix automaticaly camera "
                                             "lens aberrations using LensFun library."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2008, Adrian Schroeter\n"
                                       "(c) 2008, Gilles Caulier"), 
                                       KLocalizedString(),
                                       "http://www.digikam.org");

    about->addAuthor(ki18n("Adrian Schroeter"), ki18n("Author and maintainer"),
                     "adrian at suse dot de");

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Developer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Andrew Zabolotny"), ki18n("LensFun library author."),
                     "zap at homelink dot ru");

    setAboutData(about);

    // -------------------------------------------------------------

    m_settingsWidget  = new QWidget(mainWidget());
    QGridLayout *grid = new QGridLayout(m_settingsWidget);

#if 0
    m_maskPreviewLabel = new QLabel(firstPage);
    m_maskPreviewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_maskPreviewLabel->setWhatsThis(i18n("<p>You can see here a thumbnail preview of the "
                                          "correction applied to a cross pattern."));
#endif

    m_cameraSelector = new KLFDeviceSelector(m_settingsWidget);
    KSeparator *line = new KSeparator(Qt::Horizontal, m_settingsWidget);

    // -------------------------------------------------------------

    m_filterCCA  = new QCheckBox(i18n("Chromatic Aberration"), m_settingsWidget);
    m_filterCCA->setWhatsThis(i18n("Chromatic aberration is easily recognised as color fringes "
                                   "towards the image corners. CA is due to a variing lens focus "
                                   "for different colors."));
    m_filterVig  = new QCheckBox(i18n("Vignetting"), m_settingsWidget);
    m_filterVig->setWhatsThis(i18n("Vignetting refers to an image darkening in the corners. "
                                   "Optical and natural vignetting can be cancelled out with this option, "
                                   "whereas mechanical vignetting will not be cured."));
    m_filterCCI  = new QCheckBox(i18n("Color Correction"), m_settingsWidget);
    m_filterCCI->setWhatsThis(i18n("Lenses all have a slight color tinge to them, "
                                   "mostly due to the anti-reflective coating. "
                                   "The tinge can be taken away when the respective data is know for the lens."));
    m_filterDist = new QCheckBox(i18n("Distortion"), m_settingsWidget);
    m_filterDist->setWhatsThis(i18n("Distortion refers to an image deformation, which is most pronounced "
                                    "towards the corners. These Seidel aberrations are known as pincushion "
                                    "and barrel distorsions."));
    m_filterGeom = new QCheckBox(i18n("Geometry"), m_settingsWidget);
    m_filterGeom->setWhatsThis(i18n("Four geometries are handeled here: Rectilinear (99 percent of all lenses), "
                                    "Fisheye, Cylindrical, Equirectangular."));

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

    connect(m_cameraSelector, SIGNAL(signalLensSettingsChanged()),
            this, SLOT(slotLensChanged()));

    connect(m_filterCCA,  SIGNAL(stateChanged(int)), 
            this, SLOT(slotSetFilters()));

    connect(m_filterVig,  SIGNAL(stateChanged(int)), 
            this, SLOT(slotSetFilters()));

    connect(m_filterCCI,  SIGNAL(stateChanged(int)), 
            this, SLOT(slotSetFilters()));

    connect(m_filterDist, SIGNAL(stateChanged(int)), 
            this, SLOT(slotSetFilters()));

    connect(m_filterGeom, SIGNAL(stateChanged(int)), 
            this, SLOT(slotSetFilters()));
}

ImageEffect_AutoCorrection::~ImageEffect_AutoCorrection()
{
}

void ImageEffect_AutoCorrection::slotInit()
{
    resetValues(); // for checking Exif data
    readUserSettings(); 
}

void ImageEffect_AutoCorrection::slotLensChanged()
{
    m_filterCCA->setEnabled(m_cameraSelector->getKLFObject()->supportsCCA());
    m_filterVig->setEnabled(m_cameraSelector->getKLFObject()->supportsVig());
    m_filterCCI->setEnabled(m_cameraSelector->getKLFObject()->supportsVig());
    m_filterDist->setEnabled(m_cameraSelector->getKLFObject()->supportsDistortion());
    m_filterGeom->setEnabled(m_cameraSelector->getKLFObject()->supportsDistortion());
    slotSetFilters();
}

void ImageEffect_AutoCorrection::slotSetFilters()
{
    m_cameraSelector->getKLFObject()->setCorrection(
        (m_filterCCA->checkState()  == Qt::Checked && m_filterCCA->isEnabled())  ? true : false,
        (m_filterVig->checkState()  == Qt::Checked && m_filterVig->isEnabled())  ? true : false,
        (m_filterCCI->checkState()  == Qt::Checked && m_filterCCI->isEnabled())  ? true : false,
        (m_filterDist->checkState() == Qt::Checked && m_filterDist->isEnabled()) ? true : false,
        (m_filterGeom->checkState() == Qt::Checked && m_filterGeom->isEnabled()) ? true : false
     );

    slotTimer();
}

void ImageEffect_AutoCorrection::readUserSettings()
{
    m_settingsWidget->blockSignals(true);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Lens Auto-Correction Tool Dialog");

    m_filterCCA->setCheckState(group.readEntry("CCA", true)         ? Qt::Checked : Qt::Unchecked);
    m_filterVig->setCheckState(group.readEntry("Vignetting", true)  ? Qt::Checked : Qt::Unchecked);
    m_filterCCI->setCheckState(group.readEntry("CCI", true)         ? Qt::Checked : Qt::Unchecked);
    m_filterDist->setCheckState(group.readEntry("Distortion", true) ? Qt::Checked : Qt::Unchecked);
    m_filterGeom->setCheckState(group.readEntry("Geometry", true)   ? Qt::Checked : Qt::Unchecked);

    m_settingsWidget->blockSignals(false);
    slotSetFilters();
}

void ImageEffect_AutoCorrection::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Lens Auto-Correction Tool Dialog");
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

void ImageEffect_AutoCorrection::resetValues()
{
    m_settingsWidget->blockSignals(true);

    // Read Exif informations ...
    Digikam::DImg      *img = m_imagePreviewWidget->imageIface()->getOriginalImg();
    Digikam::DMetadata  meta;
    meta.setExif(img->getExif());
    meta.setIptc(img->getIptc());
    meta.setXmp(img->getXmp());
    m_cameraSelector->findFromMetadata(meta);

    m_settingsWidget->blockSignals(false);
} 

void ImageEffect_AutoCorrection::prepareEffect()
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

void ImageEffect_AutoCorrection::prepareFinal()
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

void ImageEffect_AutoCorrection::putPreviewData()
{
    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->imageIface()->putPreviewImage(imDest.bits());
    m_imagePreviewWidget->updatePreview();
}

void ImageEffect_AutoCorrection::putFinalData()
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Lens Auto-Correction"), m_threadedFilter->getTargetImage().bits());
}

void ImageEffect_AutoCorrection::renderingFinished()
{
    m_settingsWidget->setEnabled(true);
}

}  // NameSpace DigikamAutoCorrectionImagesPlugin
