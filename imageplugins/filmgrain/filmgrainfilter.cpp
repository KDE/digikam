/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : filter to add Film Grain to image.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "filmgrainfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QDateTime>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "globals.h"

namespace DigikamFilmGrainImagesPlugin
{

FilmGrainFilter::FilmGrainFilter(DImg* orgImage, QObject* parent, int sensibility)
               : DImgThreadedFilter(orgImage, parent, "FilmGrain")
{
    m_sensibility = sensibility;
    initFilter();
}

void FilmGrainFilter::filterImage()
{
    // m_sensibility: 800..6400
    if (m_sensibility <= 0) return;

    DColor color;
    int    h, s, l;
    int    nRand, progress;

    int  width   = m_orgImage.width();
    int  height  = m_orgImage.height();
    bool sb      = m_orgImage.sixteenBit();
    int  noise   = ((m_sensibility+200) / 1000) * 3 * (sb ? 256 : 1);
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    uint seed    = (uint) dt.secsTo(Y2000);

#ifdef _WIN32
    srand(seed);
#endif

    for (int x = 0; !m_cancel && x < width; ++x)
    {
        for (int y = 0; !m_cancel && y < height; ++y)
        {
            color = m_orgImage.getPixelColor(x, y);
            color.getHSL(&h, &s, &l);

#ifndef _WIN32
            nRand = (rand_r(&seed) % noise);
#else
            nRand = (rand() % noise);
#endif

            l += nRand;

            color.setRGB(h, s, l, sb);
            m_destImage.setPixelColor(x, y, color);
        }

        // Update progress bar in dialog.
        progress = (int) (((double)x * 100.0) / width);

        if (progress%5 == 0)
            postProgress( progress );
    }
}

}  // namespace DigikamFilmGrainImagesPlugin
