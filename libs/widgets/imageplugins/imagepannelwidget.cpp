/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-01
 * Description : a widget to draw a control pannel image tool.
 * 
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <qhbuttongroup.h> 
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpixmap.h>

// KDE includes.

#include <kdialog.h>
#include <klocale.h>
#include <kcursor.h>
#include <kprogress.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kseparator.h>

// Local includes.

#include "ddebug.h"
#include "sidebar.h"
#include "statuszoombar.h"
#include "thumbnailsize.h"
#include "imageregionwidget.h"
#include "imagepaniconwidget.h"
#include "imagepannelwidget.h"
#include "imagepannelwidget.moc"

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
        settingsSideBar    = 0;
        splitter           = 0;        
        settingsLayout     = 0;
        settings           = 0;
        previewWidget      = 0;
        zoomBar            = 0;
    }

    QGridLayout        *mainLayout;
    
    QHButtonGroup      *separateView;
    
    QString             settingsSection;
    
    QWidget            *settings;
    QWidget            *previewWidget;
    
    QVBoxLayout        *settingsLayout;
        
    QSplitter          *splitter;
    
    KProgress          *progressBar;
    
    ImageRegionWidget  *imageRegionWidget;
    ImagePanIconWidget *imagePanIconWidget;
    
    Sidebar            *settingsSideBar;

    StatusZoomBar      *zoomBar;
};
    
