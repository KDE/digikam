/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-06
 * Description : Black and White conversion tool.
 * 
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
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
 
#include <qframe.h>
#include <qvgroupbox.h>
#include <qcombobox.h> 
#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtimer.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Digikam includes.

#include <imageiface.h>
#include <imagefilters.h>
#include <imagewidget.h>

// Local includes.

#include "imageeffect_bwsepia.h"


ImageEffect_BWSepia::ImageEffect_BWSepia(QWidget* parent)
                   : KDialogBase(Plain, i18n("Convert to Black & White"),
                                 Help|Ok|Cancel, Ok,
                                 parent, 0, true, true),
                     m_parent(parent)
{
    setHelp("blackandwhitetool.anchor", "digikam");

    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320,frame);
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);

    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Type:"), plainPage());
    m_typeCB = new QComboBox( false, plainPage() );
    m_typeCB->insertItem( previewEffectPic("neutralbw"), i18n("Neutral Black & White") );
    m_typeCB->insertItem( previewEffectPic("bwgreen"),   i18n("Black & White with Green Filter") );
    m_typeCB->insertItem( previewEffectPic("bworange"),  i18n("Black & White with Orange Filter") );
    m_typeCB->insertItem( previewEffectPic("bwred"),     i18n("Black & White with Red Filter") );
    m_typeCB->insertItem( previewEffectPic("bwyellow"),  i18n("Black & White with Yellow Filter") );
    m_typeCB->insertItem( previewEffectPic("sepia"),     i18n("Black & White with Sepia Tone") );
    m_typeCB->insertItem( previewEffectPic("browntone"), i18n("Black & White with Brown Tone") );
    m_typeCB->insertItem( previewEffectPic("coldtone"),  i18n("Black & White with Cold Tone") );
    m_typeCB->insertItem( previewEffectPic("selenium"),  i18n("Black & White with Selenium Tone") );
    m_typeCB->insertItem( previewEffectPic("platinum"),  i18n("Black & White with Platinum Tone") );
    m_typeCB->setCurrentText( i18n("Neutral Black & White") );
    QWhatsThis::add( m_typeCB, i18n("<p>Select here the black and white conversion type:<p>"
                                    "<b>Neutral</b>: simulate black and white neutral film exposure.<p>"
                                    "<b>Green Filter</b>: simulate black and white film exposure using green filter. "
                                    "This provides an universal asset for all scenics, especially suited for portraits "
                                    "photographed against sky.<p>"
                                    "<b>Orange Filter</b>: simulate black and white film exposure using orange filter. "
                                    "This will enhances landscapes, marine scenes and aerial photography.<p>"
                                    "<b>Red Filter</b>: simulate black and white film exposure using red filter. "
                                    "Using this one creates dramatic sky effects and simulates moonlight scenes in daytime.<p>"
                                    "<b>Yellow Filter</b>: simulate black and white film exposure using yellow filter. "
                                    "Most natural tonal correction and improves contrast. Ideal for landscapes.<p>"
                                    "<b>Sepia Tone</b>: gives a warm highlight and mid-tone while adding a bit of coolness to the "
                                    "shadows-very similar to the process of bleaching a print and re-developing in a sepia toner.<p>"
                                    "<b>Brown Tone</b>: more neutral than Sepia Tone filter.<p>"
                                    "<b>Cold Tone</b>: start subtle and replicate printing on a cold tone black and white paper such "
                                    "as a bromide enlarging paper.<p>"
                                    "<b>Selenium Tone</b>: effect that replicate traditional selenium chemical toning done "
                                    "in the darkroom.<p>"
                                    "<b>Platinum Tone</b>: effect that replicate traditional platinum chemical toning done "
                                    "in the darkroom."
                                    ));

    hlay->addWidget(label, 1);
    hlay->addWidget(m_typeCB, 5);

    adjustSize();
    disableResize();    
    
    QTimer::singleShot(0, this, SLOT(slotEffect()));

    // -------------------------------------------------------------

    connect(m_typeCB, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));
}

ImageEffect_BWSepia::~ImageEffect_BWSepia()
{
}

void ImageEffect_BWSepia::closeEvent(QCloseEvent *e)
{
    delete m_previewWidget;

    e->accept();
}

