/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-01
 * Description : a widget to draw a control panel image tool.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepanelwidget.moc"

// Qt includes

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QResizeEvent>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "thumbnailsize.h"
#include "imageregionwidget.h"

namespace Digikam
{

class ImagePanelWidgetPriv
{
public:

    ImagePanelWidgetPriv()
    {
        imageRegionWidget  = 0;
        separateView       = 0;
        sepaBBox           = 0;
    }

    QString             settingsSection;

    QButtonGroup*       separateView;

    QWidget*            previewWidget;
    QWidget*            sepaBBox;

    ImageRegionWidget*  imageRegionWidget;
};

ImagePanelWidget::ImagePanelWidget(uint w, uint h, const QString& settingsSection,
                                   QWidget *parent, int separateViewMode)
                : QWidget(parent), d(new ImagePanelWidgetPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    d->settingsSection    = settingsSection;
    QGridLayout *grid     = new QGridLayout(this);

    // -------------------------------------------------------------

    QFrame *preview = new QFrame(this);
    QVBoxLayout* l1 = new QVBoxLayout(preview);
    l1->setSpacing(5);
    l1->setMargin(0);
    d->imageRegionWidget = new ImageRegionWidget(w, h, preview);
    d->imageRegionWidget->setFrameStyle(QFrame::NoFrame);
    preview->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    d->imageRegionWidget->setWhatsThis( i18n("<p>Here you can see the original clip image "
                                             "which will be used for the preview computation.</p>"
                                             "<p>Click and drag the mouse cursor in the "
                                             "image to change the clip focus.</p>"));
    l1->addWidget(d->imageRegionWidget, 0);

    // -------------------------------------------------------------

    d->sepaBBox       = new QWidget(this);
    QHBoxLayout *hlay = new QHBoxLayout(d->sepaBBox);
    d->separateView   = new QButtonGroup(d->sepaBBox);
    d->separateView->setExclusive(true);
    hlay->setSpacing(0);
    hlay->setMargin(0);

    if (separateViewMode == SeparateViewDuplicate ||
        separateViewMode == SeparateViewAll)
    {
        QToolButton *duplicateHorButton = new QToolButton( d->sepaBBox );
       d->separateView->addButton(duplicateHorButton, ImageRegionWidget::SeparateViewDuplicateHorz);
       hlay->addWidget(duplicateHorButton);
       duplicateHorButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/duplicatebothhorz.png")));
       duplicateHorButton->setCheckable(true);
       duplicateHorButton->setWhatsThis( i18n("If this option is enabled, the preview area will be split "
                                              "horizontally, displaying the original and target image "
                                              "at the same time. The target is duplicated from the original "
                                              "below the red dashed line." ) );

       QToolButton *duplicateVerButton = new QToolButton( d->sepaBBox );
       d->separateView->addButton(duplicateVerButton, ImageRegionWidget::SeparateViewDuplicateVert);
       hlay->addWidget(duplicateVerButton);
       duplicateVerButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/duplicatebothvert.png")));
       duplicateVerButton->setCheckable(true);
       duplicateVerButton->setWhatsThis( i18n("If this option is enabled, the preview area will be split "
                                              "vertically, displaying the original and target image "
                                              "at the same time. The target is duplicated from the original to "
                                              "the right of the red dashed line." ) );
    }

    if (separateViewMode == SeparateViewNormal ||
        separateViewMode == SeparateViewAll)
    {
        QToolButton *separateHorButton = new QToolButton( d->sepaBBox );
       d->separateView->addButton(separateHorButton, ImageRegionWidget::SeparateViewHorizontal);
       hlay->addWidget(separateHorButton);
       separateHorButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/bothhorz.png")));
       separateHorButton->setCheckable(true);
       separateHorButton->setWhatsThis( i18n( "If this option is enabled, the preview area will be split "
                                              "horizontally, displaying the original and target image "
                                              "at the same time. The original is above the "
                                              "red dashed line, the target below it." ) );

       QToolButton *separateVerButton = new QToolButton( d->sepaBBox );
       d->separateView->addButton(separateVerButton, ImageRegionWidget::SeparateViewVertical);
       hlay->addWidget(separateVerButton);
       separateVerButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/bothvert.png")));
       separateVerButton->setCheckable(true);
       separateVerButton->setWhatsThis( i18n( "If this option is enabled, the preview area will be split "
                                              "vertically, displaying the original and target image "
                                              "at the same time. The original is to the left of the "
                                              "red dashed line, the target to the right of it." ) );
    }

    QToolButton *noSeparateButton = new QToolButton( d->sepaBBox );
    d->separateView->addButton(noSeparateButton, ImageRegionWidget::SeparateViewNone);
    hlay->addWidget(noSeparateButton);
    noSeparateButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/target.png")));
    noSeparateButton->setCheckable(true);
    noSeparateButton->setWhatsThis( i18n( "If this option is enabled, the preview area will not "
                                          "be split into two." ) );

    // -------------------------------------------------------------

    grid->addWidget(preview,        0, 0, 2, 5);
    grid->addWidget(d->sepaBBox,    2, 2, 1, 3);
    grid->setRowStretch(1, 10);
    grid->setColumnStretch(1, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotInitGui()));

    // -------------------------------------------------------------

    connect(d->imageRegionWidget, SIGNAL(signalContentsMovedEvent(bool)),
            this, SLOT(slotOriginalImageRegionChanged(bool)));

    connect(d->imageRegionWidget, SIGNAL(signalSelectionTakeFocus()),
            this, SLOT(slotSelectionTakeFocus()));

    connect(d->separateView, SIGNAL(buttonReleased(int)),
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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->settingsSection);
    int mode                  = group.readEntry("Separate View", (int)ImageRegionWidget::SeparateViewDuplicateVert);
    mode                      = qMax((int)ImageRegionWidget::SeparateViewHorizontal, mode);
    mode                      = qMin((int)ImageRegionWidget::SeparateViewDuplicateHorz, mode);

    d->imageRegionWidget->blockSignals(true);
    d->separateView->blockSignals(true);
    d->imageRegionWidget->slotSeparateViewToggled( mode );
    d->separateView->button(mode)->setChecked(true);
    d->imageRegionWidget->blockSignals(false);
    d->separateView->blockSignals(false);
}

void ImagePanelWidget::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->settingsSection);
    group.writeEntry("Separate View", d->separateView->checkedId());
    config->sync();
}

void ImagePanelWidget::slotOriginalImageRegionChanged(bool target)
{
    QRect rect = getOriginalImageRegion();

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

void ImagePanelWidget::slotSelectionTakeFocus()
{
    d->imageRegionWidget->restorePixmapRegion();
}

void ImagePanelWidget::setPanIconHighLightPoints(const QPolygon& pt)
{
    d->imageRegionWidget->setHighLightPoints(pt);
}

void ImagePanelWidget::setEnable(bool b)
{
    d->imageRegionWidget->setEnabled(b);
    d->sepaBBox->setEnabled(b);
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

void ImagePanelWidget::ICCSettingsChanged()
{
    d->imageRegionWidget->viewport()->repaint();
}

void ImagePanelWidget::exposureSettingsChanged()
{
    // NOTE : not yet managed here by imageRegionWidget.
}

}  // namespace Digikam
