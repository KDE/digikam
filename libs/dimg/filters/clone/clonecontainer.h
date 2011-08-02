/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-06-08
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-06-08 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#ifndef CLONECONTAINER_H
#define CLONECONTAINER_H

#include"clonebrush.h"

namespace Digikam
{

class DIGIKAM_EXPORT CloneContainer
{
public:

    CloneContainer()
    {
        opacity    = 100;
        selectMode = false;
        drawMode   = false;
    };

    ~CloneContainer(){};

public:

    CloneBrush brush;      // selected brush
    int        brushID;    // id in the brushMap of selected brush
    int        brushDia;   // diameter of the brush shape
    int        mainDia;
    int        opacity;
    bool       selectMode; // set to true if the left button is clicked ,in this mode, click on the image to select a center point of the source area
    bool       drawMode;   // set to true if the right button is clicked ,in this mode, when move the mouse, a stroke will be draw on the image
};

} // namespace Digikam

#endif // CLONECONTAINER_H
