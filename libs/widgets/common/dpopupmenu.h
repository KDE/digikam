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

#ifndef DPOPUPMENU_H
#define DPOPUPMENU_H

// Qt includes.

#include <qcolor.h>
#include <qimage.h>
#include <qrect.h>

// KDE includes.

#include <kpopupmenu.h>

// Local includes.

#include "digikam_export.h"

class QSize;

namespace Digikam
{

class DIGIKAM_EXPORT DPopupMenu : public KPopupMenu
{
    Q_OBJECT

public:

    DPopupMenu(QWidget *parent = 0, const char *name = 0);

    int sidePixmapWidth() const { return _dpopupmenu_sidePixmap_.width(); }

private:

    /** Loads and prepares the sidebar image */
    void generateSidePixmap();

    /** Returns the available size for the image */
    QRect sideImageRect() const;

    /** Calculates a color that matches the current colorscheme */
    QColor calcPixmapColor();

    void setMinimumSize( const QSize& s );
    void setMaximumSize( const QSize& s );
    void setMinimumSize( int w, int h );
    void setMaximumSize( int w, int h );

    void resizeEvent( QResizeEvent* e );
    void resize( int width, int height );

    void paintEvent( QPaintEvent* e );

private:

    static QImage _dpopupmenu_sidePixmap_;
    static QColor _dpopupmenu_sidePixmapColor_;
};

}  // namespace Digikam

#endif /*DPOPUPMENU_H*/
