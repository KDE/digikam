/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-01
 * Description : a widget to draw a control pannel image tool.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
#include <qhbuttongroup.h> 
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpixmap.h>

// KDE includes.

#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kprogress.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kseparator.h>

// Local includes.

#include "imageregionwidget.h"
#include "imagepaniconwidget.h"
#include "imagepannelwidget.h"

namespace Digikam
{

class ImagePannelWidgetPriv
{
public:

    ImagePannelWidgetPriv()
    {
        imageRegionWidget  = 0;
        imagePanIconWidget = 0;
        mainLayout         = 0;
        separateView       = 0;
        progressBar        = 0;
    }

    QGridLayout        *mainLayout;
    
    QHButtonGroup      *separateView;
    
    QString             settingsSection;
    
    KProgress          *progressBar;
    
    ImageRegionWidget  *imageRegionWidget;
    ImagePanIconWidget *imagePanIconWidget;
};
    
ImagePannelWidget::ImagePannelWidget(uint w, uint h, QString settingsSection, QWidget *parent,
                                     bool progress, int separateViewMode)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePannelWidgetPriv;
    d->settingsSection = settingsSection;
    
    d->mainLayout = new QGridLayout( this, 2, 2 , KDialog::marginHint(), KDialog::spacingHint());
    
    QFrame *frame1 = new QFrame(this);
    frame1->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l1 = new QVBoxLayout(frame1, 5, 0);
    d->imageRegionWidget = new Digikam::ImageRegionWidget(w, h, frame1, false);
    d->imageRegionWidget->setFrameStyle(QFrame::NoFrame);
    QWhatsThis::add( d->imageRegionWidget, i18n("<p>Here you can see the original clip image "
                                               "which will be used for the preview computation."
                                               "<p>Click and drag the mouse cursor in the "
                                               "image to change the clip focus."));
    l1->addWidget(d->imageRegionWidget, 0);
    d->mainLayout->addMultiCellWidget(frame1, 0, 1, 0, 0);
    d->mainLayout->setRowStretch(1, 10);
    d->mainLayout->setColStretch(0, 10);

    // -------------------------------------------------------------
        
    QVBoxLayout *l2 = new QVBoxLayout( KDialog::spacingHint() ); 
    
    QFrame *frame3 = new QFrame(this);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3 = new QVBoxLayout(frame3, 5, 0);
    d->imagePanIconWidget = new Digikam::ImagePanIconWidget(360, 240, frame3);
    QWhatsThis::add( d->imagePanIconWidget, i18n("<p>Here you can see the original image panel "
                                                "which can help you to select the clip preview."
                                                "<p>Click and drag the mouse cursor in the "
                                                "red rectangle to change the clip focus."));
    l3->addWidget(d->imagePanIconWidget, 0, Qt::AlignCenter);
    
    d->progressBar = new KProgress(100, this);
    QWhatsThis::add(d->progressBar ,i18n("<p>This is the current percentage of the task completed."));
    d->progressBar->setProgress(0);
    d->progressBar->setMaximumHeight( fontMetrics().height() );
    setProgressVisible(progress);
    
    d->separateView = new QHButtonGroup(this);
    d->separateView->setExclusive(true);
    d->separateView->setInsideMargin( 0 );
    d->separateView->setFrameShape(QFrame::NoFrame);
    
    if (separateViewMode == SeparateViewDuplicate ||
        separateViewMode == SeparateViewAll)
    {
       QPushButton *duplicateHorButton = new QPushButton( d->separateView );
       d->separateView->insert(duplicateHorButton, Digikam::ImageRegionWidget::SeparateViewDuplicateHorz);
       KGlobal::dirs()->addResourceType("duplicateheight", KGlobal::dirs()->kde_default("data") + "digikam/data");
       QString directory = KGlobal::dirs()->findResourceDir("duplicateheight", "duplicateheight.png");
       duplicateHorButton->setPixmap( QPixmap( directory + "duplicateheight.png" ) );
       duplicateHorButton->setToggleButton(true);
       QToolTip::add( duplicateHorButton, i18n("<p>If you enable this option, you will separate the preview area "
                                               "horizontally, displaying the original and target image "
                                               "at the same time. The target is duplicated from the original "
                                               "below the red dashed line." ) );
        
       QPushButton *duplicateVerButton = new QPushButton( d->separateView );
       d->separateView->insert(duplicateVerButton, Digikam::ImageRegionWidget::SeparateViewDuplicateVert);
       KGlobal::dirs()->addResourceType("duplicatewidth", KGlobal::dirs()->kde_default("data") + "digikam/data");
       directory = KGlobal::dirs()->findResourceDir("duplicatewidth", "duplicatewidth.png");
       duplicateVerButton->setPixmap( QPixmap( directory + "duplicatewidth.png" ) );
       duplicateVerButton->setToggleButton(true);
       QToolTip::add( duplicateVerButton, i18n("<p>If you enable this option, you will separate the preview area "
                                               "vertically, displaying the original and target image "
                                               "at the same time. The target is duplicated from the original to "
                                               "the right of the red dashed line." ) );
    }
        
