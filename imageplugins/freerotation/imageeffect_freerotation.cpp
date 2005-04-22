/* ============================================================
 * File  : imageeffect_freerotation.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-28
 * Description : a Digikam image editor plugin for process image 
 *               free rotation.
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

// C++ includes.

#include <cmath>
#include <cstdio>
#include <cstdlib>
 
// Qt includes. 
 
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qwmatrix.h> 

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <knuminput.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_freerotation.h"

namespace DigikamFreeRotationImagesPlugin
{

ImageEffect_FreeRotation::ImageEffect_FreeRotation(QWidget* parent)
                        : KDialogBase(Plain, i18n("Free Rotation"),
                                      Help|User1|Ok|Cancel, Ok,
                                      parent, 0, true, true,
                                      i18n("&Reset Values")),
                          m_parent(parent)
{
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all parameters to the default values.") );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Free Rotation"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to process image free "
                                       "rotation."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Free Rotation Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QGridLayout* topLayout = new QGridLayout( plainPage(), 4, 4 , marginHint(), spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Free Rotation"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 4);

    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------
    
    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageGuideWidget(480, 320, frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the free rotation operation preview."
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be drawn "
                                           "to guide you in adjusting the rotation angle. "
                                           "Press the left mouse button to freeze the dashed "
                                           "line's position."));
                                           
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 4);
    
    // -------------------------------------------------------------
    
    QLabel *label1 = new QLabel(i18n("New width:"), plainPage());
    m_newWidthLabel = new QLabel(plainPage());
    QLabel *label2 = new QLabel(i18n("New height:"), plainPage());
    m_newHeightLabel = new QLabel(plainPage());
    topLayout->addMultiCellWidget(label1, 2, 2, 0, 0);
    topLayout->addMultiCellWidget(m_newWidthLabel, 2, 2, 1, 1);
    topLayout->addMultiCellWidget(label2, 2, 2, 3, 3);
    topLayout->addMultiCellWidget(m_newHeightLabel, 2, 2, 4, 4);
        
    QLabel *label3 = new QLabel(i18n("Angle:"), plainPage());
    m_angleInput = new KDoubleNumInput(plainPage());
    m_angleInput->setPrecision(1);
    m_angleInput->setRange(-180.0, 180.0, 0.1, true);
    m_angleInput->setValue(0.0);
    QWhatsThis::add( m_angleInput, i18n("<p>An angle in degrees by which to rotate the image. "
                                        "A positive angle rotates the image clockwise; "
                                        "a negative angle rotates it counter-clockwise."));
        
    topLayout->addMultiCellWidget(label3, 3, 3, 0, 0);
    topLayout->addMultiCellWidget(m_angleInput, 3, 3, 1, 4);
        
    adjustSize();
    disableResize();

    // -------------------------------------------------------------
    
    connect(m_angleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));
}

ImageEffect_FreeRotation::~ImageEffect_FreeRotation()
{
}

void ImageEffect_FreeRotation::slotUser1()
{
    blockSignals(true);
                   
    m_angleInput->setValue(0.0);
    
    blockSignals(false);
    slotEffect();  
} 

void ImageEffect_FreeRotation::slotHelp()
{
    KApplication::kApplication()->invokeHelp("freerotation",
                                             "digikamimageplugins");
}

void ImageEffect_FreeRotation::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint*  data  = iface->getPreviewData();
    int    w     = iface->previewWidth();
    int    h     = iface->previewHeight();
    double angle = m_angleInput->value();
    
    QImage src, dest;
    QWMatrix matrix;
    src.create( w, h, 32 );
    memcpy(src.bits(), data, src.numBytes());
    matrix.rotate(angle);
    src = src.xForm(matrix);
    QSize newSize = matrix.mapRect(QRect::QRect(0, 0, iface->originalWidth(), iface->originalHeight())).size();
    src = src.smoothScale(w, h, QImage::ScaleMin);
    dest.create( w, h, 32 );
    dest.fill(m_previewWidget->colorGroup().background().rgb());
    bitBlt( &dest, (w-src.width())/2, (h-src.height())/2, &src, 0, 0, src.width(), src.height());
    iface->putPreviewData((uint*)dest.bits());

    delete [] data;
    m_previewWidget->update();
    
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void ImageEffect_FreeRotation::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint*  data  = iface.getOriginalData();
    int    w     = iface.originalWidth();
    int    h     = iface.originalHeight();
    double angle = m_angleInput->value();

    QImage src;
    QWMatrix matrix;
    src.create( w, h, 32 );
    memcpy(src.bits(), data, src.numBytes());
    matrix.rotate(angle);
    src = src.xForm(matrix);
    Digikam::ImageFilters::smartBlurImage((uint*)src.bits(), src.width(), src.height());
    iface.putOriginalData(i18n("Free Rotation"), (uint*)src.bits(), src.width(), src.height());
        
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

}  // NameSpace DigikamFreeRotationImagesPlugin

#include "imageeffect_freerotation.moc"
