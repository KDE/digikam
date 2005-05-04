/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-20
 * Description : 
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

// Qt includes.

#include <qlayout.h>
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
#include "imagepreviewwidget.h"

namespace Digikam
{

ImagePreviewWidget::ImagePreviewWidget(uint w, uint h, const QString &title, 
                                       QWidget *parent, bool progress)
                  : QWidget(parent, 0, Qt::WDestructiveClose)
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this, 0, KDialog::spacingHint());
    
    QVGroupBox *gbox1 = new QVGroupBox(i18n("Image Preview Selection"), this);
    QFrame *frame3 = new QFrame(gbox1);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3 = new QVBoxLayout(frame3, 5, 0);
    m_imagePanIconWidget = new Digikam::ImagePanIconWidget(360, 240, frame3);
    QToolTip::add( m_imagePanIconWidget, i18n( "Original image panel." ) );
    QWhatsThis::add( m_imagePanIconWidget, i18n("<p>You can see here the original image panel "
                                                "which can help you to select the clip preview."
                                                "<p>Click and drag the mouse cursor in the "
                                                "red rectangle to change the clip focus."));
    l3->addWidget(m_imagePanIconWidget, 0, Qt::AlignCenter);
    
    m_topLeftSelectionInfoLabel = new QLabel(gbox1);
    m_topLeftSelectionInfoLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    m_BottomRightSelectionInfoLabel = new QLabel(gbox1);
    m_BottomRightSelectionInfoLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    
    m_progressBar = new KProgress(100, gbox1);
    setProgressVisible(progress);
    
    mainLayout->addWidget(gbox1);
    
    QVGroupBox *gbox2 = new QVGroupBox(title, this);
    QFrame *frame1 = new QFrame(gbox2);
    frame1->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l1 = new QVBoxLayout(frame1, 5, 0);
    m_imageRegionWidget = new Digikam::ImageRegionWidget(w, h, frame1, false);
    m_imageRegionWidget->setFrameStyle(QFrame::NoFrame);
    QToolTip::add( m_imageRegionWidget, i18n( "Original image preview." ) );
    QWhatsThis::add( m_imageRegionWidget, i18n("<p>You can see here the original clip image "
                                               "which will be used for the preview computation."
                                               "<p>Click and drag the mouse cursor in the "
                                               "image to change the clip focus."));
    l1->addWidget(m_imageRegionWidget, 0, Qt::AlignCenter);
    

    QFrame *frame2 = new QFrame(gbox2);
    frame2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l2 = new QVBoxLayout(frame2, 5, 0);
    m_previewTargetLabel = new QLabel(frame2);
    m_previewTargetLabel->setFixedSize(w, h);
    QToolTip::add( m_previewTargetLabel, i18n( "Target image preview." ) );
    QWhatsThis::add( m_previewTargetLabel, i18n("<p>You can see here the image clip preview "
                                                "computation result."));
    l2->addWidget(m_previewTargetLabel, 0, Qt::AlignCenter);
    
    mainLayout->addWidget(gbox2);

    setCenterImageRegionPosition();
    slotOriginalImageRegionChanged();
    
    connect(m_imageRegionWidget, SIGNAL(contentsMovedEvent()),
            this, SLOT(slotOriginalImageRegionChanged()));

    connect(m_imagePanIconWidget, SIGNAL(signalSelectionMoved (QRect, bool)),
            this, SLOT(slotSetImageRegionPosition(QRect, bool)));
}

ImagePreviewWidget::~ImagePreviewWidget()
{
}

void ImagePreviewWidget::setEnable(bool b)
{
    m_imageRegionWidget->setEnabled(b);
    m_imagePanIconWidget->setEnabled(b);
}

void ImagePreviewWidget::setProgress(int val)
{
    m_progressBar->setValue(val);
}

void ImagePreviewWidget::setProgressVisible(bool b)
{
    if (b) m_progressBar->show();
    else m_progressBar->hide();
}

void ImagePreviewWidget::setProgressWhatsThis(QString desc)
{
    QWhatsThis::add( m_progressBar, desc);
}

void ImagePreviewWidget::setPreviewImageWaitCursor(bool enable)
{
    if ( enable )
       m_previewTargetLabel->setCursor( KCursor::waitCursor() );
    else 
       m_previewTargetLabel->setCursor( KCursor::arrowCursor() );
}

QRect ImagePreviewWidget::getOriginalImageRegion(void)
{
    return ( m_imageRegionWidget->getImageRegion() );
}

QImage ImagePreviewWidget::getOriginalClipImage(void)
{
    return ( m_imageRegionWidget->getImageRegionData() );
}

void ImagePreviewWidget::setPreviewImageData(QImage img)
{
    QPixmap pix;
    pix.convertFromImage(img);
    m_previewTargetLabel->setPixmap(pix);
}    

void ImagePreviewWidget::setCenterImageRegionPosition(void)
{
    m_imageRegionWidget->setCenterClipPosition();
}

void ImagePreviewWidget::slotSetImageRegionPosition(QRect rect, bool targetDone)
{
    m_imageRegionWidget->setClipPosition(rect.x(), rect.y(), targetDone);
}

void ImagePreviewWidget::slotOriginalImageRegionChanged(void)
{
    QRect rect = getOriginalImageRegion();
    m_imagePanIconWidget->setRegionSelection( rect );
    updateSelectionInfo(rect);
    emit signalOriginalClipFocusChanged();
}

void ImagePreviewWidget::updateSelectionInfo(QRect rect)
{
    m_topLeftSelectionInfoLabel->setText(i18n("Top left: (%1, %2)").arg(rect.left()).arg(rect.top()));
    m_BottomRightSelectionInfoLabel->setText(i18n("Bottom right: (%1, %2)").arg(rect.right()).arg(rect.bottom()));
}
    
}  // NameSpace Digikam

#include "imagepreviewwidget.moc"
