/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-03
 * Description : DImgLoader observer interface
 *
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_DIMG_LOADER_OBSERVER_H
#define DIGIKAM_DIMG_LOADER_OBSERVER_H

// Qt includes

#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT DImgLoaderObserver
{

public:

    virtual ~DImgLoaderObserver()
    {
    };

    /** Posts progress information about image IO
     */
    virtual void progressInfo(DImg* const img, float progress)
    {
        Q_UNUSED(img);
        Q_UNUSED(progress);
    };

    /** Queries whether the image IO operation shall be continued
     */
    virtual bool continueQuery(DImg* const img)
    {
        Q_UNUSED(img);
        return true;
    };

    /** Return a relative value which determines the granularity, the frequency
     *  with which the DImgLoaderObserver is checked and progress is posted.
     *  Standard is 1.0. Values < 1 mean less granularity (fewer checks),
     *  values > 1 mean higher granularity (more checks).
     */
    virtual float granularity()
    {
        return 1.0;
    };
};

}      // namespace Digikam

#endif // DIGIKAM_DIMG_LOADER_OBSERVER_H
