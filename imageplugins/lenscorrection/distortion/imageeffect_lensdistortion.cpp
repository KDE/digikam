/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-27
 * Description : a plugin to reduce lens distorsions to an image.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <cmath>

// Qt includes.

#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QGridLayout>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kglobal.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "lensdistortion.h"
#include "imageeffect_lensdistortion.h"
#include "imageeffect_lensdistortion.moc"

using namespace KDcrawIface;

namespace DigikamLensDistortionImagesPlugin
{

ImageEffect_LensDistortion::ImageEffect_LensDistortion(QWidget* parent)
                          : Digikam::ImageGuideDlg(parent, i18n("Lens Distortion Correction"),
                                                   "lensdistortion", false, true, true,
                                                   Digikam::ImageGuideWidget::HVGuideMode)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Lens Distortion Correction"),
                                       digiKamVersion().toAscii(),
                                       ki18n("A tool to reduce camera lens spherical aberrations."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2004-2006, Gilles Caulier\n"
                                       "(c) 2006-2008, Gilles Caulier and Marcel Wiesweg"),
                                       KLocalizedString(),
                                       Digikam::webProjectUrl().url().toUtf8());

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    about->addAuthor(ki18n("David Hodson"), ki18n("Lens distortion correction algorithm."),
                     "hodsond at acm dot org");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(mainWidget());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings );

    m_maskPreviewLabel = new QLabel( gboxSettings );
    m_maskPreviewLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    m_maskPreviewLabel->setWhatsThis( i18n("<p>You can see here a thumbnail preview of the "
                                           "distortion correction applied to a cross pattern.") );

    // -------------------------------------------------------------

    QLabel *label1 = new QLabel(i18n("Main:"), gboxSettings);

    m_mainInput = new KDoubleNumInput(gboxSettings);
    m_mainInput->setDecimals(1);
    m_mainInput->setRange(-100.0, 100.0, 0.1, true);
    m_mainInput->setWhatsThis( i18n("<p>This value controls the amount of distortion. Negative values "
                                    "correct lens barrel distortion, while positive values correct lens "
                                    "pincushion distortion."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Edge:"), gboxSettings);

    m_edgeInput = new KDoubleNumInput(gboxSettings);
    m_edgeInput->setDecimals(1);
    m_edgeInput->setRange(-100.0, 100.0, 0.1, true);
    m_edgeInput->setWhatsThis( i18n("<p>This value controls in the same manner as the Main control, "
                                    "but has more effect at the edges of the image than at the center."));

    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Zoom:"), gboxSettings);

    m_rescaleInput = new KDoubleNumInput(gboxSettings);
    m_rescaleInput->setDecimals(1);
    m_rescaleInput->setRange(-100.0, 100.0, 0.1, true);
    m_rescaleInput->setWhatsThis( i18n("<p>This value rescales the overall image size."));

    // -------------------------------------------------------------

    QLabel *label4 = new QLabel(i18n("Brighten:"), gboxSettings);

    m_brightenInput = new KDoubleNumInput(gboxSettings);
    m_brightenInput->setDecimals(1);
    m_brightenInput->setRange(-100.0, 100.0, 0.1, true);
    m_brightenInput->setWhatsThis( i18n("<p>This value adjusts the brightness in image corners."));

    // -------------------------------------------------------------

    gridSettings->addWidget(m_maskPreviewLabel, 0, 0, 1, 2);
    gridSettings->addWidget(label1,             1, 0, 1, 2);
    gridSettings->addWidget(m_mainInput,        2, 0, 1, 2);
    gridSettings->addWidget(label2,             3, 0, 1, 2);
    gridSettings->addWidget(m_edgeInput,        4, 0, 1, 2);
    gridSettings->addWidget(label3,             5, 0, 1, 2);
    gridSettings->addWidget(m_rescaleInput,     6, 0, 1, 2);
    gridSettings->addWidget(label4,             7, 0, 1, 2);
    gridSettings->addWidget(m_brightenInput,    8, 0, 1, 2);
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_mainInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_edgeInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_rescaleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_brightenInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    // -------------------------------------------------------------

    /* Calc transform preview.
       We would like a checkered area to demonstrate the effect.
       We do not have any drawing support in DImg, so we let Qt draw.
       First we create a white QImage. We convert this to a QPixmap,
       on which we can draw. Then we convert back to QImage,
       convert the QImage to a DImg which we only need to create once, here.
       Later, we apply the effect on a copy and convert the DImg to QPixmap.
       Longing for Qt4 where we can paint directly on the QImage...
    */

    QPixmap pix(120, 120);
    pix.fill(Qt::white);
    QPainter pt(&pix);
    pt.setPen(QPen(Qt::black, 1));
    pt.fillRect(0, 0, pix.width(), pix.height(), QBrush(Qt::black, Qt::CrossPattern));
    pt.drawRect(0, 0, pix.width(), pix.height());
    pt.end();
    QImage preview       = pix.toImage();
    m_previewRasterImage = Digikam::DImg(preview.width(), preview.height(), false, false, preview.bits());
}

ImageEffect_LensDistortion::~ImageEffect_LensDistortion()
{
}

void ImageEffect_LensDistortion::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("lensdistortion Tool Dialog");

    m_mainInput->blockSignals(true);
    m_edgeInput->blockSignals(true);
    m_rescaleInput->blockSignals(true);
    m_brightenInput->blockSignals(true);

    m_mainInput->setValue(group.readEntry("2nd Order Distortion", 0.0));
    m_edgeInput->setValue(group.readEntry("4th Order Distortion",0.0));
    m_rescaleInput->setValue(group.readEntry("Zoom Factor", 0.0));
    m_brightenInput->setValue(group.readEntry("Brighten", 0.0));

    m_mainInput->blockSignals(false);
    m_edgeInput->blockSignals(false);
    m_rescaleInput->blockSignals(false);
    m_brightenInput->blockSignals(false);

    slotEffect();
}

