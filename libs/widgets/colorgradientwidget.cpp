/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-28
 * Description : a color gradient widget
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

// Qt includes. 
 
#include <qimage.h>
#include <qpainter.h>
#include <qdrawutil.h>
 
// KDE includes.

#include <kimageeffect.h>

// Local includes.

#include "colorgradientwidget.h"

namespace Digikam
{

ColorGradientWidget::ColorGradientWidget(int o, int size, QWidget *parent)
                   : QWidget(parent, 0, Qt::WDestructiveClose), m_orientation(o)
{
    if ( o == Horizontal )
        setFixedHeight( size );
    else
        setFixedWidth( size );    
    
    m_color1.setRgb( 0, 0, 0 );
    m_color2.setRgb( 255, 255, 255 );
}      

ColorGradientWidget::~ColorGradientWidget()
{
}
    
void ColorGradientWidget::setColors( const QColor &col1, const QColor &col2 )
{
    m_color1 = col1; 
    m_color2 = col2; 
    update();
}

void ColorGradientWidget::paintEvent( QPaintEvent * )
{
    QImage image( width(), height(), 32 );

    QColor col, color1, color2;
    float scale;
    
    // Widget is disable : drawing grayed frame.
    if ( !isEnabled() )
       {
       color1 = palette().disabled().foreground();
       color2 = palette().disabled().background();
       } 
    else 
       {
       color1 = m_color1;
       color2 = m_color2;
       } 

    int redDiff   = color2.red()   - color1.red();
    int greenDiff = color2.green() - color1.green();
    int blueDiff  = color2.blue()  - color1.blue();

    if ( m_orientation == Vertical )
        {
        for ( int y = 0; y < image.height(); y++ )
            {
            scale = 1.0 * y / image.height();
            col.setRgb( color1.red()   + int(redDiff*scale),
                        color1.green() + int(greenDiff*scale),
                        color1.blue()  + int(blueDiff*scale) );

            unsigned int *p = (uint *) image.scanLine( y );
            
            for ( int x = 0; x < image.width(); x++ )
                *p++ = col.rgb();
            }
        }
    else
        {
        unsigned int *p = (uint *) image.scanLine( 0 );

        for ( int x = 0; x < image.width(); x++ )
            {
            scale = 1.0 * x / image.width();
            col.setRgb( color1.red()   + int(redDiff*scale),
                        color1.green() + int(greenDiff*scale),
                        color1.blue()  + int(blueDiff*scale) );
            *p++ = col.rgb();
            }

        for ( int y = 1; y < image.height(); y++ )
            {
            memcpy( image.scanLine( y ), image.scanLine( y - 1),
                 sizeof( unsigned int ) * image.width() );
            }
        }

    QColor ditherPalette[8];

    for ( int s = 0; s < 8; s++ )
        {
        ditherPalette[s].setRgb( color1.red()  + redDiff * s / 8,
                                color1.green() + greenDiff * s / 8,
                                color1.blue()  + blueDiff * s / 8 );
        }

    KImageEffect::dither( image, ditherPalette, 8 );

    QPixmap pm;
    pm.convertFromImage( image );
    bitBlt(this, 0, 0, &pm);
}

}  // namespace Digikam

#include "colorgradientwidget.moc"
