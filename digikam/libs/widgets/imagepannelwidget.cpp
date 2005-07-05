/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-01
 * Description : a widget to draw a control pannel image tool.
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

#include <qframe.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qtimer.h>
#include <qcheckbox.h>

// KDE includes.

#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kprogress.h>
#include <kapplication.h>
#include <kconfig.h>

// Local includes.

#include "imageregionwidget.h"
#include "imagepaniconwidget.h"
#include "imagepannelwidget.h"

namespace Digikam
{

ImagePannelWidget::ImagePannelWidget(uint w, uint h, QWidget *parent, bool progress)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_mainLayout = new QGridLayout( this, 2, 2 , KDialog::marginHint(), KDialog::spacingHint());
    
    QFrame *frame1 = new QFrame(this);
    frame1->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l1 = new QVBoxLayout(frame1, 5, 0);
    m_imageRegionWidget = new Digikam::ImageRegionWidget(w, h, frame1, false);
    m_imageRegionWidget->setFrameStyle(QFrame::NoFrame);
    QWhatsThis::add( m_imageRegionWidget, i18n("<p>You can see here the original clip image "
                                               "which will be used for the preview computation."
                                               "<p>Click and drag the mouse cursor in the "
                                               "image to change the clip focus. "
                                               "<p>If <b>Separate View</b> option is enable, the original "
                                               "image is on the left of the red dashed line, and "
                                               "target image on the right."));
    l1->addWidget(m_imageRegionWidget, 0);
    m_mainLayout->addMultiCellWidget(frame1, 0, 1, 0, 0);
    m_mainLayout->setRowStretch(1, 10);
    m_mainLayout->setColStretch(0, 10);

    // -------------------------------------------------------------
        
    QVBoxLayout *l2 = new QVBoxLayout( KDialog::spacingHint() ); 
    
    QFrame *frame3 = new QFrame(this);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3 = new QVBoxLayout(frame3, 5, 0);
    m_imagePanIconWidget = new Digikam::ImagePanIconWidget(360, 240, frame3);
    QWhatsThis::add( m_imagePanIconWidget, i18n("<p>You can see here the original image panel "
                                                "which can help you to select the clip preview."
                                                "<p>Click and drag the mouse cursor in the "
                                                "red rectangle to change the clip focus."));
    l3->addWidget(m_imagePanIconWidget, 0, Qt::AlignCenter);
    
    m_progressBar = new KProgress(100, this);
    QWhatsThis::add(m_progressBar ,i18n("<p>This is the current percentage of the task completed."));
    m_progressBar->setProgress(0);
    setProgressVisible(progress);
    
    m_separateView = new QCheckBox(i18n("Separate View"), this);
    QWhatsThis::add( m_separateView, i18n("<p>If you enable this option, you will separe the "
                                          "preview area to display original and target image "
                                          "at the same time. The original is on the left of the "
                                          "red dashed line, target on the right"));
        
    l2->addWidget(frame3, 0, Qt::AlignHCenter);
    QHBoxLayout *h1 = new QHBoxLayout( KDialog::spacingHint() ); 
    h1->addWidget(m_separateView);
    h1->addWidget(m_progressBar, 10);
    l2->addLayout(h1);
    l2->addStretch();
    
    m_mainLayout->addMultiCellLayout(l2, 0, 0, 1, 1);
    
    // -------------------------------------------------------------
    
    QTimer::singleShot(0, this, SLOT(slotInitGui())); 
    
    // -------------------------------------------------------------
    
    connect(m_imageRegionWidget, SIGNAL(contentsMovedEvent(bool)),
            this, SLOT(slotOriginalImageRegionChanged(bool)));

    connect(m_imagePanIconWidget, SIGNAL(signalSelectionMoved(QRect, bool)),
            this, SLOT(slotSetImageRegionPosition(QRect, bool)));

    connect(m_imagePanIconWidget, SIGNAL(signalSelectionTakeFocus()),
            this, SLOT(slotPanIconTakeFocus()));
            
