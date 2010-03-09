/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a plugin to enhance image with local contrasts (as human eye does).
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at gmail dot com>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LOCALCONTRASTFILTER_H
#define LOCALCONTRASTFILTER_H

// Qt includes

#include <QImage>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"
#include "tonemappingparameters.h"

using namespace Digikam;

namespace Digikam
{

class LocalContrastFilterPriv;

class DIGIKAM_EXPORT LocalContrastFilter : public DImgThreadedFilter
{

public:

    LocalContrastFilter(DImg* image, QObject* parent=0, const ToneMappingParameters& par=ToneMappingParameters());
    ~LocalContrastFilter();

    void progressCallback(int progress);

private:

    void filterImage();

private:

    LocalContrastFilterPriv* const d;
};

} // namespace Digikam

#endif /* LOCALCONTRASTFILTER_H */
