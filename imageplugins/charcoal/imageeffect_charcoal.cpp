/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2004-08-26
 * Description : a digikam image editor plugin for 
 *               simulate charcoal drawing.
 * 
 * Copyright 2004-2007 by Gilles Caulier 
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

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "charcoal.h"
#include "imageeffect_charcoal.h"
#include "imageeffect_charcoal.moc"

namespace DigikamCharcoalImagesPlugin
{

ImageEffect_Charcoal::ImageEffect_Charcoal(QWidget* parent)
                    : Digikam::CtrlPanelDlg(parent, i18n("Charcoal Drawing"), 
                                            "charcoal", false, false, true,
                                            Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;
        
    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("Charcoal Drawing"), 
                                       digikam_version,
                                       I18N_NOOP("A digiKam charcoal drawing image effect plugin."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2007, Gilles Caulier", 
                                       0,
                                       "http://www.digikam.org");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");
    
    setAboutData(about);
        
    // -------------------------------------------------------------
    
    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 3, 1, 0, spacingHint());
    QLabel *label1 = new QLabel(i18n("Pencil size:"), gboxSettings);
    
    m_pencilInput = new KIntNumInput(gboxSettings);
    m_pencilInput->setRange(1, 100, 1, true);  
    m_pencilInput->setValue(5);
    QWhatsThis::add( m_pencilInput, i18n("<p>Set here the charcoal pencil size used to simulate the drawing."));

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 1);
    gridSettings->addMultiCellWidget(m_pencilInput, 1, 1, 0, 1);
    
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Smooth:"), gboxSettings);
    
    m_smoothInput = new KIntNumInput(gboxSettings);
    m_smoothInput->setRange(1, 100, 1, true);  
    m_smoothInput->setValue(10);
    QWhatsThis::add( m_smoothInput, i18n("<p>This value controls the smoothing effect of the pencil "
                                         "under the canvas."));

    gridSettings->addMultiCellWidget(label2, 2, 2, 0, 1);
    gridSettings->addMultiCellWidget(m_smoothInput, 3, 3, 0, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_pencilInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));      
                
    connect(m_smoothInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));      
}

ImageEffect_Charcoal::~ImageEffect_Charcoal()
{
}

void ImageEffect_Charcoal::renderingFinished()
{
    m_pencilInput->setEnabled(true);
    m_smoothInput->setEnabled(true);
}

void ImageEffect_Charcoal::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("charcoal Tool Dialog");
    m_pencilInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_pencilInput->setValue(config->readNumEntry("PencilAjustment", 5));
    m_smoothInput->setValue(config->readNumEntry("SmoothAjustment", 10));
    m_pencilInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
}

void ImageEffect_Charcoal::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("charcoal Tool Dialog");
    config->writeEntry("PencilAjustment", m_pencilInput->value());
    config->writeEntry("SmoothAjustment", m_smoothInput->value());
    config->sync();
}

void ImageEffect_Charcoal::resetValues()
{
    m_pencilInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_pencilInput->setValue(5);
    m_smoothInput->setValue(10);
    m_pencilInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
} 

void ImageEffect_Charcoal::prepareEffect()
{
    m_pencilInput->setEnabled(false);
    m_smoothInput->setEnabled(false);
            
    double pencil = (double)m_pencilInput->value()/10.0;
    double smooth = (double)m_smoothInput->value();
    
    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new Charcoal(&image, this, pencil, smooth));
}

void ImageEffect_Charcoal::prepareFinal()
{
    m_pencilInput->setEnabled(false);
    m_smoothInput->setEnabled(false);
    
    double pencil = (double)m_pencilInput->value()/10.0;
    double smooth = (double)m_smoothInput->value();

    Digikam::ImageIface iface(0, 0);
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new Charcoal(iface.getOriginalImg(), 
                                                                   this, pencil, smooth));
}

void ImageEffect_Charcoal::putPreviewData(void)
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_Charcoal::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Charcoal"), m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamCharcoalImagesPlugin

