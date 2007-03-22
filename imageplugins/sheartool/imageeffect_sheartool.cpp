/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2004-12-23
 * Description : a digiKam image editor plugin to process 
 *               shearing image.
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
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qimage.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kcursor.h>
#include <kconfig.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "sheartool.h"
#include "imageeffect_sheartool.h"
#include "imageeffect_sheartool.moc"

namespace DigikamShearToolImagesPlugin
{

ImageEffect_ShearTool::ImageEffect_ShearTool(QWidget* parent)
                     : Digikam::ImageGuideDlg(parent, i18n("Shear Tool"), "sheartool",
                                              false, true, true, 
                                              Digikam::ImageGuideWidget::HVGuideMode)
{
    // No need Abort button action.
    showButton(User1, false); 
    
    QString whatsThis;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("Shear Tool"), 
                                       digikam_version,
                                       I18N_NOOP("A digiKam image plugin to shear an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2007, Gilles Caulier",
                                       0,
                                       "http://www.digikam.org");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");
    
    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Shear algorithm"), 
                     "pieter dot voloshyn at gmail dot com"); 

    setAboutData(about);
    
    QWhatsThis::add( m_imagePreviewWidget, i18n("<p>This is the shearing image operation preview. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be drawn "
                                           "to guide you in adjusting the shearing correction. "
                                           "Release the left mouse button to freeze the dashed "
                                           "line's position."));
                                           
    // -------------------------------------------------------------

    QString temp;
    Digikam::ImageIface iface(0, 0);

    QWidget *gboxSettings     = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 11, 2, spacingHint());
    
    QLabel *label1 = new QLabel(i18n("New width:"), gboxSettings);
    m_newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"), gboxSettings);
    m_newWidthLabel->setAlignment( AlignBottom | AlignRight );
    
    QLabel *label2 = new QLabel(i18n("New height:"), gboxSettings);
    m_newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"), gboxSettings);
    m_newHeightLabel->setAlignment( AlignBottom | AlignRight );

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_newWidthLabel, 0, 0, 1, 2);
    gridSettings->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_newHeightLabel, 1, 1, 1, 2);
    
    KSeparator *line = new KSeparator (Horizontal, gboxSettings);
    gridSettings->addMultiCellWidget(line, 2, 2, 0, 2);
    
    QLabel *label3 = new QLabel(i18n("Main horizontal angle:"), gboxSettings);
    m_mainHAngleInput = new KIntNumInput(gboxSettings);
    m_mainHAngleInput->setRange(-45, 45, 1, true);
    m_mainHAngleInput->setValue(0);
    QWhatsThis::add( m_mainHAngleInput, i18n("<p>The main horizontal shearing angle, in degrees."));
    gridSettings->addMultiCellWidget(label3, 3, 3, 0, 2);
    gridSettings->addMultiCellWidget(m_mainHAngleInput, 4, 4, 0, 2);
            
    QLabel *label4 = new QLabel(i18n("Fine horizontal angle:"), gboxSettings);
    m_fineHAngleInput = new KDoubleNumInput(gboxSettings);
    m_fineHAngleInput->setRange(-5.0, 5.0, 0.01, true);
    m_fineHAngleInput->setValue(0);
    QWhatsThis::add( m_fineHAngleInput, i18n("<p>This value in degrees will be added to main horizontal angle value "
                                             "to set fine adjustments."));
    gridSettings->addMultiCellWidget(label4, 5, 5, 0, 2);
    gridSettings->addMultiCellWidget(m_fineHAngleInput, 6, 6, 0, 2);
                
    QLabel *label5 = new QLabel(i18n("Main vertical angle:"), gboxSettings);
    m_mainVAngleInput = new KIntNumInput(gboxSettings);
    m_mainVAngleInput->setRange(-45, 45, 1, true);
    m_mainVAngleInput->setValue(0);
    QWhatsThis::add( m_mainVAngleInput, i18n("<p>The main vertical shearing angle, in degrees."));
    gridSettings->addMultiCellWidget(label5, 7, 7, 0, 0);
    gridSettings->addMultiCellWidget(m_mainVAngleInput, 8, 8, 0, 2);

    QLabel *label6 = new QLabel(i18n("Fine vertical angle:"), gboxSettings);
    m_fineVAngleInput = new KDoubleNumInput(gboxSettings);
    m_fineVAngleInput->setRange(-5.0, 5.0, 0.01, true);
    m_fineVAngleInput->setValue(0);
    QWhatsThis::add( m_fineVAngleInput, i18n("<p>This value in degrees will be added to main vertical angle value "
                                             "to set fine adjustments."));
    gridSettings->addMultiCellWidget(label6, 9, 9, 0, 2);
    gridSettings->addMultiCellWidget(m_fineVAngleInput, 10, 10, 0, 2);
                                             
    m_antialiasInput = new QCheckBox(i18n("Anti-Aliasing"), gboxSettings);
    QWhatsThis::add( m_antialiasInput, i18n("<p>Enable this option to process anti-aliasing filter "
                                            "to the sheared image. "
                                            "To smooth the target image, it will be blurred a little."));
    gridSettings->addMultiCellWidget(m_antialiasInput, 11, 11, 0, 2);
            
    setUserAreaWidget(gboxSettings); 

    // -------------------------------------------------------------
    
    connect(m_mainHAngleInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_fineHAngleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
                
    connect(m_mainVAngleInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));
    
    connect(m_fineVAngleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
    
    connect(m_antialiasInput, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));  
}

