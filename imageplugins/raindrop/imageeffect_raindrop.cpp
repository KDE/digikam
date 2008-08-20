/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-30
 * Description : a plugin to add rain drop over an image
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

// Qt includes.

#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qimage.h>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "raindrop.h"
#include "imageeffect_raindrop.h"
#include "imageeffect_raindrop.moc"

using namespace KDcrawIface;

namespace DigikamRainDropImagesPlugin
{

ImageEffect_RainDrop::ImageEffect_RainDrop(QWidget* parent)
                    : Digikam::ImageGuideDlg(parent, i18n("Add Raindrops to Photograph"),
                                             "raindrops", false, true, false,
                                             Digikam::ImageGuideWidget::HVGuideMode)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("Raindrops"),
                                       digikam_version,
                                       I18N_NOOP("A digiKam image plugin to add raindrops to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier\n"
                                       "(c) 2006-2008, Gilles Caulier and Marcel Wiesweg",
                                       0,
                                       Digikam::webProjectUrl());

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Raindrops algorithm"),
                     "pieter dot voloshyn at gmail dot com");

    about->addAuthor("Marcel Wiesweg", I18N_NOOP("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    QWhatsThis::add( m_imagePreviewWidget, i18n("<p>This is the preview of the Raindrop effect."
                                           "<p>Note: if you have previously selected an area in the editor, "
                                           "this will be unaffected by the filter. You can use this method to "
                                           "disable the Raindrops effect on a human face, for example.") );

    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 5, 2, spacingHint());

    QLabel *label1 = new QLabel(i18n("Drop size:"), gboxSettings);

    m_dropInput = new RIntNumInput(gboxSettings);
    m_dropInput->setRange(0, 200, 1);
    m_dropInput->setDefaultValue(80);
    QWhatsThis::add( m_dropInput, i18n("<p>Set here the raindrops' size."));

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 2);
    gridSettings->addMultiCellWidget(m_dropInput, 1, 1, 0, 2);

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Number:"), gboxSettings);

    m_amountInput = new RIntNumInput(gboxSettings);
    m_amountInput->setRange(1, 500, 1);
    m_amountInput->setDefaultValue(150);
    QWhatsThis::add( m_amountInput, i18n("<p>This value controls the maximum number of raindrops."));

    gridSettings->addMultiCellWidget(label2, 2, 2, 0, 2);
    gridSettings->addMultiCellWidget(m_amountInput, 3, 3, 0, 2);

    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Fish eyes:"), gboxSettings);

    m_coeffInput = new RIntNumInput(gboxSettings);
    m_coeffInput->setRange(1, 100, 1);
    m_coeffInput->setDefaultValue(30);
    QWhatsThis::add( m_coeffInput, i18n("<p>This value is the fish-eye-effect optical "
                                        "distortion coefficient."));

    gridSettings->addMultiCellWidget(label3, 4, 4, 0, 2);
    gridSettings->addMultiCellWidget(m_coeffInput, 5, 5, 0, 2);

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_dropInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_amountInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_coeffInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

ImageEffect_RainDrop::~ImageEffect_RainDrop()
{
}

void ImageEffect_RainDrop::renderingFinished()
{
    m_dropInput->setEnabled(true);
    m_amountInput->setEnabled(true);
    m_coeffInput->setEnabled(true);
}

void ImageEffect_RainDrop::readUserSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("raindrops Tool Dialog");

    m_dropInput->blockSignals(true);
    m_amountInput->blockSignals(true);
    m_coeffInput->blockSignals(true);

    m_dropInput->setValue(config->readNumEntry("DropAdjustment",
                          m_dropInput->defaultValue()));
    m_amountInput->setValue(config->readNumEntry("AmountAdjustment",
                            m_amountInput->defaultValue()));
    m_coeffInput->setValue(config->readNumEntry("CoeffAdjustment",
                           m_coeffInput->defaultValue()));

    m_dropInput->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_coeffInput->blockSignals(false);

    slotEffect();
}

void ImageEffect_RainDrop::writeUserSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("raindrops Tool Dialog");
    config->writeEntry("DropAdjustment", m_dropInput->value());
    config->writeEntry("AmountAdjustment", m_amountInput->value());
    config->writeEntry("CoeffAdjustment", m_coeffInput->value());
    config->sync();
}

void ImageEffect_RainDrop::resetValues()
{
    m_dropInput->blockSignals(true);
    m_amountInput->blockSignals(true);
    m_coeffInput->blockSignals(true);

    m_dropInput->slotReset();
    m_amountInput->slotReset();
    m_coeffInput->slotReset();

    m_dropInput->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_coeffInput->blockSignals(false);
}

void ImageEffect_RainDrop::prepareEffect()
{
    m_dropInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_coeffInput->setEnabled(false);

    int d        = m_dropInput->value();
    int a        = m_amountInput->value();
    int c        = m_coeffInput->value();

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();

    // Selected data from the image
    QRect selection( iface->selectedXOrg(), iface->selectedYOrg(),
                     iface->selectedWidth(), iface->selectedHeight() );

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new RainDrop(iface->getOriginalImg(), this, d, a, c, &selection));
}

void ImageEffect_RainDrop::prepareFinal()
{
    m_dropInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_coeffInput->setEnabled(false);

    int d       = m_dropInput->value();
    int a       = m_amountInput->value();
    int c       = m_coeffInput->value();

    Digikam::ImageIface iface(0, 0);

    // Selected data from the image
    QRect selection( iface.selectedXOrg(), iface.selectedYOrg(),
                     iface.selectedWidth(), iface.selectedHeight() );

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new RainDrop(iface.getOriginalImg(), this, d, a, c, &selection));
}

void ImageEffect_RainDrop::putPreviewData(void)
{
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();

    Digikam::DImg imDest = m_threadedFilter->getTargetImage()
            .smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreviewImage(imDest.bits());

    m_imagePreviewWidget->updatePreview();
}

void ImageEffect_RainDrop::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("RainDrop"),
                           m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamRainDropImagesPlugin

