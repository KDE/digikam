/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-11-11
 * Description : a popup menu with a decorative graphic banner
 *               at the left border.
 * 
 * Copyright 1996-2000 the kicker authors.
 * Copyright 2005 Mark Kretschmann <markey@web.de>
 * Copyright 2006 by Gilles Caulier
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

#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kiconeffect.h>
#include <kstandarddirs.h>

// Local includes.

#include "dpopupmenu.h"
#include "dpopupmenu.moc"

namespace Digikam
{

QImage DPopupMenu::_dpopupmenu_sidePixmap_;
QColor DPopupMenu::_dpopupmenu_sidePixmapColor_;

DPopupMenu::DPopupMenu( QWidget* parent, const char* name )
          : KPopupMenu( parent, name )
{
    // Must be initialized so that we know the size on first invocation
    if ( _dpopupmenu_sidePixmap_.isNull() )
        generateSidePixmap();
}

void DPopupMenu::generateSidePixmap()
{
    const QColor newColor = calcPixmapColor();

    if ( newColor != _dpopupmenu_sidePixmapColor_ ) 
    {
        _dpopupmenu_sidePixmapColor_ = newColor;
        _dpopupmenu_sidePixmap_.load( locate( "data","digikam/data/menusidepixmap.png" ) );
        KIconEffect::colorize( _dpopupmenu_sidePixmap_, newColor, 1.0 );
    }
}

QRect DPopupMenu::sideImageRect() const
{
    return QStyle::visualRect( QRect( frameWidth(), frameWidth(), 
                                      _dpopupmenu_sidePixmap_.width(),
                                      height() - 2*frameWidth() ), this );
}

QColor DPopupMenu::calcPixmapColor()
{
    KConfig *config = KGlobal::config();
    config->setGroup("WM");
    QColor color = QApplication::palette().active().highlight();
//     QColor activeTitle = QApplication::palette().active().background();
//     QColor inactiveTitle = QApplication::palette().inactive().background();
    QColor activeTitle = config->readColorEntry("activeBackground", &color);
    QColor inactiveTitle = config->readColorEntry("inactiveBackground", &color);

    // figure out which color is most suitable for recoloring to
    int h1, s1, v1, h2, s2, v2, h3, s3, v3;
    activeTitle.hsv(&h1, &s1, &v1);
    inactiveTitle.hsv(&h2, &s2, &v2);
    QApplication::palette().active().background().hsv(&h3, &s3, &v3);

    if ( (kAbs(h1-h3)+kAbs(s1-s3)+kAbs(v1-v3) < kAbs(h2-h3)+kAbs(s2-s3)+kAbs(v2-v3)) &&
            ((kAbs(h1-h3)+kAbs(s1-s3)+kAbs(v1-v3) < 32) || (s1 < 32)) && (s2 > s1))
        color = inactiveTitle;
    else
        color = activeTitle;

    // limit max/min brightness
    int r, g, b;
    color.rgb(&r, &g, &b);
    int gray = qGray(r, g, b);
    if (gray > 180) 
    {
        r = (r - (gray - 180) < 0 ? 0 : r - (gray - 180));
        g = (g - (gray - 180) < 0 ? 0 : g - (gray - 180));
        b = (b - (gray - 180) < 0 ? 0 : b - (gray - 180));
    }
    else if (gray < 76) 
    {
        r = (r + (76 - gray) > 255 ? 255 : r + (76 - gray));
        g = (g + (76 - gray) > 255 ? 255 : g + (76 - gray));
        b = (b + (76 - gray) > 255 ? 255 : b + (76 - gray));
    }
    color.setRgb(r, g, b);

    return color;
}

void DPopupMenu::setMinimumSize(const QSize & s)
{
    KPopupMenu::setMinimumSize(s.width() + _dpopupmenu_sidePixmap_.width(), s.height());
}

void DPopupMenu::setMaximumSize(const QSize & s)
{
    KPopupMenu::setMaximumSize(s.width() + _dpopupmenu_sidePixmap_.width(), s.height());
}

void DPopupMenu::setMinimumSize(int w, int h)
{
    KPopupMenu::setMinimumSize(w + _dpopupmenu_sidePixmap_.width(), h);
}

void DPopupMenu::setMaximumSize(int w, int h)
{
  KPopupMenu::setMaximumSize(w + _dpopupmenu_sidePixmap_.width(), h);
}

void DPopupMenu::resizeEvent(QResizeEvent * e)
{
    KPopupMenu::resizeEvent( e );

    setFrameRect( QStyle::visualRect( QRect( _dpopupmenu_sidePixmap_.width(), 0,
                                      width() - _dpopupmenu_sidePixmap_.width(), 
                                      height() ), this ) );
}

//Workaround Qt3.3.x sizing bug, by ensuring we're always wide enough.
void DPopupMenu::resize( int width, int height )
{
    width = kMax(width, maximumSize().width());
    KPopupMenu::resize(width, height);
}

void DPopupMenu::paintEvent( QPaintEvent* e )
{
    generateSidePixmap();

    QPainter p( this );

    QRect r = sideImageRect();
    r.setTop( r.bottom() - _dpopupmenu_sidePixmap_.height() );
    if ( r.intersects( e->rect() ) )
    {
        QRect drawRect = r.intersect( e->rect() ).intersect( sideImageRect() );
        QRect pixRect = drawRect;
        pixRect.moveBy( -r.left(), -r.top() );
        p.drawImage( drawRect.topLeft(), _dpopupmenu_sidePixmap_, pixRect );
    }

    p.setClipRegion( e->region() );

    //NOTE The order is important here. drawContents() must be called before drawPrimitive(),
    //     otherwise we get rendering glitches.

    drawContents( &p );

    style().drawPrimitive( QStyle::PE_PanelPopup, &p,
                           QRect( 0, 0, width(), height() ),
                           colorGroup(), QStyle::Style_Default,
                           QStyleOption( frameWidth(), 0 ) );
}

}  // namespace Digikam

