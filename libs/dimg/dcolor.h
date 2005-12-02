/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-12-02
 * Description : 16 bits color management class
 *
 * Copyright 2005 by Gilles Caulier
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
 * ============================================================ */

#ifndef DCOLOR_H
#define DCOLOR_H

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DColor
{
public:

    DColor();
    DColor(uchar *data, bool sixteenBits);
    DColor(int red, int green, int blue, int alpha);
    DColor& operator=(const DColor& col);
    ~DColor();

    int red  ();
    int green();
    int blue ();
    int alpha();

    void setRed  (int red);
    void setGreen(int green);
    void setBlue (int blue);
    void setAlpha(int alpha);

private:

    int m_red;
    int m_green;
    int m_blue;
    int m_alpha;
};

}  // NameSpace Digikam

#endif /* DCOLOR_H */