    connect(m_separateView, SIGNAL(toggled(bool)),
            m_imageRegionWidget, SLOT(slotSeparateViewToggled(bool)));               
    
    connect(m_separateView, SIGNAL(toggled(bool)),
            m_imagePanIconWidget, SLOT(slotSeparateViewToggled(bool)));
}

ImagePannelWidget::~ImagePannelWidget()
{
    writeSettings();
}

void ImagePannelWidget::slotInitGui(void)
{
    readSettings();
    setCenterImageRegionPosition();
    slotOriginalImageRegionChanged(true);
}

void ImagePannelWidget::slotPanIconTakeFocus(void)
{
    m_imageRegionWidget->updateOriginalImage();
}

void ImagePannelWidget::setUserAreaWidget(QWidget *w)
{
    QVBoxLayout *vLayout = new QVBoxLayout( KDialog::spacingHint() ); 
    vLayout->addWidget(w);
    vLayout->addStretch();
    m_mainLayout->addMultiCellLayout(vLayout, 1, 1, 1, 1);    
}

void ImagePannelWidget::setEnable(bool b)
{
    m_imageRegionWidget->setEnabled(b);
    m_imagePanIconWidget->setEnabled(b);
    m_separateView->setEnabled(b);    
}

void ImagePannelWidget::setProgress(int val)
{
    m_progressBar->setValue(val);
}

void ImagePannelWidget::setProgressVisible(bool b)
{
    if (b) m_progressBar->show();
    else m_progressBar->hide();
}

void ImagePannelWidget::setProgressWhatsThis(QString desc)
{
    QWhatsThis::add( m_progressBar, desc);
}

void ImagePannelWidget::setPreviewImageWaitCursor(bool enable)
{
    if ( enable )
       m_imageRegionWidget->setCursor( KCursor::waitCursor() );
    else 
       m_imageRegionWidget->setCursor( KCursor::arrowCursor() );
}

QRect ImagePannelWidget::getOriginalImageRegion(void)
{
    return ( m_imageRegionWidget->getImageRegion() );
}

QRect ImagePannelWidget::getOriginalImageRegionToRender(void)
{
    return ( m_imageRegionWidget->getImageRegionToRender() );
}

QImage ImagePannelWidget::getOriginalClipImage(void)
{
    return ( m_imageRegionWidget->getImageRegionData() );
}

void ImagePannelWidget::setPreviewImageData(QImage img)
{
    m_imageRegionWidget->updatePreviewImage(&img);
}    

void ImagePannelWidget::setCenterImageRegionPosition(void)
{
    m_imageRegionWidget->setCenterClipPosition();
}

void ImagePannelWidget::slotSetImageRegionPosition(QRect rect, bool targetDone)
{
    m_imageRegionWidget->setClipPosition(rect.x(), rect.y(), targetDone);
}

void ImagePannelWidget::slotOriginalImageRegionChanged(bool target)
{
    QRect rect = getOriginalImageRegion();
    m_imagePanIconWidget->setRegionSelection( rect );
    updateSelectionInfo(rect);
    
    if (target)
        emit signalOriginalClipFocusChanged();
}

void ImagePannelWidget::updateSelectionInfo(QRect rect)
{
    QToolTip::add( m_imagePanIconWidget, 
                   i18n("Top left: (%1, %2)<br>Bottom right: (%3, %4)")
                        .arg(rect.left()).arg(rect.top())
                        .arg(rect.right()).arg(rect.bottom()));
}

void ImagePannelWidget::readSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Control Panel Settings");
    
    m_separateView->setChecked(config->readNumEntry("Separate View", true) );
}
    
void ImagePannelWidget::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Control Panel Settings");
    
    config->writeEntry( "Separate View", m_separateView->isChecked() );
    
    config->sync();
}
    
}  // NameSpace Digikam

#include "imagepannelwidget.moc"
