/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin to emboss
 *               an image.
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
#include <qlayout.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "daboutdata.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "emboss.h"
#include "imageeffect_emboss.h"
#include "imageeffect_emboss.moc"

using namespace KDcrawIface;
namespace DigikamEmbossImagesPlugin
{

ImageEffect_Emboss::ImageEffect_Emboss(QWidget* parent)
                  : Digikam::CtrlPanelDlg(parent, i18n("Emboss Image"), "emboss", false, false, true,
                                          Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("Emboss Image"),
                                       digikam_version,
                                       I18N_NOOP("Emboss image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2006, Gilles Caulier\n"
                                       "(c) 2006-2008, Gilles Caulier and Marcel Wiesweg",
                                       0,
                                       Digikam::webProjectUrl());

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Emboss algorithm"),
                     "pieter dot voloshyn at gmail dot com");

    about->addAuthor("Marcel Wiesweg", I18N_NOOP("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 1, 1, 0, spacingHint());
    QLabel *label1 = new QLabel(i18n("Depth:"), gboxSettings);

    m_depthInput = new RIntNumInput(gboxSettings);
    m_depthInput->setRange(10, 300, 1);
    QWhatsThis::add( m_depthInput, i18n("<p>Set here the depth of the embossing image effect.") );

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 1);
    gridSettings->addMultiCellWidget(m_depthInput, 1, 1, 0, 1);

    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_depthInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));
}

ImageEffect_Emboss::~ImageEffect_Emboss()
{
}

void ImageEffect_Emboss::renderingFinished()
{
    m_depthInput->setEnabled(true);
}

void ImageEffect_Emboss::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("emboss Tool Dialog");
    m_depthInput->blockSignals(true);
    m_depthInput->setValue(config->readNumEntry("DepthAjustment", 30));
    m_depthInput->blockSignals(false);
}

void ImageEffect_Emboss::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("emboss Tool Dialog");
    config->writeEntry("DepthAjustment", m_depthInput->value());
    config->sync();
}

void ImageEffect_Emboss::resetValues()
{
    m_depthInput->blockSignals(true);
    m_depthInput->setValue(30);
    m_depthInput->blockSignals(false);
}

void ImageEffect_Emboss::prepareEffect()
{
    m_depthInput->setEnabled(false);

    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();

    int depth = m_depthInput->value();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new Emboss(&image, this, depth));
}

void ImageEffect_Emboss::prepareFinal()
{
    m_depthInput->setEnabled(false);

    int depth = m_depthInput->value();

    Digikam::ImageIface iface(0, 0);
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new Emboss(iface.getOriginalImg(), this, depth));
}

void ImageEffect_Emboss::putPreviewData(void)
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_Emboss::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Emboss"),
                           m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamEmbossImagesPlugin

