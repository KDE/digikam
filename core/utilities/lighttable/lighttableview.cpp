/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : a widget to display 2 preview image on
 *               lightable to compare pictures.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttableview.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QApplication>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "dzoombar.h"
#include "thumbnailsize.h"
#include "lighttablepreview.h"
#include "previewlayout.h"
#include "dimgpreviewitem.h"

namespace Digikam
{

class LightTableView::Private
{
public:

    Private() :
        syncPreview(false),
        grid(0),
        leftFrame(0),
        rightFrame(0),
        leftPreview(0),
        rightPreview(0)
    {
    }

    bool               syncPreview;

    QGridLayout*       grid;

    // These labels are used to draw a frame around preview views to identify easily which item has the focus.
    QLabel*            leftFrame;
    QLabel*            rightFrame;

    LightTablePreview* leftPreview;
    LightTablePreview* rightPreview;
};

LightTableView::LightTableView(QWidget* const parent)
    : QFrame(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::NoFrame);
    setLineWidth(0);

    d->grid                 = new QGridLayout();
    setLayout(d->grid);

    d->leftFrame            = new QLabel(this);
    d->leftPreview          = new LightTablePreview(this);
    QVBoxLayout* const llay = new QVBoxLayout(d->leftFrame);
    llay->addWidget(d->leftPreview);
    llay->setContentsMargins(3, 3, 3, 3);
    llay->setSpacing(0);

    d->rightFrame           = new QLabel(this);
    d->rightPreview         = new LightTablePreview(this);
    QVBoxLayout* const rlay = new QVBoxLayout(d->rightFrame);
    rlay->addWidget(d->rightPreview);
    rlay->setContentsMargins(3, 3, 3, 3);
    rlay->setSpacing(0);

    d->grid->addWidget(d->leftFrame,  0, 0, 1, 1);
    d->grid->addWidget(d->rightFrame, 0, 1, 1, 1);
    d->grid->setColumnStretch(0, 10);
    d->grid->setColumnStretch(1, 10);
    d->grid->setRowStretch(0, 10);

    // Left panel connections ------------------------------------------------

    connect(d->leftPreview, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalLeftPopupTagsView()));

    connect(d->leftPreview->layout(), SIGNAL(zoomFactorChanged(double)),
            this, SLOT(slotLeftZoomFactorChanged(double)));

    connect(d->leftPreview, SIGNAL(contentsMoving(int,int)),
            this, SLOT(slotLeftContentsMoved(int,int)));

    connect(d->leftPreview, SIGNAL(signalSlideShow()),
            this, SIGNAL(signalSlideShow()));

    connect(d->leftPreview, SIGNAL(signalSlideShowCurrent()),
            this, SIGNAL(signalLeftSlideShowCurrent()));

    connect(d->leftPreview, SIGNAL(signalDroppedItems(ImageInfoList)),
            this, SIGNAL(signalLeftDroppedItems(ImageInfoList)));

    connect(d->leftPreview, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotLeftPreviewLoaded(bool)));

    connect(d->leftPreview, SIGNAL(leftButtonClicked()),
            this, SIGNAL(signalLeftPanelLeftButtonClicked()));

    connect(d->leftPreview, SIGNAL(signalDeleteItem()),
            this, SLOT(slotDeleteLeftItem()));

    // Right panel connections ------------------------------------------------

    connect(d->rightPreview, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalRightPopupTagsView()));

    connect(d->rightPreview->layout(), SIGNAL(zoomFactorChanged(double)),
            this, SLOT(slotRightZoomFactorChanged(double)));

    connect(d->rightPreview, SIGNAL(contentsMoving(int,int)),
            this, SLOT(slotRightContentsMoved(int,int)));

    connect(d->rightPreview, SIGNAL(signalDroppedItems(ImageInfoList)),
            this, SIGNAL(signalRightDroppedItems(ImageInfoList)));

    connect(d->rightPreview, SIGNAL(signalSlideShow()),
            this, SIGNAL(signalSlideShow()));

    connect(d->rightPreview, SIGNAL(signalSlideShowCurrent()),
            this, SIGNAL(signalRightSlideShowCurrent()));

    connect(d->rightPreview, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotRightPreviewLoaded(bool)));

    connect(d->rightPreview, SIGNAL(leftButtonClicked()),
            this, SIGNAL(signalRightPanelLeftButtonClicked()));

    connect(d->rightPreview, SIGNAL(signalDeleteItem()),
            this, SLOT(slotDeleteRightItem()));
}

