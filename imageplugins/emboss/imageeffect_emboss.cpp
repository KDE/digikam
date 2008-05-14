/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin to emboss
 *               an image.
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QLabel>
#include <QGridLayout>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "emboss.h"
#include "imageeffect_emboss.h"
#include "imageeffect_emboss.moc"

namespace DigikamEmbossImagesPlugin
{

ImageEffect_Emboss::ImageEffect_Emboss(QWidget* parent)
                  : Digikam::CtrlPanelDlg(parent, i18n("Emboss Image"), "emboss",
                                          false, false, true,
                                          Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Emboss Image"),
                                       digiKamVersion().toAscii(),
                                       ki18n("Emboss image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2004-2006, Gilles Caulier\n"
                                       "(c) 2006-2008, Gilles Caulier and Marcel Wiesweg"),
                                       KLocalizedString(),
                                       "http://www.digikam.org");

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Pieter Z. Voloshyn"), ki18n("Emboss algorithm"),
                     "pieter dot voloshyn at gmail dot com");

    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings );

    QLabel *label1 = new QLabel(i18n("Depth:"), gboxSettings);

    m_depthInput   = new KIntNumInput(gboxSettings);
    m_depthInput->setRange(10, 300, 1);
    m_depthInput->setSliderEnabled(true);
    m_depthInput->setWhatsThis( i18n("<p>Set here the depth of the embossing image effect.") );

    // -------------------------------------------------------------

    gridSettings->addWidget(label1, 0, 0, 1, 2 );
    gridSettings->addWidget(m_depthInput, 1, 0, 1, 2 );
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("emboss Tool Dialog");
    m_depthInput->blockSignals(true);
    m_depthInput->setValue(group.readEntry("DepthAjustment", 30));
    m_depthInput->blockSignals(false);
}

void ImageEffect_Emboss::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("emboss Tool Dialog");
    group.writeEntry("DepthAjustment", m_depthInput->value());
    group.sync();
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
