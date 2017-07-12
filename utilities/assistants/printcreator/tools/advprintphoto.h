/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-12-09
 * Description : a tool to print images
 *
 * Copyright (C) 2002-2003 by Todd Shoemaker <todd at theshoemakers dot net>
 * Copyright (C) 2007-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef ADV_PRINT_PHOTO_H
#define ADV_PRINT_PHOTO_H

// Qt includes

#include <QRect>
#include <QFont>
#include <QColor>
#include <QUrl>
#include <QPointer>
#include <QIcon>
#include <QList>

// Local includes

#include "dinfointerface.h"

namespace Digikam
{

class AdvPrintPhotoSize
{
public:

    QString       label;
    int           dpi;
    bool          autoRotate;
    QList<QRect*> layouts;     // first element is page size
    QIcon         icon;
};

// -----------------------------------------------------------

class AdvPrintAdditionalInfo
{
public:

    explicit AdvPrintAdditionalInfo();
    AdvPrintAdditionalInfo(const AdvPrintAdditionalInfo& ai);
    ~AdvPrintAdditionalInfo();

public:

    int    m_unit;
    int    m_printPosition;
    int    m_scaleMode;
    bool   m_keepRatio;
    bool   m_autoRotate;
    double m_printWidth;
    double m_printHeight;
    bool   m_enlargeSmallerImages;
};

// -----------------------------------------------------------

class AdvPrintCaptionInfo
{
public:

    enum AvailableCaptions
    {
        NoCaptions = 0,
        FileNames,
        ExifDateTime,
        Comment,
        Custom
    };

public:

    explicit AdvPrintCaptionInfo();
    /// Copy constructor to get old photo info.
    AdvPrintCaptionInfo(const AdvPrintCaptionInfo& ci);
    ~AdvPrintCaptionInfo();

public:

    AvailableCaptions m_captionType;
    QFont             m_captionFont;
    QColor            m_captionColor;
    int               m_captionSize;
    QString           m_captionText;
};

// -----------------------------------------------------------

class AdvPrintPhoto
{

public:

    explicit AdvPrintPhoto(int thumbnailSize, DInfoInterface* const iface);
    AdvPrintPhoto(const AdvPrintPhoto&);
    ~AdvPrintPhoto();

    QImage& thumbnail();
    QImage  loadPhoto();
    int     width();
    int     height();
    QSize&  size();

    double scaleWidth(double unitToInches);
    double scaleHeight(double unitToInches);

public:

    QUrl                    m_url;

    int                     m_thumbnailSize;

    QRect                   m_cropRegion;

    // to get first copy quickly
    bool                    m_first;

    // number of copies
    int                     m_copies;

    int                     m_rotation;
    AdvPrintAdditionalInfo* m_pAddInfo;
    AdvPrintCaptionInfo*    m_pAdvPrintCaptionInfo;
    DInfoInterface*         m_iface;

private:

    void loadCache();

private:

    QImage*                 m_thumbnail;
    QSize*                  m_size;
};

} // Namespace Digikam

#endif // ADV_PRINT_PHOTO_H