LightTableView::~LightTableView()
{
    delete d;
}

void LightTableView::setPreviewSettings(const PreviewSettings& settings)
{
    d->leftPreview->previewItem()->setPreviewSettings(settings);
    d->rightPreview->previewItem()->setPreviewSettings(settings);
}

void LightTableView::setSyncPreview(bool sync)
{
    d->syncPreview = sync;

    // Left panel like a reference to resync preview.
    if (d->syncPreview)
    {
        slotLeftZoomFactorChanged(d->leftPreview->layout()->zoomFactor());
        slotLeftContentsMoved(d->leftPreview->contentsX(),
                              d->leftPreview->contentsY());
    }
}

void LightTableView::setNavigateByPair(bool b)
{
    d->leftPreview->setDragAndDropEnabled(!b);
    d->rightPreview->setDragAndDropEnabled(!b);
}

void LightTableView::slotDecreaseLeftZoom()
{
    d->leftPreview->layout()->decreaseZoom();
}

void LightTableView::slotIncreaseLeftZoom()
{
    d->leftPreview->layout()->increaseZoom();
}

void LightTableView::slotDecreaseRightZoom()
{
    d->rightPreview->layout()->decreaseZoom();
}

void LightTableView::slotIncreaseRightZoom()
{
    d->rightPreview->layout()->increaseZoom();
}

void LightTableView::setLeftZoomFactor(double z)
{
    d->leftPreview->layout()->setZoomFactor(z);
}

void LightTableView::setRightZoomFactor(double z)
{
    d->rightPreview->layout()->setZoomFactor(z);
}

void LightTableView::slotLeftZoomTo100()
{
    d->leftPreview->layout()->toggleFitToWindowOr100();
}

void LightTableView::slotRightZoomTo100()
{
    d->rightPreview->layout()->toggleFitToWindowOr100();
}

void LightTableView::slotLeftFitToWindow()
{
    d->leftPreview->layout()->fitToWindow();
}

void LightTableView::slotRightFitToWindow()
{
    d->rightPreview->layout()->fitToWindow();
}

double LightTableView::leftZoomMax() const
{
    return d->leftPreview->layout()->maxZoomFactor();
}

double LightTableView::leftZoomMin() const
{
    return d->leftPreview->layout()->minZoomFactor();
}

bool LightTableView::leftMaxZoom() const
{
    return d->leftPreview->layout()->atMaxZoom();
}

bool LightTableView::leftMinZoom() const
{
    return d->leftPreview->layout()->atMinZoom();
}

double LightTableView::rightZoomMax() const
{
    return d->rightPreview->layout()->maxZoomFactor();
}

double LightTableView::rightZoomMin() const
{
    return d->rightPreview->layout()->minZoomFactor();
}

bool LightTableView::rightMaxZoom() const
{
    return d->rightPreview->layout()->atMaxZoom();
}

bool LightTableView::rightMinZoom() const
{
    return d->rightPreview->layout()->atMinZoom();
}

void LightTableView::slotLeftZoomSliderChanged(int size)
{
    double zmin = d->leftPreview->layout()->minZoomFactor();
    double zmax = d->leftPreview->layout()->maxZoomFactor();
    double z    = DZoomBar::zoomFromSize(size, zmin, zmax);
    d->leftPreview->layout()->setZoomFactorSnapped(z);
}

void LightTableView::slotRightZoomSliderChanged(int size)
{
    double zmin = d->rightPreview->layout()->minZoomFactor();
    double zmax = d->rightPreview->layout()->maxZoomFactor();
    double z    = DZoomBar::zoomFromSize(size, zmin, zmax);
    d->rightPreview->layout()->setZoomFactorSnapped(z);
}

void LightTableView::leftReload()
{
    d->leftPreview->previewItem()->reload();
}

void LightTableView::rightReload()
{
    d->rightPreview->previewItem()->reload();
}

void LightTableView::slotLeftContentsMoved(int x, int y)
{
    if (d->syncPreview && !leftPreviewLoading())
    {
        d->rightPreview->blockSignals(true);
        d->rightPreview->setContentsPos(x, y);
        d->rightPreview->blockSignals(false);
    }
}

void LightTableView::slotRightContentsMoved(int x, int y)
{
    if (d->syncPreview && !rightPreviewLoading())
    {
        d->leftPreview->blockSignals(true);
        d->leftPreview->setContentsPos(x, y);
        d->leftPreview->blockSignals(false);
    }
}

