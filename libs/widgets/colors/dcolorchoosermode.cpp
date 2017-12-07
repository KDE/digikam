/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-20
 * Description : color chooser widgets
 *
 * Copyright (C)      2010 by Christoph Feck <christoph at maxiom dot de>
 * Copyright (C) 2015-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dcolorchoosermode_p.h"

// Qt includes

#include <QColor>

namespace Digikam
{

qreal getComponentValue(const QColor& color, DColorChooserMode chooserMode)
{
    switch (chooserMode)
    {
        case ChooserRed:
            return color.redF();
        case ChooserGreen:
            return color.greenF();
        case ChooserBlue:
            return color.blueF();
        case ChooserHue:
            return color.hueF();
        case ChooserSaturation:
            return color.saturationF();
        default:
            return color.valueF();
    }
}

void setComponentValue(QColor& color, DColorChooserMode chooserMode, qreal value)
{
    if (chooserMode >= ChooserRed)
    {
        if (chooserMode == ChooserRed)
        {
            color.setRedF(value);
        }
        else if (chooserMode == ChooserGreen)
        {
            color.setGreenF(value);
        }
        else
        {
            // chooserMode == ChooserBlue
            color.setBlueF(value);
        }
    }
    else
    {
        qreal h=0, s=0, v=0, a=0;
        color.getHsvF(&h, &s, &v, &a);

        if (chooserMode == ChooserHue)
        {
            h = value;
        }
        else if (chooserMode == ChooserSaturation)
        {
            s = value;
        }
        else
        {
            // chooserMode == ChooserValue
            v = value;
        }

        color.setHsvF(h, s, v, a);
    }
}

} // namespace Digikam
