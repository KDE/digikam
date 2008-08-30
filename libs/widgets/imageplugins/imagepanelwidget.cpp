/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-01
 * Description : a widget to draw a control panel image tool.
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
#include <kapplication.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kstandarddirs.h>

// Local includes.

#include "ddebug.h"
#include "thumbnailsize.h"
#include "imageregionwidget.h"
#include "imagepaniconwidget.h"
#include "imagepanelwidget.h"
#include "imagepanelwidget.moc"

namespace Digikam
{

class ImagePanelWidgetPriv
{
public:

    ImagePanelWidgetPriv()
    {
        imagePanIconWidget = 0;
        imageRegionWidget  = 0;
        separateView       = 0;
    }

    QString             settingsSection;

    QHButtonGroup      *separateView;

    ImagePanIconWidget *imagePanIconWidget;

    ImageRegionWidget  *imageRegionWidget;
};

ImagePanelWidget::ImagePanelWidget(uint w, uint h, const QString& settingsSection,
                                   ImagePanIconWidget *pan, QWidget *parent, int separateViewMode)
                : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePanelWidgetPriv;
    d->settingsSection    = settingsSection;
    d->imagePanIconWidget = pan;
    QGridLayout *grid     = new QGridLayout(this, 2, 3);

    // -------------------------------------------------------------

    QFrame *preview      = new QFrame(this);
    QVBoxLayout* l1      = new QVBoxLayout(preview, 5, 0);
    d->imageRegionWidget = new ImageRegionWidget(w, h, preview, false);
    d->imageRegionWidget->setFrameStyle(QFrame::NoFrame);
    preview->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QWhatsThis::add( d->imageRegionWidget, i18n("<p>Here you can see the original clip image "
                                                "which will be used for the preview computation."
                                                "<p>Click and drag the mouse cursor in the "
                                                "image to change the clip focus."));
    l1->addWidget(d->imageRegionWidget, 0);

    // -------------------------------------------------------------

    QString directory;
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

    grid->addMultiCellWidget(preview,         0, 1, 0, 3);
    grid->addMultiCellWidget(d->separateView, 2, 2, 3, 3);
    grid->setRowStretch(1, 10);
    grid->setColStretch(1, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotInitGui())); 

    // -------------------------------------------------------------

    connect(d->imageRegionWidget, SIGNAL(signalContentsMovedEvent(bool)),
            this, SLOT(slotOriginalImageRegionChanged(bool)));

    connect(d->imagePanIconWidget, SIGNAL(signalSelectionMoved(const QRect&, bool)),
            this, SLOT(slotSetImageRegionPosition(const QRect&, bool)));

    connect(d->imagePanIconWidget, SIGNAL(signalSelectionTakeFocus()),
            this, SLOT(slotPanIconTakeFocus()));

    connect(d->separateView, SIGNAL(released(int)),
            d->imagePanIconWidget, SLOT(slotSeparateViewToggled(int)));

    connect(d->separateView, SIGNAL(released(int)),
            d->imageRegionWidget, SLOT(slotSeparateViewToggled(int)));
}

ImagePanelWidget::~ImagePanelWidget()
{
    writeSettings();
    delete d;
}

ImageRegionWidget *ImagePanelWidget::previewWidget() const
{
    return d->imageRegionWidget;
}

void ImagePanelWidget::readSettings()
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

void ImagePanelWidget::writeSettings()
{
    KConfig *config = kapp->config();
    config->setGroup(d->settingsSection);
    config->writeEntry( "Separate View", d->separateView->selectedId() );
    config->sync();
}

void ImagePanelWidget::slotOriginalImageRegionChanged(bool target)
{
    d->imagePanIconWidget->slotZoomFactorChanged(d->imageRegionWidget->zoomFactor());
    QRect rect = getOriginalImageRegion();
    d->imagePanIconWidget->setRegionSelection(rect);
    updateSelectionInfo(rect);

    if (target)
    {
        d->imageRegionWidget->backupPixmapRegion();
        emit signalOriginalClipFocusChanged();
    }
}

void ImagePanelWidget::slotZoomSliderChanged(int size)
{
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->imageRegionWidget->zoomMin();
    double zmax = d->imageRegionWidget->zoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    double z    = a*size+b; 

    d->imageRegionWidget->setZoomFactorSnapped(z);
}

void ImagePanelWidget::resizeEvent(QResizeEvent *)
{
    emit signalResized();
}

void ImagePanelWidget::slotInitGui()
{
    readSettings();
    setCenterImageRegionPosition();
    slotOriginalImageRegionChanged(true);
}

void ImagePanelWidget::setPanIconHighLightPoints(const QPointArray& pt) 
{
    d->imageRegionWidget->setHighLightPoints(pt);
    d->imagePanIconWidget->setHighLightPoints(pt);
}

void ImagePanelWidget::slotPanIconTakeFocus()
{
    d->imageRegionWidget->restorePixmapRegion();
}

void ImagePanelWidget::setEnable(bool b)
{
    d->imageRegionWidget->setEnabled(b);
    d->separateView->setEnabled(b);
}

QRect ImagePanelWidget::getOriginalImageRegion()
{
    return ( d->imageRegionWidget->getImageRegion() );
}

QRect ImagePanelWidget::getOriginalImageRegionToRender()
{
    return ( d->imageRegionWidget->getImageRegionToRender() );
}

DImg ImagePanelWidget::getOriginalRegionImage()
{
    return ( d->imageRegionWidget->getImageRegionImage() );
}

void ImagePanelWidget::setPreviewImage(DImg img)
{
    d->imageRegionWidget->updatePreviewImage(&img);
    d->imageRegionWidget->repaintContents(false);
}

void ImagePanelWidget::setCenterImageRegionPosition()
{
    d->imageRegionWidget->setCenterContentsPosition();
}

void ImagePanelWidget::slotSetImageRegionPosition(const QRect& rect, bool targetDone)
{
    d->imageRegionWidget->setContentsPosition(rect.x(), rect.y(), targetDone);
}

void ImagePanelWidget::updateSelectionInfo(const QRect& rect)
{
    QToolTip::add( d->imagePanIconWidget,
                   i18n("<nobr>(%1,%2)(%3x%4)</nobr>")
                        .arg(rect.left()).arg(rect.top())
                        .arg(rect.width()).arg(rect.height()));
}

}  // NameSpace Digikam
