/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : a widget to display 2 preview image on 
 *               lightable to compare pictures.
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qlayout.h>

// KDE includes.

#include <kdialogbase.h>

// Local includes

#include "thumbnailsize.h"
#include "lighttablepreview.h"
#include "lighttableview.h"
#include "lighttableview.moc"

namespace Digikam
{

class LightTableViewPriv
{
public:

    LightTableViewPriv()
    {
        syncPreview  = false;
        leftPreview  = 0;
        rightPreview = 0;
        grid         = 0;
    }

    bool               syncPreview;

    QGridLayout       *grid;

    LightTablePreview *leftPreview;
    LightTablePreview *rightPreview;
};

LightTableView::LightTableView(QWidget *parent)
              : QFrame(parent, 0, Qt::WDestructiveClose)
{
    d = new LightTableViewPriv;

    d->grid         = new QGridLayout(this, 1, 1, 0, KDialogBase::spacingHint());
    d->leftPreview  = new LightTablePreview(this);
    d->rightPreview = new LightTablePreview(this);

    d->grid->setColStretch(0, 10),
    d->grid->setColStretch(1, 10),
    d->grid->setRowStretch(0, 10),

    d->grid->addMultiCellWidget(d->leftPreview,  0, 0, 0, 0);
    d->grid->addMultiCellWidget(d->rightPreview, 0, 0, 1, 1);

    setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    setMargin(0);
    setLineWidth(1);

    connect(d->leftPreview, SIGNAL(signalZoomFactorChanged(double)),
            this, SIGNAL(signalLeftZoomFactorChanged(double)));
    
    connect(d->rightPreview, SIGNAL(signalZoomFactorChanged(double)),
            this, SIGNAL(signalRightZoomFactorChanged(double)));

    connect(d->leftPreview, SIGNAL(signalSlideShow()),
            this, SIGNAL(signalSlideShow()));
    
    connect(d->rightPreview, SIGNAL(signalDeleteItem(ImageInfo*)),
            this, SIGNAL(signalDeleteItem(ImageInfo*)));

    connect(d->leftPreview, SIGNAL(signalDeleteItem(ImageInfo*)),
            this, SIGNAL(signalDeleteItem(ImageInfo*)));
    
    connect(d->rightPreview, SIGNAL(signalEditItem(ImageInfo*)),
            this, SIGNAL(signalEditItem(ImageInfo*)));

    connect(d->leftPreview, SIGNAL(signalEditItem(ImageInfo*)),
            this, SIGNAL(signalEditItem(ImageInfo*)));

    connect(d->rightPreview, SIGNAL(signalDroppedItems(const ImageInfoList&)),
            this, SIGNAL(signalRightDroppedItems(const ImageInfoList&)));

    connect(d->leftPreview, SIGNAL(signalDroppedItems(const ImageInfoList&)),
            this, SIGNAL(signalLeftDroppedItems(const ImageInfoList&)));
    
    connect(d->rightPreview, SIGNAL(signalSlideShow()),
            this, SIGNAL(signalSlideShow()));

    connect(d->leftPreview, SIGNAL(contentsMoving(int, int)),
            this, SLOT(slotLeftContentsMoved(int, int)));

    connect(d->rightPreview, SIGNAL(contentsMoving(int, int)),
            this, SLOT(slotRightContentsMoved(int, int)));

    connect(d->leftPreview, SIGNAL(signalPreviewLoaded()),
            this, SLOT(slotPreviewLoaded()));

    connect(d->rightPreview, SIGNAL(signalPreviewLoaded()),
            this, SLOT(slotPreviewLoaded()));
}

LightTableView::~LightTableView()
{
    delete d;
}

void LightTableView::setSyncPreview(bool sync)
{
    d->syncPreview = sync;
}

void LightTableView::setLeftImageInfo(ImageInfo* info)
{
    d->leftPreview->setImageInfo(info);    
}

void LightTableView::setRightImageInfo(ImageInfo* info)
{
    d->rightPreview->setImageInfo(info);    
}

ImageInfo* LightTableView::leftImageInfo() const
{
    return d->leftPreview->getImageInfo();
}

ImageInfo* LightTableView::rightImageInfo() const
{
    return d->rightPreview->getImageInfo();
}

void LightTableView::slotDecreaseZoom()
{
    if (!d->syncPreview) return;

    slotDecreaseLeftZoom();
}   

void LightTableView::slotIncreaseZoom()
{
    if (!d->syncPreview) return;

    slotIncreaseLeftZoom();
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

void LightTableView::fitToWindow()
{
    d->leftPreview->fitToWindow(); 
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
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->leftPreview->zoomMin();
    double zmax = d->leftPreview->zoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    double z    = a*size+b; 

    d->leftPreview->setZoomFactor(z);
}

void LightTableView::slotRightZoomSliderChanged(int size)
{
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->rightPreview->zoomMin();
    double zmax = d->rightPreview->zoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    double z    = a*size+b; 

    d->rightPreview->setZoomFactor(z);
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
    if (d->syncPreview)
    {
        d->rightPreview->blockSignals(true);
        setRightZoomFactor(d->leftPreview->zoomFactor());
        emit signalRightZoomFactorChanged(d->leftPreview->zoomFactor());
        d->rightPreview->setContentsPos(x, y);
        d->rightPreview->blockSignals(false);
    }
}

void LightTableView::slotRightContentsMoved(int x, int y)
{
    if (d->syncPreview)
    {
        d->leftPreview->blockSignals(true);
        setLeftZoomFactor(d->rightPreview->zoomFactor());
        emit signalLeftZoomFactorChanged(d->rightPreview->zoomFactor());
        d->leftPreview->setContentsPos(x, y);
        d->leftPreview->blockSignals(false);
    }
}

void LightTableView::slotPreviewLoaded()
{
    if (d->leftPreview->getImageSize() == d->rightPreview->getImageSize())
        d->syncPreview = true; 
    else
        d->syncPreview = false; 

    emit signalToggleOnSyncPreview(d->syncPreview); 
}

}  // namespace Digikam

