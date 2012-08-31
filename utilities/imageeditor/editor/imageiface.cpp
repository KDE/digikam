/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : image data interface for image plugins
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "iccmanager.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "dimginterface.h"
#include "dmetadata.h"

namespace Digikam
{

class ImageIface::Private
{
public:

    Private() :
        usePreviewSelection(false),
        originalWidth(0),
        originalHeight(0),
        originalBytesDepth(0),
        constrainWidth(0),
        constrainHeight(0),
        previewWidth(0),
        previewHeight(0)
    {
    }

    QPixmap checkPixmap();

    uchar*  getPreviewImage();

    uchar*  setPreviewImageSize(int w, int h);

public:

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

QPixmap ImageIface::Private::checkPixmap()
{
    if (qcheck.isNull())
    {
        qcheck = QPixmap(8, 8);

        QPainter p;
        p.begin(&qcheck);
        p.fillRect(0, 0, 4, 4, QColor(144, 144, 144));
        p.fillRect(4, 4, 4, 4, QColor(144, 144, 144));
        p.fillRect(0, 4, 4, 4, QColor(100, 100, 100));
        p.fillRect(4, 0, 4, 4, QColor(100, 100, 100));
        p.end();
    }

    return qcheck;
}

uchar* ImageIface::Private::getPreviewImage()
{
    if (previewImage.isNull())
    {
        DImg* im = 0;

        if (!usePreviewSelection)
        {
            im = DImgInterface::defaultInterface()->getImg();

            if (!im || im->isNull())
            {
                return 0;
            }
        }
        else
        {
            int    x, y, w, h;
            bool   s    = DImgInterface::defaultInterface()->sixteenBit();
            bool   a    = DImgInterface::defaultInterface()->hasAlpha();

            QScopedArrayPointer<uchar> data(DImgInterface::defaultInterface()->getImageSelection());

            DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
            im = new DImg(w, h, s, a, data.data(), true);

            if (!im)
            {
                return 0;
            }

            if (im->isNull())
            {
                delete im;
                return 0;
            }

            im->setIccProfile(DImgInterface::defaultInterface()->getEmbeddedICC());
        }

        QSize sz(im->width(), im->height());
        sz.scale(constrainWidth, constrainHeight, Qt::KeepAspectRatio);

        previewImage       = im->smoothScale(sz.width(), sz.height());
        previewWidth       = previewImage.width();
        previewHeight      = previewImage.height();

        // only create another copy if needed, in putPreviewImage
        targetPreviewImage = previewImage;

        if (usePreviewSelection)
        {
            delete im;
        }
    }

    DImg previewData = previewImage.copyImageData();
    return previewData.stripImageData();
}

uchar* ImageIface::Private::setPreviewImageSize(int w, int h)
{
    previewImage.reset();
    targetPreviewImage.reset();

    constrainWidth  = w;
    constrainHeight = h;

    return (getPreviewImage());
}

// ------------------------------------------------------------------------------------------------------

ImageIface::ImageIface(int w, int h)
    : d(new Private)
{
    d->constrainWidth     = w;
    d->constrainHeight    = h;
    d->originalWidth      = DImgInterface::defaultInterface()->origWidth();
    d->originalHeight     = DImgInterface::defaultInterface()->origHeight();
    d->originalBytesDepth = DImgInterface::defaultInterface()->bytesDepth();
}

ImageIface::~ImageIface()
{
    delete d;
}

void ImageIface::setPreviewType(bool useSelect)
{
    d->usePreviewSelection = useSelect;
}

bool ImageIface::previewType() const
{
    return d->usePreviewSelection;
}

DColor ImageIface::getColorInfoFromOriginalImage(const QPoint& point) const
{
    if (!DImgInterface::defaultInterface()->getImage() || point.x() > originalWidth() || point.y() > originalHeight())
    {
        kWarning() << "Coordinate out of range or no image data available!";
        return DColor();
    }

    return DImgInterface::defaultInterface()->getImg()->getPixelColor(point.x(), point.y());
}

DColor ImageIface::getColorInfoFromPreviewImage(const QPoint& point) const
{
    if (d->previewImage.isNull() || point.x() > previewWidth() || point.y() > previewHeight())
    {
        kWarning() << "Coordinate out of range or no image data available!";
        return DColor();
    }

    return d->previewImage.getPixelColor(point.x(), point.y());
}

DColor ImageIface::getColorInfoFromTargetPreviewImage(const QPoint& point) const
{
    if (d->targetPreviewImage.isNull() || point.x() > previewWidth() || point.y() > previewHeight())
    {
        kWarning() << "Coordinate out of range or no image data available!";
        return DColor();
    }

    return d->targetPreviewImage.getPixelColor(point.x(), point.y());
}

DImg ImageIface::setPreviewImgSize(int w, int h) const
{
    uchar* const data = d->setPreviewImageSize(w, h);
    return DImg(previewWidth(), previewHeight(), previewSixteenBit(), previewHasAlpha(), data);
}

DImg ImageIface::getPreviewImg() const
{
    return DImg(previewWidth(), previewHeight(), previewSixteenBit(), previewHasAlpha(), d->getPreviewImage());
}

DImg* ImageIface::getOriginalImg() const
{
    return DImgInterface::defaultInterface()->getImg();
}

DImg ImageIface::getImgSelection() const
{
    return DImg(selectedWidth(), selectedHeight(), originalSixteenBit(), originalHasAlpha(),
                DImgInterface::defaultInterface()->getImageSelection(), false);
}

void ImageIface::putPreviewIccProfile(const IccProfile& profile)
{
    d->targetPreviewImage.detach();
    d->targetPreviewImage.setIccProfile(profile);
}

void ImageIface::putOriginalIccProfile(const IccProfile& profile)
{
    DImgInterface::defaultInterface()->putIccProfile(profile);
}

int ImageIface::previewWidth() const
{
    return d->previewWidth;
}

int ImageIface::previewHeight() const
{
    return d->previewHeight;
}

bool ImageIface::previewSixteenBit() const
{
    return originalSixteenBit();
}

bool ImageIface::previewHasAlpha() const
{
    return originalHasAlpha();
}

int ImageIface::originalWidth() const
{
    return DImgInterface::defaultInterface()->origWidth();
}

int ImageIface::originalHeight() const
{
    return DImgInterface::defaultInterface()->origHeight();
}

bool ImageIface::originalSixteenBit() const
{
    return DImgInterface::defaultInterface()->sixteenBit();
}

bool ImageIface::originalHasAlpha() const
{
    return DImgInterface::defaultInterface()->hasAlpha();
}

int ImageIface::selectedWidth() const
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return w;
}

int ImageIface::selectedHeight() const
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return h;
}

