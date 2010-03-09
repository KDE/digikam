/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a plugin to enhance image with local contrasts (as human eye does).
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at gmail dot com>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "localcontrastfilter.h"

// KDE includes

#include <kdebug.h>

// Tonemapping includes

#include "tonemappingfloat.h"

namespace DigikamLocalContrastImagesPlugin
{

static void CallbackForToneMapping(void* data, int progress)
{
    if (data)
    {
        LocalContrastFilter *d = static_cast<LocalContrastFilter*>(data);
        if (d) return d->progressCallback(progress);
    }
}

class LocalContrastFilterPriv
{
public:

    LocalContrastFilterPriv()
    {
        tonemappingFloat = 0;
    }

    ToneMappingParameters par;

    ToneMappingFloat*     tonemappingFloat;
};

LocalContrastFilter::LocalContrastFilter(DImg* image, QObject* parent, const ToneMappingParameters& par)
                   : DImgThreadedFilter(image, parent, "LocalContrast"),
                     d(new LocalContrastFilterPriv)
{
    d->par = par;
    d->par.setCancel(&m_cancel);
    d->par.setProgressCallBackFunction(this, CallbackForToneMapping);

    initFilter();
}

LocalContrastFilter::~LocalContrastFilter()
{
    delete d;
}

void LocalContrastFilter::filterImage()
{
    progressCallback(0);

    d->tonemappingFloat = new ToneMappingFloat();
    d->tonemappingFloat->apply_parameters(d->par);

    // Process image

    if(!m_orgImage.isNull())
    {
        int size = m_orgImage.width()*m_orgImage.height()*3;
        int i, j;

        if(m_orgImage.sixteenBit())
        {
            // sixteen bit image
            unsigned short* data    = new unsigned short[size];
            unsigned short* dataImg = (unsigned short*)(m_orgImage.bits());

            for(i=0, j=0; !m_cancel && (i < size); i+=3, j+=4)
            {
                data[i]   = dataImg[j];
                data[i+1] = dataImg[j+1];
                data[i+2] = dataImg[j+2];
            }

            progressCallback(10);

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
        else
        {
            // eight bit image

            uchar* data = new uchar[size];

            for(i=0, j=0; !m_cancel && (i < size); i+=3, j+=4)
            {
                data[i]   = m_orgImage.bits()[j];
                data[i+1] = m_orgImage.bits()[j+1];
                data[i+2] = m_orgImage.bits()[j+2];
            }

            progressCallback(10);

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
    progressCallback(100);
}

void LocalContrastFilter::progressCallback(int progress)
{
    if (progress%5 == 0)
    {
        postProgress(progress);
//        kDebug() << "ToneMapping progress: " << progress;
    }
}

} // namespace DigikamLocalContrastFilterImagesPlugin
