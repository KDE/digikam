/* ============================================================
 * File  : imageeffect_solarize.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-14
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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
 
#include <qlayout.h>
#include <qframe.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

// KDE includes.

#include <knuminput.h>
#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>

// Digikam includes.

#include <digikam/imageiface.h>
#include <digikam/imagewidget.h>

// Local includes.

#include "version.h"
#include "imageeffect_solarize.h"


namespace DigikamSolarizeImagesPlugin
{

ImageEffect_Solarize::ImageEffect_Solarize(QWidget* parent)
                    : KDialogBase(Plain, i18n("Solarize Image"),
                                  Help|Ok|Cancel, Ok,
                                  parent, 0, true, true),
                      m_parent(parent)
{
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Solarize Image"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A solarize image plugin for Digikam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Renchi Raju", 
                                       0,
                                       "http://digikam.sourceforge.net");
    
    about->addAuthor("Renchi Raju", I18N_NOOP("Author and maintainer"),
                     "renchi@pooh.tam.uiuc.edu");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Solarize Image handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------
        
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint());

    QVGroupBox *gbox = new QVGroupBox(i18n("Solarize Image"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320,frame);
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);

    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Intensity:"), plainPage());
    m_numInput = new KDoubleNumInput(plainPage());
    m_numInput->setPrecision(1);
    m_numInput->setRange(0.0, 100.0, 0.1, true);
    hlay->addWidget(label,1);
    hlay->addWidget(m_numInput,5);

    // -------------------------------------------------------------
        
    connect(m_numInput, SIGNAL(valueChanged (double)),
            SLOT(slotEffect()));

    adjustSize();
}

ImageEffect_Solarize::~ImageEffect_Solarize()
{
    
}

void ImageEffect_Solarize::slotHelp()
{
    KApplication::kApplication()->invokeHelp("solarizeimage",
                                             "digikamimageplugins");
}

void ImageEffect_Solarize::closeEvent(QCloseEvent *e)
{
    delete m_numInput;
    delete m_previewWidget;
    
    e->accept();    
}

void ImageEffect_Solarize::slotEffect()
{
    Digikam::ImageIface* iface =
        m_previewWidget->imageIface();

    uint * data = iface->getPreviewData();
    int w       = iface->previewWidth();
    int h       = iface->previewHeight();

    double factor = m_numInput->value();

    solarize(factor, data, w, h);

    iface->putPreviewData(data);

    delete [] data;

    m_previewWidget->update();
}

void ImageEffect_Solarize::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface =
        m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();

    if (data) {

        double factor = m_numInput->value();
    
        solarize(factor, data, w, h);

        iface->putOriginalData(data);
        
        delete [] data;
    }
    
    m_parent->setCursor( KCursor::arrowCursor() );        
    accept();
}

void ImageEffect_Solarize::solarize(double factor, uint *data, int w, int h)
{
    uint *ptr  = data;
    uint a,r,g,b;
    
    uint threshold = (uint)((100-factor)*(255+1)/100);
    threshold = QMAX(1,threshold);
    bool stretch = true;
    
    for (int x=0; x<w*h; x++) {

        a = (*ptr >> 24) & 0xff;
        r = (*ptr >> 16) & 0xff;
        g = (*ptr >> 8 ) & 0xff;
        b = (*ptr      ) & 0xff;


        if (stretch) {

            r = (r > threshold) ? (255-r)*255/(255-threshold) : r*255/threshold;
            g = (g > threshold) ? (255-g)*255/(255-threshold) : g*255/threshold;
            b = (b > threshold) ? (255-b)*255/(255-threshold) : b*255/threshold;

        }
        else {
            
            if (r > threshold)
                r = (255-r);
            if (g > threshold)
                g = (255-g);
            if (b > threshold)
                b = (255-b);
        }

        *ptr = a << 24 | r << 16 | g << 8 | b; 
        ptr++;
    }
}

}  // NameSpace DigikamSolarizeImagesPlugin

#include "imageeffect_solarize.moc"
