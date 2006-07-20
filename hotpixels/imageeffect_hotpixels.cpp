/* ============================================================
* File  : imageeffect_hotpixels.cpp
* Author: Unai Garro <ugarro at users dot sourceforge dot net>
*         Gilles Caulier <caulier dot gilles at free dot fr>
* Date  : 2005-03-27
* Description : a digiKam image plugin for fixing dots produced by
*               hot/stuck/dead pixels from a CCD.
*
* Copyright 2005 by Unai Garro and Gilles Caulier
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

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qpointarray.h>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kimageio.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_hotpixels.h"
#include "blackframelistview.h"

namespace DigikamHotPixelsImagesPlugin
{

ImageEffect_HotPixels::ImageEffect_HotPixels(QWidget* parent)
                     : CtrlPanelDialog(parent, i18n("Hot Pixels Correction"), "hotpixels", 
                                       false, false, false, 
                                       Digikam::ImagePannelWidget::SeparateViewDuplicate)
{
    // No need Abort button action.
    showButton(User1, false); 
    
    QString whatsThis;

    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Hot Pixels Correction"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin for fixing dots produced by "
                                                 "hot/stuck/dead pixels from a CCD."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Unai Garro", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                
    about->addAuthor("Unai Garro", I18N_NOOP("Author and maintainer"),
                     "ugarro at sourceforge dot net");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Developer"),
                     "caulier dot gilles at free.fr");
        
    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 2, 2, marginHint(), spacingHint());
    
    QLabel *filterMethodLabel = new QLabel(i18n("Filter:"), gboxSettings);
    m_filterMethodCombo = new QComboBox(gboxSettings);
    m_filterMethodCombo->insertItem(i18n("Average"));
    m_filterMethodCombo->insertItem(i18n("Linear"));
    m_filterMethodCombo->insertItem(i18n("Quadratic"));
    m_filterMethodCombo->insertItem(i18n("Cubic"));

    m_blackFrameButton = new QPushButton(i18n("Black Frame..."), gboxSettings);    
    setButtonWhatsThis( Apply, i18n("<p>Use this button to add a new black frame file which will "
                                    "be used by the hot pixels removal filter.") );  

    gridSettings->addMultiCellWidget(filterMethodLabel, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_filterMethodCombo, 0, 0, 1, 1);
    gridSettings->addMultiCellWidget(m_blackFrameButton, 0, 0, 2, 2);    
    
    m_blackFrameListView = new BlackFrameListView(gboxSettings);
    gridSettings->addMultiCellWidget(m_blackFrameListView, 1, 2, 0, 2);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);

    readSettings();
            
    // -------------------------------------------------------------
    
    connect(m_filterMethodCombo, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    connect(m_blackFrameButton, SIGNAL(clicked()),
            this, SLOT(slotAddBlackFrame()));
                                                  
    connect(m_blackFrameListView, SIGNAL(blackFrameSelected(QValueList<HotPixel>, const KURL&)),
            this, SLOT(slotBlackFrame(QValueList<HotPixel>, const KURL&))); 
}

ImageEffect_HotPixels::~ImageEffect_HotPixels()
{
    writeSettings();
}

void ImageEffect_HotPixels::readSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Hot Pixels Tool Settings");
    m_blackFrameURL = KURL::KURL( config->readEntry("Last Black Frame File", QString::null) );
    m_filterMethodCombo->setCurrentItem( config->readNumEntry("Filter Method",
                                         HotPixelFixer::QUADRATIC_INTERPOLATION) );
    
    if (m_blackFrameURL.isValid())
        new BlackFrameListViewItem(m_blackFrameListView, m_blackFrameURL);
}

void ImageEffect_HotPixels::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Hot Pixels Tool Settings");
    config->writeEntry( "Last Black Frame File", m_blackFrameURL.url() );
    config->writeEntry( "Filter Method", m_filterMethodCombo->currentItem() );
    config->sync();
}

void ImageEffect_HotPixels::slotAddBlackFrame()
{
    //Does one need to do this if digikam did so already?
    KImageIO::registerFormats(); 
    
    KFileDialog *fileSelectDialog = new KFileDialog(QString::null, KImageIO::pattern(), this, "", true);
    fileSelectDialog->setCaption(i18n("Select Black Frame Image"));
    fileSelectDialog->setURL(m_blackFrameURL.path());
    
    if (fileSelectDialog->exec() != QDialog::Rejected)
       {
       //Load the selected file and insert into the list
        
       m_blackFrameURL = fileSelectDialog->selectedURL();
       m_blackFrameListView->clear();
       new BlackFrameListViewItem(m_blackFrameListView, m_blackFrameURL);
       }
        
    delete fileSelectDialog;
}

void ImageEffect_HotPixels::renderingFinished(void)
{
    m_filterMethodCombo->setEnabled(true);
    m_blackFrameListView->setEnabled(true);
    enableButton(Apply, true);     
}

void ImageEffect_HotPixels::resetValues(void)
{
    m_filterMethodCombo->blockSignals(true);
    m_filterMethodCombo->setCurrentItem(HotPixelFixer::QUADRATIC_INTERPOLATION);
    m_filterMethodCombo->blockSignals(false);
}  

void ImageEffect_HotPixels::prepareEffect()
{
    m_filterMethodCombo->setEnabled(false);
    m_blackFrameListView->setEnabled(false);
    enableButton(Apply, false);     

    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    
    int interpolationMethod = m_filterMethodCombo->currentItem();

    QValueList<HotPixel> hotPixelsRegion;
    QRect area = m_imagePreviewWidget->getOriginalImageRegionToRender();
    QValueList<HotPixel>::Iterator end(m_hotPixelsList.end()); 
    for (QValueList<HotPixel>::Iterator it = m_hotPixelsList.begin() ; it != end ; ++it )
        {
        HotPixel hp = (*it);
        
        if ( area.contains( hp.rect ) )
           {
           hp.rect.moveTopLeft(QPoint::QPoint( hp.rect.x()-area.x(), hp.rect.y()-area.y() ));
           hotPixelsRegion.append(hp);
           }
        }

    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new HotPixelFixer(
                       &image, this, hotPixelsRegion, interpolationMethod));
}

void ImageEffect_HotPixels::prepareFinal()
{
    m_filterMethodCombo->setEnabled(false);
    m_blackFrameListView->setEnabled(false);
    enableButton(Apply, false);     
    
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    int interpolationMethod = m_filterMethodCombo->currentItem();

    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new HotPixelFixer(
                       &orgImage, this, m_hotPixelsList, interpolationMethod));
    delete [] data;
}

void ImageEffect_HotPixels::putPreviewData(void)
{
    QImage imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImageData(imDest);
}

void ImageEffect_HotPixels::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalData(i18n("Hot Pixels Correction"), 
                          (uint*)m_threadedFilter->getTargetImage().bits());
}

void ImageEffect_HotPixels::slotBlackFrame(QValueList<HotPixel> hpList, const KURL& blackFrameURL)
{
    m_blackFrameURL = blackFrameURL;
    m_hotPixelsList = hpList;
    
    QPointArray pointList(m_hotPixelsList.size());
    QValueList <HotPixel>::Iterator it;
    int i = 0;
    QValueList <HotPixel>::Iterator end(m_hotPixelsList.end());
    for (it = m_hotPixelsList.begin() ; it != end ; ++it, i++)
       pointList.setPoint(i, (*it).rect.center());
        
    m_imagePreviewWidget->setPanIconHighLightPoints(pointList);
    
    slotEffect();
}

}  // NameSpace DigikamHotPixelsImagesPlugin

#include "imageeffect_hotpixels.moc"
