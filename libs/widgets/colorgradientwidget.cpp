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

// KDE includes.

#include "colorgradientwidget.h"

namespace Digikam
{

ColorGradientWidget::ColorGradientWidget(Orientation o, int size, QWidget *parent)
                   : KGradientSelector (o, parent)
{
    if ( o == KSelector::Horizontal )
        setFixedHeight( size );
    else
        setFixedWidth( size );
     
}      

ColorGradientWidget::~ColorGradientWidget()
{
}
    
void ColorGradientWidget::drawArrow( QPainter *, bool , const QPoint & )
{
    // Do nothing !!! We won't arrow...
}

}

#include "colorgradientwidget.moc"
