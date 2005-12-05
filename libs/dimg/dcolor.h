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

// QT includes.

#include <qcolor.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DColor
{
public:

    DColor();
    DColor(uchar *data, bool sixteenBit);
    DColor(int red, int green, int blue, int alpha, bool sixteenBit);
    DColor(const DColor& color);
    DColor(const QColor& color);
    DColor& operator=(const DColor& color);
    ~DColor();

    int  red  ();
    int  green();
    int  blue ();
    int  alpha();
    bool sixteenBit();

    void setRed  (int red);
    void setGreen(int green);
    void setBlue (int blue);
    void setAlpha(int alpha);
    void setSixteenBit(bool sixteenBit);

    QColor getQColor();
    
    void getHSL(int* h, int* s, int* l);
    void setRGB(int h, int s, int l, bool sixteenBit);

private:

    int  m_red;
    int  m_green;
    int  m_blue;
    int  m_alpha;

    bool m_sixteenBit;
};

}  // NameSpace Digikam

#endif /* DCOLOR_H */
