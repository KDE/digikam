/* ============================================================
 * File  : imageeffect_perspective.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-17
 * Description : a digiKam image editor plugin for process image 
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

// Qt includes. 
 
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcheckbox.h>

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
#include <kseparator.h>

// Local includes.

#include "version.h"
#include "perspectivewidget.h"
#include "imageeffect_perspective.h"

namespace DigikamPerspectiveImagesPlugin
{

ImageEffect_Perspective::ImageEffect_Perspective(QWidget* parent, QString title, QFrame* banner)
                      : Digikam::ImageDlgBase(parent, title, "perspective", false, banner)
{
    QString whatsThis;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Perspective"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to process image perspective adjustment."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    setAboutData(about);    
    
    // -------------------------------------------------------------
    
    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new PerspectiveWidget(525, 350, frame);
    l->addWidget(m_previewWidget);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the perspective transformation operation preview. "
                                           "You can use the mouse for dragging the corner to adjust the "
                                           "perspective transformation area."));
    setPreviewAreaWidget(frame); 
    
    // -------------------------------------------------------------
    
    QWidget *gbox2 = new QWidget(plainPage());
    QGridLayout *gridLayout = new QGridLayout( gbox2, 8, 2, marginHint(), spacingHint());    
    QLabel *label1 = new QLabel(i18n("New width:"), gbox2);
    m_newWidthLabel = new QLabel(gbox2);
    QLabel *label2 = new QLabel(i18n("New height:"), gbox2);
    m_newHeightLabel = new QLabel(gbox2);
    
    gridLayout->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridLayout->addMultiCellWidget(m_newWidthLabel, 0, 0, 1, 2);
    gridLayout->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridLayout->addMultiCellWidget(m_newHeightLabel, 1, 1, 1, 2);
    
    // -------------------------------------------------------------
    
    KSeparator *line = new KSeparator (Horizontal, gbox2);
    
    QLabel *angleLabel = new QLabel(i18n("Angles (in degrees):"), gbox2);
    QLabel *label3 = new QLabel(i18n("  Top left:"), gbox2);
    m_topLeftAngleLabel = new QLabel(gbox2);
    QLabel *label4 = new QLabel(i18n("  Top right:"), gbox2);
    m_topRightAngleLabel = new QLabel(gbox2);
    QLabel *label5 = new QLabel(i18n("  Bottom left:"), gbox2);
    m_bottomLeftAngleLabel = new QLabel(gbox2);
    QLabel *label6 = new QLabel(i18n("  Bottom right:"), gbox2);
    m_bottomRightAngleLabel = new QLabel(gbox2);
    
    gridLayout->addMultiCellWidget(line, 2, 2, 0, 2);
    gridLayout->addMultiCellWidget(angleLabel, 3, 3, 0, 2);
    gridLayout->addMultiCellWidget(label3, 4, 4, 0, 0);
    gridLayout->addMultiCellWidget(m_topLeftAngleLabel, 4, 4, 1, 2);
    gridLayout->addMultiCellWidget(label4, 5, 5, 0, 0);
    gridLayout->addMultiCellWidget(m_topRightAngleLabel, 5, 5, 1, 2);
    gridLayout->addMultiCellWidget(label5, 6, 6, 0, 0);
    gridLayout->addMultiCellWidget(m_bottomLeftAngleLabel, 6, 6, 1, 2);
    gridLayout->addMultiCellWidget(label6, 7, 7, 0, 0);
    gridLayout->addMultiCellWidget(m_bottomRightAngleLabel, 7, 7, 1, 2);
    gridLayout->setRowStretch(8, 10);        
    setUserAreaWidget(gbox2);
    
    // -------------------------------------------------------------
    
    connect(m_previewWidget, SIGNAL(signalPerspectiveChanged(QRect, float, float, float, float)),
            this, SLOT(slotUpdateInfo(QRect, float, float, float, float)));  
}

ImageEffect_Perspective::~ImageEffect_Perspective()
{
}

void ImageEffect_Perspective::slotDefault()
{
    m_previewWidget->reset();
} 

void ImageEffect_Perspective::slotOk()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_previewWidget->applyPerspectiveAdjusment();
    accept();   
    kapp->restoreOverrideCursor();       
}

void ImageEffect_Perspective::slotUpdateInfo(QRect newSize, float topLeftAngle, float topRightAngle,
                                             float bottomLeftAngle, float bottomRightAngle)
{
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width())   + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
    
    m_topLeftAngleLabel->setText(temp.setNum( topLeftAngle, 'f', 1 ));
    m_topRightAngleLabel->setText(temp.setNum( topRightAngle, 'f', 1 ));
    m_bottomLeftAngleLabel->setText(temp.setNum( bottomLeftAngle, 'f', 1 ));
    m_bottomRightAngleLabel->setText(temp.setNum( bottomRightAngle, 'f', 1 ));
}

}  // NameSpace DigikamPerspectiveImagesPlugin

#include "imageeffect_perspective.moc"
