/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Emboss threaded image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * Original Emboss algorithm copyrighted 2004 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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

#ifndef EMBOSS_H
#define EMBOSS_H

// Local includes

#include "dimgthreadedfilter.h"

using namespace Digikam;

namespace DigikamEmbossImagesPlugin
{

class Emboss : public DImgThreadedFilter
{

public:

    explicit Emboss(DImg* orgImage, QObject* parent=0, int depth=30);
    ~Emboss(){};

private:

    void filterImage();

    inline int Lim_Max (int Now, int Up, int Max);
    inline int getOffset(int Width, int X, int Y, int bytesDepth);

private:

    int m_depth;
};

}  // namespace DigikamEmbossImagesPlugin

#endif /* EMBOSS_H */
