/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-03-26
 * Description : a digiKam image editor plugin to restore 
 *               a photograph
 * 
 * Copyright 2005-2007 by Gilles Caulier
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
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qimage.h>

// KDE includes.

#include <kurllabel.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

// Local includes.

#include "version.h"
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "greycstorationiface.h"
#include "imageeffect_restoration.h"
#include "imageeffect_restoration.moc"

namespace DigikamRestorationImagesPlugin
{

ImageEffect_Restoration::ImageEffect_Restoration(QWidget* parent, QString title, QFrame* banner)
                       : Digikam::CtrlPanelDlg(parent, title, "restoration", true, true, true,
                                               Digikam::ImagePannelWidget::SeparateViewAll, banner)
{
    QString whatsThis;

    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Photograph Restoration"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to restore a photograph."),
                                       KAboutData::License_GPL,
                                       "(c) 2005-2007, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");
                     
    about->addAuthor("David Tschumperle", I18N_NOOP("CImg library"), 0,
                     "http://cimg.sourceforge.net");
                        
    about->addAuthor("Gerhard Kulzer", I18N_NOOP("Feedback and plugin polishing"), 
                     "gerhard at kulzer.net");
    
    setAboutData(about);
    
    // -------------------------------------------------------------

    m_mainTab = new QTabWidget( m_imagePreviewWidget );
    
    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid = new QGridLayout( firstPage, 2, 2, marginHint(), spacingHint());
    m_mainTab->addTab( firstPage, i18n("Preset") );

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("cimg-logo", KGlobal::dirs()->kde_default("data") + "digikamimageplugins/data");
    QString directory = KGlobal::dirs()->findResourceDir("cimg-logo", "cimg-logo.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "cimg-logo.png" ) );
    QToolTip::add(cimgLogoLabel, i18n("Visit CImg library website"));
    
    QLabel *typeLabel = new QLabel(i18n("Filtering type:"), firstPage);
    m_restorationTypeCB = new QComboBox( false, firstPage ); 
    m_restorationTypeCB->insertItem( i18n("None") );
    m_restorationTypeCB->insertItem( i18n("Reduce Uniform Noise") );
    m_restorationTypeCB->insertItem( i18n("Reduce JPEG Artefacts") );
    m_restorationTypeCB->insertItem( i18n("Reduce Texturing") );
    QWhatsThis::add( m_restorationTypeCB, i18n("<p>Select here the filter preset to use for photograph restoration:<p>"
                                               "<b>None</b>: Most common values. Puts settings to default.<p>"
                                               "<b>Reduce Uniform Noise</b>: reduce small image artifacts like sensor noise.<p>"
                                               "<b>Reduce JPEG Artefacts</b>: reduce large image artifacts like JPEG compression mosaic.<p>"
                                               "<b>Reduce Texturing</b>: reduce image artifacts like paper texture or Moire patterns "
                                               "of a scanned image.<p>"));

    grid->addMultiCellWidget(cimgLogoLabel, 0, 0, 1, 1);
    grid->addMultiCellWidget(typeLabel, 1, 1, 0, 0);
    grid->addMultiCellWidget(m_restorationTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);
    
    // -------------------------------------------------------------
    
    m_settingsWidget = new DigikamImagePlugins::GreycstorationWidget( m_mainTab );
    m_imagePreviewWidget->setUserAreaWidget(m_mainTab);
    
    // -------------------------------------------------------------
    
    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));
}

ImageEffect_Restoration::~ImageEffect_Restoration()
{
}

void ImageEffect_Restoration::renderingFinished()
{
    m_imagePreviewWidget->setEnable(true);                 
    m_mainTab->setEnabled(true);
}

void ImageEffect_Restoration::resetValues()
{
    DigikamImagePlugins::GreycstorationSettings settings;
    settings.setRestorationDefaultSettings();    

    switch(m_restorationTypeCB->currentItem())
    {
        case ReduceUniformNoise:
        {
        // TODO
        //    settings.m_timeStepInput->setValue(40.0);
            break;
        }
        
        case ReduceJPEGArtefacts:
        {
        // TODO
          /*  m_detailInput->setValue(0.3);
            m_blurInput->setValue(1.0);
            m_timeStepInput->setValue(100.0);
            m_blurItInput->setValue(2.0);*/
            break;
        }
        
        case ReduceTexturing:
        {
        // TODO
/*            m_detailInput->setValue(0.5);
            m_blurInput->setValue(1.5);
            m_timeStepInput->setValue(100.0);
            m_blurItInput->setValue(2.0);*/
            break;
        }
    }
                    
    m_settingsWidget->setSettings(settings);
} 

