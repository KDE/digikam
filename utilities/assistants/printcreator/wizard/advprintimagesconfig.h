/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-08
 * Description : a tool to print images
 *
 * Copyright (C) 2009-2012 by Angelo Naselli <anaselli at linux dot it>
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

#ifndef ADV_PRINT_IMAGES_CONFIG_H
#define ADV_PRINT_IMAGES_CONFIG_H

// Qt includes

#include <QCoreApplication>
#include <QDebug>

// KDE includes

#include <kconfigskeleton.h>

// Local includes

#include "advprintoptionspage.h"

namespace Digikam
{

class AdvPrintImagesConfig : public KConfigSkeleton
{
public:

    static AdvPrintImagesConfig* self();
    ~AdvPrintImagesConfig();

    static void setPrintPosition( int v )
    {
        if (!self()->isImmutable( QString::fromLatin1( "PrintPosition" ) ))
            self()->mPrintPosition = v;
    }

    static int printPosition()
    {
        return self()->mPrintPosition;
    }

    // -------------------

    static void setPrintScaleMode(AdvPrintOptionsPage::ScaleMode v)
    {
        if (!self()->isImmutable( QString::fromLatin1( "PrintScaleMode" ) ))
            self()->mPrintScaleMode = v;
    }

    static AdvPrintOptionsPage::ScaleMode printScaleMode()
    {
        return static_cast<AdvPrintOptionsPage::ScaleMode>(self()->mPrintScaleMode);
    }

    // -------------------

    static void setPrintEnlargeSmallerImages( bool v )
    {
        if (!self()->isImmutable( QString::fromLatin1( "PrintEnlargeSmallerImages" ) ))
            self()->mPrintEnlargeSmallerImages = v;
    }

    static bool printEnlargeSmallerImages()
    {
        return self()->mPrintEnlargeSmallerImages;
    }

    // -------------------

    static void setPrintWidth( double v )
    {
        if (!self()->isImmutable( QString::fromLatin1( "PrintWidth" ) ))
            self()->mPrintWidth = v;
    }

    static double printWidth()
    {
        return self()->mPrintWidth;
    }

    // -------------------

    static void setPrintHeight( double v )
    {
        if (!self()->isImmutable( QString::fromLatin1( "PrintHeight" ) ))
            self()->mPrintHeight = v;
    }

    static double printHeight()
    {
        return self()->mPrintHeight;
    }

    // -------------------

    static void setPrintUnit( AdvPrintOptionsPage::Unit v )
    {
        if (!self()->isImmutable( QString::fromLatin1( "PrintUnit" ) ))
            self()->mPrintUnit = v;
    }

    static AdvPrintOptionsPage::Unit printUnit()
    {
        return static_cast<AdvPrintOptionsPage::Unit>(self()->mPrintUnit);
    }

    // -------------------

    static void setPrintKeepRatio( bool v )
    {
        if (!self()->isImmutable( QString::fromLatin1( "PrintKeepRatio" ) ))
            self()->mPrintKeepRatio = v;
    }

    static bool printKeepRatio()
    {
        return self()->mPrintKeepRatio;
    }

    // -------------------

    static void setPrintAutoRotate( bool v )
    {
        if (!self()->isImmutable( QString::fromLatin1( "PrintAutoRotate" ) ))
            self()->mPrintAutoRotate = v;
    }

    static bool printAutoRotate()
    {
        return self()->mPrintAutoRotate;
    }

protected:

    AdvPrintImagesConfig();
    friend class AdvPrintImagesConfigHelper;

protected:

    int     mPrintPosition;
    int     mPrintScaleMode;
    bool    mPrintEnlargeSmallerImages;
    double  mPrintWidth;
    double  mPrintHeight;
    int     mPrintUnit;
    bool    mPrintKeepRatio;
    bool    mPrintAutoRotate;
};

} // namespace Digikam

#endif // ADV_PRINT_IMAGES_CONFIG_H
