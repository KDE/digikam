/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-25
 * Description : a plugin to simulate Oil Painting
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
#include <QImage>
#include <QGridLayout>

// KDE includes.

#include <kcursor.h>
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
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "oilpaint.h"
#include "imageeffect_oilpaint.h"
#include "imageeffect_oilpaint.moc"

namespace DigikamOilPaintImagesPlugin
{

ImageEffect_OilPaint::ImageEffect_OilPaint(QWidget* parent)
                    : Digikam::CtrlPanelDlg(parent, i18n("Apply Oil Paint Effect"),
                                            "oilpaint", false, false, true,
                                            Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Oil Paint"),
                                       digiKamVersion().toAscii(),
                                       ki18n("An oil painting image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2004-2005, Gilles Caulier\n"
                                       "(c) 2006-2008, Gilles Caulier and Marcel Wiesweg"),
                                       KLocalizedString(),
                                       "http://wwww.digikam.org");

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Pieter Z. Voloshyn"), ki18n("Oil paint algorithm"),
                     "pieter dot voloshyn at gmail dot com");

    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout(gboxSettings);

    QLabel *label1   = new QLabel(i18n("Brush size:"), gboxSettings);
    m_brushSizeInput = new KIntNumInput(gboxSettings);
    m_brushSizeInput->setRange(1, 5, 1);
    m_brushSizeInput->setSliderEnabled(true);
    m_brushSizeInput->setWhatsThis( i18n("<p>Set here the brush size to use for "
                                         "simulating the oil painting.") );

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Smooth:"), gboxSettings);
    m_smoothInput  = new KIntNumInput(gboxSettings);
    m_smoothInput->setRange(10, 255, 1);
    m_smoothInput->setSliderEnabled(true);
    m_smoothInput->setWhatsThis( i18n("<p>This value controls the smoothing effect "
                                      "of the brush under the canvas.") );

    gridSettings->setMargin(0);
    gridSettings->setSpacing(spacingHint());
    gridSettings->addWidget(label1, 0, 0, 1, 2 );
    gridSettings->addWidget(m_brushSizeInput, 1, 0, 1, 2 );
    gridSettings->addWidget(label2, 2, 0, 1, 2 );
    gridSettings->addWidget(m_smoothInput, 3, 0, 1, 2 );

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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("oilpaint Tool Dialog");
    m_brushSizeInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_brushSizeInput->setValue(group.readEntry("BrushSize", 1));
    m_smoothInput->setValue(group.readEntry("SmoothAjustment", 30));
    m_brushSizeInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
}

void ImageEffect_OilPaint::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("oilpaint Tool Dialog");
    group.writeEntry("BrushSize", m_brushSizeInput->value());
    group.writeEntry("SmoothAjustment", m_smoothInput->value());
    group.sync();
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
