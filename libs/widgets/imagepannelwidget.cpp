/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
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

    enum ZoomOptions 
    {
        ZoomX10=10,   //  1.0 zoom factor
        ZoomX15=15,   //  1.5 zoom factor
        ZoomX20=20,   //  2.0 zoom factor
        ZoomX25=25,   //  2.5 zoom factor
        ZoomX30=30    //  3.0 zoom factor
    };

    ImagePannelWidgetPriv()
    {
        imageRegionWidget  = 0;
        imagePanIconWidget = 0;
        mainLayout         = 0;
        separateView       = 0;
        progressBar        = 0;
        zoomButtons        = 0;
    }

    QGridLayout        *mainLayout;
    
    QHButtonGroup      *separateView;
    QHButtonGroup      *zoomButtons;
    
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
    d->mainLayout      = new QGridLayout( this, 2, 3 , KDialog::marginHint(), KDialog::spacingHint());

    // -------------------------------------------------------------
    
    QLabel *zoomLabel = new QLabel(i18n("Zoom Factor:"), this);

    d->zoomButtons = new QHButtonGroup(this);
    d->zoomButtons->setExclusive(true);
    d->zoomButtons->setInsideMargin( 0 );
    d->zoomButtons->setFrameShape(QFrame::NoFrame);

    QPushButton *zoomX10Button = new QPushButton( "x1", d->zoomButtons );
    zoomX10Button->setMaximumHeight( fontMetrics().height() );
    d->zoomButtons->insert(zoomX10Button, ImagePannelWidgetPriv::ZoomX10);
    zoomX10Button->setToggleButton(true);
    QWhatsThis::add( zoomX10Button, i18n( "<p>Press this buttom to not magnify image" ) );

    QPushButton *zoomX15Button = new QPushButton( "x1.5", d->zoomButtons );
    zoomX15Button->setMaximumHeight( fontMetrics().height() );
    d->zoomButtons->insert(zoomX15Button, ImagePannelWidgetPriv::ZoomX15);
    zoomX15Button->setToggleButton(true);
    QWhatsThis::add( zoomX15Button, i18n( "<p>Press this buttom to magnify image using 1.5:1 zoom factor." ) );

    QPushButton *zoomX20Button = new QPushButton( "x2", d->zoomButtons );
    zoomX20Button->setMaximumHeight( fontMetrics().height() );
    d->zoomButtons->insert(zoomX20Button, ImagePannelWidgetPriv::ZoomX20);
    zoomX20Button->setToggleButton(true);
    QWhatsThis::add( zoomX20Button, i18n( "<p>Press this buttom to magnify image using 2:1 zoom factor." ) );

    QPushButton *zoomX25Button = new QPushButton( "x2.5", d->zoomButtons );
    zoomX25Button->setMaximumHeight( fontMetrics().height() );
    d->zoomButtons->insert(zoomX25Button, ImagePannelWidgetPriv::ZoomX25);
    zoomX25Button->setToggleButton(true);
    QWhatsThis::add( zoomX25Button, i18n( "<p>Press this buttom to magnify image using 2.5:1 zoom factor." ) );

    QPushButton *zoomX30Button = new QPushButton( "x3", d->zoomButtons );
    zoomX30Button->setMaximumHeight( fontMetrics().height() );
    d->zoomButtons->insert(zoomX30Button, ImagePannelWidgetPriv::ZoomX30);
    zoomX30Button->setToggleButton(true);
    QWhatsThis::add( zoomX30Button, i18n( "<p>Press this buttom to magnify image using 3:1 zoom factor." ) );

    d->zoomButtons->setButton(ImagePannelWidgetPriv::ZoomX10);

    // -------------------------------------------------------------

    QFrame *preview = new QFrame(this);
    preview->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l1 = new QVBoxLayout(preview, 5, 0);
    d->imageRegionWidget = new ImageRegionWidget(w, h, preview, false);
    d->imageRegionWidget->setFrameStyle(QFrame::NoFrame);
    QWhatsThis::add( d->imageRegionWidget, i18n("<p>Here you can see the original clip image "
                                                "which will be used for the preview computation."
                                                "<p>Click and drag the mouse cursor in the "
                                                "image to change the clip focus."));
    l1->addWidget(d->imageRegionWidget, 0);

    // -------------------------------------------------------------
        
    QVBoxLayout *l2 = new QVBoxLayout( KDialog::spacingHint() ); 
    
    QFrame *frame3 = new QFrame(this);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3 = new QVBoxLayout(frame3, 5, 0);
    d->imagePanIconWidget = new ImagePanIconWidget(360, 240, frame3);
    QWhatsThis::add( d->imagePanIconWidget, i18n("<p>Here you can see the original image panel "
                                                 "which can help you to select the clip preview."
                                                 "<p>Click and drag the mouse cursor in the "
                                                 "red rectangle to change the clip focus."));
    l3->addWidget(d->imagePanIconWidget, 0, Qt::AlignCenter);
    
    d->separateView = new QHButtonGroup(this);
    d->separateView->setExclusive(true);
    d->separateView->setInsideMargin( 0 );
    d->separateView->setFrameShape(QFrame::NoFrame);
    
    if (separateViewMode == SeparateViewDuplicate ||
        separateViewMode == SeparateViewAll)
    {
       QPushButton *duplicateHorButton = new QPushButton( d->separateView );
       d->separateView->insert(duplicateHorButton, ImageRegionWidget::SeparateViewDuplicateHorz);
       KGlobal::dirs()->addResourceType("duplicatebothhorz", KGlobal::dirs()->kde_default("data") + "digikam/data");
       QString directory = KGlobal::dirs()->findResourceDir("duplicatebothhorz", "duplicatebothhorz.png");
       duplicateHorButton->setPixmap( QPixmap( directory + "duplicatebothhorz.png" ) );
       duplicateHorButton->setToggleButton(true);
       QWhatsThis::add( duplicateHorButton, i18n("<p>If you enable this option, you will separate the preview area "
                                                 "horizontally, displaying the original and target image "
                                                 "at the same time. The target is duplicated from the original "
                                                 "below the red dashed line." ) );
        
       QPushButton *duplicateVerButton = new QPushButton( d->separateView );
       d->separateView->insert(duplicateVerButton, ImageRegionWidget::SeparateViewDuplicateVert);
       KGlobal::dirs()->addResourceType("duplicatebothvert", KGlobal::dirs()->kde_default("data") + "digikam/data");
       directory = KGlobal::dirs()->findResourceDir("duplicatebothvert", "duplicatebothvert.png");
       duplicateVerButton->setPixmap( QPixmap( directory + "duplicatebothvert.png" ) );
       duplicateVerButton->setToggleButton(true);
       QWhatsThis::add( duplicateVerButton, i18n("<p>If you enable this option, you will separate the preview area "
                                                 "vertically, displaying the original and target image "
                                                 "at the same time. The target is duplicated from the original to "
                                                 "the right of the red dashed line." ) );
    }
        
    if (separateViewMode == SeparateViewNormal ||
        separateViewMode == SeparateViewAll)
       {
       QPushButton *separateHorButton = new QPushButton( d->separateView );
       d->separateView->insert(separateHorButton, ImageRegionWidget::SeparateViewHorizontal);
       KGlobal::dirs()->addResourceType("bothhorz", KGlobal::dirs()->kde_default("data") + "digikam/data");
       QString directory = KGlobal::dirs()->findResourceDir("bothhorz", "bothhorz.png");
       separateHorButton->setPixmap( QPixmap( directory + "bothhorz.png" ) );
       separateHorButton->setToggleButton(true);
       QWhatsThis::add( separateHorButton, i18n( "<p>If you enable this option, you will separate the preview area "
                                                 "horizontally, displaying the original and target image "
                                                 "at the same time. The original is above the "
                                                 "red dashed line, the target below it." ) );
        
       QPushButton *separateVerButton = new QPushButton( d->separateView );
       d->separateView->insert(separateVerButton, ImageRegionWidget::SeparateViewVertical);
       KGlobal::dirs()->addResourceType("bothvert", KGlobal::dirs()->kde_default("data") + "digikam/data");
       directory = KGlobal::dirs()->findResourceDir("bothvert", "bothvert.png");
       separateVerButton->setPixmap( QPixmap( directory + "bothvert.png" ) );
       separateVerButton->setToggleButton(true);
       QWhatsThis::add( separateVerButton, i18n( "<p>If you enable this option, you will separate the preview area "
                                                 "vertically, displaying the original and target image "
                                                 "at the same time. The original is to the left of the "
                                                 "red dashed line, the target to the right of it." ) );
       }
       
    QPushButton *noSeparateButton = new QPushButton( d->separateView );
    d->separateView->insert(noSeparateButton, ImageRegionWidget::SeparateViewNone);
    KGlobal::dirs()->addResourceType("target", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("target", "target.png");
    noSeparateButton->setPixmap( QPixmap( directory + "target.png" ) );
    noSeparateButton->setToggleButton(true);
    QWhatsThis::add( noSeparateButton, i18n( "<p>If you enable this option, the preview area will not "
                                             "be separated." ) );
    
    d->progressBar = new KProgress(100, this);
    QWhatsThis::add(d->progressBar ,i18n("<p>This is the current percentage of the task completed."));
    d->progressBar->setProgress(0);
    d->progressBar->setMaximumHeight( fontMetrics().height() );
    setProgressVisible(progress);
    
    QHBoxLayout *h1 = new QHBoxLayout( KDialog::spacingHint() ); 
    h1->addWidget(d->separateView);
    h1->addWidget(d->progressBar);
    
    l2->addWidget(frame3, 0, Qt::AlignHCenter);
    l2->addLayout(h1);
    l2->addStretch();

    d->mainLayout->addMultiCellWidget(preview, 0, 1, 0, 2);
    d->mainLayout->addMultiCellWidget(zoomLabel, 2, 2, 0, 0);
    d->mainLayout->addMultiCellWidget(d->zoomButtons, 2, 2, 1, 1);
    d->mainLayout->addMultiCellLayout(l2, 0, 0, 3, 3);
    d->mainLayout->setRowStretch(1, 10);
    d->mainLayout->setColStretch(2, 10);

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

    connect(this, SIGNAL(signalZoomFactorChanged(double)),
            d->imageRegionWidget, SLOT(slotZoomFactorChanged(double)));

    connect(this, SIGNAL(signalZoomFactorChanged(double)),
            d->imagePanIconWidget, SLOT(slotZoomFactorChanged(double)));

    connect(d->zoomButtons, SIGNAL(released(int)),
            this, SLOT(slotZoomButtonReleased(int)));
}

ImagePannelWidget::~ImagePannelWidget()
{
    writeSettings();
    delete d;
}

void ImagePannelWidget::slotZoomButtonReleased(int buttonId)
{
    emit signalZoomFactorChanged((double)(buttonId/10.0));
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
    d->mainLayout->addMultiCellLayout(vLayout, 1, 2, 3, 3);
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
    int mode = config->readNumEntry("Separate View", ImageRegionWidget::SeparateViewDuplicateVert);
    mode = QMAX(ImageRegionWidget::SeparateViewVertical, mode);
    mode = QMIN(ImageRegionWidget::SeparateViewDuplicateHorz, mode);
    
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
