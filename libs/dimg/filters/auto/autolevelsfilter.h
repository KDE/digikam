/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : auto levels image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef AUTOLEVELSFILTER_H
#define AUTOLEVELSFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

using namespace Digikam;

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT AutoLevelsFilter : public DImgThreadedFilter
{

public:

    AutoLevelsFilter(DImg* orgImage, const DImg* refImage, QObject* parent=0);
    virtual ~AutoLevelsFilter();

private:

    void filterImage();
    void autoLevelsCorrectionImage();

private:

    const DImg m_refImage;
};

}  // namespace Digikam

#endif /* AUTOLEVELSFILTER_H */
