/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-01
 * Description : a widget to display an image preview with some 
 *               modes to compare effect results.
 * 
 * Copyright 2006 by Gilles Caulier
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
 
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qhbuttongroup.h>
#include <qpushbutton.h>

// KDE includes.

#include <ksqueezedtextlabel.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>

// Local includes

#include "imagewidget.h"

namespace Digikam
{

class ImageWidgetPriv
{
public:

    ImageWidgetPriv()
    {
        spotInfoLabel  = 0;
        previewButtons = 0;
        previewWidget  = 0;
    }

    QHButtonGroup      *previewButtons;

    KSqueezedTextLabel *spotInfoLabel;

    ImageGuideWidget   *previewWidget;
};

ImageWidget::ImageWidget(QWidget *parent, const QString& previewWhatsThis, 
                         bool prevModeOptions, int guideMode, bool guideVisible)
           : QWidget(parent)
{
    d = new ImageWidgetPriv;

    // -------------------------------------------------------------
    
    QGridLayout* grid = new QGridLayout(this, 1, 3);

    d->spotInfoLabel = new KSqueezedTextLabel(this);
    d->spotInfoLabel->setAlignment(Qt::AlignRight);

    // -------------------------------------------------------------
    
    d->previewButtons = new QHButtonGroup(this);
    d->previewButtons->setExclusive(true);
    d->previewButtons->setInsideMargin( 0 );
    d->previewButtons->setFrameShape(QFrame::NoFrame);

    QPushButton *previewOriginalButton = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewOriginalButton, ImageGuideWidget::PreviewOriginalImage);
    KGlobal::dirs()->addResourceType("original", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("original", "original.png");
    previewOriginalButton->setPixmap( QPixmap( directory + "original.png" ) );
    previewOriginalButton->setToggleButton(true);
    QWhatsThis::add( previewOriginalButton, i18n( "<p>If you enable this option, you will see "
                                                  "the original image." ) );

    QPushButton *previewBothButtonVert = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewBothButtonVert, ImageGuideWidget::PreviewBothImagesVertCont);
    KGlobal::dirs()->addResourceType("bothvert", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("bothvert", "bothvert.png");
    previewBothButtonVert->setPixmap( QPixmap( directory + "bothvert.png" ) );
    previewBothButtonVert->setToggleButton(true);
    QWhatsThis::add( previewBothButtonVert, i18n( "<p>If you enable this option, the preview area will "
                                                  "be separated vertically. The original and target images are contiguous" ) );

    QPushButton *previewBothButtonHorz = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewBothButtonHorz, ImageGuideWidget::PreviewBothImagesHorzCont);
    KGlobal::dirs()->addResourceType("bothhorz", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("bothhorz", "bothhorz.png");
    previewBothButtonHorz->setPixmap( QPixmap( directory + "bothhorz.png" ) );
    previewBothButtonHorz->setToggleButton(true);
    QWhatsThis::add( previewBothButtonHorz, i18n( "<p>If you enable this option, the preview area will "
                                                  "be separated horizontally. The original and target images are contiguous" ) );

    QPushButton *previewDuplicateBothButtonVert = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewDuplicateBothButtonVert, ImageGuideWidget::PreviewBothImagesVert);
    KGlobal::dirs()->addResourceType("duplicatebothvert", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("duplicatebothvert", "duplicatebothvert.png");
    previewDuplicateBothButtonVert->setPixmap( QPixmap( directory + "duplicatebothvert.png" ) );
    previewDuplicateBothButtonVert->setToggleButton(true);
    QWhatsThis::add( previewDuplicateBothButtonVert, i18n( "<p>If you enable this option, the preview area will "
                                                           "be separated vertically. The original and target images are duplicated" ) );

    QPushButton *previewDupplicateBothButtonHorz = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewDupplicateBothButtonHorz, ImageGuideWidget::PreviewBothImagesHorz);
    KGlobal::dirs()->addResourceType("duplicatebothhorz", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("duplicatebothhorz", "duplicatebothhorz.png");
    previewDupplicateBothButtonHorz->setPixmap( QPixmap( directory + "duplicatebothhorz.png" ) );
    previewDupplicateBothButtonHorz->setToggleButton(true);
    QWhatsThis::add( previewDupplicateBothButtonHorz, i18n( "<p>If you enable this option, the preview area will "
                                                            "be separated horizontally. The original and target images are duplicated" ) );

    QPushButton *previewtargetButton = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewtargetButton, ImageGuideWidget::PreviewTargetImage);
    KGlobal::dirs()->addResourceType("target", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("target", "target.png");
    previewtargetButton->setPixmap( QPixmap( directory + "target.png" ) );
    previewtargetButton->setToggleButton(true);
    QWhatsThis::add( previewtargetButton, i18n( "<p>If you enable this option, you will see "
                                                "the target image." ) );

    QPushButton *previewToogleMouseOverButton = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewToogleMouseOverButton, ImageGuideWidget::PreviewToogleOnMouseOver);
    KGlobal::dirs()->addResourceType("tooglemouseover", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("tooglemouseover", "tooglemouseover.png");
    previewToogleMouseOverButton->setPixmap( QPixmap( directory + "tooglemouseover.png" ) );
    previewToogleMouseOverButton->setToggleButton(true);
    QWhatsThis::add( previewToogleMouseOverButton, i18n( "<p>If you enable this option, you will see "
                                                         "the original image when the mouse is over image area, "
                                                         "else the target image." ) );

    // -------------------------------------------------------------
    
    QFrame *frame    = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l   = new QVBoxLayout(frame, 5, 0);
    d->previewWidget = new ImageGuideWidget(480, 320, frame, guideVisible, 
                                           guideMode,
                                           Qt::red, 1, false);
    QWhatsThis::add( d->previewWidget, previewWhatsThis);
    l->addWidget(d->previewWidget, 0);

    // -------------------------------------------------------------
    
    grid->setRowSpacing(0, KDialog::spacingHint());
    grid->addMultiCellWidget(d->previewButtons, 1, 1, 0, 0);
    grid->addMultiCellWidget(d->spotInfoLabel, 1, 1, 1, 1);
    grid->setRowSpacing(2, KDialog::spacingHint());
    grid->addMultiCellWidget(frame, 3, 3, 0, 1);
    grid->setRowStretch(3, 10);
    grid->setColStretch(1, 10);

    // -------------------------------------------------------------
    
    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SIGNAL(signalResized()));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )),
            this, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotUpdateSpotInfo( const Digikam::DColor &, const QPoint & )));
    
    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotUpdateSpotInfo( const Digikam::DColor &, const QPoint & )));

    connect(d->previewButtons, SIGNAL(released(int)),
            d->previewWidget, SLOT(slotChangeRenderingPreviewMode(int)));

    // -------------------------------------------------------------
    
    if (prevModeOptions)
        setRenderingPreviewMode(ImageGuideWidget::PreviewBothImagesVertCont);
    else
    {
        setRenderingPreviewMode(ImageGuideWidget::NoPreviewMode);
        d->spotInfoLabel->hide();
        d->previewButtons->hide();    
    }     
}