    if (separateViewMode == SeparateViewNormal ||
        separateViewMode == SeparateViewAll)
       {
       QPushButton *separateHorButton = new QPushButton( d->separateView );
       d->separateView->insert(separateHorButton, Digikam::ImageRegionWidget::SeparateViewHorizontal);
       KGlobal::dirs()->addResourceType("centerheight", KGlobal::dirs()->kde_default("data") + "digikam/data");
       QString directory = KGlobal::dirs()->findResourceDir("centerheight", "centerheight.png");
       separateHorButton->setPixmap( QPixmap( directory + "centerheight.png" ) );
       separateHorButton->setToggleButton(true);
       QToolTip::add( separateHorButton, i18n( "<p>If you enable this option, you will separate the preview area "
                                               "horizontally, displaying the original and target image "
                                               "at the same time. The original is above the "
                                               "red dashed line, the target below it." ) );
        
       QPushButton *separateVerButton = new QPushButton( d->separateView );
       d->separateView->insert(separateVerButton, Digikam::ImageRegionWidget::SeparateViewVertical);
       KGlobal::dirs()->addResourceType("centerwidth", KGlobal::dirs()->kde_default("data") + "digikam/data");
       directory = KGlobal::dirs()->findResourceDir("centerwidth", "centerwidth.png");
       separateVerButton->setPixmap( QPixmap( directory + "centerwidth.png" ) );
       separateVerButton->setToggleButton(true);
       QToolTip::add( separateVerButton, i18n( "<p>If you enable this option, you will separate the preview area "
                                               "vertically, displaying the original and target image "
                                               "at the same time. The original is to the left of the "
                                               "red dashed line, the target to the right of it." ) );
       }
       
