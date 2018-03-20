/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image curves manipulation methods.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef CURVESCONTAINER_H
#define CURVESCONTAINER_H

// Qt includes

#include <QPolygon>
#include <QString>

// Local includes

#include "digikam_globals.h"
#include "digikam_export.h"

namespace Digikam
{

class FilterAction;

class DIGIKAM_EXPORT CurvesContainer
{

public:

    /**
     * Provides a convenient storage for a curve.
     * Initially, the values are empty.
     * Call initialize() before adjusting values manually.
     */

    CurvesContainer();
    CurvesContainer(int type, bool sixteenBit);

    /**
     * Fills the values with a linear curve suitable for type and sixteenBit parameters.
     */
    void initialize();

    /**
     * An empty container is interpreted as a linear curve.
     * A non-empty container can also be linear; test for isLinear()
     * of the resulting ImageCurves.
     * Note: If an ImageCurves is linear, it will return an empty container.
     */
    bool isEmpty() const;

    bool operator==(const CurvesContainer& other) const;

    /**
     * Serialize from and to FilterAction.
     * isStoredLosslessly returns false if the curve cannot be losslessly stored
     * in XML because it would be too large (free 16 bit). It is then lossily compressed.
     */
    bool isStoredLosslessly() const;
    void writeToFilterAction(FilterAction& action, const QString& prefix = QString()) const;
    static CurvesContainer fromFilterAction(const FilterAction& action, const QString& prefix = QString());

public:

    /**
     *  Smooth : QPolygon have size of 18 points.
     *  Free   : QPolygon have size of 255 or 65535 values.
     */
    int                     curvesType;
    QPolygon                values[ColorChannels];

    bool                    sixteenBit;
};


}  // namespace Digikam

#endif /* CURVESCONTAINER_H */
