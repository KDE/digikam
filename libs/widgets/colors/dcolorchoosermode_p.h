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

#ifndef DCOLORCHOOSERMODE_PRIVATE_H
#define DCOLORCHOOSERMODE_PRIVATE_H

#include "dcolorchoosermode.h"

// Qt includes

#include <QtGlobal>

class QColor;

namespace Digikam
{

/** get/set color component
 */
extern qreal getComponentValue(const QColor& color, DColorChooserMode chooserMode);
extern void  setComponentValue(QColor& color, DColorChooserMode chooserMode, qreal value);

/** number of linear gradient ranges needed for color component
 */
static inline int componentValueSteps(DColorChooserMode chooserMode)
{
    if (chooserMode == ChooserHue)
    {
        return 6;
    }
    else
    {
        return 1;
    }
}

/** color component that is used for X in the XY selector
 */
static inline DColorChooserMode chooserXMode(DColorChooserMode chooserMode)
{
    if (chooserMode >= ChooserRed)
    {
        return (chooserMode == ChooserRed ? ChooserGreen : ChooserRed);
    }
    else
    {
        return (chooserMode == ChooserHue ? ChooserSaturation : ChooserHue);
    }
}

/** color component that is used for Y in the XY selector
 */
static inline DColorChooserMode chooserYMode(DColorChooserMode chooserMode)
{
    if (chooserMode >= ChooserRed)
    {
        return (chooserMode == ChooserBlue ? ChooserGreen : ChooserBlue);
    }
    else
    {
        return (chooserMode == ChooserValue ? ChooserSaturation : ChooserValue);
    }
}

static inline int componentXSteps(DColorChooserMode chooserMode)
{
    return componentValueSteps(chooserXMode(chooserMode));
}

static inline int componentYSteps(DColorChooserMode chooserMode)
{
    return componentValueSteps(chooserYMode(chooserMode));
}

static inline qreal getComponentX(const QColor& color, DColorChooserMode chooserMode)
{
    return getComponentValue(color, chooserXMode(chooserMode));
}

static inline qreal getComponentY(const QColor& color, DColorChooserMode chooserMode)
{
    return getComponentValue(color, chooserYMode(chooserMode));
}

static inline void setComponentX(QColor& color, DColorChooserMode chooserMode, qreal x)
{
    setComponentValue(color, chooserXMode(chooserMode), x);
}

static inline void setComponentY(QColor& color, DColorChooserMode chooserMode, qreal y)
{
    setComponentValue(color, chooserYMode(chooserMode), y);
}

} // namespace Digikam

#endif // DCOLORCHOOSERMODE_PRIVATE_H