void ImageEffect_Restoration::processCImgURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void ImageEffect_Restoration::prepareEffect()
{
    m_restorationTypeCB->setEnabled(false);
    m_mainTab->setEnabled(false);
    
    Digikam::DImg previewImage = m_imagePreviewWidget->getOriginalRegionImage();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new DigikamImagePlugins::GreycstorationIface(
                                    &previewImage, m_settingsWidget->getSettings(),
                                    DigikamImagePlugins::GreycstorationIface::Restore, 
                                    0, 0, 0, this));
}

void ImageEffect_Restoration::prepareFinal()
{
    m_restorationTypeCB->setEnabled(false);
    m_mainTab->setEnabled(false);
    
    Digikam::ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    Digikam::DImg originalImage(iface.originalWidth(), iface.originalHeight(),
                                iface.originalSixteenBit(), iface.originalHasAlpha(), data);
   
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new DigikamImagePlugins::GreycstorationIface(
                                    &originalImage, m_settingsWidget->getSettings(),
                                    DigikamImagePlugins::GreycstorationIface::Restore, 
                                    0, 0, 0, this));

    delete [] data;                                    
}

void ImageEffect_Restoration::putPreviewData(void)
{
    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImage(imDest);
}

void ImageEffect_Restoration::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Restoration"),
                           m_threadedFilter->getTargetImage().bits());
}

void ImageEffect_Restoration::slotUser3()
{
    KURL loadRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Restoration Settings File to Load")) );
    if( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.path());
    
    if ( file.open(IO_ReadOnly) )   
    {
        QTextStream stream( &file );
        
        if ( stream.readLine() != "# Photograph Restoration Configuration File V2" )
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Restoration settings text file.")
                        .arg(loadRestorationFile.fileName()));
           file.close();            
           return;
        }
        
        blockSignals(true);

        DigikamImagePlugins::GreycstorationSettings settings;
        settings.fastApprox = stream.readLine().toInt();
        settings.interp     = stream.readLine().toInt();
        settings.amplitude  = stream.readLine().toDouble();
        settings.sharpness  = stream.readLine().toDouble();
        settings.anisotropy = stream.readLine().toDouble();
        settings.alpha      = stream.readLine().toDouble();
        settings.sigma      = stream.readLine().toDouble();
        settings.gaussPrec  = stream.readLine().toDouble();
        settings.dl         = stream.readLine().toDouble();
        settings.da         = stream.readLine().toDouble();
        settings.nbIter     = stream.readLine().toInt();
        settings.tile       = stream.readLine().toInt();
        settings.btile      = stream.readLine().toInt();   
        m_settingsWidget->setSettings(settings);

        blockSignals(false);
        slotEffect();  
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Restoration text file."));

    file.close();             
}

void ImageEffect_Restoration::slotUser2()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Restoration Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());
    
    if ( file.open(IO_WriteOnly) )   
    {
        DigikamImagePlugins::GreycstorationSettings settings = m_settingsWidget->getSettings();
        QTextStream stream( &file );        
        stream << "# Photograph Restoration Configuration File V2\n";    
        stream << settings.fastApprox << "\n";    
        stream << settings.interp << "\n";    
        stream << settings.amplitude << "\n";    
        stream << settings.sharpness << "\n";    
        stream << settings.anisotropy << "\n";    
        stream << settings.alpha << "\n";    
        stream << settings.sigma << "\n";    
        stream << settings.gaussPrec << "\n";    
        stream << settings.dl << "\n";    
        stream << settings.da << "\n";    
        stream << settings.nbIter << "\n";    
        stream << settings.tile << "\n";    
        stream << settings.btile << "\n";    
    }
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Restoration text file."));
    
    file.close();        
}

}  // NameSpace DigikamRestorationImagesPlugin