    QPushButton *noSeparateButton = new QPushButton( d->separateView );
    d->separateView->insert(noSeparateButton, Digikam::ImageRegionWidget::SeparateViewNone);
    KGlobal::dirs()->addResourceType("nocenter", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("nocenter", "nocenter.png");
    noSeparateButton->setPixmap( QPixmap( directory + "nocenter.png" ) );
    noSeparateButton->setToggleButton(true);
    QToolTip::add( noSeparateButton, i18n( "<p>If you enable this option, the preview area will not "
                                           "be separated." ) );
    
    l2->addWidget(frame3, 0, Qt::AlignHCenter);
    QHBoxLayout *h1 = new QHBoxLayout( KDialog::spacingHint() ); 
    h1->addWidget(d->separateView);
    h1->addWidget(d->progressBar, 10);
    l2->addLayout(h1);
    l2->addStretch();
    
    d->mainLayout->addMultiCellLayout(l2, 0, 0, 1, 1);
    
    // -------------------------------------------------------------
    
    QTimer::singleShot(0, this, SLOT(slotInitGui())); 
    
    // -------------------------------------------------------------
    
    connect(d->imageRegionWidget, SIGNAL(contentsMovedEvent(bool)),
            this, SLOT(slotOriginalImageRegionChanged(bool)));

    connect(d->imagePanIconWidget, SIGNAL(signalSelectionMoved(QRect, bool)),
            this, SLOT(slotSetImageRegionPosition(QRect, bool)));

    connect(d->imagePanIconWidget, SIGNAL(signalSelectionTakeFocus()),
            this, SLOT(slotPanIconTakeFocus()));
            
    connect(d->separateView, SIGNAL(released(int)),
            d->imageRegionWidget, SLOT(slotSeparateViewToggled(int)));
    
    connect(d->separateView, SIGNAL(released(int)),
            d->imagePanIconWidget, SLOT(slotSeparateViewToggled(int)));
}

ImagePannelWidget::~ImagePannelWidget()
{
    writeSettings();
    delete d;
}

KProgress *ImagePannelWidget::progressBar(void)
{
    return d->progressBar;
}

void ImagePannelWidget::resizeEvent(QResizeEvent *)
{
    emit signalResized();
}

void ImagePannelWidget::slotInitGui(void)
{
    readSettings();
    setCenterImageRegionPosition();
    slotOriginalImageRegionChanged(true);
}

void ImagePannelWidget::setPanIconHighLightPoints(QPointArray pt) 
{
    d->imageRegionWidget->setHighLightPoints(pt);
    d->imagePanIconWidget->setHighLightPoints(pt);
}

void ImagePannelWidget::slotPanIconTakeFocus(void)
{
    d->imageRegionWidget->restorePixmapRegion();
}

void ImagePannelWidget::setUserAreaWidget(QWidget *w, bool separator)
{
    QVBoxLayout *vLayout = new QVBoxLayout( KDialog::spacingHint() ); 
    
    if (separator)
    {
       KSeparator *line = new KSeparator (Horizontal, this);
       vLayout->addWidget(line);
    }
       
    vLayout->addWidget(w);
    vLayout->addStretch();
    d->mainLayout->addMultiCellLayout(vLayout, 1, 1, 1, 1);
}

void ImagePannelWidget::setEnable(bool b)
{
    d->imageRegionWidget->setEnabled(b);
    d->imagePanIconWidget->setEnabled(b);
    d->separateView->setEnabled(b);
}

void ImagePannelWidget::setProgress(int val)
{
    d->progressBar->setValue(val);
}

void ImagePannelWidget::setProgressVisible(bool b)
{
    if (b) d->progressBar->show();
    else d->progressBar->hide();
}

void ImagePannelWidget::setProgressWhatsThis(QString desc)
{
    QWhatsThis::add( d->progressBar, desc);
}

void ImagePannelWidget::setPreviewImageWaitCursor(bool enable)
{
    if ( enable )
       d->imageRegionWidget->setCursor( KCursor::waitCursor() );
    else 
       d->imageRegionWidget->setCursor( KCursor::arrowCursor() );
}

QRect ImagePannelWidget::getOriginalImageRegion(void)
{
    return ( d->imageRegionWidget->getImageRegion() );
}

QRect ImagePannelWidget::getOriginalImageRegionToRender(void)
{
    return ( d->imageRegionWidget->getImageRegionToRender() );
}

DImg ImagePannelWidget::getOriginalRegionImage(void)
{
    return ( d->imageRegionWidget->getImageRegionImage() );
}

void ImagePannelWidget::setPreviewImage(DImg img)
{
    d->imageRegionWidget->updatePreviewImage(&img);
}    

void ImagePannelWidget::setCenterImageRegionPosition(void)
{
    d->imageRegionWidget->setCenterContentsPosition();
}

void ImagePannelWidget::slotSetImageRegionPosition(QRect rect, bool targetDone)
{
    d->imageRegionWidget->setContentsPosition(rect.x(), rect.y(), targetDone);
}

void ImagePannelWidget::slotOriginalImageRegionChanged(bool target)
{
    QRect rect = getOriginalImageRegion();
    d->imagePanIconWidget->setRegionSelection( rect );
    updateSelectionInfo(rect);
    
    if (target)
    {
        d->imageRegionWidget->backupPixmapRegion();
        emit signalOriginalClipFocusChanged();
    }
}

void ImagePannelWidget::updateSelectionInfo(QRect rect)
{
    QToolTip::add( d->imagePanIconWidget,
                   i18n("Top left: (%1, %2)<br>Bottom right: (%3, %4)")
                        .arg(rect.left()).arg(rect.top())
                        .arg(rect.right()).arg(rect.bottom()));
}

void ImagePannelWidget::readSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup(d->settingsSection);
    int mode = config->readNumEntry("Separate View", Digikam::ImageRegionWidget::SeparateViewVertical);
    mode = QMAX(Digikam::ImageRegionWidget::SeparateViewVertical, mode);
    mode = QMIN(Digikam::ImageRegionWidget::SeparateViewDuplicateHorz, mode);
    
    d->imageRegionWidget->blockSignals(true);
    d->imagePanIconWidget->blockSignals(true);
    d->separateView->blockSignals(true);
    d->imageRegionWidget->slotSeparateViewToggled( mode );
    d->imagePanIconWidget->slotSeparateViewToggled( mode );
    d->separateView->setButton( mode );
    d->imageRegionWidget->blockSignals(false);
    d->imagePanIconWidget->blockSignals(false);
    d->separateView->blockSignals(false);
}
    
void ImagePannelWidget::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup(d->settingsSection);
    config->writeEntry( "Separate View", d->separateView->selectedId() );
    config->sync();
}

// FIXME remove these methods when all image plugins will be ported to DIMG.

QImage ImagePannelWidget::getOriginalClipImage(void)
{
    return ( d->imageRegionWidget->getImageRegionData() );
}

void ImagePannelWidget::setPreviewImageData(QImage img)
{
    d->imageRegionWidget->updatePreviewImage(&img);
}    

}  // NameSpace Digikam

#include "imagepannelwidget.moc"