ImagePannelWidget::ImagePannelWidget(uint w, uint h, const QString& settingsSection, 
                                     QWidget *parent, int separateViewMode)
                 : QHBox(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePannelWidgetPriv;
    d->settingsSection = settingsSection;
    d->splitter        = new QSplitter(this);
    d->previewWidget   = new QWidget(d->splitter);
    d->mainLayout      = new QGridLayout( d->previewWidget, 2, 3, 0, KDialog::spacingHint());
    
    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );    
    d->splitter->setOpaqueResize(false);

    // -------------------------------------------------------------

    QFrame *preview      = new QFrame(d->previewWidget);
    QVBoxLayout* l1      = new QVBoxLayout(preview, 5, 0);
    d->imageRegionWidget = new ImageRegionWidget(w, h, preview, false);
    d->imageRegionWidget->setFrameStyle(QFrame::NoFrame);
    preview->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QWhatsThis::add( d->imageRegionWidget, i18n("<p>Here you can see the original clip image "
                                                "which will be used for the preview computation."
                                                "<p>Click and drag the mouse cursor in the "
                                                "image to change the clip focus."));
    l1->addWidget(d->imageRegionWidget, 0);

    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    d->previewWidget->setSizePolicy(rightSzPolicy);

    // -------------------------------------------------------------
    
    d->zoomBar = new StatusZoomBar(d->previewWidget);
    QWhatsThis::add( d->zoomBar, i18n("<p>Here set the zoom factor of the preview area.") );

    // -------------------------------------------------------------
    
    QString directory;
    d->separateView = new QHButtonGroup(d->previewWidget);
    d->separateView->setExclusive(true);
    d->separateView->setInsideMargin( 0 );
    d->separateView->setFrameShape(QFrame::NoFrame);
    
    if (separateViewMode == SeparateViewDuplicate ||
        separateViewMode == SeparateViewAll)
    {
       QPushButton *duplicateHorButton = new QPushButton( d->separateView );
       d->separateView->insert(duplicateHorButton, ImageRegionWidget::SeparateViewDuplicateHorz);
       KGlobal::dirs()->addResourceType("duplicatebothhorz", KGlobal::dirs()->kde_default("data") + "digikam/data");
       directory = KGlobal::dirs()->findResourceDir("duplicatebothhorz", "duplicatebothhorz.png");
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
       directory = KGlobal::dirs()->findResourceDir("bothhorz", "bothhorz.png");
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
    directory = KGlobal::dirs()->findResourceDir("target", "target.png");
    noSeparateButton->setPixmap( QPixmap( directory + "target.png" ) );
    noSeparateButton->setToggleButton(true);
    QWhatsThis::add( noSeparateButton, i18n( "<p>If you enable this option, the preview area will not "
                                             "be separated." ) );
    
    // -------------------------------------------------------------
    
    d->progressBar = new KProgress(100, d->previewWidget);
    QWhatsThis::add(d->progressBar ,i18n("<p>This is the percentage of the task which has been completed up to this point."));
    d->progressBar->setProgress(0);
    d->progressBar->setMaximumHeight( fontMetrics().height() );

    // -------------------------------------------------------------
        
    d->mainLayout->addMultiCellWidget(preview, 0, 1, 0, 3);
    d->mainLayout->addMultiCellWidget(d->zoomBar, 2, 2, 0, 0);
    d->mainLayout->addMultiCellWidget(d->progressBar, 2, 2, 2, 2);
    d->mainLayout->addMultiCellWidget(d->separateView, 2, 2, 3, 3);
    
    d->mainLayout->setRowStretch(1, 10);
    d->mainLayout->setColStretch(1, 10);
        
    // -------------------------------------------------------------

    QString sbName(d->settingsSection + QString(" Image Plugin Sidebar"));
    d->settingsSideBar = new Sidebar(this, sbName.ascii(), Sidebar::Right);
    d->settingsSideBar->setSplitter(d->splitter);
    
    d->settings       = new QWidget(d->settingsSideBar);
    d->settingsLayout = new QVBoxLayout(d->settings);    
    
    QFrame *frame3 = new QFrame(d->settings);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3 = new QVBoxLayout(frame3, 5, 0);
    d->imagePanIconWidget = new ImagePanIconWidget(360, 240, frame3);
    QWhatsThis::add( d->imagePanIconWidget, i18n("<p>Here you can see the original image panel "
                                                 "which can help you to select the clip preview."
                                                 "<p>Click and drag the mouse cursor in the "
                                                 "red rectangle to change the clip focus."));
    l3->addWidget(d->imagePanIconWidget, 0, Qt::AlignCenter);

    d->settingsLayout->addWidget(frame3, 0, Qt::AlignHCenter);
    d->settingsLayout->addSpacing(KDialog::spacingHint());

    d->settingsSideBar->appendTab(d->settings, SmallIcon("configure"), i18n("Settings"));    
    d->settingsSideBar->loadViewState();

    // -------------------------------------------------------------
    
    setProgressVisible(false);
    QTimer::singleShot(0, this, SLOT(slotInitGui())); 
    
    // -------------------------------------------------------------
    
    connect(d->imageRegionWidget, SIGNAL(signalContentsMovedEvent(bool)),
            this, SLOT(slotOriginalImageRegionChanged(bool)));

    connect(d->imagePanIconWidget, SIGNAL(signalSelectionMoved(const QRect&, bool)),
            this, SLOT(slotSetImageRegionPosition(const QRect&, bool)));

    connect(d->imagePanIconWidget, SIGNAL(signalSelectionTakeFocus()),
            this, SLOT(slotPanIconTakeFocus()));
            
    connect(d->separateView, SIGNAL(released(int)),
            d->imageRegionWidget, SLOT(slotSeparateViewToggled(int)));
    
    connect(d->separateView, SIGNAL(released(int)),
            d->imagePanIconWidget, SLOT(slotSeparateViewToggled(int)));

    connect(d->zoomBar, SIGNAL(signalZoomMinusClicked()),
            d->imageRegionWidget, SLOT(slotDecreaseZoom()));

    connect(d->zoomBar, SIGNAL(signalZoomPlusClicked()),
            d->imageRegionWidget, SLOT(slotIncreaseZoom()));

    connect(d->zoomBar, SIGNAL(signalZoomSliderReleased(int)),
            this, SLOT(slotZoomSliderChanged(int)));
}

ImagePannelWidget::~ImagePannelWidget()
{
    writeSettings();
    delete d->settingsSideBar;
    delete d;
}

