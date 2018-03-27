/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Sorter
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
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

#ifndef IMG_QSORT_H
#define IMG_QSORT_H

// Local includes

#include "dimg.h"
#include "digikam_globals.h"
#include "imagequalitysettings.h"

namespace Digikam
{

class ImgQSort
{
public:

    /** Standard constructor with picklabel container to fill at end of analyse
     */
    explicit ImgQSort(const DImg& img, const ImageQualitySettings& imq, PickLabel* const label);
    ~ImgQSort();

    /** Perform  quality estimation and fill Pick Label value accordingly.
     */
    void startAnalyse();
    void cancelAnalyse();

private:

    /** Internal method dedicated to convert DImg pixels from integer values to float values.
     *  These ones will by used internally by ImgQSort through OpenCV API.
     */
    void readImage() const;

    /**
     * @function CannyThreshold
     * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
     */
    void   CannyThreshold(int, void*) const;

    double blurdetector()             const;
    short  blurdetector2()            const;
    double noisedetector()            const;
    int    compressiondetector()      const;
    int    exposureamount()           const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // IMG_QSORT_H
