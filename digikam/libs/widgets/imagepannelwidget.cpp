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

// KDE includes.

#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kprogress.h>

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
    QToolTip::add( m_imageRegionWidget, i18n( "Original image preview." ) );
    QWhatsThis::add( m_imageRegionWidget, i18n("<p>You can see here the original clip image "
                                               "which will be used for the preview computation."
                                               "<p>Click and drag the mouse cursor in the "
                                               "image to change the clip focus."));
    l1->addWidget(m_imageRegionWidget, 0);
    m_mainLayout->addMultiCellWidget(frame1, 0, 1, 0, 0);
    m_mainLayout->setRowStretch(0, 10);
    m_mainLayout->setRowStretch(1, 10);
    m_mainLayout->setColStretch(0, 10);

    // -------------------------------------------------------------
        
    QFrame *frame3 = new QFrame(this);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3 = new QVBoxLayout(frame3, 5, 0);
    m_imagePanIconWidget = new Digikam::ImagePanIconWidget(360, 240, frame3);
    QToolTip::add( m_imagePanIconWidget, i18n( "Original image panel." ) );
    QWhatsThis::add( m_imagePanIconWidget, i18n("<p>You can see here the original image panel "
                                                "which can help you to select the clip preview."
                                                "<p>Click and drag the mouse cursor in the "
                                                "red rectangle to change the clip focus."));
    m_progressBar = new KProgress(100, frame3);
    setProgressVisible(progress);
    l3->addWidget(m_imagePanIconWidget, 0, Qt::AlignCenter);
    l3->addWidget(m_progressBar, 0, Qt::AlignCenter);
    
    m_mainLayout->addMultiCellWidget(frame3, 0, 0, 1, 1);
    
    // -------------------------------------------------------------
    
    QTimer::singleShot(0, this, SLOT(slotInitGui())); 
    
    // -------------------------------------------------------------
    
    connect(m_imageRegionWidget, SIGNAL(contentsMovedEvent()),
            this, SLOT(slotOriginalImageRegionChanged()));

    connect(m_imagePanIconWidget, SIGNAL(signalSelectionMoved(QRect, bool)),
            this, SLOT(slotSetImageRegionPosition(QRect, bool)));

    connect(m_imagePanIconWidget, SIGNAL(signalSelectionTakeFocus()),
            this, SLOT(slotPanIconTakeFocus()));
}

ImagePannelWidget::~ImagePannelWidget()
{
}

void ImagePannelWidget::slotInitGui(void)
{
    setCenterImageRegionPosition();
    slotOriginalImageRegionChanged();
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

void ImagePannelWidget::slotOriginalImageRegionChanged(void)
{
    QRect rect = getOriginalImageRegion();
    m_imagePanIconWidget->setRegionSelection( rect );
    updateSelectionInfo(rect);
    emit signalOriginalClipFocusChanged();
}

void ImagePannelWidget::updateSelectionInfo(QRect rect)
{
    QToolTip::add( this, i18n("Top left: (%1, %2)<br>Bottom right: (%3, %4)")
                         .arg(rect.left()).arg(rect.top())
                         .arg(rect.right()).arg(rect.bottom()));
}
    
}  // NameSpace Digikam

#include "imagepannelwidget.moc"
