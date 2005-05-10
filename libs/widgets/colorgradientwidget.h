/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-28
 * Description : a color gradient widget without arrow.
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef COLORGRADIENTWIDGET_H
#define COLORGRADIENTWIDGET_H

// KDE includes.

#include <kselect.h>
#include "digikam_export.h"
namespace Digikam
{

class DIGIKAM_EXPORT ColorGradientWidget : public KGradientSelector
{
Q_OBJECT

public:

    ColorGradientWidget(Orientation o, int size, QWidget *parent=0);
    ~ColorGradientWidget();
    
protected:
    
    virtual void drawArrow( QPainter *, bool, const QPoint &);
    
};

}

#endif /* COLORGRADIENTWIDGET_H */