void LightTableView::slotLeftZoomFactorChanged(double zoom)
{
    if (d->syncPreview && !leftPreviewLoading())
    {
        d->rightPreview->layout()->blockSignals(true);
        d->rightPreview->blockSignals(true);

        setRightZoomFactor(zoom);
        emit signalRightZoomFactorChanged(zoom);

        d->rightPreview->blockSignals(false);
        d->rightPreview->layout()->blockSignals(false);
    }

    emit signalLeftZoomFactorChanged(zoom);
}

void LightTableView::slotRightZoomFactorChanged(double zoom)
{
    if (d->syncPreview && !rightPreviewLoading())
    {
        d->leftPreview->layout()->blockSignals(true);
        d->leftPreview->blockSignals(true);

        setLeftZoomFactor(zoom);
        emit signalLeftZoomFactorChanged(zoom);

        d->leftPreview->blockSignals(false);
        d->leftPreview->layout()->blockSignals(false);
    }

    emit signalRightZoomFactorChanged(zoom);
}

ImageInfo LightTableView::leftImageInfo() const
{
    return d->leftPreview->getImageInfo();
}

ImageInfo LightTableView::rightImageInfo() const
{
    return d->rightPreview->getImageInfo();
}

void LightTableView::setLeftImageInfo(const ImageInfo& info)
{
    d->leftPreview->setImageInfo(info);

    if (info.isNull())
    {
        d->leftPreview->showDragAndDropMessage();
    }
}

void LightTableView::setRightImageInfo(const ImageInfo& info)
{
    d->rightPreview->setImageInfo(info);

    if (info.isNull())
    {
        d->rightPreview->showDragAndDropMessage();
    }
}

void LightTableView::slotLeftPreviewLoaded(bool success)
{
    checkForSyncPreview();
    slotRightContentsMoved(d->rightPreview->contentsX(), d->rightPreview->contentsY());

    emit signalLeftPreviewLoaded(success);
}

void LightTableView::slotRightPreviewLoaded(bool success)
{
    checkForSyncPreview();
    slotLeftContentsMoved(d->leftPreview->contentsX(), d->leftPreview->contentsY());

    emit signalRightPreviewLoaded(success);
}

void LightTableView::checkForSyncPreview()
{
    if (!d->leftPreview->getImageInfo().isNull()  &&
        !d->rightPreview->getImageInfo().isNull() &&
        d->leftPreview->previewItem()->image().size() == d->rightPreview->previewItem()->image().size())
    {
        d->syncPreview = true;
    }
    else
    {
        d->syncPreview = false;
    }

    emit signalToggleOnSyncPreview(d->syncPreview);
}

void LightTableView::checkForSelection(const ImageInfo& info)
{
    QString selected    = QString::fromUtf8("QLabel { background-color: %1; }")
                          .arg(qApp->palette().color(QPalette::Highlight).name());

    QString notSelected = QString::fromUtf8("QLabel { background-color: %1; }")
                          .arg(qApp->palette().color(QPalette::Base).name());

    if (info.isNull())
    {
        d->leftFrame->setStyleSheet(notSelected);
        d->rightFrame->setStyleSheet(notSelected);
        return;
    }

    if (!d->leftPreview->getImageInfo().isNull())
    {
        bool onLeft = (d->leftPreview->getImageInfo() == info);
        d->leftFrame->setStyleSheet(onLeft ? selected : notSelected);
    }

    if (!d->rightPreview->getImageInfo().isNull())
    {
        bool onRight = (d->rightPreview->getImageInfo() == info);
        d->rightFrame->setStyleSheet(onRight ? selected : notSelected);
    }
}

void LightTableView::slotDeleteLeftItem()
{
    emit signalDeleteItem(d->leftPreview->getImageInfo());
}

void LightTableView::slotDeleteRightItem()
{
    emit signalDeleteItem(d->rightPreview->getImageInfo());
}

bool LightTableView::leftPreviewLoading() const
{
    return (d->leftPreview->previewItem()->state() == DImgPreviewItem::Loading);
}

bool LightTableView::rightPreviewLoading() const
{
    return (d->rightPreview->previewItem()->state() == DImgPreviewItem::Loading);
}

void LightTableView::toggleFullScreen(bool set)
{
    d->leftPreview->toggleFullScreen(set);
    d->rightPreview->toggleFullScreen(set);
}

}  // namespace Digikam