void ImagePannelWidget::readSettings()
{
    KConfig *config = kapp->config();
    config->setGroup(d->settingsSection);
    int mode = config->readNumEntry("Separate View", ImageRegionWidget::SeparateViewDuplicateVert);
    mode     = QMAX(ImageRegionWidget::SeparateViewHorizontal, mode);
    mode     = QMIN(ImageRegionWidget::SeparateViewDuplicateHorz, mode);
    
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
    
void ImagePannelWidget::writeSettings()
{
    KConfig *config = kapp->config();
    config->setGroup(d->settingsSection);
    config->writeEntry( "Separate View", d->separateView->selectedId() );
    config->sync();
}

void ImagePannelWidget::slotOriginalImageRegionChanged(bool target)
{
    slotZoomFactorChanged(d->imageRegionWidget->zoomFactor());
    QRect rect = getOriginalImageRegion();
    d->imagePanIconWidget->setRegionSelection(rect);
    updateSelectionInfo(rect);

    if (target)
    {
        d->imageRegionWidget->backupPixmapRegion();
        emit signalOriginalClipFocusChanged();
    }
}

void ImagePannelWidget::slotZoomFactorChanged(double zoom)
{
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->imageRegionWidget->zoomMin();
    double zmax = d->imageRegionWidget->zoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    int size    = (int)((zoom - b) /a); 

    d->zoomBar->setZoomSliderValue(size);
    d->zoomBar->setZoomTrackerText(i18n("zoom: %1%").arg((int)(zoom*100.0)));

    d->zoomBar->setEnableZoomPlus(true);
    d->zoomBar->setEnableZoomMinus(true);

    if (d->imageRegionWidget->maxZoom())
        d->zoomBar->setEnableZoomPlus(false);

    if (d->imageRegionWidget->minZoom())
        d->zoomBar->setEnableZoomMinus(false);

    d->imagePanIconWidget->slotZoomFactorChanged(zoom);
}

void ImagePannelWidget::slotZoomSliderChanged(int size)
{
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->imageRegionWidget->zoomMin();
    double zmax = d->imageRegionWidget->zoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    double z    = a*size+b; 

    d->imageRegionWidget->setZoomFactor(z);
}

KProgress *ImagePannelWidget::progressBar()
{
    return d->progressBar;
}

void ImagePannelWidget::resizeEvent(QResizeEvent *)
{
    emit signalResized();
}

void ImagePannelWidget::slotInitGui()
{
    readSettings();
    setCenterImageRegionPosition();
    slotOriginalImageRegionChanged(true);
}

void ImagePannelWidget::setPanIconHighLightPoints(const QPointArray& pt) 
{
    d->imageRegionWidget->setHighLightPoints(pt);
    d->imagePanIconWidget->setHighLightPoints(pt);
}

void ImagePannelWidget::slotPanIconTakeFocus()
{
    d->imageRegionWidget->restorePixmapRegion();
}

void ImagePannelWidget::setUserAreaWidget(QWidget *w)
{
    w->reparent( d->settings, QPoint(0, 0) );
    d->settingsLayout->addSpacing(KDialog::spacingHint());
    d->settingsLayout->addWidget(w);
    d->settingsLayout->addStretch();
}

void ImagePannelWidget::setEnable(bool b)
{
    d->imageRegionWidget->setEnabled(b);
    d->imagePanIconWidget->setEnabled(b);
    d->separateView->setEnabled(b);
    d->zoomBar->setEnabled(b);
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

void ImagePannelWidget::setProgressWhatsThis(const QString& desc)
{
    QWhatsThis::add( d->progressBar, desc);
}

void ImagePannelWidget::setPreviewImageWaitCursor(bool enable)
{
    if ( enable )
       d->imageRegionWidget->setCursor( KCursor::waitCursor() );
    else 
       d->imageRegionWidget->unsetCursor();
}

QRect ImagePannelWidget::getOriginalImageRegion()
{
    return ( d->imageRegionWidget->getImageRegion() );
}

QRect ImagePannelWidget::getOriginalImageRegionToRender()
{
    return ( d->imageRegionWidget->getImageRegionToRender() );
}

DImg ImagePannelWidget::getOriginalRegionImage()
{
    return ( d->imageRegionWidget->getImageRegionImage() );
}

void ImagePannelWidget::setPreviewImage(DImg img)
{
    d->imageRegionWidget->updatePreviewImage(&img);
}    

void ImagePannelWidget::setCenterImageRegionPosition()
{
    d->imageRegionWidget->setCenterContentsPosition();
}

void ImagePannelWidget::slotSetImageRegionPosition(const QRect& rect, bool targetDone)
{
    d->imageRegionWidget->setContentsPosition(rect.x(), rect.y(), targetDone);
}

void ImagePannelWidget::updateSelectionInfo(const QRect& rect)
{
    QToolTip::add( d->imagePanIconWidget,
                   i18n("<nobr>(%1,%2)(%3x%4)</nobr>")
                        .arg(rect.left()).arg(rect.top())
                        .arg(rect.width()).arg(rect.height()));
}

}  // NameSpace Digikam
