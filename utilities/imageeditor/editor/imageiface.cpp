/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : image data interface for image plugins
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageiface.h"

// Qt includes

#include <QWidget>
#include <QSize>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

// KDE includes

#include <kdebug.h>

// Local includes

#include "exposurecontainer.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "dimginterface.h"
#include "bcgmodifier.h"
#include "dmetadata.h"

namespace Digikam
{

class ImageIfacePriv
{
public:

    ImageIfacePriv()
    {
        usePreviewSelection = false;
        previewWidth        = 0;
        previewHeight       = 0;
    }

    bool    usePreviewSelection;

    int     originalWidth;
    int     originalHeight;
    int     originalBytesDepth;

    int     constrainWidth;
    int     constrainHeight;

    int     previewWidth;
    int     previewHeight;

    QPixmap qcheck;

    DImg    previewImage;
    DImg    targetPreviewImage;
};

ImageIface::ImageIface(int w, int h)
          : d(new ImageIfacePriv)
{
    d->constrainWidth     = w;
    d->constrainHeight    = h;
    d->originalWidth      = DImgInterface::defaultInterface()->origWidth();
    d->originalHeight     = DImgInterface::defaultInterface()->origHeight();
    d->originalBytesDepth = DImgInterface::defaultInterface()->bytesDepth();

    d->qcheck = QPixmap(8, 8);

    QPainter p;
    p.begin(&d->qcheck);
    p.fillRect(0, 0, 4, 4, QColor(144,144,144));
    p.fillRect(4, 4, 4, 4, QColor(144,144,144));
    p.fillRect(0, 4, 4, 4, QColor(100,100,100));
    p.fillRect(4, 0, 4, 4, QColor(100,100,100));
    p.end();
}

ImageIface::~ImageIface()
{
    delete d;
}

void ImageIface::setPreviewType(bool useSelect)
{
    d->usePreviewSelection = useSelect;
}

bool ImageIface::previewType()
{
    return d->usePreviewSelection;
}

DColor ImageIface::getColorInfoFromOriginalImage(const QPoint& point)
{
    if ( !DImgInterface::defaultInterface()->getImage() || point.x() > originalWidth() || point.y() > originalHeight() )
    {
        kWarning(50003) << "Coordinate out of range or no image data available!";
        return DColor();
    }

    return DImgInterface::defaultInterface()->getImg()->getPixelColor(point.x(), point.y());
}

DColor ImageIface::getColorInfoFromPreviewImage(const QPoint& point)
{
    if ( d->previewImage.isNull() || point.x() > previewWidth() || point.y() > previewHeight() )
    {
        kWarning(50003) << "Coordinate out of range or no image data available!";
        return DColor();
    }

    return d->previewImage.getPixelColor(point.x(), point.y());
}

DColor ImageIface::getColorInfoFromTargetPreviewImage(const QPoint& point)
{
    if ( d->targetPreviewImage.isNull() || point.x() > previewWidth() || point.y() > previewHeight() )
    {
        kWarning(50003) << "Coordinate out of range or no image data available!";
        return DColor::DColor();
    }

    return d->targetPreviewImage.getPixelColor(point.x(), point.y());
}

uchar* ImageIface::setPreviewImageSize(int w, int h) const
{
    d->previewImage.reset();
    d->targetPreviewImage.reset();

    d->constrainWidth  = w;
    d->constrainHeight = h;

    return (getPreviewImage());
}

uchar* ImageIface::getPreviewImage() const
{
    if (d->previewImage.isNull())
    {
        DImg *im = 0;

        if (!d->usePreviewSelection)
        {
            im = DImgInterface::defaultInterface()->getImg();
            if (!im || im->isNull())
                return 0;
        }
        else
        {
            int    x, y, w, h;
            bool   s    = DImgInterface::defaultInterface()->sixteenBit();
            bool   a    = DImgInterface::defaultInterface()->hasAlpha();
            uchar *data = DImgInterface::defaultInterface()->getImageSelection();
            DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
            im = new DImg(w, h, s, a, data, true);
            delete [] data;

            if (!im)
                return 0;

            if (im->isNull())
            {
                delete im;
                return 0;
            }
        }

        QSize sz(im->width(), im->height());
        sz.scale(d->constrainWidth, d->constrainHeight, Qt::KeepAspectRatio);

        d->previewImage  = im->smoothScale(sz.width(), sz.height());
        d->previewWidth  = d->previewImage.width();
        d->previewHeight = d->previewImage.height();

        // only create another copy if needed, in putPreviewImage
        d->targetPreviewImage = d->previewImage;

        if (d->usePreviewSelection)
            delete im;
    }

    DImg previewData = d->previewImage.copyImageData();
    return previewData.stripImageData();
}

uchar* ImageIface::getOriginalImage() const
{
    DImg *im = DImgInterface::defaultInterface()->getImg();

    if (!im || im->isNull())
        return 0;

    DImg origData = im->copyImageData();
    return origData.stripImageData();
}

DImg* ImageIface::getOriginalImg() const
{
    return DImgInterface::defaultInterface()->getImg();
}

uchar* ImageIface::getImageSelection() const
{
    return DImgInterface::defaultInterface()->getImageSelection();
}

void ImageIface::putPreviewImage(uchar* data)
{
    if (!data)
        return;

    if (d->targetPreviewImage == d->previewImage)
    {
        d->targetPreviewImage = DImg(d->previewImage.width(), d->previewImage.height(),
                                     d->previewImage.sixteenBit(), d->previewImage.hasAlpha(), data);
        d->targetPreviewImage.setIccProfile( d->previewImage.getIccProfile() );
    }
    else
    {
        d->targetPreviewImage.putImageData(data);
    }
}

void ImageIface::putOriginalImage(const QString& caller, uchar* data, int w, int h)
{
    if (!data)
        return;

    DImgInterface::defaultInterface()->putImage(caller, data, w, h);
}

void ImageIface::setEmbeddedICCToOriginalImage(const IccProfile& profile)
{
    DImgInterface::defaultInterface()->setEmbeddedICCToOriginalImage( profile );
}

void ImageIface::putImageSelection(const QString& caller, uchar* data)
{
    if (!data)
        return;

    DImgInterface::defaultInterface()->putImageSelection(caller, data);
}

int ImageIface::previewWidth()
{
    return d->previewWidth;
}

int ImageIface::previewHeight()
{
    return d->previewHeight;
}

bool ImageIface::previewSixteenBit()
{
    return originalSixteenBit();
}

bool ImageIface::previewHasAlpha()
{
    return originalHasAlpha();
}

int ImageIface::originalWidth()
{
    return DImgInterface::defaultInterface()->origWidth();
}

int ImageIface::originalHeight()
{
    return DImgInterface::defaultInterface()->origHeight();
}

bool ImageIface::originalSixteenBit()
{
    return DImgInterface::defaultInterface()->sixteenBit();
}

bool ImageIface::originalHasAlpha()
{
    return DImgInterface::defaultInterface()->hasAlpha();
}

int ImageIface::selectedWidth()
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return w;
}

