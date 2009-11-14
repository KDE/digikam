/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a plugin to enhance image with local contrasts (as human eye does).
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at gmail dot com>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LOCALCONTRAST_H
#define LOCALCONTRAST_H

// Qt includes

#include <QImage>

// Local includes

#include "dimgthreadedfilter.h"
#include "ToneMappingParameters.h"

using namespace Digikam;

namespace DigikamLocalContrastImagesPlugin
{

class LocalContrastPriv;

class LocalContrast : public Digikam::DImgThreadedFilter
{

public:

    LocalContrast(DImg *image, ToneMappingParameters *par, QObject *parent=0);
    ~LocalContrast();

    void progressCallback(int progress);

private:

    void filterImage();

private:

    LocalContrastPriv* const d;
};

} // namespace DigikamLocalContrastImagesPlugin

#endif /* LOCALCONTRAST_H */
