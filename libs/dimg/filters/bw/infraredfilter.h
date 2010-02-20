/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date   : 2005-05-25
 * Description : Infrared threaded image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef INFRAREDFILTER_H
#define INFRAREDFILTER_H

// Local includes

#include "dimgthreadedfilter.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT InfraredFilter : public DImgThreadedFilter
{

public:

    explicit InfraredFilter(DImg* orgImage, QObject* parent=0, int sensibility=1, bool grain=true);

    ~InfraredFilter(){};

private:

    bool m_grain;

    int  m_sensibility;

private:

    void filterImage();
    void infraredImage(int Sensibility, bool Grain);
};

}  // namespace Digikam

#endif /* INFRAREDFILTER_H */
