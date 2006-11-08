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
#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>

// Local includes

#include "ddebug.h"
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

    QString             settingsSection;

    QHButtonGroup      *previewButtons;

    KSqueezedTextLabel *spotInfoLabel;

    ImageGuideWidget   *previewWidget;
};

ImageWidget::ImageWidget(const QString& settingsSection, QWidget *parent, 
                         const QString& previewWhatsThis, bool prevModeOptions, 
                         int guideMode, bool guideVisible)
           : QWidget(parent)
{
    d = new ImageWidgetPriv;
    d->settingsSection = settingsSection;

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
                                                  "be separated vertically. "
                                                  "A contiguous area of the image will be shown, "
                                                  "with one half from the original image, "
                                                  "the other half from the target image.") );

    QPushButton *previewBothButtonHorz = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewBothButtonHorz, ImageGuideWidget::PreviewBothImagesHorzCont);
    KGlobal::dirs()->addResourceType("bothhorz", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("bothhorz", "bothhorz.png");
    previewBothButtonHorz->setPixmap( QPixmap( directory + "bothhorz.png" ) );
    previewBothButtonHorz->setToggleButton(true);
    QWhatsThis::add( previewBothButtonHorz, i18n( "<p>If you enable this option, the preview area will "
                                                  "be separated horizontally. "
                                                  "A contiguous area of the image will be shown, "
                                                  "with one half from the original image, "
                                                  "the other half from the target image.") );

    QPushButton *previewDuplicateBothButtonVert = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewDuplicateBothButtonVert, ImageGuideWidget::PreviewBothImagesVert);
    KGlobal::dirs()->addResourceType("duplicatebothvert", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("duplicatebothvert", "duplicatebothvert.png");
    previewDuplicateBothButtonVert->setPixmap( QPixmap( directory + "duplicatebothvert.png" ) );
    previewDuplicateBothButtonVert->setToggleButton(true);
    QWhatsThis::add( previewDuplicateBothButtonVert, i18n( "<p>If you enable this option, the preview area will "
                                                           "be separated vertically. "
                                                           "The same part of the original and the target image "
                                                           "will be shown side by side.") );

    QPushButton *previewDupplicateBothButtonHorz = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewDupplicateBothButtonHorz, ImageGuideWidget::PreviewBothImagesHorz);
    KGlobal::dirs()->addResourceType("duplicatebothhorz", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("duplicatebothhorz", "duplicatebothhorz.png");
    previewDupplicateBothButtonHorz->setPixmap( QPixmap( directory + "duplicatebothhorz.png" ) );
    previewDupplicateBothButtonHorz->setToggleButton(true);
    QWhatsThis::add( previewDupplicateBothButtonHorz, i18n( "<p>If you enable this option, the preview area will "
                                                            "be separated horizontally. "
                                                            "The same part of the original and the target image "
                                                            "will be shown side by side.") );

    QPushButton *previewtargetButton = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewtargetButton, ImageGuideWidget::PreviewTargetImage);
    KGlobal::dirs()->addResourceType("target", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("target", "target.png");
    previewtargetButton->setPixmap( QPixmap( directory + "target.png" ) );
    previewtargetButton->setToggleButton(true);
    QWhatsThis::add( previewtargetButton, i18n( "<p>If you enable this option, you will see "
                                                "the target image." ) );

    QPushButton *previewToggleMouseOverButton = new QPushButton( d->previewButtons );
    d->previewButtons->insert(previewToggleMouseOverButton, ImageGuideWidget::PreviewToggleOnMouseOver);
    KGlobal::dirs()->addResourceType("togglemouseover", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("togglemouseover", "togglemouseover.png");
    previewToggleMouseOverButton->setPixmap( QPixmap( directory + "togglemouseover.png" ) );
    previewToggleMouseOverButton->setToggleButton(true);
    QWhatsThis::add( previewToggleMouseOverButton, i18n( "<p>If you enable this option, you will see "
                                                         "the original image when the mouse is over image area, "
                                                         "else the target image." ) );

    // -------------------------------------------------------------
    
    QFrame *frame    = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l   = new QVBoxLayout(frame, 5, 0);
    d->previewWidget = new ImageGuideWidget(480, 320, frame, guideVisible, 
                                            guideMode, Qt::red, 1, false);
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
        readSettings();
    else
    {
        setRenderingPreviewMode(ImageGuideWidget::NoPreviewMode);
        d->spotInfoLabel->hide();
        d->previewButtons->hide();    
    }     
}

ImageWidget::~ImageWidget()
{
    writeSettings();
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

void ImageWidget::readSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup(d->settingsSection);
    int mode = config->readNumEntry("Separate View", ImageGuideWidget::PreviewBothImagesVertCont);
    mode = QMAX(ImageGuideWidget::PreviewOriginalImage, mode);
    mode = QMIN(ImageGuideWidget::NoPreviewMode, mode);
    setRenderingPreviewMode(mode);
}
    
void ImageWidget::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup(d->settingsSection);
    config->writeEntry( "Separate View", getRenderingPreviewMode() );
    config->sync();
}

}  // namespace Digikam

#include "imagewidget.moc"