int ImageIface::selectedXOrg() const
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return x;
}

int ImageIface::selectedYOrg() const
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return y;
}

void ImageIface::convertOriginalColorDepth(int depth)
{
    DImgInterface::defaultInterface()->convertDepth(depth);
}

QPixmap ImageIface::convertToPixmap(DImg& img) const
{
    return DImgInterface::defaultInterface()->convertToPixmap(img);
}

IccProfile ImageIface::getOriginalIccProfile() const
{
    return DImgInterface::defaultInterface()->getEmbeddedICC();
}

KExiv2Data ImageIface::getOriginalMetadata() const
{
    return DImgInterface::defaultInterface()->getImg()->getMetadata();
}

void ImageIface::setOriginalMetadata(const KExiv2Data& meta)
{
    DImgInterface::defaultInterface()->getImg()->setMetadata(meta);
}

PhotoInfoContainer ImageIface::getPhotographInformation() const
{
    DMetadata meta(DImgInterface::defaultInterface()->getImg()->getMetadata());
    return meta.getPhotographInformation();
}

void ImageIface::paint(QPaintDevice* device, int x, int y, int w, int h, QPainter* painter)
{
    QPainter  localPainter;
    QPainter* p = 0;

    if (painter)
    {
        p = painter;
    }
    else
    {
        p = &localPainter;
        p->begin(device);
    }

    int width  = w > 0 ? qMin(d->previewWidth, w)  : d->previewWidth;
    int height = h > 0 ? qMin(d->previewHeight, h) : d->previewHeight;

    if (!d->targetPreviewImage.isNull())
    {
        if (d->targetPreviewImage.hasAlpha())
        {
            p->drawTiledPixmap(x, y, width, height, d->checkPixmap());
        }

        QPixmap pixImage;
        ICCSettingsContainer iccSettings = DImgInterface::defaultInterface()->getICCSettings();

        if (iccSettings.enableCM && iccSettings.useManagedView)
        {
            IccManager manager(d->targetPreviewImage);
            IccTransform monitorICCtrans = manager.displayTransform();
            pixImage = d->targetPreviewImage.convertToPixmap(monitorICCtrans);
        }
        else
        {
            pixImage = d->targetPreviewImage.convertToPixmap();
        }

        p->drawPixmap(x, y, pixImage, 0, 0, width, height);

        // Show the Over/Under exposure pixels indicators

        ExposureSettingsContainer* expoSettings = DImgInterface::defaultInterface()->getExposureSettings();

        if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
        {
            ExposureSettingsContainer* expoSettings = DImgInterface::defaultInterface()->getExposureSettings();
            QImage pureColorMask                    = d->targetPreviewImage.pureColorMask(expoSettings);
            QPixmap pixMask                         = QPixmap::fromImage(pureColorMask);
            p->drawPixmap(x, y, pixMask, 0, 0, width, height);
        }
    }

    if (!painter)
    {
        p->end();
    }
}

// Deprecated methods ------------------------------------------------------------------------------------------------

void ImageIface::putPreviewImage(uchar* data)
{
    if (!data)
    {
        return;
    }

    d->targetPreviewImage.detach();
    d->targetPreviewImage.putImageData(data);
}

void ImageIface::putOriginalImage(const QString& caller, const FilterAction& action, uchar* data, int w, int h)
{
    if (!data)
    {
        return;
    }

    DImgInterface::defaultInterface()->putImage(caller, action, data, w, h);
}

void ImageIface::putImageSelection(const QString& caller, const FilterAction& action, uchar* data)
{
    if (!data)
    {
        return;
    }

    DImgInterface::defaultInterface()->putImageSelection(caller, action, data);
}

}   // namespace Digikam
