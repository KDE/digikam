/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-25
 * Description : a plugin to simulate Oil Painting
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

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "oilpaint.h"
#include "imageeffect_oilpaint.h"
#include "imageeffect_oilpaint.moc"

using namespace KDcrawIface;
namespace DigikamOilPaintImagesPlugin
{

ImageEffect_OilPaint::ImageEffect_OilPaint(QWidget* parent)
                    : Digikam::CtrlPanelDlg(parent, i18n("Apply Oil Paint Effect"),
                                            "oilpaint", false, false, true,
                                            Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("Oil Paint"),
                                       digikam_version,
                                       I18N_NOOP("An oil painting image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier\n"
                                       "(c) 2006-2008, Gilles Caulier and Marcel Wiesweg",
                                       0,
                                       "http://wwww.digikam.org");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Oil paint algorithm"),
                     "pieter dot voloshyn at gmail dot com");

    about->addAuthor("Marcel Wiesweg", I18N_NOOP("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 3, 1, 0, spacingHint());

    QLabel *label1   = new QLabel(i18n("Brush size:"), gboxSettings);
    m_brushSizeInput = new RIntNumInput(gboxSettings);
    m_brushSizeInput->setRange(1, 5, 1);
    QWhatsThis::add( m_brushSizeInput, i18n("<p>Set here the brush size to use for "
                                            "simulating the oil painting.") );

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 1);
    gridSettings->addMultiCellWidget(m_brushSizeInput, 1, 1, 0, 1);

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Smooth:"), gboxSettings);
    m_smoothInput  = new RIntNumInput(gboxSettings);
    m_smoothInput->setRange(10, 255, 1);
    QWhatsThis::add( m_smoothInput, i18n("<p>This value controls the smoothing effect "
                                         "of the brush under the canvas.") );

    gridSettings->addMultiCellWidget(label2, 2, 2, 0, 1);
    gridSettings->addMultiCellWidget(m_smoothInput, 3, 3, 0, 1);

    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_brushSizeInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_smoothInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));
}

ImageEffect_OilPaint::~ImageEffect_OilPaint()
{
}

void ImageEffect_OilPaint::renderingFinished()
{
    m_brushSizeInput->setEnabled(true);
    m_smoothInput->setEnabled(true);
}

void ImageEffect_OilPaint::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("oilpaint Tool Dialog");
    m_brushSizeInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_brushSizeInput->setValue(config->readNumEntry("BrushSize", 1));
    m_smoothInput->setValue(config->readNumEntry("SmoothAjustment", 30));
    m_brushSizeInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
}

void ImageEffect_OilPaint::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("oilpaint Tool Dialog");
    config->writeEntry("BrushSize", m_brushSizeInput->value());
    config->writeEntry("SmoothAjustment", m_smoothInput->value());
    config->sync();
}

void ImageEffect_OilPaint::resetValues()
{
    m_brushSizeInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_brushSizeInput->setValue(1);
    m_smoothInput->setValue(30);
    m_brushSizeInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
}

void ImageEffect_OilPaint::prepareEffect()
{
    m_brushSizeInput->setEnabled(false);
    m_smoothInput->setEnabled(false);

    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();

    int b = m_brushSizeInput->value();
    int s = m_smoothInput->value();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new OilPaint(&image, this, b, s));
}

void ImageEffect_OilPaint::prepareFinal()
{
    m_brushSizeInput->setEnabled(false);
    m_smoothInput->setEnabled(false);

    int b = m_brushSizeInput->value();
    int s = m_smoothInput->value();

    Digikam::ImageIface iface(0, 0);
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new OilPaint(iface.getOriginalImg(), this, b, s));
}

void ImageEffect_OilPaint::putPreviewData(void)
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_OilPaint::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Oil Paint"),
                        m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamOilPaintImagesPlugin

