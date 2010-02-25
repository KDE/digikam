/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Curves image filter
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CURVESFILTER_H
#define CURVESFILTER_H

// Qt includes

#include <QPolygon>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

using namespace Digikam;

namespace Digikam
{

class DImg;
class CurvesFilterPriv;

class DIGIKAM_EXPORT CurvesFilter : public DImgThreadedFilter
{

public:

    CurvesFilter(DImg* orgImage, QObject* parent=0, const QPolygon& curvesPts=QPolygon());
    virtual ~CurvesFilter();

private:

    void filterImage();

private:

    QPolygon m_curvesPts;
};

}  // namespace Digikam

#endif /* CURVESFILTER_H */
