/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2005-02-17
 * Description : a digiKam image editor plugin for process image 
 *               perspective adjustment.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcheckbox.h>

// KDE includes.

#include <kcolorbutton.h>
#include <kcursor.h>
#include <kconfig.h>
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
#include "imageeffect_perspective.moc"

namespace DigikamPerspectiveImagesPlugin
{

ImageEffect_Perspective::ImageEffect_Perspective(QWidget* parent, QString title, QFrame* banner)
                       : Digikam::ImageDlgBase(parent, title, "perspective", false, false, banner)
{
    QString whatsThis;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Perspective"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to process image perspective adjustment."),
                                       KAboutData::License_GPL,
                                       "(c) 2005-2006, Gilles Caulier\n"
                                       "(c) 2006-2007, Gilles Caulier and Marcel Wiesweg",
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at kdemail dot net");

    about->addAuthor("Marcel Wiesweg", I18N_NOOP("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l  = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new PerspectiveWidget(525, 350, frame);
    l->addWidget(m_previewWidget);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the perspective transformation operation preview. "
                                           "You can use the mouse for dragging the corner to adjust the "
                                           "perspective transformation area."));
    setPreviewAreaWidget(frame); 
    
    // -------------------------------------------------------------

    QString temp;
    Digikam::ImageIface iface(0, 0);

    QWidget *gbox2          = new QWidget(plainPage());
    QGridLayout *gridLayout = new QGridLayout( gbox2, 13, 2, marginHint(), spacingHint());

    QLabel *label1  = new QLabel(i18n("New width:"), gbox2);
    m_newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"), gbox2);
    m_newWidthLabel->setAlignment( AlignBottom | AlignRight );

    QLabel *label2   = new QLabel(i18n("New height:"), gbox2);
    m_newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"), gbox2);
    m_newHeightLabel->setAlignment( AlignBottom | AlignRight );
    
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

    // -------------------------------------------------------------

    KSeparator *line2 = new KSeparator (Horizontal, gbox2);

    m_drawWhileMovingCheckBox = new QCheckBox(i18n("Draw preview while moving"), gbox2);
    gridLayout->addMultiCellWidget(line2, 8, 8, 0, 2);
    gridLayout->addMultiCellWidget(m_drawWhileMovingCheckBox, 9, 9, 0, 2);

    m_drawGridCheckBox = new QCheckBox(i18n("Draw grid"), gbox2);
    gridLayout->addMultiCellWidget(m_drawGridCheckBox, 10, 10, 0, 2);

    // -------------------------------------------------------------

    QLabel *label7 = new QLabel(i18n("Guide color:"), gbox2);
    m_guideColorBt = new KColorButton( QColor( Qt::red ), gbox2 );
    QWhatsThis::add( m_guideColorBt, i18n("<p>Set here the color used to draw guides dashed-lines."));
    gridLayout->addMultiCellWidget(label7, 11, 11, 0, 0);
    gridLayout->addMultiCellWidget(m_guideColorBt, 11, 11, 2, 2);

    QLabel *label8 = new QLabel(i18n("Guide width:"), gbox2);
    m_guideSize    = new QSpinBox( 1, 5, 1, gbox2);
    QWhatsThis::add( m_guideSize, i18n("<p>Set here the width in pixels used to draw guides dashed-lines."));
    gridLayout->addMultiCellWidget(label8, 12, 12, 0, 0);
    gridLayout->addMultiCellWidget(m_guideSize, 12, 12, 2, 2);

    gridLayout->setColStretch(1, 10);
    gridLayout->setRowStretch(13, 10);

    setUserAreaWidget(gbox2);

    // -------------------------------------------------------------

    connect(m_previewWidget, SIGNAL(signalPerspectiveChanged(QRect, float, float, float, float)),
            this, SLOT(slotUpdateInfo(QRect, float, float, float, float)));  

    connect(m_drawWhileMovingCheckBox, SIGNAL(toggled(bool)),
            m_previewWidget, SLOT(slotToggleDrawWhileMoving(bool)));

    connect(m_drawGridCheckBox, SIGNAL(toggled(bool)),
            m_previewWidget, SLOT(slotToggleDrawGrid(bool)));

    connect(m_guideColorBt, SIGNAL(changed(const QColor &)),
            m_previewWidget, SLOT(slotChangeGuideColor(const QColor &)));

    connect(m_guideSize, SIGNAL(valueChanged(int)),
            m_previewWidget, SLOT(slotChangeGuideSize(int)));
}

ImageEffect_Perspective::~ImageEffect_Perspective()
{
}

void ImageEffect_Perspective::readUserSettings(void)
{
    QColor defaultGuideColor(Qt::red);
    KConfig *config = kapp->config();
    config->setGroup("perspective Tool Dialog");
    m_drawWhileMovingCheckBox->setChecked(config->readBoolEntry("Draw While Moving", true));
    m_drawGridCheckBox->setChecked(config->readBoolEntry("Draw Grid", false));
    m_guideColorBt->setColor(config->readColorEntry("Guide Color", &defaultGuideColor));
    m_guideSize->setValue(config->readNumEntry("Guide Width", 1));
    m_previewWidget->slotToggleDrawWhileMoving(m_drawWhileMovingCheckBox->isChecked());
    m_previewWidget->slotToggleDrawGrid(m_drawGridCheckBox->isChecked());
    m_previewWidget->slotChangeGuideColor(m_guideColorBt->color());
    m_previewWidget->slotChangeGuideSize(m_guideSize->value());
}

void ImageEffect_Perspective::writeUserSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("perspective Tool Dialog");
    config->writeEntry("Draw While Moving", m_drawWhileMovingCheckBox->isChecked());
    config->writeEntry("Draw Grid", m_drawGridCheckBox->isChecked());
    config->writeEntry("Guide Color", m_guideColorBt->color());
    config->writeEntry("Guide Width", m_guideSize->value());
    config->sync();
}

void ImageEffect_Perspective::resetValues()
{
    m_previewWidget->reset();
}

void ImageEffect_Perspective::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_previewWidget->applyPerspectiveAdjustment();
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