void ImageEffect_LensDistortion::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("lensdistortion Tool Dialog");
    group.writeEntry("2nd Order Distortion", m_mainInput->value());
    group.writeEntry("4th Order Distortion", m_edgeInput->value());
    group.writeEntry("Zoom Factor", m_rescaleInput->value());
    group.writeEntry("Brighten", m_brightenInput->value());
    config->sync();
}

void ImageEffect_LensDistortion::resetValues()
{
    m_mainInput->blockSignals(true);
    m_edgeInput->blockSignals(true);
    m_rescaleInput->blockSignals(true);
    m_brightenInput->blockSignals(true);

    m_mainInput->setValue(0.0);
    m_edgeInput->setValue(0.0);
    m_rescaleInput->setValue(0.0);
    m_brightenInput->setValue(0.0);

    m_mainInput->blockSignals(false);
    m_edgeInput->blockSignals(false);
    m_rescaleInput->blockSignals(false);
    m_brightenInput->blockSignals(false);
}

void ImageEffect_LensDistortion::prepareEffect()
{
    m_mainInput->setEnabled(false);
    m_edgeInput->setEnabled(false);
    m_rescaleInput->setEnabled(false);
    m_brightenInput->setEnabled(false);

    double m = m_mainInput->value();
    double e = m_edgeInput->value();
    double r = m_rescaleInput->value();
    double b = m_brightenInput->value();

    LensDistortion transformPreview(&m_previewRasterImage, 0, m, e, r, b, 0, 0);
    transformPreview.startFilterDirectly();       // Run filter without to use multithreading.
    m_maskPreviewLabel->setPixmap(transformPreview.getTargetImage().convertToPixmap());

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new LensDistortion(iface->getOriginalImg(), this, m, e, r, b, 0, 0));
}

void ImageEffect_LensDistortion::prepareFinal()
{
    m_mainInput->setEnabled(false);
    m_edgeInput->setEnabled(false);
    m_rescaleInput->setEnabled(false);
    m_brightenInput->setEnabled(false);

    double m = m_mainInput->value();
    double e = m_edgeInput->value();
    double r = m_rescaleInput->value();
    double b = m_brightenInput->value();

    Digikam::ImageIface iface(0, 0);

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new LensDistortion(iface.getOriginalImg(), this, m, e, r, b, 0, 0));
}

void ImageEffect_LensDistortion::putPreviewData()
{
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();

    Digikam::DImg imDest = m_threadedFilter->getTargetImage()
            .smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreviewImage(imDest.bits());

    m_imagePreviewWidget->updatePreview();
}

void ImageEffect_LensDistortion::putFinalData()
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Lens Distortion"),
                           m_threadedFilter->getTargetImage().bits());
}

void ImageEffect_LensDistortion::renderingFinished()
{
    m_mainInput->setEnabled(true);
    m_edgeInput->setEnabled(true);
    m_rescaleInput->setEnabled(true);
    m_brightenInput->setEnabled(true);
}

}  // NameSpace DigikamLensDistortionImagesPlugin
