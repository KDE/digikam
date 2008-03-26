/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : a digiKam image plugin for fixing dots produced by
 *               hot/stuck/dead pixels from a CCD.
 * 
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
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

#include <QPolygon>
#include <QProgressBar>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kimageio.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imagedialog.h"
#include "blackframelistview.h"
#include "imageeffect_hotpixels.h"
#include "imageeffect_hotpixels.moc"

namespace DigikamHotPixelsImagesPlugin
{

ImageEffect_HotPixels::ImageEffect_HotPixels(QWidget* parent)
                     : CtrlPanelDlg(parent, i18n("Hot Pixels Correction"), 
                                    "hotpixels", false, false, false, 
                                    Digikam::ImagePannelWidget::SeparateViewDuplicate)
{
    // No need Abort button action.
    showButton(User1, false); 
    
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Hot Pixels Correction"), 
                                       digikam_version,
                                       ki18n("A digiKam image plugin for fixing dots produced by "
                                             "hot/stuck/dead pixels from a CCD."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005-2006, Unai Garro\n(c) 2005-2008, Gilles Caulier"), 
                                       KLocalizedString(),
                                       "http://www.digikam.org");
                
    about->addAuthor(ki18n("Unai Garro"), ki18n("Author and maintainer"),
                     "ugarro at sourceforge dot net");
    
    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Developer"),
                     "caulier dot gilles at gmail dot com");
        
    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings );
    
    QLabel *filterMethodLabel = new QLabel(i18n("Filter:"), gboxSettings);
    m_filterMethodCombo       = new QComboBox(gboxSettings);
    m_filterMethodCombo->addItem(i18n("Average"));
    m_filterMethodCombo->addItem(i18n("Linear"));
    m_filterMethodCombo->addItem(i18n("Quadratic"));
    m_filterMethodCombo->addItem(i18n("Cubic"));

    m_blackFrameButton = new QPushButton(i18n("Black Frame..."), gboxSettings);    
    setButtonWhatsThis( Apply, i18n("<p>Use this button to add a new black frame file which will "
                                    "be used by the hot pixels removal filter.") );  

    m_blackFrameListView = new BlackFrameListView(gboxSettings);
    m_progressBar        = new QProgressBar(gboxSettings);
    m_progressBar->setRange(0, 100);
    m_progressBar->hide();

    // -------------------------------------------------------------

    gridSettings->addWidget(filterMethodLabel,    0, 0, 1, 1);
    gridSettings->addWidget(m_filterMethodCombo,  0, 1, 1, 1);
    gridSettings->addWidget(m_blackFrameButton,   0, 2, 1, 1);    
    gridSettings->addWidget(m_blackFrameListView, 1, 0, 2, 3);
    gridSettings->addWidget(m_progressBar,        3, 0, 1, 3);
    gridSettings->setMargin(0);
    gridSettings->setSpacing(spacingHint());
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------
    
    connect(m_filterMethodCombo, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    connect(m_blackFrameButton, SIGNAL(clicked()),
            this, SLOT(slotAddBlackFrame()));
                                                  
    connect(m_blackFrameListView, SIGNAL(blackFrameSelected(QList<HotPixel>, const KUrl&)),
            this, SLOT(slotBlackFrame(QList<HotPixel>, const KUrl&))); 
}

ImageEffect_HotPixels::~ImageEffect_HotPixels()
{
}

void ImageEffect_HotPixels::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hotpixels Tool Dialog");
    m_blackFrameURL = KUrl(group.readEntry("Last Black Frame File", QString()));
    m_filterMethodCombo->setCurrentIndex(group.readEntry("Filter Method",
                                         (int)HotPixelFixer::QUADRATIC_INTERPOLATION));
    
    if (m_blackFrameURL.isValid())
    {
        BlackFrameListViewItem *item = new BlackFrameListViewItem(m_blackFrameListView, m_blackFrameURL);

        connect(item, SIGNAL(signalLoadingProgress(float)),
                this, SLOT(slotLoadingProgress(float)));

        connect(item, SIGNAL(signalLoadingComplete()),
                this, SLOT(slotLoadingComplete()));
    }
}

