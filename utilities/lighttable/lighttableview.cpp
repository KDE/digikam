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
        leftPreview  = 0;
        rightPreview = 0;
        grid         = 0;
    }

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

}

LightTableView::~LightTableView()
{
    delete d;
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

}  // namespace Digikam

