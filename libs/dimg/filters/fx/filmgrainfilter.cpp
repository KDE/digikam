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

namespace Digikam
{

FilmGrainFilter::FilmGrainFilter(DImg* orgImage, QObject* parent, int sensibility, int shadows, int midtones, int highlights)
               : DImgThreadedFilter(orgImage, parent, "FilmGrain")
{
    m_sensibility = sensibility;
    m_shadows     = shadows;
    m_midtones    = midtones;
    m_highlights  = highlights;
    
    initFilter();
}

FilmGrainFilter::FilmGrainFilter(DImgThreadedFilter* parentFilter,
                                 const DImg& orgImage, const DImg& destImage,
                                 int progressBegin, int progressEnd, int sensibility, int shadows, int midtones, int highlights)
          : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                               parentFilter->filterName() + ": FilmGrain")
{
    m_sensibility = sensibility;
    m_shadows     = shadows;
    m_midtones    = midtones;
    m_highlights  = highlights;
    
    filterImage();
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
            double lightness;
            int local_noise;
            color = m_orgImage.getPixelColor(x, y);
            color.getHSL(&h, &s, &l);
            lightness= l / (sb ? 65535.0 : 255.0); 
            local_noise = interpolate(m_shadows,m_midtones,m_highlights,lightness) * noise+1;
            
#ifndef _WIN32
            nRand = (rand_r(&seed) % local_noise);
#else
            nRand = (rand() % local_noise);
#endif
            nRand = nRand - local_noise/2.0;
            
            if (!sb)
            {
              l = CLAMP0255(l+nRand);
            }
            else
            {
              l = CLAMP065535(l+nRand);
            }
            color.setRGB(h, s, l, sb);
            m_destImage.setPixelColor(x, y, color);
        }

        // Update progress bar in dialog.
        progress = (int) (((double)x * 100.0) / width);

        if (progress%5 == 0)
            postProgress( progress );
    }
}

double FilmGrainFilter::interpolate(int shadows,int midtones,int highlights, double x)
{
  double s = (shadows   +100)/200.0;
  double m = (midtones  +100)/200.0; 
  double h = (highlights+100)/200.0;
  
  if (x>=0 && x <=0.5)
  {
      return (s+2*(m-s)*x);
  }
  else if (x>=0.5 && x <=1.0)
  {
      return (2*(h-m)*x+2*m-h);
  }
  else
      return 1.0;
}

}  // namespace Digikam