QPixmap ImageEffect_BWSepia::previewEffectPic(QString name)
{
    KGlobal::dirs()->addResourceType(name.ascii(), KGlobal::dirs()->kde_default("data") + "digikam/data");
    return ( QPixmap::QPixmap(KGlobal::dirs()->findResourceDir(name.ascii(), name + ".png") + name + ".png") );
}

void ImageEffect_BWSepia::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint * data = iface->getPreviewData();
    int w       = iface->previewWidth();
    int h       = iface->previewHeight();

    int type = m_typeCB->currentItem();

    blackAndWhiteConversion(data, w, h, type);

    iface->putPreviewData(data);

    delete [] data;

    m_previewWidget->update();
}

void ImageEffect_BWSepia::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();

    if (data) 
       {
       int type = m_typeCB->currentItem();

       blackAndWhiteConversion(data, w, h, type);

       iface->putOriginalData(i18n("Black & White"), data);

       delete [] data;
       }

    m_parent->setCursor( KCursor::arrowCursor() );
    accept();
}

// This method is based on the Convert to Black & White tutorial (channel mixer method) 
// from GimpGuru.org web site available at this url : http://www.gimpguru.org/Tutorials/Color2BW/

void ImageEffect_BWSepia::blackAndWhiteConversion(uint *data, int w, int h, int type)
{
    switch (type)
       {
       case BWNeutral:
          Digikam::ImageFilters::channelMixerImage(data, w, h,      // Image data.
                   true,                                            // Preserve luminosity.    
                   true,                                            // Monochrome.
                   0.3, 0.59 , 0.11,                                // Red channel gains.
                   0.0, 1.0,   0.0,                                 // Green channel gains (not used).
                   0.0, 0.0,   1.0);                                // Blue channel gains (not used).
          break;
       
       case BWGreenFilter:
          Digikam::ImageFilters::channelMixerImage(data, w, h,      // Image data.
                   true,                                            // Preserve luminosity.    
                   true,                                            // Monochrome.
                   0.1, 0.7, 0.2,                                   // Red channel gains.
                   0.0, 1.0, 0.0,                                   // Green channel gains (not used).
                   0.0, 0.0, 1.0);                                  // Blue channel gains (not used).
          break;
       
       case BWOrangeFilter:
          Digikam::ImageFilters::channelMixerImage(data, w, h,      // Image data.
                   true,                                            // Preserve luminosity.    
                   true,                                            // Monochrome.
                   0.78, 0.22, 0.0,                                 // Red channel gains.
                   0.0,  1.0,  0.0,                                 // Green channel gains (not used).
                   0.0,  0.0,  1.0);                                // Blue channel gains (not used).
          break;
       
       case BWRedFilter:
          Digikam::ImageFilters::channelMixerImage(data, w, h,      // Image data.
                   true,                                            // Preserve luminosity.    
                   true,                                            // Monochrome.
                   0.9, 0.1, 0.0,                                   // Red channel gains.
                   0.0, 1.0, 0.0,                                   // Green channel gains (not used).
                   0.0, 0.0, 1.0);                                  // Blue channel gains (not used).
          break;
       
       case BWYellowFilter:
          Digikam::ImageFilters::channelMixerImage(data, w, h,      // Image data.
                   true,                                            // Preserve luminosity.    
                   true,                                            // Monochrome.
                   0.6, 0.28, 0.12,                                 // Red channel gains.
                   0.0, 1.0,  0.0,                                  // Green channel gains (not used).
                   0.0, 0.0,  1.0);                                 // Blue channel gains (not used).
          break;
       
       case BWSepia:
          Digikam::ImageFilters::changeTonality(data, h, w, 162, 132, 101);
          break;
       
       case BWBrown:
          Digikam::ImageFilters::changeTonality(data, h, w, 129, 115, 104);
          break;
       
       case BWCold:
          Digikam::ImageFilters::changeTonality(data, h, w, 102, 109, 128);
          break;
       
       case BWSelenium:
          Digikam::ImageFilters::changeTonality(data, h, w, 122, 115, 122);
          break;
       
       case BWPlatinum:
          Digikam::ImageFilters::changeTonality(data, h, w, 115, 110, 106);
          break;
       }
}

#include "imageeffect_bwsepia.moc"

