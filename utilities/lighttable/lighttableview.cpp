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

// KDE includes

#include <kdialog.h>
#include <kdebug.h>

// Local includes

#include "dzoombar.h"
#include "thumbnailsize.h"
#include "lighttablepreview.h"

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
        leftPreview(0),
        rightPreview(0)
    {
    }

    bool               syncPreview;
    bool               leftLoading;     // To not sync right panel during left loading.
    bool               rightLoading;    // To not sync left panel during right loading.

    QGridLayout*       grid;

    LightTablePreview* leftPreview;
    LightTablePreview* rightPreview;
};

LightTableView::LightTableView(QWidget* parent)
    : QFrame(parent), d(new LightTableViewPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::NoFrame);
    setLineWidth(0);

    d->grid         = new QGridLayout();
    setLayout(d->grid);
    d->leftPreview  = new LightTablePreview(this);
    d->rightPreview = new LightTablePreview(this);

    d->grid->addWidget(d->leftPreview,  0, 0, 1, 1);
    d->grid->addWidget(d->rightPreview, 0, 1, 1, 1);

    d->grid->setColumnStretch(0, 10);
    d->grid->setColumnStretch(1, 10);
    d->grid->setRowStretch(0, 10);

    // Left panel connections ------------------------------------------------

    connect(d->leftPreview, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalLeftPopupTagsView()));

    connect(d->leftPreview, SIGNAL(signalZoomFactorChanged(double)),
            this, SIGNAL(signalLeftZoomFactorChanged(double)));

    connect(d->leftPreview, SIGNAL(contentsMoving(int, int)),
            this, SLOT(slotLeftContentsMoved(int, int)));

    connect(d->leftPreview, SIGNAL(signalSlideShow()),
            this, SIGNAL(signalSlideShow()));

    connect(d->leftPreview, SIGNAL(signalDeleteItem(const ImageInfo&)),
            this, SIGNAL(signalDeleteItem(const ImageInfo&)));

    connect(d->leftPreview, SIGNAL(signalEditItem(const ImageInfo&)),
            this, SIGNAL(signalEditItem(const ImageInfo&)));

    connect(d->leftPreview, SIGNAL(signalDroppedItems(const ImageInfoList&)),
            this, SIGNAL(signalLeftDroppedItems(const ImageInfoList&)));

    connect(d->leftPreview, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotLeftPreviewLoaded(bool)));

    connect(d->leftPreview, SIGNAL(signalLeftButtonClicked()),
            this, SIGNAL(signalLeftPanelLeftButtonClicked()));

    // Right panel connections ------------------------------------------------

    connect(d->rightPreview, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalRightPopupTagsView()));

    connect(d->rightPreview, SIGNAL(signalZoomFactorChanged(double)),
            this, SIGNAL(signalRightZoomFactorChanged(double)));

    connect(d->rightPreview, SIGNAL(contentsMoving(int, int)),
            this, SLOT(slotRightContentsMoved(int, int)));

    connect(d->rightPreview, SIGNAL(signalDeleteItem(const ImageInfo&)),
            this, SIGNAL(signalDeleteItem(const ImageInfo&)));

    connect(d->rightPreview, SIGNAL(signalEditItem(const ImageInfo&)),
            this, SIGNAL(signalEditItem(const ImageInfo&)));

    connect(d->rightPreview, SIGNAL(signalDroppedItems(const ImageInfoList&)),
            this, SIGNAL(signalRightDroppedItems(const ImageInfoList&)));

    connect(d->rightPreview, SIGNAL(signalSlideShow()),
            this, SIGNAL(signalSlideShow()));

    connect(d->rightPreview, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotRightPreviewLoaded(bool)));

    connect(d->rightPreview, SIGNAL(signalLeftButtonClicked()),
            this, SIGNAL(signalRightPanelLeftButtonClicked()));
}

LightTableView::~LightTableView()
{
    delete d;
}

void LightTableView::setLoadFullImageSize(bool b)
{
    d->leftPreview->setLoadFullImageSize(b);
    d->rightPreview->setLoadFullImageSize(b);
}

void LightTableView::setSyncPreview(bool sync)
{
    d->syncPreview = sync;

    // Left panel like a reference to resync preview.
    if (d->syncPreview)
    {
        slotLeftContentsMoved(d->leftPreview->contentsX(), d->leftPreview->contentsY());
    }
}

void LightTableView::setNavigateByPair(bool b)
{
    d->leftPreview->setDragAndDropEnabled(!b);
    d->rightPreview->setDragAndDropEnabled(!b);
}

void LightTableView::slotDecreaseLeftZoom()
{
    d->leftPreview->slotDecreaseZoom();
}

void LightTableView::slotIncreaseLeftZoom()
{
    d->leftPreview->slotIncreaseZoom();
}

void LightTableView::slotDecreaseRightZoom()
{
    d->rightPreview->slotDecreaseZoom();
}

