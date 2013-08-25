/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Sorter
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013 by Gowtham Ashok <gwty93 at gmail dot com>
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

#ifndef IMGQSORT_H
#define IMGQSORT_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "dimgthreadedanalyser.h"

namespace Digikam
{
class DIGIKAM_EXPORT ImgQSort : public DImgThreadedAnalyser
{
public:

    /** Standard constructor with image container to parse
     */
    explicit ImgQSort(DImg* const img, QObject* const parent=0);
    ~ImgQSort();

    /** Perform  quality estimation
     */
    void startAnalyse();

private:

    /** Internal method dedicated to convert DImg pixels from integer values to float values.
     *  These ones will by used internally by ImgQSort through OpenCV API.
     */
    void readImage();

    void   CannyThreshold(int, void*);
    double blurdetector() const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* IMGQSORT_H */