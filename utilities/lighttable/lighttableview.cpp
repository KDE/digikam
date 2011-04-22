/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : a widget to display 2 preview image on
 *               lightable to compare pictures.
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttableview.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>

// KDE includes

#include <kdialog.h>
#include <kdebug.h>
#include <kapplication.h>

// Local includes

#include "dimg.h"
#include "dzoombar.h"
#include "thumbnailsize.h"
#include "lighttablepreview.h"
#include "previewlayout.h"
#include "dimgpreviewitem.h"

namespace Digikam
{

class LightTableView::LightTableViewPriv
{
public:

    LightTableViewPriv() :
        syncPreview(false),
        leftLoading(false),
        rightLoading(false),
        grid(0),
        leftFrame(0),
        rightFrame(0),
        leftPreview(0),
        rightPreview(0)
    {
    }

    bool               syncPreview;
    bool               leftLoading;     // To not sync right panel during left loading.
    bool               rightLoading;    // To not sync left panel during right loading.

    QGridLayout*       grid;

    QLabel*            leftFrame;
    QLabel*            rightFrame;

    LightTablePreview* leftPreview;
    LightTablePreview* rightPreview;
};

LightTableView::LightTableView(QWidget* parent)
    : QFrame(parent), d(new LightTableViewPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::NoFrame);
    setLineWidth(0);

    d->grid           = new QGridLayout();
    setLayout(d->grid);

    d->leftFrame      = new QLabel(this);
    d->leftPreview    = new LightTablePreview(this);
    QVBoxLayout* llay = new QVBoxLayout(d->leftFrame);
    llay->addWidget(d->leftPreview);
    llay->setMargin(3);
    llay->setSpacing(0);

    d->rightFrame     = new QLabel(this);
    d->rightPreview   = new LightTablePreview(this);
    QVBoxLayout* rlay = new QVBoxLayout(d->rightFrame);
    rlay->addWidget(d->rightPreview);
    rlay->setMargin(3);
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
            this, SIGNAL(signalLeftZoomFactorChanged(double)));

    connect(d->leftPreview, SIGNAL(contentsMoving(int, int)),
            this, SLOT(slotLeftContentsMoved(int, int)));

    connect(d->leftPreview, SIGNAL(signalSlideShow()),
            this, SIGNAL(signalSlideShow()));

    connect(d->leftPreview, SIGNAL(signalDroppedItems(const ImageInfoList&)),
            this, SIGNAL(signalLeftDroppedItems(const ImageInfoList&)));

    connect(d->leftPreview, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotLeftPreviewLoaded(bool)));

    connect(d->leftPreview, SIGNAL(leftButtonClicked()),
            this, SIGNAL(signalLeftPanelLeftButtonClicked()));

    // Right panel connections ------------------------------------------------

    connect(d->rightPreview, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalRightPopupTagsView()));

    connect(d->rightPreview->layout(), SIGNAL(zoomFactorChanged(double)),
            this, SIGNAL(signalRightZoomFactorChanged(double)));

    connect(d->rightPreview, SIGNAL(contentsMoving(int, int)),
            this, SLOT(slotRightContentsMoved(int, int)));

    connect(d->rightPreview, SIGNAL(signalDroppedItems(const ImageInfoList&)),
            this, SIGNAL(signalRightDroppedItems(const ImageInfoList&)));

    connect(d->rightPreview, SIGNAL(signalSlideShow()),
            this, SIGNAL(signalSlideShow()));

    connect(d->rightPreview, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotRightPreviewLoaded(bool)));

    connect(d->rightPreview, SIGNAL(leftButtonClicked()),
            this, SIGNAL(signalRightPanelLeftButtonClicked()));
}

LightTableView::~LightTableView()
{
    delete d;
}

void LightTableView::setLoadFullImageSize(bool b)
{
    d->leftPreview->previewItem()->setLoadFullImageSize(b);
    d->rightPreview->previewItem()->setLoadFullImageSize(b);
}