ImageEffect_ShearTool::~ImageEffect_ShearTool()
{
}

void ImageEffect_ShearTool::readUserSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("sheartool Tool Dialog");
    m_mainHAngleInput->setValue(config->readNumEntry("Main HAngle", 0));
    m_mainVAngleInput->setValue(config->readNumEntry("Main VAngle", 0));
    m_fineHAngleInput->setValue(config->readDoubleNumEntry("Fine HAngle", 0.0));
    m_fineVAngleInput->setValue(config->readDoubleNumEntry("Fine VAngle", 0.0));
    m_antialiasInput->setChecked(config->readBoolEntry("Anti Aliasing", true));
    slotEffect();
}

void ImageEffect_ShearTool::writeUserSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("sheartool Tool Dialog");
    config->writeEntry("Main HAngle", m_mainHAngleInput->value());
    config->writeEntry("Main VAngle", m_mainVAngleInput->value());
    config->writeEntry("Fine HAngle", m_fineHAngleInput->value());
    config->writeEntry("Fine VAngle", m_fineVAngleInput->value());
    config->writeEntry("Anti Aliasing", m_antialiasInput->isChecked());
    config->sync();
}

void ImageEffect_ShearTool::resetValues()
{
    m_mainHAngleInput->blockSignals(true);
    m_mainVAngleInput->blockSignals(true);
    m_fineHAngleInput->blockSignals(true);
    m_fineVAngleInput->blockSignals(true);
    m_antialiasInput->blockSignals(true);
    m_mainHAngleInput->setValue(0);
    m_mainVAngleInput->setValue(0);
    m_fineHAngleInput->setValue(0.0);
    m_fineVAngleInput->setValue(0.0);
    m_antialiasInput->setChecked(true);
    m_mainHAngleInput->blockSignals(false);
    m_mainVAngleInput->blockSignals(false);
    m_fineHAngleInput->blockSignals(false);
    m_fineVAngleInput->blockSignals(false);
    m_antialiasInput->blockSignals(false);    
} 

void ImageEffect_ShearTool::prepareEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_mainHAngleInput->setEnabled(false);
    m_mainVAngleInput->setEnabled(false);
    m_fineHAngleInput->setEnabled(false);
    m_fineVAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);

    float hAngle      = m_mainHAngleInput->value() + m_fineHAngleInput->value();
    float vAngle      = m_mainVAngleInput->value() + m_fineVAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();    
    QColor background = paletteBackgroundColor().rgb();

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    int orgW = iface->originalWidth();
    int orgH = iface->originalHeight();
    
    uchar *data = iface->getPreviewImage();
    Digikam::DImg image(iface->previewWidth(), iface->previewHeight(), iface->previewSixteenBit(),
                        iface->previewHasAlpha(), data);
    delete [] data;

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new ShearTool(&image, this, hAngle, vAngle, antialiasing, background, orgW, orgH));    
}

void ImageEffect_ShearTool::prepareFinal()
{
    m_mainHAngleInput->setEnabled(false);
    m_mainVAngleInput->setEnabled(false);
    m_fineHAngleInput->setEnabled(false);
    m_fineVAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);

    float hAngle      = m_mainHAngleInput->value() + m_fineHAngleInput->value();
    float vAngle      = m_mainVAngleInput->value() + m_fineVAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();    
    QColor background = Qt::black;
            
    Digikam::ImageIface iface(0, 0);
    int orgW = iface.originalWidth();
    int orgH = iface.originalHeight();

    uchar *data = iface.getOriginalImage();
    Digikam::DImg orgImage(orgW, orgH, iface.originalSixteenBit(),
                           iface.originalHasAlpha(), data);
    delete [] data;
    
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new ShearTool(&orgImage, this, hAngle, vAngle, antialiasing, background, orgW, orgH));
}

void ImageEffect_ShearTool::putPreviewData(void)
{
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    int w = iface->previewWidth();
    int h = iface->previewHeight();
        
    Digikam::DImg imTemp = m_threadedFilter->getTargetImage().smoothScale(w, h, QSize::ScaleMin);
    Digikam::DImg imDest( w, h, m_threadedFilter->getTargetImage().sixteenBit(),
                                m_threadedFilter->getTargetImage().hasAlpha() );
    
    imDest.fill( Digikam::DColor(paletteBackgroundColor().rgb(),
                                 m_threadedFilter->getTargetImage().sixteenBit()) );
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());
                 
    m_imagePreviewWidget->updatePreview();    
    QSize newSize = dynamic_cast<ShearTool *>(m_threadedFilter)->getNewSize();
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void ImageEffect_ShearTool::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);    
    Digikam::DImg targetImage = m_threadedFilter->getTargetImage();
    iface.putOriginalImage(i18n("Shear Tool"),
                           targetImage.bits(),
                           targetImage.width(), targetImage.height());
}

void ImageEffect_ShearTool::renderingFinished()
{
    m_mainHAngleInput->setEnabled(true);
    m_mainVAngleInput->setEnabled(true);
    m_fineHAngleInput->setEnabled(true);
    m_fineVAngleInput->setEnabled(true);
    m_antialiasInput->setEnabled(true);
    kapp->restoreOverrideCursor();
}

}  // NameSpace DigikamShearToolImagesPlugin

