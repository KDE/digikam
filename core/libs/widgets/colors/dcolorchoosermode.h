/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-20
 * Description : color chooser widgets
 *
 * Copyright (C)      2007 by Timo A. Hummel <timo at timohummel dot com>
 * Copyright (C) 2015-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DCOLOR_CHOOSER_MODE_H
#define DIGIKAM_DCOLOR_CHOOSER_MODE_H

namespace Digikam
{

enum DColorChooserMode
{
     ChooserClassic   = 0x0000,
     ChooserHue       = 0x0001,
     ChooserSaturation= 0x0002,
     ChooserValue     = 0x0003,
     ChooserRed       = 0x0004,
     ChooserGreen     = 0x0005,
     ChooserBlue      = 0x0006
};

} // namespace Digikam

#endif // DIGIKAM_DCOLOR_CHOOSER_MODE_H