void ImageEffect_HotPixels::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hotpixels Tool Dialog");
    group.writeEntry("Last Black Frame File", m_blackFrameURL.url());
    group.writeEntry("Filter Method", m_filterMethodCombo->currentIndex());
    group.sync();
}

void ImageEffect_HotPixels::resetValues()
{
    m_filterMethodCombo->blockSignals(true);
    m_filterMethodCombo->setCurrentIndex(HotPixelFixer::QUADRATIC_INTERPOLATION);
    m_filterMethodCombo->blockSignals(false);
} 

void ImageEffect_HotPixels::slotAddBlackFrame()
{
    KUrl url = Digikam::ImageDialog::getImageURL(this, m_blackFrameURL,
                                                 i18n("Select Black Frame Image"));

    if (!url.isEmpty())
    {
        // Load the selected file and insert into the list.

        m_blackFrameURL = url;
        m_blackFrameListView->clear();
        BlackFrameListViewItem *item = new BlackFrameListViewItem(m_blackFrameListView, m_blackFrameURL);

        connect(item, SIGNAL(signalLoadingProgress(float)),
                this, SLOT(slotLoadingProgress(float)));

        connect(item, SIGNAL(signalLoadingComplete()),
                this, SLOT(slotLoadingComplete()));
    }
}

void ImageEffect_HotPixels::slotLoadingProgress(float v)
{
    m_progressBar->show();
    m_progressBar->setValue((int)(v*100));
}

void ImageEffect_HotPixels::slotLoadingComplete()
{
    m_progressBar->hide();
}

void ImageEffect_HotPixels::renderingFinished()
{
    m_filterMethodCombo->setEnabled(true);
    m_blackFrameListView->setEnabled(true);
    enableButton(Apply, true);     
}

void ImageEffect_HotPixels::prepareEffect()
{
    m_filterMethodCombo->setEnabled(false);
    m_blackFrameListView->setEnabled(false);
    enableButton(Apply, false);     

    Digikam::DImg image     = m_imagePreviewWidget->getOriginalRegionImage();
    int interpolationMethod = m_filterMethodCombo->currentIndex();

    QList<HotPixel> hotPixelsRegion;
    QRect area = m_imagePreviewWidget->getOriginalImageRegionToRender();
    QList<HotPixel>::Iterator end(m_hotPixelsList.end()); 
    
    for (QList<HotPixel>::Iterator it = m_hotPixelsList.begin() ; it != end ; ++it )
    {
        HotPixel hp = (*it);
        
        if ( area.contains( hp.rect ) )
        {
           hp.rect.moveTopLeft(QPoint( hp.rect.x()-area.x(), hp.rect.y()-area.y() ));
           hotPixelsRegion.append(hp);
        }
    }

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new HotPixelFixer(&image, this, hotPixelsRegion, interpolationMethod));
}

void ImageEffect_HotPixels::prepareFinal()
{
    m_filterMethodCombo->setEnabled(false);
    m_blackFrameListView->setEnabled(false);
    enableButton(Apply, false);     
        
    int interpolationMethod = m_filterMethodCombo->currentIndex();

    Digikam::ImageIface iface(0, 0);
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new HotPixelFixer(iface.getOriginalImg(), this,m_hotPixelsList,interpolationMethod));
}

void ImageEffect_HotPixels::putPreviewData()
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_HotPixels::putFinalData()
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Hot Pixels Correction"), m_threadedFilter->getTargetImage().bits());
}

void ImageEffect_HotPixels::slotBlackFrame(QList<HotPixel> hpList, const KUrl& blackFrameURL)
{
    m_blackFrameURL = blackFrameURL;
    m_hotPixelsList = hpList;
    
    QPolygon pointList(m_hotPixelsList.size());
    QList <HotPixel>::Iterator it;
    int i = 0;
    QList <HotPixel>::Iterator end(m_hotPixelsList.end());
    
    for (it = m_hotPixelsList.begin() ; it != end ; ++it, i++)
       pointList.setPoint(i, (*it).rect.center());
        
    m_imagePreviewWidget->setPanIconHighLightPoints(pointList);
    
    slotEffect();
}

}  // NameSpace DigikamHotPixelsImagesPlugin
