/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-01
 * Description : a widget to display an image preview with some 
 *               modes to compare effect results.
 * 
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
 

#include <QLayout>
#include <QFrame>
#include <QButtonGroup>
#include <QPushButton>
#include <QGridLayout>
#include <QPixmap>
#include <QVBoxLayout>

// KDE includes.

#include <ksqueezedtextlabel.h>
#include <kdialog.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kglobal.h>

// Local includes

#include "ddebug.h"
#include "imagewidget.h"
#include "imagewidget.moc"

namespace Digikam
{

class ImageWidgetPriv
{
public:

    ImageWidgetPriv()
    {
        spotInfoLabel       = 0;
        previewButtons      = 0;
        underExposureButton = 0;
        overExposureButton  = 0;
        previewWidget       = 0;
        prevBBox            = 0;
        expoBBox            = 0;
    }

    QWidget            *prevBBox;
    QWidget            *expoBBox;

    QString             settingsSection;

    QButtonGroup       *previewButtons;

    QPushButton        *underExposureButton;
    QPushButton        *overExposureButton;

    KSqueezedTextLabel *spotInfoLabel;

    ImageGuideWidget   *previewWidget;
};

ImageWidget::ImageWidget(const QString& settingsSection, QWidget *parent, 
                         const QString& previewWhatsThis, bool prevModeOptions, 
                         int guideMode, bool guideVisible, bool useImageSelection)
           : QWidget(parent)
{
    d = new ImageWidgetPriv;
    d->settingsSection = settingsSection;

    // -------------------------------------------------------------
    
    QGridLayout* grid = new QGridLayout(this);
    setLayout(grid);

    d->spotInfoLabel = new KSqueezedTextLabel(this);
    d->spotInfoLabel->setAlignment(Qt::AlignRight);

    // -------------------------------------------------------------
    
    d->prevBBox       = new QWidget(this);
    d->previewButtons = new QButtonGroup(d->prevBBox);
    d->previewButtons->setExclusive(true);

    QPushButton *previewOriginalButton = new QPushButton( d->prevBBox );
    d->previewButtons->addButton(previewOriginalButton, ImageGuideWidget::PreviewOriginalImage);
    KGlobal::dirs()->addResourceType("original", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("original", "original.png");
    previewOriginalButton->setIcon( QPixmap( directory + "original.png" ) );
    previewOriginalButton->setCheckable(true);
    previewOriginalButton->setWhatsThis( i18n( "<p>If you enable this option, you will see "
                                               "the original image." ) );

    QPushButton *previewBothButtonVert = new QPushButton( d->prevBBox );
    d->previewButtons->addButton(previewBothButtonVert, ImageGuideWidget::PreviewBothImagesVertCont);
    KGlobal::dirs()->addResourceType("bothvert", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("bothvert", "bothvert.png");
    previewBothButtonVert->setIcon( QPixmap( directory + "bothvert.png" ) );
    previewBothButtonVert->setCheckable(true);
    previewBothButtonVert->setWhatsThis( i18n( "<p>If you enable this option, the preview area will "
                                               "be separated vertically. "
                                               "A contiguous area of the image will be shown, "
                                               "with one half from the original image, "
                                               "the other half from the target image.") );

    QPushButton *previewBothButtonHorz = new QPushButton( d->prevBBox );
    d->previewButtons->addButton(previewBothButtonHorz, ImageGuideWidget::PreviewBothImagesHorzCont);
    KGlobal::dirs()->addResourceType("bothhorz", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("bothhorz", "bothhorz.png");
    previewBothButtonHorz->setIcon( QPixmap( directory + "bothhorz.png" ) );
    previewBothButtonHorz->setCheckable(true);
    previewBothButtonHorz->setWhatsThis( i18n( "<p>If you enable this option, the preview area will "
                                               "be separated horizontally. "
                                               "A contiguous area of the image will be shown, "
                                               "with one half from the original image, "
                                               "the other half from the target image.") );

    QPushButton *previewDuplicateBothButtonVert = new QPushButton( d->prevBBox );
    d->previewButtons->addButton(previewDuplicateBothButtonVert, ImageGuideWidget::PreviewBothImagesVert);
    KGlobal::dirs()->addResourceType("duplicatebothvert", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("duplicatebothvert", "duplicatebothvert.png");
    previewDuplicateBothButtonVert->setIcon( QPixmap( directory + "duplicatebothvert.png" ) );
    previewDuplicateBothButtonVert->setCheckable(true);
    previewDuplicateBothButtonVert->setWhatsThis( i18n( "<p>If you enable this option, the preview area will "
                                                        "be separated vertically. "
                                                        "The same part of the original and the target image "
                                                        "will be shown side by side.") );

    QPushButton *previewDupplicateBothButtonHorz = new QPushButton( d->prevBBox );
    d->previewButtons->addButton(previewDupplicateBothButtonHorz, ImageGuideWidget::PreviewBothImagesHorz);
    KGlobal::dirs()->addResourceType("duplicatebothhorz", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("duplicatebothhorz", "duplicatebothhorz.png");
    previewDupplicateBothButtonHorz->setIcon( QPixmap( directory + "duplicatebothhorz.png" ) );
    previewDupplicateBothButtonHorz->setCheckable(true);
    previewDupplicateBothButtonHorz->setWhatsThis( i18n( "<p>If you enable this option, the preview area will "
                                                         "be separated horizontally. "
                                                         "The same part of the original and the target image "
                                                         "will be shown side by side.") );

    QPushButton *previewtargetButton = new QPushButton( d->prevBBox );
    d->previewButtons->addButton(previewtargetButton, ImageGuideWidget::PreviewTargetImage);
    KGlobal::dirs()->addResourceType("target", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("target", "target.png");
    previewtargetButton->setIcon( QPixmap( directory + "target.png" ) );
    previewtargetButton->setCheckable(true);
    previewtargetButton->setWhatsThis( i18n( "<p>If you enable this option, you will see "
                                             "the target image." ) );

    QPushButton *previewToggleMouseOverButton = new QPushButton( d->prevBBox );
    d->previewButtons->addButton(previewToggleMouseOverButton, ImageGuideWidget::PreviewToggleOnMouseOver);
    KGlobal::dirs()->addResourceType("togglemouseover", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("togglemouseover", "togglemouseover.png");
    previewToggleMouseOverButton->setIcon( QPixmap( directory + "togglemouseover.png" ) );
    previewToggleMouseOverButton->setCheckable(true);
    previewToggleMouseOverButton->setWhatsThis( i18n( "<p>If you enable this option, you will see "
                                                      "the original image when the mouse is over image area, "
                                                      "else the target image." ) );

    // -------------------------------------------------------------
    
    d->expoBBox                   = new QWidget(this);
    QButtonGroup *exposureButtons = new QButtonGroup(d->expoBBox);

    d->underExposureButton = new QPushButton(d->expoBBox);
    exposureButtons->addButton(d->underExposureButton, UnderExposure);
    d->underExposureButton->setIcon(SmallIcon("underexposure"));
    d->underExposureButton->setCheckable(true);
    d->underExposureButton->setWhatsThis( i18n("<p>Set this option on to display pure black "
                                               "over-colored on preview. This will help you to avoid "
                                               "under-exposing the image." ) );

    d->overExposureButton = new QPushButton(d->expoBBox);
    exposureButtons->addButton(d->overExposureButton, OverExposure);
    d->overExposureButton->setIcon(SmallIcon("overexposure"));
    d->overExposureButton->setCheckable(true);
    d->overExposureButton->setWhatsThis( i18n("<p>Set this option on to display pure white "
                                              "over-colored on preview. This will help you to avoid "
                                              "over-exposing the image." ) );

    // -------------------------------------------------------------
    
    QFrame *frame    = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l   = new QVBoxLayout(frame);
    l->setMargin(5);
    l->setSpacing(0);
    d->previewWidget = new ImageGuideWidget(480, 320, frame, guideVisible, 
                                            guideMode, Qt::red, 1, false, 
                                            useImageSelection);
    d->previewWidget->setWhatsThis( previewWhatsThis);
    l->addWidget(d->previewWidget, 0);

    // -------------------------------------------------------------
    
    grid->addWidget(d->prevBBox, 1, 1, 0, 0);
    grid->addWidget(d->spotInfoLabel, 1, 1, 1, 1);
    grid->addWidget(d->expoBBox, 1, 1, 2, 2);
    grid->addWidget(frame, 3, 3, 0, 2);
    grid->setColumnMinimumWidth(1, KDialog::spacingHint());
    grid->setRowMinimumHeight(0, KDialog::spacingHint());
    grid->setRowMinimumHeight(2, KDialog::spacingHint());
    grid->setRowStretch(3, 10);
    grid->setColumnStretch(1, 10);

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

    connect(d->underExposureButton, SIGNAL(toggled(bool)),
            d->previewWidget, SLOT(slotToggleUnderExposure(bool)));

    connect(d->overExposureButton, SIGNAL(toggled(bool)),
            d->previewWidget, SLOT(slotToggleOverExposure(bool)));

    // -------------------------------------------------------------
    
    if (prevModeOptions)
        readSettings();
    else
    {
        setRenderingPreviewMode(ImageGuideWidget::NoPreviewMode);
        d->spotInfoLabel->hide();
        d->prevBBox->hide();    
        d->expoBBox->hide();
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
    d->previewButtons->button(mode)->setChecked(true);
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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->settingsSection);

    d->underExposureButton->setChecked(group.readEntry("Under Exposure Indicator", false));
    d->overExposureButton->setChecked(group.readEntry("Over Exposure Indicator", false));

    int mode = group.readEntry("Separate View", (int)ImageGuideWidget::PreviewBothImagesVertCont);
    mode = qMax((int)ImageGuideWidget::PreviewOriginalImage, mode);
    mode = qMin((int)ImageGuideWidget::NoPreviewMode, mode);
    setRenderingPreviewMode(mode);
}
    
void ImageWidget::writeSettings(void)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->settingsSection);
    group.writeEntry("Separate View", getRenderingPreviewMode());
    group.writeEntry("Under Exposure Indicator", d->underExposureButton->isChecked());
    group.writeEntry("Over Exposure Indicator", d->overExposureButton->isChecked());
    config->sync();
}

}  // namespace Digikam