void LightTableView::setSyncPreview(bool sync)
{
    d->syncPreview = sync;

    // Left panel like a reference to resync preview.
    if (d->syncPreview)
    {
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
    d->leftPreview->layout()->setZoomFactor(1.0);
}

void LightTableView::slotRightZoomTo100()
{
    d->rightPreview->layout()->setZoomFactor(1.0);
}

void LightTableView::slotLeftFitToWindow()
{
    d->leftPreview->layout()->fitToWindow();
}

void LightTableView::slotRightFitToWindow()
{
    d->rightPreview->layout()->fitToWindow();
}

double LightTableView::leftZoomMax()
{
    return d->leftPreview->layout()->maxZoomFactor();
}

double LightTableView::leftZoomMin()
{
    return d->leftPreview->layout()->minZoomFactor();
}

bool LightTableView::leftMaxZoom()
{
    return d->leftPreview->layout()->atMaxZoom();
}

bool LightTableView::leftMinZoom()
{
    return d->leftPreview->layout()->atMinZoom();
}

double LightTableView::rightZoomMax()
{
    return d->rightPreview->layout()->maxZoomFactor();
}

double LightTableView::rightZoomMin()
{
    return d->rightPreview->layout()->minZoomFactor();
}

bool LightTableView::rightMaxZoom()
{
    return d->rightPreview->layout()->atMaxZoom();
}

bool LightTableView::rightMinZoom()
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
    if (d->syncPreview && !d->leftLoading)
    {
        disconnect(d->rightPreview->layout(), SIGNAL(zoomFactorChanged(double)),
                   this, SIGNAL(signalRightZoomFactorChanged(double)));

        disconnect(d->rightPreview, SIGNAL(contentsMoving(int, int)),
                   this, SLOT(slotRightContentsMoved(int, int)));

        setRightZoomFactor(d->leftPreview->layout()->zoomFactor());
        emit signalRightZoomFactorChanged(d->leftPreview->layout()->zoomFactor());
        d->rightPreview->setContentsPos(x, y);

        connect(d->rightPreview->layout(), SIGNAL(zoomFactorChanged(double)),
                this, SIGNAL(signalRightZoomFactorChanged(double)));

        connect(d->rightPreview, SIGNAL(contentsMoving(int, int)),
                this, SLOT(slotRightContentsMoved(int, int)));
    }
}

void LightTableView::slotRightContentsMoved(int x, int y)
{
    if (d->syncPreview && !d->rightLoading)
    {
        disconnect(d->leftPreview->layout(), SIGNAL(zoomFactorChanged(double)),
                   this, SIGNAL(signalLeftZoomFactorChanged(double)));

        disconnect(d->leftPreview, SIGNAL(contentsMoving(int, int)),
                   this, SLOT(slotLeftContentsMoved(int, int)));


        setLeftZoomFactor(d->rightPreview->layout()->zoomFactor());
        emit signalLeftZoomFactorChanged(d->rightPreview->layout()->zoomFactor());
        d->leftPreview->setContentsPos(x, y);

        connect(d->leftPreview->layout(), SIGNAL(zoomFactorChanged(double)),
                this, SIGNAL(signalLeftZoomFactorChanged(double)));

        connect(d->leftPreview, SIGNAL(contentsMoving(int, int)),
                this, SLOT(slotLeftContentsMoved(int, int)));
    }
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
    d->leftLoading = true;
    d->leftPreview->setImageInfo(info);
    if (info.isNull()) d->leftPreview->setDragAndDropMessage();
}

void LightTableView::setRightImageInfo(const ImageInfo& info)
{
    d->rightLoading = true;
    d->rightPreview->setImageInfo(info);
    if (info.isNull()) d->rightPreview->setDragAndDropMessage();
}

void LightTableView::slotLeftPreviewLoaded(bool success)
{
    checkForSyncPreview();
    d->leftLoading = false;
    slotRightContentsMoved(d->rightPreview->contentsX(), d->rightPreview->contentsY());

    emit signalLeftPreviewLoaded(success);
}

void LightTableView::slotRightPreviewLoaded(bool success)
{
    checkForSyncPreview();
    d->rightLoading = false;
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
    QString selected    = QString("QLabel { background-color: %1; }")
                          .arg(kapp->palette().color(QPalette::Highlight).name());

    QString notSelected = QString("QLabel { background-color: %1; }")
                          .arg(kapp->palette().color(QPalette::Base).name());

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

}  // namespace Digikam