void LightTableView::slotIncreaseRightZoom()
{
    d->rightPreview->slotIncreaseZoom();
}

void LightTableView::setLeftZoomFactor(double z)
{
    d->leftPreview->setZoomFactor(z);
}

void LightTableView::setRightZoomFactor(double z)
{
    d->rightPreview->setZoomFactor(z);
}

void LightTableView::slotLeftZoomTo100()
{
    d->leftPreview->setZoomFactor(1.0, true);
}

void LightTableView::slotRightZoomTo100()
{
    d->rightPreview->setZoomFactor(1.0, true);
}

void LightTableView::slotLeftFitToWindow()
{
    d->leftPreview->fitToWindow();
}

void LightTableView::slotRightFitToWindow()
{
    d->rightPreview->fitToWindow();
}

double LightTableView::leftZoomMax()
{
    return d->leftPreview->zoomMax();
}

double LightTableView::leftZoomMin()
{
    return d->leftPreview->zoomMin();
}

bool LightTableView::leftMaxZoom()
{
    return d->leftPreview->maxZoom();
}

bool LightTableView::leftMinZoom()
{
    return d->leftPreview->minZoom();
}

double LightTableView::rightZoomMax()
{
    return d->rightPreview->zoomMax();
}

double LightTableView::rightZoomMin()
{
    return d->rightPreview->zoomMin();
}

bool LightTableView::rightMaxZoom()
{
    return d->rightPreview->maxZoom();
}

bool LightTableView::rightMinZoom()
{
    return d->rightPreview->minZoom();
}

void LightTableView::slotLeftZoomSliderChanged(int size)
{
    double zmin = d->leftPreview->zoomMin();
    double zmax = d->leftPreview->zoomMax();
    double z    = DZoomBar::zoomFromSize(size, zmin, zmax);
    d->leftPreview->setZoomFactorSnapped(z);
}

void LightTableView::slotRightZoomSliderChanged(int size)
{
    double zmin = d->rightPreview->zoomMin();
    double zmax = d->rightPreview->zoomMax();
    double z    = DZoomBar::zoomFromSize(size, zmin, zmax);
    d->rightPreview->setZoomFactorSnapped(z);
}

void LightTableView::leftReload()
{
    d->leftPreview->reload();
}

void LightTableView::rightReload()
{
    d->rightPreview->reload();
}

void LightTableView::slotLeftContentsMoved(int x, int y)
{
    if (d->syncPreview && !d->leftLoading)
    {
        disconnect(d->rightPreview, SIGNAL(signalZoomFactorChanged(double)),
                   this, SIGNAL(signalRightZoomFactorChanged(double)));

        disconnect(d->rightPreview, SIGNAL(contentsMoving(int, int)),
                   this, SLOT(slotRightContentsMoved(int, int)));

        setRightZoomFactor(d->leftPreview->zoomFactor());
        emit signalRightZoomFactorChanged(d->leftPreview->zoomFactor());
        d->rightPreview->setContentsPos(x, y);

        connect(d->rightPreview, SIGNAL(signalZoomFactorChanged(double)),
                this, SIGNAL(signalRightZoomFactorChanged(double)));

        connect(d->rightPreview, SIGNAL(contentsMoving(int, int)),
                this, SLOT(slotRightContentsMoved(int, int)));
    }
}

void LightTableView::slotRightContentsMoved(int x, int y)
{
    if (d->syncPreview && !d->rightLoading)
    {
        disconnect(d->leftPreview, SIGNAL(signalZoomFactorChanged(double)),
                   this, SIGNAL(signalLeftZoomFactorChanged(double)));

        disconnect(d->leftPreview, SIGNAL(contentsMoving(int, int)),
                   this, SLOT(slotLeftContentsMoved(int, int)));


        setLeftZoomFactor(d->rightPreview->zoomFactor());
        emit signalLeftZoomFactorChanged(d->rightPreview->zoomFactor());
        d->leftPreview->setContentsPos(x, y);

        connect(d->leftPreview, SIGNAL(signalZoomFactorChanged(double)),
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
}

void LightTableView::setRightImageInfo(const ImageInfo& info)
{
    d->rightLoading = true;
    d->rightPreview->setImageInfo(info);
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
        d->leftPreview->getImageSize() == d->rightPreview->getImageSize())
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
    if (info.isNull())
    {
        d->leftPreview->setSelected(false);
        d->rightPreview->setSelected(false);
        return;
    }

    if (!d->leftPreview->getImageInfo().isNull())
    {
        bool onLeft = (d->leftPreview->getImageInfo() == info);
        d->leftPreview->setSelected(onLeft);
    }

    if (!d->rightPreview->getImageInfo().isNull())
    {
        bool onRight = (d->rightPreview->getImageInfo() == info);
        d->rightPreview->setSelected(onRight);
    }
}

}  // namespace Digikam
