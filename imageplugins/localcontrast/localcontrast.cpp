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
        par              = 0;
        tonemappingFloat = 0;
    }

    ToneMappingParameters* par;

    ToneMappingFloat*      tonemappingFloat;
};

LocalContrast::LocalContrast(DImg *image, ToneMappingParameters *par, QObject *parent)
              : Digikam::DImgThreadedFilter(image, parent, "LocalContrast"),
                d(new LocalContrastPriv)
{
    d->par              = par;
    d->par->info_cancel = &m_cancel;

    initFilter();
}

LocalContrast::~LocalContrast()
{
    delete d;
}

void LocalContrast::filterImage()
{
/*
    if(d->par->info_fast_mode)
    {
        // fast mode
    }
    else
    {
        // no fast mode
    }
*/

    d->tonemappingFloat = new ToneMappingFloat();
    d->tonemappingFloat->apply_parameters(*d->par);

    // Process image

    if(!m_orgImage.isNull())
    {
        int size = m_orgImage.width()*m_orgImage.height()*3;
        int i, j;

        if(m_orgImage.sixteenBit())
        {
            // sixteen bit image
            unsigned short *data    = new unsigned short[size];
            unsigned short *dataImg = (unsigned short*)(m_orgImage.bits());

            for(i=0, j=0; !m_cancel && (i < size); i+=3, j+=4)
            {
                data[i]   = dataImg[j];
                data[i+1] = dataImg[j+1];
                data[i+2] = dataImg[j+2];
            }

            d->tonemappingFloat->process_16bit_rgb_image(data, m_orgImage.width(), m_orgImage.height());

            for(uint x=0; !m_cancel && (x < m_orgImage.width()); x++)
            {
                for(uint y=0; !m_cancel && (y < m_orgImage.height()); y++)
                {
                    i = (m_orgImage.width() * y + x)*3;
                    m_destImage.setPixelColor(x, y, DColor((unsigned short)data[i+2],
                                                           (unsigned short)data[i+1],
                                                           (unsigned short)data[i],
                                                           65535, true));
                }
            }

            delete [] data;
        }
        else // eight bit image
        {
            uchar *data = new uchar[size];

            for(i=0, j=0; !m_cancel && (i < size); i+=3, j+=4)
            {
                data[i]   = m_orgImage.bits()[j];
                data[i+1] = m_orgImage.bits()[j+1];
                data[i+2] = m_orgImage.bits()[j+2];
            }

            d->tonemappingFloat->process_8bit_rgb_image(data, m_orgImage.width(), m_orgImage.height());

            for(uint x=0; !m_cancel && (x < m_orgImage.width()); x++)
            {
                for(uint y=0; !m_cancel && (y < m_orgImage.height()); y++)
                {
                    i = (m_orgImage.width() * y + x)*3;
                    m_destImage.setPixelColor(x, y, DColor(data[i+2], data[i+1], data[i], 255, false));
                }
            }

            delete [] data;
        }
    }

    delete d->tonemappingFloat;
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
