/* ============================================================
 * File  : imageeffect_noisereduction.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-24
 * Description : noise reduction image filter for digiKam 
 *               image editor.
 * 
 * Copyright 2004-2006 by Gilles Caulier
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
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <qimage.h>
#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "noisereduction.h"
#include "imageeffect_noisereduction.h"

namespace DigikamNoiseReductionImagesPlugin
{

ImageEffect_NoiseReduction::ImageEffect_NoiseReduction(QWidget* parent, QString title, QFrame* banner)
                          : Digikam::CtrlPanelDlg(parent, title, "noisereduction", true,
                                     false, true, Digikam::ImagePannelWidget::SeparateViewAll, banner)
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Noise Reduction"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A noise reduction image filter plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2006, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at kdemail dot net");

    about->addAuthor("Peter Heckert", I18N_NOOP("Original Noise Reduction algorithm author"),
                     "peter dot heckert at arcor dot de");
                     
    setAboutData(about);
   
    // -------------------------------------------------------------

    QTabWidget *mainTab = new QTabWidget(m_imagePreviewWidget);
    
    QWidget* firstPage = new QWidget( mainTab );
    QGridLayout* gridSettings = new QGridLayout( firstPage, 4, 1, marginHint());    
    mainTab->addTab( firstPage, i18n("Details") );
    
    QLabel *label1 = new QLabel(i18n("Radius:"), firstPage);
    
    m_radiusInput = new KDoubleNumInput(firstPage);
    m_radiusInput->setPrecision(1);
    m_radiusInput->setRange(0.0, 3.0, 0.1, true);
    QWhatsThis::add( m_radiusInput, i18n("<p>Set here the <b>Radius of gaussian blur</b> in pixels used to "
                     "filter noise. In any case it must be about the same size as noise granularity ore "
                     "somewhat more. If it is set higher than necessary, then it can cause unwanted blur."));

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_radiusInput, 0, 0, 1, 1);
    
    // -------------------------------------------------------------
  
    QLabel *label4 = new QLabel(i18n("Texture:"), firstPage);
    
    m_textureInput = new KDoubleNumInput(firstPage);
    m_textureInput->setPrecision(2);
    m_textureInput->setRange(-0.99, 0.99, 0.01, true);
    QWhatsThis::add( m_textureInput, i18n("<p>Set here the <b>Texture Accuracy</b> adjustment. "
                "This value can be used, to get more or less texture accuracy.When decreased, "
                "then noise and texture are blurred out, when increased then texture is "
                "amplified, but also noise will increase. It has almost no effect to image edges, "
                "opposed to filter <b>Threshold</b>, which would blur edges, when increased. "
                "If <b>Threshold</b> is adjusted in away so that edges are sharp, and there "
                "is still too much area noise, then texture detail could be used to reduce noise "
                "without blurring edges. Another way would be to decrease radius and to increase "
                "threshold)"));

    gridSettings->addMultiCellWidget(label4, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_textureInput, 1, 1, 1, 1);

    // -------------------------------------------------------------

    QLabel *label7 = new QLabel(i18n("Sharpness:"), firstPage);  // Filter setting "Lookahead".
    
    m_lookaheadInput = new KDoubleNumInput(firstPage);
    m_lookaheadInput->setPrecision(1);
    m_lookaheadInput->setRange(0.5, 10.0, 0.5, true);
    QWhatsThis::add( m_lookaheadInput, i18n("<p>Set here the <b>Sharpness Level</b> adjustement. "
                "This value defines the pixel distance in which the filter looks ahead for luminance "
                "variations. When this value is increased, then spikenoise is erased. "
                "You can eventually readjust filter <b>Threshold</b>, when you changed this setting. "
                "When this value is to high, then the adaptive filter cannot longer accurately track "
                "image details, and noise can reappear or blur can occur."));
    
    gridSettings->addMultiCellWidget(label7, 2, 2, 0, 0);
    gridSettings->addMultiCellWidget(m_lookaheadInput, 2, 2, 1, 1);

    // -------------------------------------------------------------

    QLabel *label5 = new QLabel(i18n("Edge:"), firstPage);     // Filter setting "Sharp".
    
    m_sharpnessInput = new KDoubleNumInput(firstPage);
    m_sharpnessInput->setPrecision(2);
    m_sharpnessInput->setRange(0.0, 2.0, 0.01, true);
    QWhatsThis::add( m_sharpnessInput, i18n("<p>Set here the <b>Edge Accuracy</b> adjustment of sharpness. "
                "This value improves the frequency response for the filter. "
                "When it is too strong then not all noise can be removed, or spike noise may appear. "
                "Set it near to maximum, if you want to remove weak noise or JPEG-artifacts, "
                "without loosing detail."));

    gridSettings->addMultiCellWidget(label5, 3, 3, 0, 0);
    gridSettings->addMultiCellWidget(m_sharpnessInput, 3, 3, 1, 1);


    // -------------------------------------------------------------

    QLabel *label10 = new QLabel(i18n("Erosion:"), firstPage);
    
    m_phaseInput = new KDoubleNumInput(firstPage);
    m_phaseInput->setPrecision(1);
    m_phaseInput->setRange(0.5, 20.0, 0.5, true);
    QWhatsThis::add( m_phaseInput, i18n("<p>Set here the <b>Phase Shift for Edges</b> adjustement. "
                "This value can be used to erodes singular spikes and it has a smooth effect to edges, "
                "and sharpens edges by erosion, so noise at edges is eroded. The effect is dependant "
                "from <b>Sharpness</b>, <b>Damping</b>, and <b>Edges</b>. Set it to minimum, "
                "if you want to remove weak noise or JPEG-artifacts. When this value is increased, "
                "then also increasing <b>Damping</b> is often useful. This setting can provides "
                "sharpening and antialiasing effect to edges when spike noise is corrected."));
    
    gridSettings->addMultiCellWidget(label10, 4, 4, 0, 0);
    gridSettings->addMultiCellWidget(m_phaseInput, 4, 4, 1, 1);

    m_imagePreviewWidget->setUserAreaWidget(mainTab);

    // -------------------------------------------------------------
    

    QWidget* secondPage = new QWidget( mainTab );
    QGridLayout* gridSettings2 = new QGridLayout( secondPage, 4, 1, marginHint());    
    mainTab->addTab( secondPage, i18n("Lightness") );
    
    QLabel *label2 = new QLabel(i18n("Luminance:"), secondPage);
    
    m_lumToleranceInput = new KDoubleNumInput(secondPage);
    m_lumToleranceInput->setPrecision(1);
    m_lumToleranceInput->setRange(0.1, 5.0, 0.1, true);
    QWhatsThis::add( m_lumToleranceInput, i18n("<p>Set here the <b>Luminance Tolerance</b> adjustement "
                "of image. It's recommended to use only <b>Color</b> or <b>Luminance</b> tolerance "
                "settings to make an image correction, not the both at the same time. This settings "
                "don't influence the main smoothing process."));

    gridSettings2->addMultiCellWidget(label2, 0, 0, 0, 0);
    gridSettings2->addMultiCellWidget(m_lumToleranceInput, 0, 0, 1, 1);                         
    
    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Threshold:"), secondPage);
    
    m_thresholdInput = new KDoubleNumInput(secondPage);
    m_thresholdInput->setPrecision(1);
    m_thresholdInput->setRange(0.0, 5.0, 0.1, true);
    QWhatsThis::add( m_thresholdInput, i18n("<p>Set here the <b>Threshold</b> for 2nd derivative of "
                     "luminance adjustment. This value should be set so that edges are clearly visible "
                     "and noise is smoothed out. This value is not bound to any intensity value, it is "
                     "bound to the second derivative of intensity values. Simply adjust it and watch the " 
                     "preview. Adjustment must be made carefully, because the gap between \"noisy\", "
                     "\"smooth\", and \"blur\" is very small. Adjust it as carefully as you would adjust "
                     "the focus of a camera."));

    gridSettings2->addMultiCellWidget(label3, 1, 1, 0, 0);
    gridSettings2->addMultiCellWidget(m_thresholdInput, 1, 1, 1, 1);
                                              
    // -------------------------------------------------------------

    QLabel *label6 = new QLabel(i18n("Color:"), secondPage);
    
    m_csmoothInput = new KDoubleNumInput(secondPage);
    m_csmoothInput->setPrecision(1);
    m_csmoothInput->setRange(1.0, 5.0, 0.1, true);
    QWhatsThis::add( m_csmoothInput, i18n("<p>Set here the <b>Color Tolerance</b> adjustement of image. "
                "It's recommended to use only <b>Color</b> or <b>Luminance</b> tolerance settings to "
                "make an image correction, not the both at the same time. This settings don't influence "
                "the main smoothing process."));

    gridSettings2->addMultiCellWidget(label6, 2, 2, 0, 0);
    gridSettings2->addMultiCellWidget(m_csmoothInput, 2, 2, 1, 1);
    
    // -------------------------------------------------------------

    QLabel *label8 = new QLabel(i18n("Gamma:"), secondPage);
    
    m_gammaInput = new KDoubleNumInput(secondPage);
    m_gammaInput->setPrecision(1);
    m_gammaInput->setRange(1.0, 5.0, 0.1, true);
    QWhatsThis::add( m_gammaInput, i18n("<p>Set here the <b>Gamma</b> adjustement. This value "
                "can be used to increase the tolerance values for darker areas (which commonly "
                "are more noisy). This results in more blur for shadow areas."));
    
    gridSettings2->addMultiCellWidget(label8, 3, 3, 0, 0);
    gridSettings2->addMultiCellWidget(m_gammaInput, 3, 3, 1, 1);

    // -------------------------------------------------------------

    QLabel *label9 = new QLabel(i18n("Damping:"), secondPage);
    
    m_dampingInput = new KDoubleNumInput(secondPage);
    m_dampingInput->setPrecision(1);
    m_dampingInput->setRange(0.5, 20.0, 0.5, true);
    QWhatsThis::add( m_dampingInput, i18n("<p>Set here the <b>Phase Jitter Damping</b> adjustement. "
                "This value defines how fast the adaptive filter-radius reacts to luminance variations. "
                "If increased, then edges appear smoother, if too high, then blur may occur. If at "
                "minimum then noise and phase jitter at edges can occur. It can supress spike noise when " "increased and this is the preferred method to remove it."));
    
    gridSettings2->addMultiCellWidget(label9, 4, 4, 0, 0);
    gridSettings2->addMultiCellWidget(m_dampingInput, 4, 4, 1, 1);
    
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_lumToleranceInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_thresholdInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            
           
    connect(m_textureInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_sharpnessInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));           

    connect(m_csmoothInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_lookaheadInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_gammaInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            
           
    connect(m_dampingInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_phaseInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));   
}

ImageEffect_NoiseReduction::~ImageEffect_NoiseReduction()
{
}

void ImageEffect_NoiseReduction::renderingFinished()
{
    m_radiusInput->setEnabled(true);
    m_lumToleranceInput->setEnabled(true);
    m_thresholdInput->setEnabled(true);
    m_textureInput->setEnabled(true);
    m_sharpnessInput->setEnabled(true);

    m_csmoothInput->setEnabled(true);
    m_lookaheadInput->setEnabled(true);
    m_gammaInput->setEnabled(true);
    m_dampingInput->setEnabled(true);
    m_phaseInput->setEnabled(true);
}

void ImageEffect_NoiseReduction::resetValues()
{
    m_radiusInput->setEnabled(true);
    m_lumToleranceInput->setEnabled(true);
    m_thresholdInput->setEnabled(true);
    m_textureInput->setEnabled(true);
    m_sharpnessInput->setEnabled(true);

    m_csmoothInput->setEnabled(true);
    m_lookaheadInput->setEnabled(true);
    m_gammaInput->setEnabled(true);
    m_dampingInput->setEnabled(true);
    m_phaseInput->setEnabled(true);
                    
    m_radiusInput->setValue(1.0);
    m_lumToleranceInput->setValue(1.0);
    m_thresholdInput->setValue(0.08);
    m_textureInput->setValue(0.0);
    m_sharpnessInput->setValue(0.25);

    m_csmoothInput->setValue(1.0);
    m_lookaheadInput->setValue(2.0);
    m_gammaInput->setValue(1.0);
    m_dampingInput->setValue(5.0);
    m_phaseInput->setValue(1.0);
    
    m_radiusInput->setEnabled(false);
    m_lumToleranceInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_textureInput->setEnabled(false);
    m_sharpnessInput->setEnabled(false);

    m_csmoothInput->setEnabled(false);
    m_lookaheadInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    m_dampingInput->setEnabled(false);
    m_phaseInput->setEnabled(false);
}

void ImageEffect_NoiseReduction::prepareEffect()
{
    m_radiusInput->setEnabled(false);
    m_lumToleranceInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_textureInput->setEnabled(false);
    m_sharpnessInput->setEnabled(false);

    m_csmoothInput->setEnabled(false);
    m_lookaheadInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    m_dampingInput->setEnabled(false);
    m_phaseInput->setEnabled(false);
    
    double r  = m_radiusInput->value();
    double l  = m_lumToleranceInput->value();
    double th = m_thresholdInput->value();
    double tx = m_textureInput->value();
    double s  = m_sharpnessInput->value();

    double c  = m_csmoothInput->value();
    double a  = m_lookaheadInput->value();
    double g  = m_gammaInput->value();
    double d  = m_dampingInput->value();
    double p  = m_phaseInput->value();
    
    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new NoiseReduction(&image,
                       this, r, l, th, tx, s, c, a, g, d, p));
}

void ImageEffect_NoiseReduction::prepareFinal()
{
    m_radiusInput->setEnabled(false);
    m_lumToleranceInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_textureInput->setEnabled(false);
    m_sharpnessInput->setEnabled(false);

    m_csmoothInput->setEnabled(false);
    m_lookaheadInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    m_dampingInput->setEnabled(false);
    m_phaseInput->setEnabled(false);
    
    double r  = m_radiusInput->value();
    double l  = m_lumToleranceInput->value();
    double th = m_thresholdInput->value();
    double tx = m_textureInput->value();
    double s  = m_sharpnessInput->value();

    double c  = m_csmoothInput->value();
    double a  = m_lookaheadInput->value();
    double g  = m_gammaInput->value();
    double d  = m_dampingInput->value();
    double p  = m_phaseInput->value();

    Digikam::ImageIface iface(0, 0);
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new NoiseReduction(iface.getOriginalImg(),
                       this, r, l, th, tx, s, c, a, g, d, p));
}

void ImageEffect_NoiseReduction::putPreviewData(void)
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_NoiseReduction::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Noise Reduction"), m_threadedFilter->getTargetImage().bits());
}

void ImageEffect_NoiseReduction::slotUser3()
{
    KURL loadRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Noise Reduction Settings File to Load")) );
    if( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.path());
    
    if ( file.open(IO_ReadOnly) )   
    {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Noise Reduction Configuration File" )
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Noise Reduction settings text file.")
                        .arg(loadRestorationFile.fileName()));
           file.close();            
           return;
        }
        
        blockSignals(true);
        m_radiusInput->setValue( stream.readLine().toDouble() );
        m_lumToleranceInput->setValue( stream.readLine().toDouble() );
        m_thresholdInput->setValue( stream.readLine().toDouble() );
        m_textureInput->setValue( stream.readLine().toDouble() );
        m_sharpnessInput->setValue( stream.readLine().toDouble() );

        m_csmoothInput->setValue( stream.readLine().toDouble() );
        m_lookaheadInput->setValue( stream.readLine().toDouble() );
        m_gammaInput->setValue( stream.readLine().toDouble() );
        m_dampingInput->setValue( stream.readLine().toDouble() );
        m_phaseInput->setValue( stream.readLine().toDouble() );
        blockSignals(false);
        slotEffect();  
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Noise Reduction text file."));

    file.close();             
}

void ImageEffect_NoiseReduction::slotUser2()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Noise Reduction Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());
    
    if ( file.open(IO_WriteOnly) )   
    {
        QTextStream stream( &file );        
        stream << "# Photograph Noise Reduction Configuration File\n";    
        stream << m_radiusInput->value() << "\n";    
        stream << m_lumToleranceInput->value() << "\n";    
        stream << m_thresholdInput->value() << "\n";    
        stream << m_textureInput->value() << "\n";    
        stream << m_sharpnessInput->value() << "\n";    

        stream << m_csmoothInput->value() << "\n";    
        stream << m_lookaheadInput->value() << "\n";    
        stream << m_gammaInput->value() << "\n";    
        stream << m_dampingInput->value() << "\n";    
        stream << m_phaseInput->value() << "\n";    

    }
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Noise Reduction text file."));
    
    file.close();        
}

}  // NameSpace DigikamNoiseReductionImagesPlugin

#include "imageeffect_noisereduction.moc"
