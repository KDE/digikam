/* ============================================================
 * File  : imageeffect_refocus.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-04-29
 * Description : a digiKam image editor plugin to refocus 
 *               an image.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
#include <qfile.h>
#include <qtextstream.h>
#include <qrect.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kdebug.h>

// Local includes.

#include "refocus.h"
#include "version.h"
#include "imageeffect_refocus.h"

#define MAX_MATRIX_SIZE 25

namespace DigikamRefocusImagesPlugin
{

ImageEffect_Refocus::ImageEffect_Refocus(QWidget* parent, QString title, QFrame* banner)
                   : Digikam::CtrlPanelDlg(parent, title, "refocus", true,
                                           false, true, Digikam::ImagePannelWidget::SeparateViewAll,
                                           banner)
{
    QString whatsThis;
        
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Refocus a Photograph"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to refocus a photograph."),
                                       KAboutData::License_GPL,
                                       "(c) 2006, Gilles Caulier",
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at kdemail dot net");

    about->addAuthor("Ernst Lippe", I18N_NOOP("FIR Wiener deconvolution filter algorithm"), 
                     "ernstl@users.sourceforge.net");
                     
    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 9, 1, 0, spacingHint() );
    
    QLabel *label2 = new QLabel(i18n("Circular sharpness:"), gboxSettings);
    m_radius = new KDoubleNumInput(gboxSettings);
    m_radius->setPrecision(2);
    m_radius->setRange(0.0, 5.0, 0.01, true);
    QWhatsThis::add( m_radius, i18n("<p>This is the Radius of the Circular convolution. It is the most important "
                                    "parameter for using the plugin. For most images the default value of 1.0 "
                                    "should give good results. Select a higher value when your image is very blurred."));
    
    gridSettings->addMultiCellWidget(label2, 0, 0, 0, 1);
    gridSettings->addMultiCellWidget(m_radius, 1, 1, 0, 1);
        
    // -------------------------------------------------------------
    
    QLabel *label4 = new QLabel(i18n("Correlation:"), gboxSettings);
    m_correlation = new KDoubleNumInput(gboxSettings);
    m_correlation->setPrecision(2);
    m_correlation->setRange(0.0, 1.0, 0.01, true);
    QWhatsThis::add( m_correlation, i18n("<p>Increasing the Correlation may help to reduce artifacts. The correlation can "
                                         "range from 0-1. Useful values are 0.5 and values close to 1, e.g. 0.95 and 0.99. "
                                         "Using a high value for the Correlation will reduce the sharpening effect of the "
                                         "plugin."));

    gridSettings->addMultiCellWidget(label4, 2, 2, 0, 1);
    gridSettings->addMultiCellWidget(m_correlation, 3, 3, 0, 1);
    
    // -------------------------------------------------------------
            
    QLabel *label5 = new QLabel(i18n("Noise filter:"), gboxSettings);
    m_noise = new KDoubleNumInput(gboxSettings);
    m_noise->setPrecision(3);
    m_noise->setRange(0.0, 1.0, 0.001, true);
    QWhatsThis::add( m_noise, i18n("<p>Increasing the Noise Filter parameter may help to reduce artifacts. The Noise Filter "
                                   "can range from 0-1 but values higher than 0.1 are rarely helpful. When the Noise Filter "
                                   "value is too low, e.g. 0.0 the image quality will be very poor. A useful value is 0.01. "
                                   "Using a high value for the Noise Filter will reduce the sharpening "
                                   "effect of the plugin."));

    gridSettings->addMultiCellWidget(label5, 4, 4, 0, 1);
    gridSettings->addMultiCellWidget(m_noise, 5, 5, 0, 1);
            
    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Gaussian sharpness:"), gboxSettings);
    m_gauss = new KDoubleNumInput(gboxSettings);
    m_gauss->setPrecision(2);
    m_gauss->setRange(0.0, 1.0, 0.01, true);
    QWhatsThis::add( m_gauss, i18n("<p>This is the Sharpness for the Gaussian convolution. Use this parameter when your "
                                   "blurring is of Gaussian type. In most cases you should set this parameter to 0, because "
                                   "it causes nasty artifacts. When you use non-zero values you will probably have to "
                                   "increase the Correlation and/or Noise Filter parameters too."));

    gridSettings->addMultiCellWidget(label3, 6, 6, 0, 1);
    gridSettings->addMultiCellWidget(m_gauss, 7, 7, 0, 1);
    
    // -------------------------------------------------------------
    
    QLabel *label1 = new QLabel(i18n("Matrix size:"), gboxSettings);
    m_matrixSize = new KIntNumInput(gboxSettings);
    m_matrixSize->setRange(0, MAX_MATRIX_SIZE, 1, true);  
    QWhatsThis::add( m_matrixSize, i18n("<p>This parameter determines the size of the transformation matrix. "
                                        "Increasing the Matrix Width may give better results, especially when you have "
                                        "chosen large values for Circular or Gaussian Sharpness."));

    gridSettings->addMultiCellWidget(label1, 8, 8, 0, 1);
    gridSettings->addMultiCellWidget(m_matrixSize, 9, 9, 0, 1);
        
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------
                
    connect(m_matrixSize, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));                        
    
    connect(m_radius, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_gauss, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_correlation, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_noise, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    // -------------------------------------------------------------
    
    // Image creation with dummy borders (mosaic mode). It needs to do it before to apply deconvolution filter 
    // on original image border pixels including on matrix size area. This way limit artefacts on image border.
    
    Digikam::ImageIface iface(0, 0);
        
    uchar* data = iface.getOriginalImage();
    int    w    = iface.originalWidth();
    int    h    = iface.originalHeight();
    bool   sb   = iface.originalSixteenBit();
    bool   a    = iface.originalHasAlpha();
    
    m_img = Digikam::DImg( w + 4*MAX_MATRIX_SIZE, h + 4*MAX_MATRIX_SIZE, sb, a);
    
    Digikam::DImg tmp;
    Digikam::DImg org(w, h, sb, a, data);

    // Copy original.
    m_img.bitBltImage(&org, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
   
    // Create dummy top border
    tmp = org.copy(0, 0, w, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 2*MAX_MATRIX_SIZE, 0);

    // Create dummy bottom border
    tmp = org.copy(0, h-2*MAX_MATRIX_SIZE, w, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE+h);

    // Create dummy left border
    tmp = org.copy(0, 0, 2*MAX_MATRIX_SIZE, h);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    m_img.bitBltImage(&tmp, 0, 2*MAX_MATRIX_SIZE);
    
    // Create dummy right border
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, 0, 2*MAX_MATRIX_SIZE, h);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    m_img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    
    // Create dummy top/left corner
    tmp = org.copy(0, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 0, 0);
    
    // Create dummy top/right corner
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, 0);

    // Create dummy bottom/left corner
    tmp = org.copy(0, h-2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 0, h+2*MAX_MATRIX_SIZE);
    
    // Create dummy bottom/right corner
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, h-2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, h+2*MAX_MATRIX_SIZE);
 
    delete [] data;
}

ImageEffect_Refocus::~ImageEffect_Refocus()
{
}

void ImageEffect_Refocus::renderingFinished(void)
{
    m_matrixSize->setEnabled(true);
    m_radius->setEnabled(true);
    m_gauss->setEnabled(true);
    m_correlation->setEnabled(true);
    m_noise->setEnabled(true);
}

void ImageEffect_Refocus::resetValues(void)
{
    m_matrixSize->blockSignals(true);
    m_radius->blockSignals(true);
    m_gauss->blockSignals(true);
    m_correlation->blockSignals(true);
    m_noise->blockSignals(true);
    
    m_matrixSize->setValue(5);
    m_radius->setValue(1.0);
    m_gauss->setValue(0.0);
    m_correlation->setValue(0.5);
    m_noise->setValue(0.03);
    
    m_matrixSize->blockSignals(false);
    m_radius->blockSignals(false);
    m_gauss->blockSignals(false);
    m_correlation->blockSignals(false);
    m_noise->blockSignals(false);
}   

void ImageEffect_Refocus::prepareEffect()
{
    m_matrixSize->setEnabled(false);
    m_radius->setEnabled(false);
    m_gauss->setEnabled(false);
    m_correlation->setEnabled(false);
    m_noise->setEnabled(false);
    
    int    ms     = m_matrixSize->value();
    double r      = m_radius->value();
    double g      = m_gauss->value();
    double c      = m_correlation->value();
    double n      = m_noise->value();

    QRect area    = m_imagePreviewWidget->getOriginalImageRegionToRender();
    QRect tmpRect;
    tmpRect.setLeft(area.left()-2*ms);
    tmpRect.setRight(area.right()+2*ms);
    tmpRect.setTop(area.top()-2*ms);
    tmpRect.setBottom(area.bottom()+2*ms);
    tmpRect.moveBy(2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    Digikam::DImg imTemp = m_img.copy(tmpRect);
        
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new Refocus(&imTemp, this, ms, r, g, c, n));
}

void ImageEffect_Refocus::prepareFinal()
{
    m_matrixSize->setEnabled(false);
    m_radius->setEnabled(false);
    m_gauss->setEnabled(false);
    m_correlation->setEnabled(false);
    m_noise->setEnabled(false);
    
    int    ms   = m_matrixSize->value();
    double r    = m_radius->value();
    double g    = m_gauss->value();
    double c    = m_correlation->value();
    double n    = m_noise->value();
    
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new Refocus(&m_img, this, ms, r, g, c, n));
}

void ImageEffect_Refocus::putPreviewData(void)
{
    int   ms   = m_matrixSize->value();
    QRect area = m_imagePreviewWidget->getOriginalImageRegionToRender();

    Digikam::DImg imDest = m_threadedFilter->getTargetImage()
                              .copy(2*ms, 2*ms, area.width(), area.height());
    m_imagePreviewWidget->setPreviewImage(imDest);
}

void ImageEffect_Refocus::putFinalData(void)
{
    QRect area = m_imagePreviewWidget->getOriginalImageRegionToRender();
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Refocus"), m_threadedFilter->getTargetImage()
                                                .copy(2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE,
                                                        iface.originalWidth(),
                                                        iface.originalHeight())
                                                .bits());
}

void ImageEffect_Refocus::slotUser3()
{
    KURL loadRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Refocus Settings File to Load")) );
    if( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.path());
    
    if ( file.open(IO_ReadOnly) )   
    {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Refocus Configuration File" )
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Refocus settings text file.")
                        .arg(loadRestorationFile.fileName()));
           file.close();            
           return;
        }
        
        blockSignals(true);
        m_matrixSize->setValue( stream.readLine().toInt() );
        m_radius->setValue( stream.readLine().toDouble() );
        m_gauss->setValue( stream.readLine().toDouble() );
        m_correlation->setValue( stream.readLine().toDouble() );
        m_noise->setValue( stream.readLine().toDouble() );
        blockSignals(false);
        slotEffect();  
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Refocus text file."));

    file.close();             
}

void ImageEffect_Refocus::slotUser2()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Refocus Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());
    
    if ( file.open(IO_WriteOnly) )   
    {
        QTextStream stream( &file );        
        stream << "# Photograph Refocus Configuration File\n";    
        stream << m_matrixSize->value() << "\n";    
        stream << m_radius->value() << "\n";    
        stream << m_gauss->value() << "\n";    
        stream << m_correlation->value() << "\n";    
        stream << m_noise->value() << "\n";    
    }
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Refocus text file."));
    
    file.close();        
}

}  // NameSpace DigikamRefocusImagesPlugin

#include "imageeffect_refocus.moc"