ImageWidget::~ImageWidget()
{
    delete d;
}

ImageIface* ImageWidget::imageIface()
{
    return d->previewWidget->imageIface();
}

void ImageWidget::updatePreview()
{
    d->previewWidget->updatePreview();
}

void ImageWidget::slotChangeGuideColor(const QColor &color)
{
    d->previewWidget->slotChangeGuideColor(color);
}

void ImageWidget::slotChangeGuideSize(int size)
{
    d->previewWidget->slotChangeGuideSize(size);
}

void ImageWidget::resetSpotPosition()
{
    d->previewWidget->resetSpotPosition();
}

QPoint ImageWidget::getSpotPosition(void)
{
    return ( d->previewWidget->getSpotPosition() );
}

DColor ImageWidget::getSpotColor(int getColorFrom)
{
    return ( d->previewWidget->getSpotColor(getColorFrom) );
}

void ImageWidget::setSpotVisible(bool spotVisible, bool blink)
{
    d->previewWidget->setSpotVisible(spotVisible, blink);
}

int ImageWidget::getRenderingPreviewMode()
{
    return ( d->previewWidget->getRenderingPreviewMode() );
}
    
void ImageWidget::setRenderingPreviewMode(int mode)
{
    d->previewButtons->setButton(mode);
    d->previewWidget->slotChangeRenderingPreviewMode(mode);
}

void ImageWidget::slotUpdateSpotInfo(const Digikam::DColor &col, const QPoint &point)
{
    DColor color = col;
    d->spotInfoLabel->setText(i18n("(%1,%2) RGBA:%3,%4,%5,%6")
                             .arg(point.x()).arg(point.y())
                             .arg(color.red()).arg(color.green())
                             .arg(color.blue()).arg(color.alpha()) );
}

}  // namespace Digikam

#include "imagewidget.moc"
