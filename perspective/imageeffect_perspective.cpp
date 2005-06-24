/* ============================================================
 * File  : imageeffect_perspective.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-17
 * Description : a Digikam image editor plugin for process image 
 *               perspective adjustment.
 * 
 * Copyright 2005 by Gilles Caulier
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

#include <cstdio>
#include <cstdlib>
 
// Qt includes. 
 
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>

// KDE includes.

#include <kdebug.h>
#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "perspectivewidget.h"
#include "imageeffect_perspective.h"


namespace DigikamPerspectiveImagesPlugin
{

ImageEffect_Perspective::ImageEffect_Perspective(QWidget* parent)
                       : KDialogBase(Plain, i18n("Perspective Adjustement"),
                                     Help|User1|Ok|Cancel, Ok,
                                     parent, 0, true, true, i18n("&Reset Values")),
                         m_parent(parent)
{
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all parameters to the default values.") );        
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Perspective"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to process image perspective adjustment."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Perspective Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QGridLayout* topLayout = new QGridLayout( plainPage(), 4, 5 , marginHint(), spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Perpective Adjustment"), headerFrame, "labelTitle" );
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
    m_previewWidget = new PerspectiveWidget(525, 350, frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the perspective transformation operation preview. "
                                           "You can use the mouse for dragging the corner to adjust the "
                                           "perspective transformation area."));
                                           
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 4);
    
    QLabel *label1 = new QLabel(i18n("New width:"), plainPage());
    m_newWidthLabel = new QLabel(plainPage());
    QLabel *label2 = new QLabel(i18n("New height:"), plainPage());
    m_newHeightLabel = new QLabel(plainPage());
    QLabel *label3 = new QLabel(i18n("Top left angle:"), plainPage());
    m_topLeftAngleLabel = new QLabel(plainPage());
    QLabel *label4 = new QLabel(i18n("Top right angle:"), plainPage());
    m_topRightAngleLabel = new QLabel(plainPage());
    QLabel *label5 = new QLabel(i18n("Bottom left angle:"), plainPage());
    m_bottomLeftAngleLabel = new QLabel(plainPage());
    QLabel *label6 = new QLabel(i18n("Bottom right angle:"), plainPage());
    m_bottomRightAngleLabel = new QLabel(plainPage());
    
    topLayout->addMultiCellWidget(label1, 2, 2, 0, 0);
    topLayout->addMultiCellWidget(m_newWidthLabel, 2, 2, 1, 1);
    topLayout->addMultiCellWidget(label2, 2, 2, 3, 3);
    topLayout->addMultiCellWidget(m_newHeightLabel, 2, 2, 4, 4);
    topLayout->addMultiCellWidget(label3, 3, 3, 0, 0);
    topLayout->addMultiCellWidget(m_topLeftAngleLabel, 3, 3, 1, 1);
    topLayout->addMultiCellWidget(label4, 3, 3, 3, 3);
    topLayout->addMultiCellWidget(m_topRightAngleLabel, 3, 3, 4, 4);
    topLayout->addMultiCellWidget(label5, 4, 4, 0, 0);
    topLayout->addMultiCellWidget(m_bottomLeftAngleLabel, 4, 4, 1, 1);
    topLayout->addMultiCellWidget(label6, 4, 4, 3, 3);
    topLayout->addMultiCellWidget(m_bottomRightAngleLabel, 4, 4, 4, 4);
    
    // -------------------------------------------------------------
    
    connect(m_previewWidget, SIGNAL(signalPerspectiveChanged(QRect, float, float, float, float)),
            this, SLOT(slotUpdateInfo(QRect, float, float, float, float)));   
            
    adjustSize();
    disableResize();
}

ImageEffect_Perspective::~ImageEffect_Perspective()
{
}

void ImageEffect_Perspective::slotHelp()
{
    KApplication::kApplication()->invokeHelp("perspective",
                                             "digikamimageplugins");
}

void ImageEffect_Perspective::slotUser1()
{
    m_previewWidget->resetSelection();
} 

void ImageEffect_Perspective::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    m_previewWidget->applyPerspectiveAdjusment();
    accept();   
    m_parent->setCursor( KCursor::arrowCursor() );           
}

void ImageEffect_Perspective::slotUpdateInfo(QRect newSize, float topLeftAngle, float topRightAngle,
                                             float bottomLeftAngle, float bottomRightAngle)
{
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
    
    m_topLeftAngleLabel->setText(temp.setNum( topLeftAngle, 'f', 1 ) + "" );
    m_topRightAngleLabel->setText(temp.setNum( topRightAngle, 'f', 1 ) + "" );
    m_bottomLeftAngleLabel->setText(temp.setNum( bottomLeftAngle, 'f', 1 ) + "" );
    m_bottomRightAngleLabel->setText(temp.setNum( bottomRightAngle, 'f', 1 ) + "" );
}

}  // NameSpace DigikamPerspectiveImagesPlugin

#include "imageeffect_perspective.moc"