int ImageIface::selectedHeight()
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return h;
}

int ImageIface::selectedXOrg()
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return x;
}

int ImageIface::selectedYOrg()
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return y;
}

void ImageIface::setPreviewBCG(double brightness, double contrast, double gamma)
{
    DImg preview = d->targetPreviewImage.copyImageData();
    BCGModifier cmod;
    cmod.setGamma(gamma);
    cmod.setBrightness(brightness);
    cmod.setContrast(contrast);
    cmod.applyBCG(preview);
    putPreviewImage(preview.bits());
}

void ImageIface::setOriginalBCG(double brightness, double contrast, double gamma)
{
    DImgInterface::defaultInterface()->setBCG(brightness, contrast, gamma);
}

void ImageIface::convertOriginalColorDepth(int depth)
{
    DImgInterface::defaultInterface()->convertDepth(depth);
}

QPixmap ImageIface::convertToPixmap(DImg& img)
{
    return DImgInterface::defaultInterface()->convertToPixmap(img);
}

IccProfile ImageIface::getEmbeddedICCFromOriginalImage()
{
    return DImgInterface::defaultInterface()->getEmbeddedICC();
}

QByteArray ImageIface::getExifFromOriginalImage()
{
    return DImgInterface::defaultInterface()->getExif();
}

QByteArray ImageIface::getIptcFromOriginalImage()
{
    return DImgInterface::defaultInterface()->getIptc();
}

QByteArray ImageIface::getXmpFromOriginalImage()
{
    return DImgInterface::defaultInterface()->getXmp();
}

PhotoInfoContainer ImageIface::getPhotographInformation() const
{
    DMetadata meta;
    meta.setExif(DImgInterface::defaultInterface()->getExif());
    meta.setIptc(DImgInterface::defaultInterface()->getIptc());
    meta.setXmp(DImgInterface::defaultInterface()->getXmp());
    return meta.getPhotographInformation();
}

void ImageIface::paint(QPaintDevice* device, int x, int y, int w, int h,
                       bool underExposure, bool overExposure, QPainter *painter)
{
    QPainter localPainter;
    QPainter *p;

    if (painter)
        p = painter;
    else
    {
        p = &localPainter;
        p->begin(device);
    }

    int width = w > 0 ? qMin(d->previewWidth, w) : d->previewWidth;
    int height = h > 0 ? qMin(d->previewHeight, h) : d->previewHeight;

    if ( !d->targetPreviewImage.isNull() )
    {
        if (d->targetPreviewImage.hasAlpha())
        {
            p->drawTiledPixmap(x, y, width, height, d->qcheck);
        }

        QPixmap pixImage;
        ICCSettingsContainer *iccSettings = DImgInterface::defaultInterface()->getICCSettings();

        if (iccSettings && iccSettings->enableCMSetting && iccSettings->managedViewSetting)
        {
            IccTransform monitorICCtrans;
            monitorICCtrans.setInputProfile(iccSettings->workspaceSetting);
            monitorICCtrans.setOutputProfile(iccSettings->monitorSetting);
            pixImage = d->targetPreviewImage.convertToPixmap(monitorICCtrans);
        }
        else
        {
            pixImage = d->targetPreviewImage.convertToPixmap();
        }

        p->drawPixmap(x, y, pixImage, 0, 0, width, height);

        // Show the Over/Under exposure pixels indicators

        if (underExposure || overExposure)
        {
            ExposureSettingsContainer expoSettings;
            expoSettings.underExposureIndicator = underExposure;
            expoSettings.overExposureIndicator  = overExposure;
            expoSettings.underExposureColor     = DImgInterface::defaultInterface()->underExposureColor();
            expoSettings.overExposureColor      = DImgInterface::defaultInterface()->overExposureColor();

            QImage pureColorMask = d->targetPreviewImage.pureColorMask(&expoSettings);
            QPixmap pixMask = QPixmap::fromImage(pureColorMask);
            p->drawPixmap(x, y, pixMask, 0, 0, width, height);
        }
    }

    if (!painter)
        p->end();
}

}   // namespace Digikam
