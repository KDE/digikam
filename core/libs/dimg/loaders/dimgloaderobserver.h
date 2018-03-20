/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
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

#ifndef DIMGLOADEROBSERVER_H
#define DIMGLOADEROBSERVER_H

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
    virtual void progressInfo(const DImg* const, float /*progress*/)
    {
    };

    /** Queries whether the image IO operation shall be continued
     */
    virtual bool continueQuery(const DImg* const)
    {
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

#endif // DIMGLOADEROBSERVER_H
