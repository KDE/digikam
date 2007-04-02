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
#include "ddebug.h"
#include "imageiface.h"
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "greycstorationiface.h"
#include "imageeffect_restoration.h"
#include "imageeffect_restoration.moc"

namespace DigikamRestorationImagesPlugin
{

ImageEffect_Restoration::ImageEffect_Restoration(QWidget* parent)
                       : Digikam::CtrlPanelDlg(parent, i18n("Photograph Restoration"), 
                                               "restoration", true, true, true,
                                               Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;

    // About data and help button.
    
    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("Photograph Restoration"), 
                                       digikam_version,
                                       I18N_NOOP("A digiKam image plugin to restore a photograph."),
                                       KAboutData::License_GPL,
                                       "(c) 2005-2007, Gilles Caulier", 
                                       0,
                                       "http://www.digikam.org");
    
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
    QGridLayout* grid  = new QGridLayout( firstPage, 2, 2, spacingHint());
    m_mainTab->addTab( firstPage, i18n("Preset") );

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("logo-cimg", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("logo-cimg", "logo-cimg.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "logo-cimg.png" ) );
    QToolTip::add(cimgLogoLabel, i18n("Visit CImg library website"));
    
    QLabel *typeLabel   = new QLabel(i18n("Filtering type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
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
    
    m_settingsWidget = new Digikam::GreycstorationWidget( m_mainTab );
    m_imagePreviewWidget->setUserAreaWidget(m_mainTab);
    
    // -------------------------------------------------------------
    
    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));

    connect(m_restorationTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));
}

ImageEffect_Restoration::~ImageEffect_Restoration()
{
}

void ImageEffect_Restoration::renderingFinished()
{
    m_imagePreviewWidget->setEnable(true);                 
    m_mainTab->setEnabled(true);
}

void ImageEffect_Restoration::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("restoration Tool Dialog");

    Digikam::GreycstorationSettings settings;
    settings.fastApprox = config->readBoolEntry("FastApprox", true);
    settings.interp     = config->readNumEntry("Interpolation",
                          Digikam::GreycstorationSettings::NearestNeighbor);
    settings.amplitude  = config->readDoubleNumEntry("Amplitude", 60.0);
    settings.sharpness  = config->readDoubleNumEntry("Sharpness", 0.7);
    settings.anisotropy = config->readDoubleNumEntry("Anisotropy", 0.3);
    settings.alpha      = config->readDoubleNumEntry("Alpha", 0.6);
    settings.sigma      = config->readDoubleNumEntry("Sigma", 1.1);
    settings.gaussPrec  = config->readDoubleNumEntry("GaussPrec", 2.0);
    settings.dl         = config->readDoubleNumEntry("Dl", 0.8);
    settings.da         = config->readDoubleNumEntry("Da", 30.0);
    settings.nbIter     = config->readNumEntry("Iteration", 1);
    settings.tile       = config->readNumEntry("Tile", 512);
    settings.btile      = config->readNumEntry("BTile", 4);
    m_settingsWidget->setSettings(settings);

    int p = config->readNumEntry("Preset", NoPreset);
    m_restorationTypeCB->setCurrentItem(p);
    if (p == NoPreset)
        m_settingsWidget->setEnabled(true);
    else        
        m_settingsWidget->setEnabled(false);
}

void ImageEffect_Restoration::writeUserSettings()
{
    Digikam::GreycstorationSettings settings = m_settingsWidget->getSettings();
    KConfig* config = kapp->config();
    config->setGroup("restoration Tool Dialog");
    config->writeEntry("Preset", m_restorationTypeCB->currentItem());
    config->writeEntry("FastApprox", settings.fastApprox);
    config->writeEntry("Interpolation", settings.interp);
    config->writeEntry("Amplitude", settings.amplitude);
    config->writeEntry("Sharpness", settings.sharpness);
    config->writeEntry("Anisotropy", settings.anisotropy);
    config->writeEntry("Alpha", settings.alpha);
    config->writeEntry("Sigma", settings.sigma);
    config->writeEntry("GaussPrec", settings.gaussPrec);
    config->writeEntry("Dl", settings.dl);
    config->writeEntry("Da", settings.da);
    config->writeEntry("Iteration", settings.nbIter);
    config->writeEntry("Tile", settings.tile);
    config->writeEntry("BTile", settings.btile);
    config->sync();
}

void ImageEffect_Restoration::slotResetValues(int i)
{
    if (i == NoPreset)
        m_settingsWidget->setEnabled(true);
    else        
        m_settingsWidget->setEnabled(false);

    resetValues();
}

void ImageEffect_Restoration::resetValues()
{
    Digikam::GreycstorationSettings settings;
    settings.setRestorationDefaultSettings();    

    switch(m_restorationTypeCB->currentItem())
    {
        case ReduceUniformNoise:
        {
            settings.amplitude = 40.0;
            break;
        }
        
        case ReduceJPEGArtefacts:
        {
            settings.sharpness = 0.3;
            settings.sigma     = 1.0;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }
        
        case ReduceTexturing:
        {
            settings.sharpness = 0.5;
            settings.sigma     = 1.5;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
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
    m_mainTab->setEnabled(false);
    
    Digikam::DImg previewImage = m_imagePreviewWidget->getOriginalRegionImage();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new Digikam::GreycstorationIface(
                                    &previewImage, m_settingsWidget->getSettings(),
                                    Digikam::GreycstorationIface::Restore, 
                                    0, 0, 0, this));
}

void ImageEffect_Restoration::prepareFinal()
{
    m_mainTab->setEnabled(false);
    
    Digikam::ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    Digikam::DImg originalImage(iface.originalWidth(), iface.originalHeight(),
                                iface.originalSixteenBit(), iface.originalHasAlpha(), data);
   
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new Digikam::GreycstorationIface(
                                    &originalImage, m_settingsWidget->getSettings(),
                                    Digikam::GreycstorationIface::Restore, 
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
        if (!m_settingsWidget->loadSettings(file, QString("# Photograph Restoration Configuration File V2")))
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Restoration settings text file.")
                        .arg(loadRestorationFile.fileName()));
           file.close();            
           return;
        }
        
        slotEffect();  
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Restoration text file."));

    file.close();
    m_restorationTypeCB->blockSignals(true);
    m_restorationTypeCB->setCurrentItem(NoPreset);
    m_restorationTypeCB->blockSignals(false);
    m_settingsWidget->setEnabled(true);
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
        m_settingsWidget->saveSettings(file, QString("# Photograph Restoration Configuration File V2"));
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Restoration text file."));
    
    file.close();        
}

}  // NameSpace DigikamRestorationImagesPlugin

