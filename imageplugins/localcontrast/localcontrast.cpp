/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a plugin to enhance image with local contrasts (as human eye does).
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at gmail dot com>
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

#include "localcontrast.h"

// Tonemapping includes

#include "ToneMappingBase.h"
#include "ToneMappingInt.h"
#include "ToneMappingFloat.h"

namespace DigikamLocalContrastImagesPlugin
{

class LocalContrastPriv
{
public:

    LocalContrastPriv()
    {
        par         = 0;
        tonemapping = 0;
        width       = 0;
        height      = 0;
        data        = 0;
    }

    ToneMappingParameters* par;

    ToneMappingBase*       tonemapping;

    int                    width;
    int                    height;

    uchar*                 data;

    bool                   sixteenBit;
};

LocalContrast::LocalContrast(DImg *image, ToneMappingParameters *par, QObject *parent)
              : Digikam::DImgThreadedFilter(image, parent, "LocalContrast"),
                     d(new LocalContrastPriv)
{
    initFilter();

    // Parameters
    d->par = par;

    // Image information
    d->width        = image->width();
    d->height       = image->height();
    d->data         = image->bits();
    d->sixteenBit   = image->sixteenBit();
}

LocalContrast::~LocalContrast()
{
    delete d;
}

void LocalContrast::filterImage()
{
    if(d->par->info_fast_mode) // fast mode
    {
        d->tonemapping = new ToneMappingInt();
    }
    else // no fast mode
    {
        d->tonemapping = new ToneMappingFloat();
    }

    // Apply parameters
    d->tonemapping->apply_parameters(*d->par);

    // Process image
    if(d->sixteenBit) // sixteen bit image
    {
        // TODO : for sixteen bits images
    }
    else // eight bit image
    {
        if(d->data != NULL)
        {
            int size    = d->width*d->height*3;
            uchar *data = new uchar [size];

            for(int i=0, j=0; i<size; i+=3, j+=4)
            {
                data[i]   = d->data[j];
                data[i+1] = d->data[j+1];
                data[i+2] = d->data[j+2];
            }

            d->tonemapping->process_8bit_rgb_image(data, d->width, d->height);

            for(int x=0; x<d->width; x++)
            {
                for(int y=0; y<d->height; y++)
                {
                    int i = (d->width * y + x)*3;
                    m_destImage.setPixelColor(x, y, DColor(data[i+2], data[i+1], data[i], 255, false));
                }
            }

            delete data;
        }
    }
}

void LocalContrast::progressCallback(int progress)
{
    Q_UNUSED(progress)
    // TODO
}

void LocalContrast::cancelFilter()
{
    // TODO
}

} // namespace DigikamLocalContrastImagesPlugin
