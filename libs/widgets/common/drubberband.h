/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-07
 * Description : Rubber band for Q3ScrollView
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DRUBBERBAND
#define DRUBBERBAND

// Qt includes

#include <Qt3Support/Q3ScrollView>
#include <QtGui/QRubberBand>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DRubberBandPrivate;

class DIGIKAM_EXPORT DRubberBand : public QRubberBand
{

public:

    DRubberBand(Q3ScrollView* parent);
    ~DRubberBand();

    QRect rubberBandAreaOnContents();

    /// Returns the active state.
    bool isActive() const;
    /** Sets the rubberband active.
     *  Call setActive(true) to show rubberband with last set first and second points.
     *  Call setActive(false) to hide the rubberband when it was shown
     *  by setSecondPoint().
     */
    void setActive(bool active);

    /** Sets one of the two points of the rubberband, no
     *  matter which one. The rubberband is only shown after
     *  setting a second point. Sets rubberband to active.
     *  Coordinates are contents coordinates of the scrollview.
     */
    void setFirstPointOnContents(const QPoint& p);
    /** Same as above, coordinates are viewport coordinates of the scrollview. */
    void setFirstPointOnViewport(const QPoint& p);

    /** Sets the second one of the two points of the rubberband,
     *  diagonal to the first one. The rubberband is shown after
     *  calling this method (if it had not been inactivated after setFirstPoint).
     *  Call setActive(false) to hide it again.
     *  Coordinates are contents coordinates of the scrollview.
     */
    void setSecondPointOnContents(const QPoint& p);
    /** Same as above, coordinates are viewport coordinates of the scrollview. */
    void setSecondPointOnViewport(const QPoint& p);

    /** Installs a restriction which the rubber band cannot exceed.
     *  Any point set as first or second point outside the restriction will be cut to
     *  the restriction borders.
     *  Call with a null rectangle to reset.*/
    void setRestrictionOnContents(const QRect& rect);

    /** Combines setFirstPoint and setSecondPoint */
    void setRectOnViewport(const QRect& rect);
    void setRectOnContents(const QRect& rect);

protected:

    void updateForContentsPosition(int contentsX, int contentsY);

    DRubberBandPrivate* const d;
};

}

#endif // DRUBBERBAND
